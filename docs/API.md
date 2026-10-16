# 🔌 API

- Base URL local: `http://localhost:3000`
- Base URL pública: `https://SUA-URL.trycloudflare.com`
- Todos os endpoints retornam JSON e aceitam CORS (`Access-Control-Allow-Origin: *`).

---

## GET /api/health

### Health check

- Retorna `200` se o servidor está no ar.

#### Resposta

```json
{
  "status": "ok",
  "uptime": 3600.5
}
```

#### Exemplo

```bash
curl <http://localhost:3000/api/health>
```

## GET /api/status

Estado atual + histórico + eventos recentes.

#### Resposta

```json
{
  "timestamp": "16/10 11:36:16",
  "modo": "ARMADO",
  "estado": "APROXIMACAO_RACK",
  "porta": "fechada",
  "armado": true,
  "acessos": 2,
  "bloqueios": 0,
  "invasoes": 0,
  "uid": "01020304",
  "ip": "192.168.65.1",
  "ultimaAtualizacao": "2026-10-16T13:36:26.448Z",
  "historico": [
    {
      "ts": "16/10 11:36:16",
      "modo": "ARMADO",
      "estado": "APROXIMACAO_RACK",
      "porta": "fechada",
      "uid": "01020304"
    }
  ],
  "eventos": [
    {
      "ts": "16/10 11:36:16",
      "tipo": "MUDANCA_ESTADO",
      "de": "ARMADO/SEGURO",
      "para": "ARMADO/APROXIMACAO_RACK"
    },
    {
      "ts": "2026-10-16T13:35:12.964Z",
      "tipo": "COMANDO_WEB",
      "comando": "TESTE"
    }
  ]
}
```

## Campos

| Campo | Tipo | Descrição |
| ----- | ---- | --------- |
| `timestamp` | string | Timestamp do ESP32 (DD/MM HH:MM:SS) |
| `modo` | string | `ARMADO` ou `DESARMADO` |
| `estado` | string | Um dos 6 estados (ver FIRMWARE.md) |
| `porta` | string | `fechada` ou `ABERTA` |
| `armado` | bool | Atalho para `modo == "ARMADO"` |
| `acessos` | int | Total de acessos RFID válidos |
| `bloqueios` | int | Total de rearmes bloqueados |
| `invasoes` | int | Total de invasões detectadas |
| `uid` | string | Último UID lido ou `-` |
| `ip` | string | IP do ESP32 (na rede Docker) |
| `ultimaAtualizacao` | string | ISO 8601 UTC |
| `historico[]` | array | Últimas N atualizações de status |
| `eventos[]` | array | Últimos eventos semânticos |

#### Exemplo

```bash
curl -s http://localhost:3000/api/status | python3 -m json.tool
```

## POST /api/status

Enviado pelo ESP32. Não é para uso manual.

- Request

```json
{
  "timestamp": "16/10 11:36:16",
  "modo": "ARMADO",
  "estado": "SEGURO",
  "porta": "fechada",
  "armado": true,
  "acessos": 2,
  "bloqueios": 0,
  "invasoes": 0,
  "uid": "01020304"
}
```

#### Resposta

```json
{ "ok": true }
```

## GET /api/comando

Consulta o próximo comando pendente. O ESP32 chama a cada 15s.

#### Resposta com comando

```json
{ "comando": "ARMAR" }
```

#### Resposta sem comando

```json
{ "comando": null }
```

## POST /api/comando

Envia um comando para o ESP32. O comando fica na fila e é entregue no próximo poll do firmware.

### Request

```json
{ "comando": "ARMAR" }
```

## Comandos válidos

| Comando | Efeito |
| ------- | ------ |
| `ARMAR` | Força modo ARMADO (bloqueia se porta aberta) |
| `DESARMAR` | Força modo DESARMADO |
| `TESTE` | Modo teste (LEDs, buzzers, sensores) |
| `RESET` | Zera todos os contadores |

### Resposta do comando

```json
{ "ok": true }
```

#### Exemplo

```bash
curl -X POST <http://localhost:3000/api/comando> \
  -H "Content-Type: application/json" \
  -d '{"comando":"DESARMAR"}'
```

## GET /api/stats

Estatísticas gerais do servidor.

#### Resposta

```json
{
  "totalStatus": 100,
  "totalEventos": 25,
  "uptime": 3600.5,
  "ultimaAtualizacao": "2026-10-16T13:36:26.448Z"
}
```

## Códigos HTTP

| Código | Significado |
| ------ | ----------- |
| `200` | Sucesso |
| `400` | Requisição inválida (ex: comando ausente) |
| `404` | Endpoint não encontrado |
| `500` | Erro interno do servidor |

## Tipos de evento

O array eventos[] contém objetos com tipo:

| Tipo | Descrição |
| ---- | --------- |
| `MUDANCA_ESTADO` | Transição de estado (com `de` e `para`) |
| `COMANDO_WEB` | Comando enviado pelo dashboard |
| `COMANDO_REMOTO` | Comando enviado por outro cliente |
| `INFO` | Evento informativo |

## Exemplos em diferentes linguagens

### cURL

```bash
curl http://localhost:3000/api/health
curl http://localhost:3000/api/status
curl -X POST http://localhost:3000/api/comando \
  -H "Content-Type: application/json" \
  -d '{"comando":"ARMAR"}'
```

### Python

```python
import requests
r = requests.get('http://localhost:3000/api/status')
print(r.json()['estado'])
requests.post('http://localhost:3000/api/comando',
              json={'comando': 'DESARMAR'})
```

### JavaScript (navegador)

```javascript
const status = await fetch('/api/status').then(r => r.json());
console.log(status.estado);
await fetch('/api/comando', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({ comando: 'ARMAR' })
});
```
