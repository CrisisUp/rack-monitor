# 🖥️ Rack Monitor

Sistema de monitoramento de rack com ESP32, RFID e dashboard web em tempo real.

![Status](https://img.shields.io/badge/status-est%C3%A1vel-green)
![Version](https://img.shields.io/badge/version-7.5-blue)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 📋 Visão geral

O Rack Monitor combina hardware (ESP32 + sensores) com software (Node.js + Docker) para fornecer:

- ✅ **Modo Vigilante** com arme/desarme via RFID
- ✅ **Auditoria** de acessos (UID + timestamp)
- ✅ **Rearme condicionado** (bloqueia se a porta estiver aberta)
- ✅ **Filtros anti-ruído** (LDR com histerese, ultrassônico com média móvel)
- ✅ **Dashboard web** em tempo real
- ✅ **Controle remoto** via navegador
- ✅ **Acesso externo** via Cloudflare Tunnel
- ✅ **URL do servidor persistida em NVS** (troca em runtime, sem recompilar)

---

## 🏗️ Arquitetura

```text
┌──────────────────────────┐
│ ESP32 (Wokwi ou real)    │
│ ├─ RFID                  │
│ ├─ PIR                   │
│ ├─ LDR                   │
│ ├─ Ultrassônico          │
│ ├─ OLED + LEDs + Buzzers │
│ └─ Máquina de estados    │
└────────────┬─────────────┘
             │ HTTPS (keep-alive)
             ▼
┌──────────────────────────┐
│ Cloudflare Tunnel        │
│ (URL pública efêmera)    │
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────┐
│ Servidor Node.js         │
│ (Docker local :3000)     │
│ ├─ API REST              │
│ ├─ Dashboard (public/)   │
│ └─ Persistência JSON     │
└────────────┬─────────────┘
             │ HTTP
             ▼
┌──────────────────────────┐
│ Navegador                │
│ (Dashboard em tempo real)│
└──────────────────────────┘
```

Detalhes em [`docs/ARQUITETURA.md`](docs/ARQUITETURA.md).

## 🚀 Início rápido (macOS)

### Pré-requisitos

| Software | Como instalar |
| -------- | ------------- |
| **Docker Desktop** | [download](https://www.docker.com/products/docker-desktop/) |
| **cloudflared** | `brew install cloudflared` |
| **Python 3** (opcional) | já vem no macOS |
| **Wokwi** | [wokwi.com](https://wokwi.com/) (navegador) |

### 1. Clonar

```bash
git clone https://github.com/SEU-USUARIO/rack-monitor.git
cd rack-monitor
```

### 2. Subir o servidor

```bash
cd server
docker compose up --build -d
```

Aguarde uns 10s e valide:

```bash
curl -s http://localhost:3000/api/health
# {"status":"ok","uptime":12.3}
```

### 3. Abrir o túnel público

Em outro terminal (deixe aberto):

```bash
cloudflared tunnel --url http://localhost:3000
```

Copie a URL gerada. Valide no PC:

```bash
curl -s https://SUA-URL.trycloudflare.com/api/health
```

### 4. Configurar o firmware

No Wokwi, cole o conteúdo de firmware/sketch.ino e ajuste:

```cpp
const char* SERVIDOR_URL_PADRAO = "<https://SUA-URL.trycloudflare.com>";
```

💡 Alternativa sem recompilar: após o boot, mande pelo Serial Monitor:

`SET URL https://SUA-URL.trycloudflare.com`
A URL fica salva em NVS e sobrevive a reboots.

### 5. Rodar no Wokwi

```text
- Cole também firmware/diagram.json (aba diagram.json)
- Cole firmware/libraries.txt (Library Manager)
- Clique em ▶ Play
```

Aguarde o boot:

```text
[NVS] Nenhuma URL salva. Usando padrao: https://...
[WIFI] Conectado! IP: 10.10.0.2
[INFO] [SRV] Conexao com servidor estabelecida!
```

### 6. Acessar o dashboard

Local: <http://localhost:3000>
Público: a URL do Cloudflare

## 📁 Estrutura

```text
rack-monitor/
├── README.md
├── .gitignore
├── docs/
│   ├── ARQUITETURA.md
│   ├── API.md
│   ├── FIRMWARE.md
│   ├── CHANGELOG.md
│   └── TROUBLESHOOTING.md
├── firmware/
│   ├── sketch.ino
│   ├── diagram.json
│   ├── libraries.txt
│   └── README.md
└── server/
    ├── server.js
    ├── Dockerfile
    ├── docker-compose.yml
    ├── package.json
    ├── data/              (histórico persistido)
    ├── public/            (dashboard)
    │   ├── index.html
    │   ├── style.css
    │   ├── app.js
    │   └── favicon.svg
    └── README.md
```

---

## ⏸️ Para aqui

**Salva (`Cmd + S`)** e confirma:

```bash
wc -l ~/Desktop/git/rack-monitor/README.md

## 🧪 Comandos úteis

### Testar servidor

```bash
curl -s http://localhost:3000/api/health
curl -s http://localhost:3000/api/status | python3 -m json.tool
```

Testar túnel

```bash
curl -s https://SUA-URL.trycloudflare.com/api/health
```

Enviar comando via API

```bash
curl -X POST http://localhost:3000/api/comando \
  -H "Content-Type: application/json" \
  -d '{"comando":"DESARMAR"}'
```

Ver logs do container

```bash
cd server
docker compose logs -f
```

### Comandos no Serial Monitor do Wokwi

Digite HELP para listar. Principais:

| Comando | O que faz |
| ------- | --------- |
| STATUS | Estado completo do sistema |
| SET URL https://... | Troca a URL do servidor (salva em NVS) |
| GET URL | Mostra a URL ativa |
| URL RESET | Volta à URL padrão |
| TESTE_SRV | Testa a conexão atual |
| RECONECTAR | Força nova conexão TLS |
| DIAG | Diagnóstico de rede |

### 🔧 Tecnologias

- ESP32 — microcontrolador (Wokwi ou físico)
- Arduino Framework — programação
- Node.js 20 + Express — servidor
- Docker — containerização
- Cloudflare Tunnel — acesso externo
- Vanilla JS — dashboard sem framework

## 📚 Documentação

| Documento | Conteúdo |
| --------- | -------- |
| docs/ARQUITETURA.md | Diagrama + decisões técnicas |
| docs/API.md | Endpoints com exemplos |
| docs/FIRMWARE.md | Comandos serial + sensores |
| docs/CHANGELOG.md | Histórico de versões |
| docs/TROUBLESHOOTING.md | Problemas comuns |

## 🤝 Contribuindo

### Fork o projeto

- Crie uma branch (git checkout -b feature/nova-funcionalidade)
- Commit (git commit -m 'feat: adiciona nova funcionalidade')
- Push (git push origin feature/nova-funcionalidade)
- Abra um Pull Request

## 📄 Licença

MIT — veja LICENSE para detalhes.

## 👤 Autor

Cristiano — GitHub

## 🙏 Agradecimentos

Wokwi pela simulação do ESP32
Cloudflare pelo túnel gratuito
Comunidade Arduino/ESP32
