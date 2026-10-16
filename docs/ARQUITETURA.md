# 🏗️ Arquitetura

## Componentes

1. **ESP32** — Sensores, RFID, lógica de controle
2. **Cloudflare Tunnel** — Expõe servidor local na internet
3. **Servidor Node.js (Docker)** — API REST + persistência
4. **Dashboard Web** — Interface HTML/JS

## Fluxo de Dados

- ESP32 → Servidor: `POST /api/status` a cada 5s
- ESP32 → Servidor: `GET /api/comando` a cada 3s
- Navegador → Servidor: `GET /api/status` a cada 2s

## Decisões Técnicas

### Cloudflare Tunnel (não ngrok)

- ngrok HTTP: bloqueia ESP32 com página de aviso
- ngrok TCP: exige cartão de crédito
- LocalTunnel: incompatível com Docker no Windows
- **Cloudflare:** gratuito, sem cartão, estável

### Filtros no ESP32

- LDR: histerese 1800/2200 + debounce 800ms
- Ultrassônico: média móvel de 5 leituras
- Botão: debounce confirmatório 250ms
- Porta: debounce 200ms
- Estado: debounce 300ms

Resultado: zero falsos positivos.

### Modo Vigilante

- Arme/desarme via RFID
- Auditoria com UID + timestamp
- Rearme bloqueado se porta aberta
- Timeout automático de 30 min
