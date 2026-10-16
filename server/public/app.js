'use strict';

/* ═══════════════════════════════════════════════════════════════
   RACK MONITOR — Frontend
   Consome /api/status (polling 2s) e /api/comando (POST).
   ═══════════════════════════════════════════════════════════════ */

// ─────────────── Configuração ───────────────
const CONFIG = {
  endpoints: {
    status:  '/api/status',
    comando: '/api/comando',
  },
  pollingMs: 2000,
  // Após 30s sem atualização, considera o dado "velho". Após 60s, offline.
  staleThresholdMs:   30_000,
  offlineThresholdMs: 60_000,
  historicoMaxItens:  30,
  eventosMaxItens:    20,
};

// ─────────────── Rótulos amigáveis ───────────────
const LABEL_ESTADO = {
  SEGURO:           'Seguro',
  ACESSO_RFID:      'Acesso RFID',
  AVISO_SALA:       'Aviso Sala',
  APROXIMACAO_RACK: 'Aprox. Rack',
  INVASAO_SALA:     'Invasão',
  EMERGENCIA_RACK:  'Emergência',
};

const LABEL_MODO = {
  ARMADO:    'Armado',
  DESARMADO: 'Desarmado',
};

const ICONE_EVENTO = {
  MUDANCA_ESTADO: '🔄',
  COMANDO_WEB:    '🖱️',
  COMANDO_REMOTO: '📡',
  INFO:           'ℹ️',
};

// ─────────────── Estado ───────────────
const state = {
  ultimaAtualizacaoValida: null,   // timestamp do servidor
  fetchesConsecutivos: 0,
  fetchErroConsecutivos: 0,
};

// ─────────────── Referências DOM (cacheadas) ───────────────
const dom = {
  connectionDot:   document.getElementById('connectionDot'),
  connectionText:  document.getElementById('connectionText'),
  cardModo:        document.getElementById('cardModo'),
  cardEstado:      document.getElementById('cardEstado'),
  cardPorta:       document.getElementById('cardPorta'),
  cardIP:          document.getElementById('cardIP'),
  cardUID:         document.getElementById('cardUID'),
  cardAcessos:     document.getElementById('cardAcessos'),
  cardInvasoes:    document.getElementById('cardInvasoes'),
  cardBloqueios:   document.getElementById('cardBloqueios'),
  logEventos:      document.getElementById('logEventos'),
  logHistorico:    document.getElementById('logHistorico'),
  footerTime:      document.getElementById('footerTime'),
};

// ─────────────── Helpers ───────────────
function text(el, value) {
  const novo = value == null ? '--' : String(value);
  if (el.textContent !== novo) {
    el.textContent = novo;
    el.classList.add('flash');
    setTimeout(() => el.classList.remove('flash'), 800);
  }
}

function setState(el, value) {
  if (value == null) {
    el.removeAttribute('data-state');
  } else {
    el.setAttribute('data-state', value);
  }
}

function escapeText(s) {
  return String(s ?? '');
}

function humanizarEstado(e) {
  return LABEL_ESTADO[e] || e || '--';
}

function humanizarModo(m) {
  return LABEL_MODO[m] || m || '--';
}

function idade(isoString) {
  if (!isoString) return Infinity;
  const t = Date.parse(isoString);
  return Number.isFinite(t) ? Date.now() - t : Infinity;
}

// ─────────────── Conexão (indicador no header) ───────────────
function atualizarConexao(status, idadeMs) {
  dom.connectionDot.classList.remove('dot--online', 'dot--offline', 'dot--stale');

  if (status === 'offline') {
    dom.connectionDot.classList.add('dot--offline');
    dom.connectionText.textContent = 'Offline';
    return;
  }

  if (status === 'stale') {
    dom.connectionDot.classList.add('dot--stale');
    dom.connectionText.textContent = `Sem atualização há ${Math.round(idadeMs / 1000)}s`;
    return;
  }

  dom.connectionDot.classList.add('dot--online');
  const seg = Math.max(0, Math.round(idadeMs / 1000));
  dom.connectionText.textContent = seg <= 1 ? 'Online' : `Online · há ${seg}s`;
}

// ─────────────── Render: cards ───────────────
function renderizarCards(d) {
  text(dom.cardModo, humanizarModo(d.modo));
  setState(dom.cardModo, d.modo);

  text(dom.cardEstado, humanizarEstado(d.estado));
  setState(dom.cardEstado, d.estado);

  text(dom.cardPorta, d.porta === 'ABERTA' ? 'Aberta' : 'Fechada');
  setState(dom.cardPorta, d.porta);

  text(dom.cardIP, d.ip || '--');
  text(dom.cardUID, d.uid || '-');

  text(dom.cardAcessos, d.acessos ?? 0);
  text(dom.cardInvasoes, d.invasoes ?? 0);
  text(dom.cardBloqueios, d.bloqueios ?? 0);
}

