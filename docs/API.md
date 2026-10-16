# 🔌 API

Base: <http://localhost:3000>

## Endpoints

### GET /api/health

Retorna status do servidor.

### GET /api/status

Estado atual + histórico.

### POST /api/status

Enviado pelo ESP32.

### GET /api/comando

Consulta comandos para o ESP32.

### POST /api/comando

Envia comando para o ESP32.

Comandos válidos: ARMAR, DESARMAR, TESTE, RESET

### GET /api/stats

Estatísticas gerais.
