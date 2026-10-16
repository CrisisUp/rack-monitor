# 🔌 Firmware

Código do ESP32 para o Rack Monitor. Versão **v7.5**.

## Arquivos

| Arquivo | Descrição |
|---------|-----------|
| `sketch.ino` | Código principal |
| `diagram.json` | Circuito no Wokwi |
| `libraries.txt` | Bibliotecas necessárias |

## Rodar no Wokwi

1. Abra [wokwi.com/projects/new/esp32](https://wokwi.com/projects/new/esp32)
2. Cole `sketch.ino` no editor de código
3. Cole `diagram.json` na aba **diagram.json**
4. Adicione as bibliotecas de `libraries.txt` no Library Manager
5. Ajuste `SERVIDOR_URL_PADRAO` (ou use `SET URL` em runtime)
6. Clique em ▶ **Play**

## Bibliotecas

Veja `libraries.txt`:

- Adafruit GFX Library
- Adafruit SSD1306
- RTClib
- MFRC522

`WiFi`, `WiFiClientSecure`, `HTTPClient`, `Preferences`, `Wire` e `SPI` já vêm no core do ESP32.

## Hardware

### Pinos

| Pino | Componente |
|------|------------|
| 5 | RFID SS |
| 4 | RFID RST |
| 18 / 19 / 23 | SPI (SCK / MISO / MOSI) |
| 27 | PIR |
| 17 | TRIG (ultrassônico) |
| 16 | ECHO (ultrassônico) |
| 34 | LDR |
| 14 | Botão (INPUT_PULLUP) |
| 25 | Switch da porta |
| 15 | Buzzer de eventos |
| 2 | Buzzer de emergência |
| 32 / 33 / 0 | LED RGB (R / G / B) |
| 21 / 22 | I2C (SDA / SCL) |

### Componentes

- ESP32 DevKit
- MFRC522 (RFID)
- Sensor PIR
- HC-SR04 (ultrassônico)
- LDR + resistor
- Botão push
- Switch fim de curso
- 2 buzzers passivos
- LED RGB (cátodo comum)
- OLED SSD1306 128×64 (I2C)
- RTC DS3231 (I2C, opcional — só em hardware real)

## Comandos do Serial

Digite `HELP` para listar. Os principais:

| Comando | Descrição |
|---------|-----------|
| `STATUS` | Status completo |
| `SET URL <https://...>` | Troca a URL do servidor |
| `GET URL` | Mostra a URL ativa |
| `TESTE_SRV` | Testa conexão |
| `RECONECTAR` | Força nova conexão TLS |
| `DIAG` | Diagnóstico de rede |
| `ARMAR` / `DESARMAR` | Controle do modo vigilante |
| `RESET` | Zera contadores |

Lista completa em [`../docs/FIRMWARE.md`](../docs/FIRMWARE.md).

## Cartões autorizados

Edite no `sketch.ino` as constantes:

- `CARTAO_AUTORIZADO_1` = `"01020304"` (Admin)
- `CARTAO_AUTORIZADO_2` = `"55667788"` (Operador)

## Persistência da URL (v7.5+)

A URL do servidor é salva em NVS e sobrevive a reboots. Trocar sem recompilar:

Digite no Serial Monitor:

    SET URL https://nova-url.trycloudflare.com

Voltar ao padrão:

    URL RESET

## Troubleshooting

Problemas comuns: [`../docs/TROUBLESHOOTING.md`](../docs/TROUBLESHOOTING.md)
