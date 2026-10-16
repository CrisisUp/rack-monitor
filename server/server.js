// ==========================================
// RACK MONITOR - SERVIDOR
// ==========================================
const express = require("express");
const cors = require("cors");
const fs = require("fs");
const path = require("path");

const app = express();
const PORT = process.env.PORT || 3000;
const DATA_DIR = path.join(__dirname, "data");
const LOG_FILE = path.join(DATA_DIR, "historico.json");

app.use(cors());
app.use(express.json({ limit: "1mb" }));
app.use(express.static(path.join(__dirname, "public")));

let estadoAtual = {
  timestamp: "--",
  modo: "DESCONHECIDO",
  estado: "DESCONHECIDO",
  porta: "--",
  armado: false,
  acessos: 0,
  bloqueios: 0,
  invasoes: 0,
  uid: "-",
  ip: "-",
  ultimaAtualizacao: null
};

let historico = [];
let eventos = [];
const MAX_HISTORICO = 100;
const MAX_EVENTOS   = 200;
let comandoPendente = null;

function garantirPastaDados() {
  if (!fs.existsSync(DATA_DIR)) fs.mkdirSync(DATA_DIR, { recursive: true });
}

function carregarHistorico() {
  try {
    if (fs.existsSync(LOG_FILE)) {
      const raw = fs.readFileSync(LOG_FILE, "utf8");
      const dados = JSON.parse(raw);
      historico = dados.historico || [];
      eventos   = dados.eventos || [];
      console.log(`[LOG] Historico carregado: ${historico.length} status, ${eventos.length} eventos`);
    }
  } catch (e) {
    console.error("[LOG] Erro ao carregar historico:", e.message);
  }
}

function salvarHistorico() {
  try {
    garantirPastaDados();
    const dados = { historico, eventos, salvoEm: new Date().toISOString() };
    fs.writeFileSync(LOG_FILE, JSON.stringify(dados, null, 2));
  } catch (e) {
    console.error("[LOG] Erro ao salvar historico:", e.message);
  }
}

// ESP32 envia status aqui (POST)
app.post("/api/status", (req, res) => {
  const d = req.body;
  const agora = new Date().toISOString();
  const ip = req.ip || req.connection?.remoteAddress || "-";

  const anterior = estadoAtual.modo + "/" + estadoAtual.estado;
  const novo     = (d.modo || "?") + "/" + (d.estado || "?");

  estadoAtual = { ...estadoAtual, ...d, ip, ultimaAtualizacao: agora };

  historico.unshift({
    ts: d.timestamp || agora,
    modo: d.modo, estado: d.estado, porta: d.porta, uid: d.uid
  });
  if (historico.length > MAX_HISTORICO) historico.pop();

  if (anterior !== novo) {
    eventos.unshift({
      ts: d.timestamp || agora,
      tipo: "MUDANCA_ESTADO",
      de: anterior, para: novo
    });
    if (eventos.length > MAX_EVENTOS) eventos.pop();
    salvarHistorico();
  }

  console.log(`[ESP32] ${d.timestamp || agora} | ${novo} | Porta: ${d.porta} | UID: ${d.uid}`);
  res.json({ ok: true });
});

// Navegador consulta status (GET)
app.get("/api/status", (req, res) => {
  res.json({
    ...estadoAtual,
    historico: historico.slice(0, 20),
    eventos:   eventos.slice(0, 20)
  });
});

// Navegador envia comando (POST)
app.post("/api/comando", (req, res) => {
  const { comando } = req.body;
  if (!comando) return res.status(400).json({ erro: "comando obrigatorio" });

  comandoPendente = comando;
  eventos.unshift({ ts: new Date().toISOString(), tipo: "COMANDO_WEB", comando });
  if (eventos.length > MAX_EVENTOS) eventos.pop();
  salvarHistorico();

  console.log(`[WEB] Comando recebido: ${comando}`);
  res.json({ ok: true });
});

// ESP32 consulta comandos pendentes (GET)
app.get("/api/comando", (req, res) => {
  const cmd = comandoPendente;
  comandoPendente = null;
  res.json({ comando: cmd });
});

// Health check
app.get("/api/health", (req, res) => {
  res.json({ status: "ok", uptime: process.uptime() });
});

// Estatisticas
app.get("/api/stats", (req, res) => {
  res.json({
    totalStatus: historico.length,
    totalEventos: eventos.length,
    uptime: process.uptime(),
    ultimaAtualizacao: estadoAtual.ultimaAtualizacao
  });
});

garantirPastaDados();
carregarHistorico();

app.listen(PORT, "0.0.0.0", () => {
  console.log("=================================================");
  console.log("  RACK MONITOR SERVER");
  console.log("=================================================");
  console.log(`  Porta:      ${PORT}`);
  console.log(`  URL local:  http://localhost:${PORT}`);
  console.log(`  Dados em:   ${DATA_DIR}`);
  console.log("=================================================");
});
