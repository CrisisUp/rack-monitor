# Firmware - ESP32

Código do sistema de monitoramento do rack.

## Arquivos

- `sketch.ino` - Código principal do ESP32
- `diagram.json` - Circuito para o Wokwi
- `libraries.txt` - Bibliotecas necessárias

## Como usar

### No Wokwi (online)

1. Acesse <https://wokwi.com/projects/new/esp32>
2. Cole o conteúdo de `sketch.ino` no editor
3. Cole o conteúdo de `diagram.json` na aba `diagram.json`
4. Cole o conteúdo de `libraries.txt` na aba `Library Manager`
5. Clique em Play

### No VS Code + Extensão Wokwi

1. Abra esta pasta no VS Code
2. Instale a extensão **Wokwi Simulator**
3. Abra `sketch.ino`
4. Pressione `F1` → `Wokwi: Start Simulator`

## Comandos do Serial Monitor

| Comando | Descrição |
| --- | --- |
| `HELP` | Lista todos os comandos |
| `STATUS` | Status completo do sistema |
| `ARMAR` | Força modo ARMADO (se porta fechada) |
| `DESARMAR` | Força modo DESARMADO |
| `TESTE` | Modo teste (LEDs/buzzers/sensores) |
| `TESTE_HTTPS` | Testa HTTPS para 3 sites |
| `TESTE_SRV` | Testa conexão com servidor remoto |
| `DIAG` | Diagnóstico de rede detalhado |
| `SET LIMIAR <5-400>` | Ajusta limiar ultrassônico |
| `SET TIMEOUT <0-1440>` | Timeout de desarme (min) |
| `RTC` | Mostra hora do RTC |
| `RESET` | Zera contadores |

## Versões

- v5.0 - Modo Vigilante + Rearme condicionado
- v6.0 - Web Server local
- v7.0 - Cliente HTTP + ngrok
- v7.2 - Cloudflare Tunnel + HTTPS

## Hardware necessário

- ESP32 DevKit C v4
- MFRC522 (leitor RFID)
- HC-SR04 (ultrassônico)
- PIR (sensor de movimento)
- LDR (fotoresistor)
- SSD1306 (display OLED)
- DS3231 (RTC)
- Botão + Switch
- 2 Buzzers
- LED RGB
