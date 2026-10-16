# 📝 Changelog

Formato baseado em [Keep a Changelog](https://keepachangelog.com/).

Versionamento segue [Semantic Versioning](https://semver.org/).

---

## [7.5] — 2026-10-16

### Adicionado — 7.5

- **URL do servidor persistida em NVS** (Preferences)
- Comando `SET URL <https://...>` — troca em runtime, sem recompilar
- Comando `GET URL` — mostra a URL ativa
- Comando `URL RESET` — volta à URL padrão
- `STATUS` estendido com URL ativa, cliente TLS e falhas seguidas

### Mudado — 7.5

- `SERVIDOR_URL` (const) → `servidorUrl` (variável) + `SERVIDOR_URL_PADRAO`
- Todas as funções HTTP usam a URL dinâmica

---

## [7.4] — 2026-10-16

### Adicionado — 7.4

- **HTTPS keep-alive** com `WiFiClientSecure` global + `HTTPClient::setReuse(true)`
- **Retry com backoff** exponencial (3 tentativas, base 500ms)
- **Circuit breaker** — pausa 60s após 5 falhas consecutivas
- Header `Connection: keep-alive`
- Comando `RECONECTAR`

### Corrigido — 7.4

- Reduziu drasticamente falhas `HTTP -1` e `HTTP -5` intermitentes

---

## [7.3] — 2026-10-16

### Adicionado — 7.3

- `WiFiClientSecure` com `setInsecure()` em todas as chamadas HTTPS
- `setConnectTimeout()` configurado antes do `begin()`
- Log de `errorToString(code)` em todas as falhas

### Corrigido — 7.3

- **Bug raiz:** `HTTP -1` permanente por falta de cliente TLS
- Timeouts curtos demais para Cloudflare

---

## [7.2] — 2026-10-16

### Adicionado — 7.2

- Integração com **Cloudflare Tunnel** (substituiu ngrok)
- Comando `TESTE_HTTPS`
- Comando `DIAG`
- Indicador visual de servidor online no OLED

### Removido — 7.2

- Suporte a ngrok (bloqueio por cartão)
- Suporte a LocalTunnel (incompatível com Docker)

---

## [7.1] — 2026-10-16

### Adicionado — 7.1

- Headers `ngrok-skip-browser-warning` e `User-Agent`

### Corrigido — 7.1

- Status `unhealthy` no Docker

---

## [7.0] — 2026-10-16

### Adicionado — 7.0

- **Cliente HTTP** para comunicação com servidor remoto
- Envio periódico de status
- Busca periódica de comandos

### Mudado — 7.0

- Arquitetura: ESP32 deixa de ser servidor, vira cliente

---

## [6.0] — 2026-10-16

### Adicionado — 6.0

- **Web Server** local no ESP32
- Dashboard embutido + API REST
- Fallback AP

---

## [5.0] — 2026-10-16

### Adicionado — 5.0

- **Rearme condicionado** (bloqueia se porta aberta)
- Debounce confirmatório do botão
- Debounce do switch da porta
- Contador de bloqueios

### Mudado — 5.0

- Timeout não rearma mais se porta aberta

---

## [4.9] — 2026-10-15

### Adicionado — 4.9

- Debounce do botão (200ms)
- Janela de clique duplo reduzida
- Contador de bounces ignorados

---

## [4.8] — 2026-10-15

### Adicionado — 4.8

- **RTC DS3231** com fallback para millis
- Filtros anti-ruído (LDR + ultrassônico)
- Debounce de estado (300ms)
- Comandos `RTC`

### Corrigido — 4.8

- Ruído no LDR (histerese)
- Falso positivo do ultrassônico (média móvel)

---

## [4.7] — 2026-10-15

### Adicionado — 4.7

- **Modo Vigilante** com arme/desarme por RFID
- Auditoria com UID + timestamp
- Timeout programável
- Botão multifunção

### Mudado — 4.7

- Botão não silencia mais — vira reset/teste

---

## [4.6] — 2026-10-15

### Adicionado — 4.6

- Sistema base de monitoramento
- Máquina de estados
- OLED, LED RGB, buzzers
- Leitura RFID básica
