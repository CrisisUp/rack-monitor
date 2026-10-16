# 🔧 Firmware

Versão atual: **v7.5**

## Visão geral

O firmware do ESP32:

- Lê sensores (RFID, PIR, LDR, ultrassônico)
- Controla atuadores (OLED, LED RGB, buzzers)
- Gerencia o **Modo Vigilante**
- Comunica com o servidor via HTTPS (keep-alive + retry)
- Persiste a URL do servidor em NVS
- Responde a comandos do dashboard

---

## Sensores e atuadores

| Pino | Componente | Função |
| ---- | ---------- | ------ |
| 5 | RFID SS | Chip select do MFRC522 |
| 4 | RFID RST | Reset do MFRC522 |
| 18/19/23 | SPI | SCK/MISO/MOSI |
| 27 | PIR | Detecção de movimento |
| 17 | TRIG | Pulso do ultrassônico |
| 16 | ECHO | Retorno do ultrassônico |
| 34 | LDR | Leitura analógica de luz |
| 14 | Botão | Botão multifunção |
| 25 | Switch | Sensor de porta |
| 15 | Buzzer E | Buzzer de eventos |
| 2 | Buzzer D | Buzzer de emergência |
| 32/33/0 | LED RGB | R/G/B PWM |
| 21/22 | I2C | SDA/SCL (OLED + RTC) |

---

## Comandos do Serial Monitor

Digite `HELP` no monitor serial para listar todos.

### Sistema

| Comando | Descrição |
| ------- | --------- |
| `HELP` | Lista todos os comandos |
| `STATUS` | Status completo (modo, estado, heap, TLS, contadores) |
| `RESET` | Zera todos os contadores |

### Servidor / URL

| Comando | Descrição |
| ------- | --------- |
| `SET URL <https://...>` | Troca a URL e salva em NVS |
| `GET URL` | Mostra a URL ativa |
| `URL RESET` | Volta à URL padrão |
| `TESTE_SRV` | Testa conexão com `/api/health` |
| `RECONECTAR` | Força nova conexão TLS |
| `DIAG` | Diagnóstico completo de rede |
| `TESTE_HTTPS` | Testa HTTPS para 3 sites públicos |

---

### Modo Vigilante

| Comando | Descrição |
| ------- | --------- |
| `ARMAR` | Força ARMADO (bloqueia se porta aberta) |
| `DESARMAR` | Força DESARMADO |
| `SET TIMEOUT <0-1440>` | Timeout de rearme automático (min) |

### Sensores

| Comando | Descrição |
| ------- | --------- |
| `SET LIMIAR <5-400>` | Limiar do ultrassônico (cm) |
| `TESTE` | Modo teste (LEDs, buzzers, leituras) |

### RTC

| Comando | Descrição |
| ------- | --------- |
| `RTC` | Mostra hora atual |
| `RTC YYYY-MM-DD HH:MM:SS` | Ajusta o RTC |

---

## Cartões RFID autorizados

```cpp
const String CARTAO_AUTORIZADO_1 = "01020304";   // Admin
const String CARTAO_AUTORIZADO_2 = "55667788";   // Operador
```

Para adicionar novos, edite estas constantes e recompile.

---

## Operação do Modo Vigilante

### Comportamento

| Ação | Resultado |
| ---- | --------- |
| Passar cartão (ARMADO) | Desarma → modo manutenção |
| Passar cartão (DESARMADO) | Arma (se porta fechada) |
| Rearme com porta aberta | **Bloqueado** |
| Timeout expirado (porta fechada) | Rearme automático |
| Clique longo do botão | Força rearme |

### Política de segurança

- Desarme é sempre permitido (para manutenção)
- Rearme exige porta fechada
- Timeout rearma automaticamente após 30 min (configurável)
- Aviso 5 min antes do rearme automático

### Máquina de estados

Os 6 estados possíveis:

| Estado | Descrição |
| ------ | --------- |
| `SEGURO` | Nenhuma anomalia |
| `AVISO_SALA` | Movimento sem luz acesa |
| `APROXIMACAO_RACK` | Algo perto do rack (ultrassônico) |
| `ACESSO_RFID` | Cartão passado recentemente |
| `INVASAO_SALA` | Movimento **com** luz acesa |
| `EMERGENCIA_RACK` | Porta aberta em modo ARMADO |

Estados críticos (`INVASAO_SALA`, `EMERGENCIA_RACK`) mudam **imediatamente**. Os demais têm debounce de 300ms.

---

## Filtros anti-ruído

### LDR

- **Histerese**: acende em 2200, apaga em 1800
- **Debounce**: 800ms

### Ultrassônico

