# 🏗️ Arquitetura

## Visão geral

O Rack Monitor é um sistema distribuído em 3 camadas:

1. **Dispositivo** — ESP32 + sensores + atuadores
2. **Servidor** — Node.js em Docker
3. **Interface** — dashboard web

## Diagrama de componentes

```text
┌───────────────────────────────────────────────────-──┐
│ DISPOSITIVO                                          │
│ ┌──────────────────────────────────────────────┐     │
│ │ ESP32                                        │     │
│ │ ┌────────┐ ┌────────┐ ┌────────┐             │     │
│ │ │ RFID   │ │ PIR    │ │ LDR    │             │     │
│ │ └───┬────┘ └───┬────┘ └───┬────┘             │     │
│ │     │          │          │                  │     │
│ │ ┌───▼──────────▼──────────▼───---─┐          │     │
│ │ │ Lógica de Controle              │          │     │
│ │ │ ├─ Modo Vigilante               │          │     │
│ │ │ ├─ Máquina de Estados           │          │     │
│ │ │ ├─ Filtros anti-ruído           │          │     │
│ │ │ └─ Fila de comandos             │          │     │
│ │ └───┬───────────┬───────────┬─────┘          │     │
│ │     │           │           │                │     │
│ │ ┌───▼────┐  ┌───▼────┐  ┌───▼────┐           │     │
│ │ │ OLED   │  │LED RGB │  │Buzzers │           │     │
│ │ └────────┘  └────────┘  └────────┘           │     │
│ └──────────────────────────────────────────────┘     │
└───────────────────────┬──────────────────────────────┘
                        │ HTTPS keep-alive
                        ▼
┌───────────────────────────────────────────────────-──┐
│ SERVIDOR                                             │
│ ┌──────────────────────────────────────────────┐     │
│ │ Node.js (Docker, porta 3000)                 │     │
│ │ ┌────────────────────────────────────────┐   │     │
│ │ │ Express                                │   │     │
│ │ │ ├─ POST /api/status (ESP32 →)          │   │     │
│ │ │ ├─ GET /api/status (browser →)         │   │     │
│ │ │ ├─ GET /api/comando (ESP32 →)          │   │     │
│ │ │ ├─ POST /api/comando (browser →)       │   │     │
│ │ │ └─ GET /api/health                     │   │     │
│ │ └────────────────────────────────────────┘   │     │
│ │ ┌────────────────────────────────────────┐   │     │
│ │ │ Persistência (JSON)                    │   │     │
│ │ │ └─ data/historico.json                 │   │     │
│ │ └────────────────────────────────────────┘   │     │
│ └──────────────────────────────────────────────┘     │
└───────────────────────┬──────────────────────────────┘
                        │ HTTP
                        ▼
┌─────────────────────────────────────────────────────┐
│ INTERFACE                                           │
│ ┌──────────────────────────────────────────────┐    │
│ │ Dashboard (HTML/CSS/JS vanilla)              │    │
│ │ ├─ Cards de status                           │    │
│ │ ├─ Log de eventos                            │    │
│ │ ├─ Botões de controle                        │    │
│ │ └─ Polling a cada 2s                         │    │
│ └──────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────┘
```

## Fluxo de dados

### ESP32 → Servidor (a cada 20s)

ESP32 Cloudflare Docker
│ │ │
│─── POST /api/status ──────────►│ │
│ {modo, estado, porta, ...} │ │
│ │─── Encaminha ─────────────►│
│ │ │─── Atualiza estado
│ │ │─── Salva histórico
│ │◄── 200 {ok:true} ──────────│
│◄── 200 {ok:true} ──────────────│ │

text

### Servidor → ESP32 (a cada 15s)

ESP32 Cloudflare Docker
│ │ │
│─── GET /api/comando ──────────►│ │
│ │─── Encaminha ─────────────►│
│ │ │─── Verifica fila
│ │◄── {comando:"ARMAR"} ──────│
│◄── {comando:"ARMAR"} ──────────│ │
│ │ │
│─── Executa comando │ │

text

### Navegador → Servidor (a cada 2s)

Navegador Docker
│                              │
│─── GET /api/status ─────────►│
│ │─── Retorna estado + histórico
│◄── JSON ─────────────────────│
│                 │
│─── Atualiza DOM │

text

## Decisões técnicas

### Por que Cloudflare Tunnel?

| Alternativa | Problema |
| ----------- | -------- |
| ngrok HTTP | Página de aviso bloqueia o ESP32 |
| ngrok TCP | Exige cartão de crédito |
| LocalTunnel | Incompatível com Docker |
| **Cloudflare** | ✅ Gratuito, sem cartão, estável |

### Por que Docker?

- Ambiente isolado e reproduzível
- Deploy em uma linha
- Volume para persistência
- Fácil migração entre máquinas

### Por que filtros no ESP32 e não no servidor?

Latência. Se o filtro ficasse no servidor, cada ruído no LDR geraria um POST HTTP. Filtros locais:

- **LDR**: histerese dupla (1800/2200) + debounce 800ms
- **Ultrassônico**: média móvel de 5 leituras
- **Botão**: debounce confirmatório 250ms
- **Porta**: debounce confirmatório 200ms
- **Estado**: debounce de transição 300ms (críticos mudam imediatamente)

Resultado: **zero falsos positivos** nos logs.

### Por que Modo Vigilante?

- Auditoria de arme/desarme (quem, quando, por quê)
- Rastreabilidade de UID
- Política de segurança: rearme **bloqueado** se porta aberta
- Alinhado conceitualmente com EN 50131 / UL 681

## Limitações conhecidas

### Wokwi

- HTTPS com rate limit — pode retornar `HTTP -1` esporadicamente
- WiFi virtual (`Wokwi-GUEST`, IP `10.10.0.2`)
- Sem acesso do navegador ao IP virtual

### Cloudflare quick tunnel

- URL muda a cada restart do `cloudflared`
- Sem SLA (matado após ~4h de uso contínuo)
- Sem balanceamento de carga

**Solução:** URL persistida em NVS, comando `SET URL` em runtime, ou túnel nomeado com domínio próprio.

## Segurança

### Implementado

- ✅ RFID para arme/desarme
- ✅ Rearme condicionado (porta fechada)
- ✅ Auditoria de UID + timestamp
- ✅ HTTPS via Cloudflare
- ✅ `setInsecure()` (aceita qualquer cert — ok para quick tunnel)

### Recomendado para produção

- 🔲 Autenticação no dashboard (HTTP Basic ou token)
- 🔲 CA raiz real em vez de `setInsecure()`
- 🔲 Túnel nomeado com domínio fixo
- 🔲 Backup periódico do `data/historico.json`
- 🔲 Rotação de credenciais WiFi
