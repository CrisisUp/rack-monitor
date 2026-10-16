
# 🖥️ Rack Monitor

Sistema profissional de monitoramento de rack de servidores com ESP32, RFID e dashboard web em tempo real.

![Status](https://img.shields.io/badge/status-em%20desenvolvimento-yellow)
![Version](https://img.shields.io/badge/version-7.2-blue)
[![License](https://img.shields.io/badge/license-MIT-green)](https://opensource.org/license/mit/)

---

## 📋 Visão Geral

O Rack Monitor combina hardware (ESP32 + sensores) com software (Node.js + Docker) para fornecer:

- ✅ **Modo Vigilante** com arme/desarme via RFID
- ✅ **Auditoria completa** (quem fez o quê, quando)
- ✅ **Rearme condicionado** (bloqueia se porta aberta)
- ✅ **Filtros anti-ruído** (LDR + ultrassônico)
- ✅ **Dashboard web** em tempo real
- ✅ **Controle remoto** via navegador
- ✅ **Túnel Cloudflare** para acesso externo

---

## 🏗️ Arquitetura
┌──────────────────────┐
│ ESP32 (Wokwi/real) │
│ - RFID │
│ - PIR │
│ - LDR │
│ - Ultrassônico │
│ - OLED │
│ - LEDs/Buzzers │
└──────────┬───────────┘
│ HTTPS
▼
┌──────────────────────┐
│ Cloudflare Tunnel │
│ (URL pública) │
└──────────┬───────────┘
│
▼
┌──────────────────────┐
│ Servidor Node.js │
│ (Docker local) │
│ - API REST │
│ - Dashboard HTML │
│ - Persistência │
└──────────┬───────────┘
│
▼
┌──────────────────────┐
│ Navegador │
│ (Dashboard) │
└──────────────────────┘

text

Documentação detalhada: [docs/ARQUITETURA.md](docs/ARQUITETURA.md)

---

## 🚀 Início Rápido

### Pré-requisitos

- **Docker Desktop** ([download](https://www.docker.com/products/docker-desktop/))
- **Node.js 20+** ([download](https://nodejs.org/))
- **cloudflared** ([download](https://github.com/cloudflare/cloudflared/releases))
- Conta no **Wokwi** ([cadastro](https://wokwi.com/))

### 1. Clonar o repositório

```powershell
git clone <URL-do-repo>
cd rack-monitor
2. Subir o servidor
powershell
cd server
docker compose up --build
Aguarde aparecer:

text
rack-monitor-server  |   RACK MONITOR SERVER
rack-monitor-server  |   Porta:      3000
3. Abrir túnel Cloudflare
Em outro terminal:

powershell
& "$HOME\cloudflared.exe" tunnel --url http://localhost:3000
Copie a URL gerada (ex: https://abc-xyz.trycloudflare.com).

4. Configurar o firmware
No firmware/sketch.ino, atualize:

cpp
const char* SERVIDOR_URL = "https://SUA-URL.trycloudflare.com";
5. Rodar o firmware
No Wokwi: cole firmware/sketch.ino, firmware/diagram.json e firmware/libraries.txt.

6. Acessar o dashboard
Abra no navegador:

Local: http://localhost:3000

Público: a URL do Cloudflare

📚 Documentação
Documento	Descrição
ARQUITETURA.md	Decisões técnicas e diagramas
INSTALACAO.md	Passo a passo detalhado
API.md	Endpoints do servidor
FIRMWARE.md	Comandos do ESP32
CHANGELOG.md	Histórico de versões
TROUBLESHOOTING.md	Problemas comuns
🧪 Testes
Testar servidor local
powershell
Invoke-RestMethod http://localhost:3000/api/health
Testar túnel
powershell
Invoke-WebRequest "https://SUA-URL.trycloudflare.com/api/health"
Testar firmware (no Serial Monitor)
text
HELP          → lista comandos
STATUS        → status completo
TESTE_HTTPS   → testa HTTPS
TESTE_SRV     → testa servidor remoto
DIAG          → diagnóstico de rede
📦 Estrutura do Projeto
text
rack-monitor/
├── docs/              → documentação
├── firmware/          → código do ESP32
└── server/            → servidor Node.js + Docker
🔧 Tecnologias
ESP32 — microcontrolador

Arduino Framework — programação

Wokwi — simulação

Node.js 20 — servidor

Express — framework HTTP

Docker — containerização

Cloudflare Tunnel — túnel público

🤝 Contribuindo
Fork o projeto

Crie uma branch (git checkout -b feature/nova-funcionalidade)

Commit (git commit -m 'Adiciona nova funcionalidade')

Push (git push origin feature/nova-funcionalidade)

Abra um Pull Request

📄 Licença
MIT — veja LICENSE para detalhes.

👤 Autor
Cristiano — GitHub

🙏 Agradecimentos
Wokwi pela simulação

Cloudflare pelo túnel gratuito

Comunidade Arduino/ESP32
'@ | Out-File -FilePath README.md -Encoding utf8

Write-Host "README.md criado!" -ForegroundColor Green

text

---

## 🚀 Passo 4 — Criar documentação em `docs/`

### 📌 4.1 — `docs/ARQUITETURA.md`

```powershell
cd $HOME\Desktop\rack-monitor\docs

@'
# 🏗️ Arquitetura do Sistema

## Visão Geral

O Rack Monitor é um sistema distribuído de 3 camadas:

1. **Dispositivo** (ESP32 + sensores)
2. **Servidor** (Node.js + Docker)
3. **Interface** (Dashboard web)

## Diagrama de Componentes
┌─────────────────────────────────────────────────────┐
│ DISPOSITIVO │
│ ┌─────────────────────────────────────────────┐ │
│ │ ESP32 │ │
│ │ ┌─────────┐ ┌─────────┐ ┌─────────┐ │ │
│ │ │ RFID │ │ PIR │ │ LDR │ │ │
│ │ └────┬────┘ └────┬────┘ └────┬────┘ │ │
│ │ │ │ │ │ │
│ │ ┌────▼────────────▼────────────▼────┐ │ │
│ │ │ Lógica de Controle │ │ │
│ │ │ - Modo Vigilante │ │ │
│ │ │ - Máquina de Estados │ │ │
│ │ │ - Filtros Anti-Ruído │ │ │
│ │ │ - Debounce │ │ │
│ │ └────┬────────────┬────────────┬────┘ │ │
│ │ │ │ │ │ │
│ │ ┌────▼────┐ ┌────▼────┐ ┌────▼────┐ │ │
│ │ │ OLED │ │LED RGB │ │Buzzers │ │ │
│ │ └─────────┘ └─────────┘ └─────────┘ │ │
│ └─────────────────────────────────────────────┘ │
└──────────────────────┬──────────────────────────────┘
│ HTTPS
▼
┌─────────────────────────────────────────────────────┐
│ SERVIDOR │
│ ┌─────────────────────────────────────────────┐ │
│ │ Node.js (Docker) │ │
│ │ ┌─────────────────────────────────────┐ │ │
│ │ │ Express │ │ │
│ │ │ - POST /api/status (ESP32 →) │ │ │
│ │ │ - GET /api/status (→ browser) │ │ │
│ │ │ - GET /api/comando (ESP32 →) │ │ │
│ │ │ - POST /api/comando (browser →) │ │ │
│ │ │ - GET /api/health │ │ │
│ │ └─────────────────────────────────────┘ │ │
│ │ ┌─────────────────────────────────────┐ │ │
│ │ │ Persistência (JSON) │ │ │
│ │ │ - historico.json │ │ │
│ │ └─────────────────────────────────────┘ │ │
│ └─────────────────────────────────────────────┘ │
└──────────────────────┬──────────────────────────────┘
│ HTTP
▼
┌─────────────────────────────────────────────────────┐
│ INTERFACE │
│ ┌─────────────────────────────────────────────┐ │
│ │ Dashboard (HTML/CSS/JS) │ │
│ │ - Cards de status │ │
│ │ - Log de eventos │ │
│ │ - Botões de controle │ │
│ │ - Atualização a cada 2s │ │
│ └─────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────┘

text

## Fluxo de Dados

### 1. ESP32 → Servidor (a cada 5s)
ESP32 Cloudflare Docker
│ │ │
│─ POST /api/status ──────►│ │
│ {modo, estado, ...} │─ Encaminha ───────────►│
│ │ │─ Atualiza estado
│ │ │─ Salva histórico
│ │◄── 200 OK ─────────────│
│◄── 200 OK ───────────────│ │

text

### 2. Servidor → ESP32 (a cada 3s)
ESP32 Cloudflare Docker
│ │ │
│─ GET /api/comando ──────►│ │
│ │─ Encaminha ───────────►│
│ │ │─ Verifica fila
│ │◄─ {comando: "..."} ────│
│◄─ {comando: "..."} ──────│ │
│ │ │
│─ Executa comando │ │

text

### 3. Navegador → Servidor
Navegador Docker
│ │
│─ GET /api/status ─────►│
│ │─ Retorna estado + histórico
│◄─ JSON ────────────────│
│ │
│─ Atualiza DOM │

text

## Decisões Técnicas

### Por que Cloudflare Tunnel?

| Alternativa | Problema |
|---|---|
| ngrok HTTP | Página de aviso bloqueia ESP32 |
| ngrok TCP | Exige cartão de crédito |
| LocalTunnel | Não funciona com Docker no Windows |
| **Cloudflare** | ✅ Gratuito, sem cartão, estável |

### Por que Docker?

- ✅ Ambiente isolado e reproduzível
- ✅ Fácil deploy (só precisa Docker)
- ✅ Volume para persistência de dados

### Por que filtros no ESP32?

- **LDR:** histerese dupla (1800/2200) + debounce 800ms
- **Ultrassônico:** média móvel de 5 leituras
- **Botão:** debounce confirmatório (250ms)
- **Porta:** debounce confirmatório (200ms)
- **Estado:** debounce de transição (300ms)

Resultado: **zero falsos positivos** nos logs.

### Por que Modo Vigilante?

- ✅ Auditoria de armamento/desarmamento
- ✅ Rastreabilidade (quem, quando, por quê)
- ✅ Política de segurança (rearme bloqueado se porta aberta)
- ✅ Alinhado com EN 50131 / UL 681

## Limitações Conhecidas

### Wokwi

- ⚠️ HTTPS limitado (rate limit de requisições)
- ⚠️ WiFi virtual (`Wokwi-GUEST`)
- ⚠️ Sem acesso do navegador ao IP virtual

### Solução

- ✅ **Desenvolvimento:** Wokwi
- ✅ **Testes de integração:** ESP32 físico
- ✅ **Produção:** ESP32 físico

## Segurança

### Implementado

- ✅ RFID para arme/desarme
- ✅ Rearme condicionado
- ✅ Auditoria com UID + timestamp
- ✅ HTTPS via Cloudflare

### Recomendado para produção

- 🔲 Autenticação no dashboard (HTTP Basic)
- 🔲 HTTPS no dashboard
- 🔲 Backup de configuração
- 🔲 Rotação de URL do Cloudflare
'@ | Out-File -FilePath ARQUITETURA.md -Encoding utf8

Write-Host "ARQUITETURA.md criado!" -ForegroundColor Green
📌 4.2 — docs/API.md
powershell
@'
# 🔌 API do Servidor

Base URL: `http://localhost:3000` (local) ou `https://sua-url.trycloudflare.com` (público)

## Endpoints

### `GET /api/health`

Verifica se o servidor está funcionando.

**Resposta:**
```json
{
  "status": "ok",
  "uptime": 3600.5
}
GET /api/status
Retorna o estado atual do sistema + histórico recente.

Resposta:

json
{
  "timestamp": "16/10 01:40:54",
  "modo": "ARMADO",
  "estado": "SEGURO",
  "porta": "fechada",
  "armado": true,
  "acessos": 5,
  "bloqueios": 1,
  "invasoes": 0,
  "uid": "01020304",
  "ip": "10.10.0.2",
  "ultimaAtualizacao": "2026-10-16T04:40:54.000Z",
  "historico": [
    {
      "ts": "16/10 01:40:54",
      "modo": "ARMADO",
      "estado": "SEGURO",
      "porta": "fechada",
      "uid": "01020304"
    }
  ],
  "eventos": [
    {
      "ts": "2026-10-16T04:40:54.000Z",
      "tipo": "MUDANCA_ESTADO",
      "de": "DESARMADO/SEGURO",
      "para": "ARMADO/SEGURO"
    }
  ]
}
POST /api/status
Enviado pelo ESP32 para atualizar o estado.

Corpo:

json
{
  "timestamp": "16/10 01:40:54",
  "modo": "ARMADO",
  "estado": "SEGURO",
  "porta": "fechada",
  "armado": true,
  "acessos": 5,
  "bloqueios": 1,
  "invasoes": 0,
  "uid": "01020304"
}
Resposta:

json
{ "ok": true }
GET /api/comando
Consulta comandos pendentes para o ESP32.

Resposta:

json
{ "comando": "ARMAR" }
Ou, se não houver comando:

json
{ "comando": null }
POST /api/comando
Envia um comando para o ESP32.

Corpo:

json
{ "comando": "ARMAR" }
Comandos válidos:

ARMAR

DESARMAR

TESTE

RESET

Resposta:

json
{ "ok": true }
GET /api/stats
Estatísticas gerais do servidor.

Resposta:

json
{
  "totalStatus": 100,
  "totalEventos": 25,
  "uptime": 3600.5,
  "ultimaAtualizacao": "2026-10-16T04:40:54.000Z"
}
Códigos de Status HTTP
Código	Significado
200	Sucesso
400	Requisição inválida (ex: comando ausente)
404	Endpoint não encontrado
500	Erro interno
Exemplos de Uso
PowerShell
powershell
# Health check
Invoke-RestMethod http://localhost:3000/api/health

# Status
Invoke-RestMethod http://localhost:3000/api/status

# Enviar comando
Invoke-RestMethod -Method POST -Uri http://localhost:3000/api/comando `
  -ContentType "application/json" -Body '{"comando":"ARMAR"}'
cURL
bash
curl http://localhost:3000/api/health
curl http://localhost:3000/api/status
curl -X POST http://localhost:3000/api/comando \
  -H "Content-Type: application/json" \
  -d '{"comando":"ARMAR"}'
JavaScript (navegador)
javascript
fetch('/api/status')
  .then(r => r.json())
  .then(d => console.log(d));

fetch('/api/comando', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({ comando: 'ARMAR' })
});
'@ | Out-File -FilePath API.md -Encoding utf8

Write-Host "API.md criado!" -ForegroundColor Green

text

### 📌 4.3 — `docs/FIRMWARE.md`

```powershell
@'
# 🔧 Firmware do ESP32

## Visão Geral

O firmware do ESP32 é responsável por:
- Ler sensores (RFID, PIR, LDR, ultrassônico)
- Controlar atuadores (OLED, LED RGB, buzzers)
- Gerenciar o **Modo Vigilante**
- Comunicar com o servidor via HTTPS
- Responder a comandos do dashboard

## Comandos do Serial Monitor

### Sistema

| Comando | Descrição |
|---|---|
| `HELP` | Lista todos os comandos |
| `STATUS` | Status completo do sistema |
| `RESET` | Zera todos os contadores |

### Modo Vigilante

| Comando | Descrição |
|---|---|
| `ARMAR` | Força modo ARMADO (se porta fechada) |
| `DESARMAR` | Força modo DESARMADO |
| `SET TIMEOUT <0-1440>` | Timeout de desarme (em minutos) |

### Sensores

| Comando | Descrição |
|---|---|
| `SET LIMIAR <5-400>` | Ajusta limiar do ultrassônico (cm) |

### Testes

| Comando | Descrição |
|---|---|
| `TESTE` | Modo teste (LEDs, buzzers, sensores) |
| `TESTE_HTTPS` | Testa HTTPS para 3 sites |
| `TESTE_SRV` | Testa conexão com servidor remoto |
| `DIAG` | Diagnóstico de rede detalhado |

### RTC

| Comando | Descrição |
|---|---|
| `RTC` | Mostra hora atual |
| `RTC YYYY-MM-DD HH:MM:SS` | Ajusta RTC |

## Cartões RFID Autorizados

Por padrão, dois cartões estão cadastrados:

```cpp
const String CARTAO_AUTORIZADO_1 = "01020304";  // Admin
const String CARTAO_AUTORIZADO_2 = "55667788";  // Operador
Para adicionar novos, edite estas constantes.

Modo Vigilante
Comportamento
Ação	Resultado
Passar cartão (ARMADO)	Desarma (modo manutenção)
Passar cartão (DESARMADO)	Arma (se porta fechada)
Rearme com porta aberta	Bloqueado
Timeout expirado	Rearme automático
Política de Segurança
Desarme é sempre permitido (para manutenção)

Rearme exige porta fechada

Timeout rearma automaticamente após 30 min

Aviso 5 min antes do rearme automático

Filtros Anti-Ruído
LDR (fotoresistor)
Histerese: acende em 2200, apaga em 1800

Debounce: 800ms

Ultrassônico
Média móvel de 5 leituras

Ignora leituras > 400 cm

Botão
Debounce confirmatório: 250ms

Clique curto: < 400ms

Clique duplo: 2 cliques em 400ms

Clique longo: > 3000ms

Porta (switch)
Debounce confirmatório: 200ms

Estado (máquina)
Debounce de transição: 300ms

Estados críticos mudam imediatamente

Display OLED
Tela Normal (ARMADO)
text
┌──────────────────────┐
│ 🔒 VIGILANTE ARMADO  │
│ ──────────────────── │
│ Porta Fechada        │
│ ──────────────────── │
│ 16/10 01:40  AB: 3   │
└──────────────────────┘
Tela Manutenção (DESARMADO)
text
┌──────────────────────┐
│ 🛡️ MANUTENCAO        │
│ ──────────────────── │
│ Rearme em: 29m45s    │
│ UID: 01020304        │
│ ──────────────────── │
│ 16/10 01:40  MANUT: 2│
└──────────────────────┘
Tela Bloqueio
text
┌──────────────────────┐
│ ⚠️ REARME BLOQUEADO  │
│ ──────────────────── │
│ Feche a porta!       │
│ Bloqueios: 1         │
│ ──────────────────── │
│ UID: 01020304        │
└──────────────────────┘
LED RGB
Estado	Cor
SEGURO (armado)	Verde fixo
DESARMADO	Azul respirando
AVISO_SALA	Amarelo
APROXIMACAO_RACK	Laranja
INVASAO_SALA	Vermelho piscando
EMERGENCIA_RACK	Vermelho fixo
Bloqueio	Amarelo piscando
Buzzers
Estado	Som
SEGURO	Silêncio
AVISO_SALA	Bip 523Hz a cada 700ms
APROXIMACAO_RACK	Bip 750Hz a cada 400ms
INVASAO_SALA	Sirene oscilante 400-900Hz
EMERGENCIA_RACK	Sirene alternada 900/1300Hz
Desarme	Tom descendente
Rearme	Tom ascendente
Bloqueio	Tom grave 500→400Hz
'@	Out-File -FilePath FIRMWARE.md -Encoding utf8
Write-Host "FIRMWARE.md criado!" -ForegroundColor Green

text

### 📌 4.4 — `docs/CHANGELOG.md`

```powershell
@'
# 📝 Changelog

Todas as mudanças notáveis neste projeto serão documentadas aqui.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/).

## [7.2] - 2026-10-16

### Adicionado
- Integração com **Cloudflare Tunnel** (substitui ngrok)
- Comando `TESTE_HTTPS` para validar HTTPS
- Comando `DIAG` para diagnóstico completo
- Indicador visual de servidor online no OLED

### Corrigido
- Headers HTTP desnecessários removidos
- Timeout de requisições aumentado para 10s

### Removido
- Suporte a ngrok (bloqueado por cartão)
- Suporte a LocalTunnel (incompatível com Docker no Windows)

## [7.1] - 2026-10-16

### Adicionado
- Headers `ngrok-skip-browser-warning` e `User-Agent`
- Comando `TESTE_NGROK`
- Diagnóstico detalhado de erros HTTP

### Corrigido
- Problema de status `unhealthy` no Docker

## [7.0] - 2026-10-16

### Adicionado
- **Cliente HTTP** para comunicação com servidor remoto
- Envio periódico de status (5s)
- Busca periódica de comandos (3s)
- Contadores de envios e comandos

### Mudado
- Arquitetura: ESP32 deixa de ser servidor, vira cliente

## [6.0] - 2026-10-16

### Adicionado
- **Web Server** local no ESP32
- Dashboard HTML embutido
- API REST (JSON)
- Fallback AP

### Removido
- Dependência de Serial Monitor para controle

## [5.0] - 2026-10-16

### Adicionado
- **Rearme condicionado** (bloqueia se porta aberta)
- Debounce confirmatório do botão
- Debounce do switch da porta
- Contador de bloqueios
- Aviso de rearme bloqueado no OLED

### Mudado
- Timeout não rearma mais se porta aberta

## [4.9] - 2026-10-15

### Adicionado
- Debounce do botão (200ms)
- Janela de clique duplo reduzida
- Contador de bounces ignorados

## [4.8] - 2026-10-15

### Adicionado
- **RTC DS3231** com fallback para millis
- Filtros anti-ruído (LDR + ultrassônico)
- Debounce de estado (300ms)
- Comandos `RTC`

### Corrigido
- Ruído no LDR (histerese)
- Falso positivo do ultrassônico (média móvel)

## [4.7] - 2026-10-15

### Adicionado
- **Modo Vigilante** com arme/desarme por RFID
- Auditoria com UID + timestamp
- Timeout programável
- Botão multifunção (curto/longo/duplo)

### Mudado
- Botão não silencia mais — vira reset/teste

## [4.6] - 2026-10-15

### Adicionado
- Sistema base de monitoramento
- Máquina de estados (SEGURO, AVISO, INVASAO, EMERGENCIA)
- OLED, LED RGB, buzzers
- Leitura RFID (autorização simples)
'@ | Out-File -FilePath CHANGELOG.md -Encoding utf8

Write-Host "CHANGELOG.md criado!" -ForegroundColor Green
📌 4.5 — docs/TROUBLESHOOTING.md
powershell
@'
# 🔧 Troubleshooting

Problemas comuns e suas soluções.

## Docker

### `Cannot connect to the Docker daemon`

**Causa:** Docker Desktop não está rodando.

**Solução:**
1. Abra o **Docker Desktop**
2. Aguarde a baleia 🐳 ficar verde
3. Tente novamente

### Container com status `unhealthy`

**Causa:** healthcheck falhando (falta `wget` na imagem Alpine).

**Solução:** edite `Dockerfile` e substitua o `HEALTHCHECK` por:

```dockerfile
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
  CMD node -e "require('http').get('http://localhost:3000/api/health',r=>process.exit(r.statusCode===200?0:1)).on('error',()=>process.exit(1))"
Depois:

powershell
docker compose down
docker compose up --build
port 3000 already in use
Solução: edite docker-compose.yml:

yaml
ports:
  - "3001:3000"   # usa 3001 no host
Cloudflare Tunnel
URL não funciona
Causa: o cloudflared foi reiniciado, gerando nova URL.

Solução:

Copie a nova URL do terminal

Atualize SERVIDOR_URL no sketch.ino

Recompile e envie para o Wokwi

Túnel caiu
Sintoma: ERR Failed to dial edge...

Solução: reinicie:

powershell
& "$HOME\cloudflared.exe" tunnel --url http://localhost:3000
Cloudflare bloqueia
Causa: possível rate limit.

Solução: aguarde 1-2 minutos e tente novamente.

Wokwi
HTTP -1 | connection refused
Causas possíveis:

URL do servidor incorreta

Cloudflare caiu

Wokwi bloqueia HTTPS temporariamente

Solução:

Rode TESTE_HTTPS no Serial Monitor

Se todos retornarem -1 → Wokwi bloqueia HTTPS

Se alguns retornarem 200 → problema é específico

WiFi não conecta
Sintoma: [WIFI] Falha. Ativando Access Point...

Causa: Wokwi-GUEST indisponível no momento.

Solução:

Aguarde e reinicie o Wokwi

O fallback AP permite testar o sistema offline

Muitas requisições HTTPS
Sintoma: funciona por 30s, depois HTTP -1 repetidamente.

Causa: rate limit do Wokwi.

Solução: aumente os intervalos no sketch.ino:

cpp
constexpr unsigned long INTERVALO_ENVIO_STATUS_MS = 20000;  // 20s
constexpr unsigned long INTERVALO_BUSCA_CMD_MS    = 15000;  // 15s
Firmware
Não compila
Causa: biblioteca faltando.

Solução: verifique firmware/libraries.txt e adicione no Wokwi:

Adafruit GFX

Adafruit SSD1306

ArduinoJson

RTClib

MFRC522

RFID não lê
Causa: cartão não está próximo o suficiente ou UID diferente.

Solução:

Aproxime o cartão do leitor

Veja o UID no Serial Monitor

Adicione o UID em CARTAO_AUTORIZADO_1 ou 2

RTC não funciona
Causa: DS3231 ausente (esperado no Wokwi).

Solução: o código usa millis() como fallback. Para hora real, adicione um RTC físico.

Servidor Node.js
Erro 500
Causa: erro no server.js.

Solução:

powershell
docker logs rack-monitor-server --tail 50
Dashboard não atualiza
Causa: WebSocket/AJAX bloqueado ou erro de JS.

Solução:

Abra o Console do navegador (F12)

Verifique erros de JavaScript

Recarregue a página (Ctrl+F5)

Rede
connection refused
Causa: servidor não está escutando na porta esperada.

Solução:

powershell
Invoke-RestMethod http://localhost:3000/api/health
Se falhar, reinicie o Docker.

Latência alta
Causa: Cloudflare roteando por região distante.

Solução: escolha uma região próxima:

powershell
& "$HOME\cloudflared.exe" tunnel --url http://localhost:3000 --region gru
gru = São Paulo.

Diagnóstico
Comando DIAG
No Serial Monitor do Wokwi:

text
DIAG
Retorna:

Status WiFi

IP / Gateway / DNS

RSSI

Teste DNS

Teste HTTP

Teste HTTPS

Comando STATUS
No Serial Monitor:

text
STATUS
Retorna:

Modo Vigilante

Estado de alarme

WiFi

Servidor remoto

Contadores

Suporte
Se o problema persistir:

Capture o log completo do Serial Monitor

Capture o log do Docker (docker logs rack-monitor-server)

Verifique se o Cloudflare Tunnel está ativo

Documente o problema em docs/logs/
'@ | Out-File -FilePath TROUBLESHOOTING.md -Encoding utf8

Write-Host "TROUBLESHOOTING.md criado!" -ForegroundColor Green

text

### 📌 4.6 — `docs/INSTALACAO.md`

```powershell
@'
# 🚀 Guia de Instalação

Passo a passo completo para configurar o Rack Monitor.

## Pré-requisitos

### Obrigatórios

| Software | Versão mínima | Download |
|---|---|---|
| **Docker Desktop** | 24.x | [link](https://www.docker.com/products/docker-desktop/) |
| **Node.js** | 20.x | [link](https://nodejs.org/) |
| **cloudflared** | 2024.x | [link](https://github.com/cloudflare/cloudflared/releases) |
| **Git** | 2.x | [link](https://git-scm.com/downloads) |

### Contas

| Conta | Necessária para |
|---|---|
| **Wokwi** | Simular ESP32 |
| **Cloudflare** | (opcional) Túnel nomeado |

## Instalação

### 1. Clonar o repositório

```powershell
cd $HOME\Desktop
git clone <URL-do-repo> rack-monitor
cd rack-monitor
2. Instalar cloudflared
powershell
Invoke-WebRequest `
  -Uri "https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-windows-amd64.exe" `
  -OutFile "$HOME\cloudflared.exe"

# Verificar
& "$HOME\cloudflared.exe" --version
3. Subir o servidor Docker
powershell
cd server
docker compose up --build
Aguarde aparecer:

text
rack-monitor-server  | =================================================
rack-monitor-server  |   RACK MONITOR SERVER
rack-monitor-server  | =================================================
rack-monitor-server  |   Porta:      3000
rack-monitor-server  |   URL local:  http://localhost:3000
rack-monitor-server  | =================================================
Deixe este terminal aberto.

4. Criar túnel Cloudflare
Em outro terminal:

powershell
& "$HOME\cloudflared.exe" tunnel --url http://localhost:3000
Aguarde a URL:

text
INF |  Your quick Tunnel has been created! Visit it at:
INF |  https://potentially-sin-hop-syntax.trycloudflare.com
Copie a URL.

5. Testar o servidor
Em outro terminal:

powershell
Invoke-WebRequest "https://SUA-URL.trycloudflare.com/api/health"
Deve retornar:

text
StatusCode        : 200
Content           : {"status":"ok","uptime":45.2}
6. Configurar o firmware
Abra firmware/sketch.ino e atualize:

cpp
const char* SERVIDOR_URL = "https://SUA-URL.trycloudflare.com";
7. Rodar no Wokwi
Acesse https://wokwi.com/projects/new/esp32

Cole firmware/sketch.ino no editor

Cole firmware/diagram.json na aba diagram.json

Cole firmware/libraries.txt no Library Manager

Clique em Play

Aguarde o boot:

text
[WIFI] Conectado! IP: 10.10.0.2
[INFO] [SRV] Conexao com servidor estabelecida!
8. Acessar o dashboard
Abra no navegador:

Local: http://localhost:3000

Público: a URL do Cloudflare

Testes
Testar API
powershell
Invoke-RestMethod http://localhost:3000/api/health
Invoke-RestMethod http://localhost:3000/api/status
Testar firmware
No Serial Monitor do Wokwi:

text
HELP
STATUS
TESTE_HTTPS
TESTE_SRV
DIAG
Testar controle remoto
Abra o dashboard

Clique em Desarmar

Observe o log do Wokwi em até 15s:

text
[INFO] [SRV] Comando remoto: DESARMAR
[INFO] [CMD] Modo Vigilante DESARMADO via comando
Estrutura de Pastas
text
rack-monitor/
├── docs/              → documentação
├── firmware/          → código do ESP32
└── server/            → servidor Node.js
Troubleshooting
Se algo não funcionar, consulte:

TROUBLESHOOTING.md

API.md

FIRMWARE.md

Próximos Passos
Personalizar cartões RFID — edite CARTAO_AUTORIZADO_1/2

Ajustar timeout — SET TIMEOUT 60 no Serial

Ajustar limiar ultrassônico — SET LIMIAR 30

Adicionar autenticação — HTTP Basic no dashboard

Migrar para ESP32 físico — funciona sem limitações do Wokwi
'@ | Out-File -FilePath INSTALACAO.md -Encoding utf8

Write-Host "INSTALACAO.md criado!" -ForegroundColor Green

text

---

## 🚀 Passo 5 — Criar `server/README.md`

```powershell
cd $HOME\Desktop\rack-monitor\server

@'
# 🖥️ Servidor Rack Monitor

Servidor Node.js que recebe dados do ESP32 e serve o dashboard.

## Tecnologias

- **Node.js 20**
- **Express 4**
- **CORS**
- **Docker** (Alpine)

## Estrutura
server/
├── src/
│ └── server.js → código principal
├── public/
│ └── index.html → dashboard
├── data/ → dados persistidos
├── Dockerfile
├── docker-compose.yml
├── package.json
└── README.md

text

## Como rodar

### Com Docker (recomendado)

```powershell
docker compose up --build
Sem Docker (só desenvolvimento)
powershell
npm install
npm start
Endpoints
Ver docs/API.md.

Variáveis de ambiente
Variável	Padrão	Descrição
PORT	3000	Porta do servidor
NODE_ENV	production	Ambiente
Persistência
Os dados são salvos em data/historico.json:

Últimas 100 atualizações de status

Últimos 200 eventos

Logs
powershell
docker logs rack-monitor-server --tail 50
docker logs rack-monitor-server -f  # seguir
Rebuild
powershell
docker compose down
docker compose up --build --force-recreate
'@ | Out-File -FilePath README.md -Encoding utf8

Write-Host "server/README.md criado!" -ForegroundColor Green

text

---

## 🚀 Passo 6 — Inicializar Git e fazer primeiro commit

```powershell
cd $HOME\Desktop\rack-monitor

# Inicializar repositório
git init

# Configurar usuário (se ainda não configurou)
git config user.name "Cristiano"
git config user.email "cristiano.1973.on@gmail.com"

# Adicionar tudo
git add .

# Ver o que será commitado
git status

# Fazer o primeiro commit
git commit -m "chore: estrutura inicial do projeto Rack Monitor v7.2"

Write-Host "Git inicializado!" -ForegroundColor Green
Se você tiver .gitignore configurado corretamente, node_modules/ e data/ não vão aparecer na lista de arquivos a commitar.

🚀 Passo 7 — Configurar repositório remoto (GitHub)
7.1 — Criar repositório no GitHub
Acesse https://github.com/new

Nome: rack-monitor

Descrição: Sistema profissional de monitoramento de rack com ESP32 + RFID + Cloudflare

Privado (recomendado) ou Público

NÃO inicializar com README, .gitignore ou license

Clique em Create repository

7.2 — Conectar o repositório local ao remoto
powershell
cd $HOME\Desktop\rack-monitor

# Substitua pela URL do seu repositório
git remote add origin https://github.com/SEU-USUARIO/rack-monitor.git

# Enviar o primeiro commit
git branch -M main
git push -u origin main
🚀 Passo 8 — Configurar .gitignore aprimorado
Crie também em server/:

powershell
cd $HOME\Desktop\rack-monitor\server

@'
node_modules/
npm-debug.log*
data/*.json
data/*.csv
.env
'@ | Out-File -FilePath .gitignore -Encoding utf8
📋 Estrutura Final
text
rack-monitor/
├── .gitignore
├── README.md
│
├── docs/
│   ├── ARQUITETURA.md
│   ├── INSTALACAO.md
│   ├── API.md
│   ├── FIRMWARE.md
│   ├── CHANGELOG.md
│   ├── TROUBLESHOOTING.md
│   └── logs/
│
├── firmware/
│   ├── sketch.ino
│   ├── diagram.json
│   ├── libraries.txt
│   └── README.md
│
└── server/
    ├── data/
    ├── public/
    │   └── index.html
    ├── .dockerignore
    ├── .gitignore
    ├── docker-compose.yml
    ├── Dockerfile
    ├── package.json
    ├── server.js
    └── README.md
✅ Checklist final
□ firmware/sketch.ino salvo
□ firmware/diagram.json salvo
□ firmware/libraries.txt salvo
□ firmware/README.md criado
□ .gitignore criado
□ README.md criado
□ docs/ARQUITETURA.md criado
□ docs/INSTALACAO.md criado
□ docs/API.md criado
□ docs/FIRMWARE.md criado
□ docs/CHANGELOG.md criado
□ docs/TROUBLESHOOTING.md criado
□ server/README.md criado
□ server/.gitignore criado
□ Git inicializado
□ Primeiro commit feito
□ Repositório remoto criado
□ Push feito
🎯 Comandos para verificar
powershell
cd $HOME\Desktop\rack-monitor

# Ver estrutura
tree /F

# Ver status do Git
git status

# Ver últimos commits
git log --oneline

# Ver repositório remoto
git remote -v
💡 Dicas profissionais
1. Commits semânticos
Use prefixos nos commits:

feat: nova funcionalidade

fix: correção de bug

docs: documentação

chore: tarefas administrativas

refactor: refatoração

test: testes

Exemplos:

bash
git commit -m "feat: adiciona integração com Cloudflare Tunnel"
git commit -m "fix: corrige debounce do botão"
git commit -m "docs: atualiza README com instruções"
2. Branches
Para novas funcionalidades:

bash
git checkout -b feature/nova-funcionalidade
# ... trabalhar
git checkout main
git merge feature/nova-funcionalidade
3. Tags
Para marcar versões:

bash
git tag -a v7.2 -m "Versão 7.2 - Cloudflare Tunnel"
git push --tags
📚 Recursos
Git Book

GitHub Docs

Keep a Changelog

Semantic Versioning

Rode os passos na ordem e me manda:

tree /F da raiz

git log --oneline após o commit

Confirmação de que o push foi feito

Bora documentar e salvar! 🚀