// ─────────────── Render: eventos ───────────────
function renderizarEventos(eventos) {
  if (!Array.isArray(eventos) || eventos.length === 0) {
    dom.logEventos.replaceChildren(criarVazio('Aguardando eventos…'));
    return;
  }

  const fragment = document.createDocumentFragment();
  eventos.slice(0, CONFIG.eventosMaxItens).forEach(ev => {
    const linha = document.createElement('div');
    linha.className = 'log__line';

    const ts = document.createElement('span');
    ts.className = 'log__time';
    ts.textContent = ev.ts || '--';

    const icone = document.createElement('span');
    icone.className = 'log__icon';
    icone.textContent = ICONE_EVENTO[ev.tipo] || '•';

    const msg = document.createElement('span');
    msg.className = 'log__msg';
    msg.appendChild(montarMensagemEvento(ev));

    linha.append(ts, icone, msg);
    fragment.appendChild(linha);
  });

  dom.logEventos.replaceChildren(fragment);
}

function montarMensagemEvento(ev) {
  const frag = document.createDocumentFragment();

  if (ev.tipo === 'MUDANCA_ESTADO') {
    frag.append('Estado: ', strong(ev.de || '?'), ' → ', strong(ev.para || '?'));
  } else if (ev.tipo === 'COMANDO_WEB' || ev.tipo === 'COMANDO_REMOTO') {
    frag.append('Comando: ', strong(ev.comando || '?'));
  } else {
    frag.append(ev.tipo || 'Evento');
    if (ev.comando) frag.append(' · ', strong(ev.comando));
  }

  return frag;
}

function strong(texto) {
  const s = document.createElement('strong');
  s.textContent = texto;
  return s;
}

// ─────────────── Render: histórico ───────────────
function renderizarHistorico(historico) {
  if (!Array.isArray(historico) || historico.length === 0) {
    dom.logHistorico.replaceChildren(criarVazio('Aguardando dados do ESP32…'));
    return;
  }

  const fragment = document.createDocumentFragment();
  historico.slice(0, CONFIG.historicoMaxItens).forEach(h => {
    const linha = document.createElement('div');
    linha.className = 'log__line';

    const ts = document.createElement('span');
    ts.className = 'log__time';
    ts.textContent = h.ts || '--';

    const msg = document.createElement('span');
    msg.className = 'log__msg';
    msg.textContent =
      `${humanizarModo(h.modo)} | ${humanizarEstado(h.estado)} ` +
      `| Porta: ${h.porta === 'ABERTA' ? 'Aberta' : 'Fechada'} ` +
      `| UID: ${h.uid || '-'}`;

    linha.append(ts, msg);
    fragment.appendChild(linha);
  });

  dom.logHistorico.replaceChildren(fragment);
}

function criarVazio(texto) {
  const p = document.createElement('p');
  p.className = 'log__empty';
  p.textContent = texto;
  return p;
}

// ─────────────── Fetch: status ───────────────
async function buscarStatus() {
  try {
    const r = await fetch(CONFIG.endpoints.status, { cache: 'no-store' });
    if (!r.ok) throw new Error(`HTTP ${r.status}`);

    const d = await r.json();
    state.fetchErroConsecutivos = 0;
    state.ultimaAtualizacaoValida = d.ultimaAtualizacao || new Date().toISOString();

    renderizarCards(d);
    renderizarEventos(d.eventos);
    renderizarHistorico(d.historico);

    const idadeMs = idade(d.ultimaAtualizacao);
    atualizarConexao('online', idadeMs);

    dom.footerTime.textContent = formatarDataUTC(d.ultimaAtualizacao);
  } catch (err) {
    state.fetchErroConsecutivos++;
    console.warn('[rack] falha ao buscar status:', err.message);

    // Só marca offline após 2 falhas seguidas (evita falso positivo em hiccup)
    if (state.fetchErroConsecutivos >= 2) {
      atualizarConexao('offline', 0);
    }
  }
}

function formatarDataUTC(iso) {
  if (!iso) return '--';
  try {
    const d = new Date(iso);
    return d.toISOString().replace('T', ' ').replace(/\.\d+Z$/, ' UTC');
  } catch {
    return iso;
  }
}

// ─────────────── Fetch: comando ───────────────
async function enviarComando(cmd, botao) {
  if (botao) {
    botao.disabled = true;
    botao.classList.add('btn--loading');
  }

  try {
    const r = await fetch(CONFIG.endpoints.comando, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ comando: cmd }),
    });

    if (!r.ok) throw new Error(`HTTP ${r.status}`);
    const resp = await r.json();
    if (!resp.ok) throw new Error(resp.erro || 'Comando rejeitado');

    // Atualiza imediatamente após enviar comando
    await buscarStatus();
  } catch (err) {
    alert(`Erro ao enviar "${cmd}": ${err.message}`);
  } finally {
    if (botao) {
      botao.disabled = false;
      botao.classList.remove('btn--loading');
    }
  }
}

// ─────────────── Wire-up dos botões ───────────────
function configurarControles() {
  document.querySelectorAll('[data-command]').forEach(btn => {
    btn.addEventListener('click', () => {
      const cmd = btn.dataset.command;
      const confirmacao = btn.dataset.confirm;

      if (confirmacao && !window.confirm(confirmacao)) return;

      enviarComando(cmd, btn);
    });
  });
}

// ─────────────── Bootstrap ───────────────
function init() {
  console.log('[rack] frontend v2.0 iniciado');
  configurarControles();
  buscarStatus();
  setInterval(buscarStatus, CONFIG.pollingMs);
}

document.addEventListener('DOMContentLoaded', init);