- **Média móvel**: 5 leituras
- Ignora leituras > 400 cm

### Botão

- **Debounce**: 250ms
- **Clique curto**: < 400ms → zera contadores
- **Clique duplo**: 2 cliques em 400ms → modo teste
- **Clique longo**: > 3000ms → força rearme

### Porta (switch)

- **Debounce**: 200ms

### Estado (máquina)

- **Debounce de transição**: 300ms
- Estados críticos: mudança imediata

---

## Display OLED

### Tela normal (ARMADO)

```text
    ┌────────────────────────┐
    │ 🔒 VIGILANTE ARMADO    │
    │ ─────────────────────  │
    │ Porta Fechada          │
    │ ─────────────────────  │
    │ 16/10 11:36   AB: 3    │
    └────────────────────────┘

### Tela manutenção (DESARMADO)

    ┌────────────────────────┐
    │ 🛡️ MANUTENÇÃO          │
    │ ─────────────────────  │
    │ Rearme em: 29m45s      │
    │ UID: 01020304          │
    │ ─────────────────────  │
    │ 16/10 11:36   MANUT: 2 │
    └────────────────────────┘

### Tela de bloqueio

    ┌────────────────────────┐
    │ ⚠️ REARME BLOQUEADO    │
    │ ─────────────────────  │
    │ Feche a porta!         │
    │ Bloqueios: 1           │
    │ ─────────────────────  │
    │ UID: 01020304          │
    └────────────────────────┘
```

---

## LED RGB

| Estado | Cor |
| ------ | --- |
| SEGURO (armado) | 🟢 Verde fixo |
| DESARMADO | 🔵 Azul respirando |
| AVISO_SALA | 🟡 Amarelo |
| APROXIMACAO_RACK | 🟠 Laranja |
| INVASAO_SALA | 🔴 Vermelho piscando |
| EMERGENCIA_RACK | 🔴 Vermelho fixo |
| Bloqueio | 🟡 Amarelo piscando |

---

## Buzzers

| Estado | Som |
| ------ | --- |
| SEGURO | Silêncio |
| AVISO_SALA | Bip 523Hz a cada 700ms |
| APROXIMACAO_RACK | Bip 750Hz a cada 400ms |
| INVASAO_SALA | Sirene oscilante 400–900Hz |
| EMERGENCIA_RACK | Sirene alternada 900/1300Hz |
| Desarme | Tom descendente |
| Rearme | Tom ascendente |
| Bloqueio | Tom grave 500→400Hz |

---

## Persistência em NVS

Desde a **v7.5**, a URL do servidor é persistida em NVS (Non-Volatile Storage) com namespace `rack` e chave `srv_url`.

**Fluxo:**

1. No boot, o firmware lê a NVS
2. Se vazia, usa `SERVIDOR_URL_PADRAO` (constante do sketch) e salva
3. Comando `SET URL <nova>` regrava a NVS em runtime
4. `URL RESET` apaga a NVS e volta ao padrão

**Benefício:** trocar de URL sem recompilar nem re-flashar.

---

## Rede

### Keep-alive

Um único `WiFiClientSecure` global é reaproveitado entre requisições, com `HTTPClient::setReuse(true)`. Isso reduz handshakes TLS de ~3/min para ~0.3/min.

### Retry com backoff

Cada POST é tentado até 3 vezes, com backoff de 500ms × tentativa. Após 5 falhas consecutivas, o circuit breaker pausa envios por 60s.

### Headers

```text
Content-Type: application/json
User-Agent: ESP32-Rack-Monitor/7.5
Connection: keep-alive
```

---

## Bibliotecas

Veja `firmware/libraries.txt`:

- `WiFi` (built-in ESP32)
- `WiFiClientSecure` (built-in ESP32)
- `HTTPClient` (built-in ESP32)
- `Preferences` (built-in ESP32)
- `Wire` (built-in)
- `SPI` (built-in)
- `MFRC522`
- `Adafruit GFX`
- `Adafruit SSD1306`
- `RTClib`

---

## Troubleshooting rápido

| Sintoma | Verificar |
| ------- | --------- |
| `HTTP -1` frequente | `DIAG`, `TESTE_HTTPS`, ver docs/TROUBLESHOOTING.md |
| `HTTP 530` | `cloudflared` caiu → reiniciar túnel |
| `HTTP 1033` no navegador | URL antiga → atualizar `SERVIDOR_URL` ou `SET URL` |
| WiFi não conecta | Wokwi indisponível; fallback AP ativa |
| RFID não lê | Aproximar cartão; ver UID no serial |

Mais detalhes em [`TROUBLESHOOTING.md`](TROUBLESHOOTING.md).
