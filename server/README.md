# 🖥️ Servidor

Servidor Node.js que recebe dados do ESP32 e serve o dashboard.

## Stack

- **Node.js 20** (Alpine)
- **Express** — framework HTTP
- **CORS** — libera requisições do navegador

## Rodar

### Com Docker (recomendado)

    cd server
    docker compose up --build -d

Valide:

    docker compose ps
    curl -s http://localhost:3000/api/health

### Sem Docker (desenvolvimento)

    cd server
    npm install
    npm start

## Endpoints

Ver [`../docs/API.md`](../docs/API.md).

Resumo:

| Método | Rota | Descrição |
|--------|------|-----------|
| GET | `/api/health` | Health check |
| GET | `/api/status` | Estado atual + histórico |
| POST | `/api/status` | Enviado pelo ESP32 |
| GET | `/api/comando` | Consulta comando pendente |
| POST | `/api/comando` | Envia comando ao ESP32 |
| GET | `/api/stats` | Estatísticas gerais |

## Estrutura

    server/
    ├── server.js            # código principal
    ├── public/              # dashboard (arquivos estáticos)
    │   ├── index.html
    │   ├── style.css
    │   ├── app.js
    │   └── favicon.svg
    ├── data/                # persistência
    │   └── historico.json
    ├── Dockerfile
    ├── docker-compose.yml
    ├── package.json
    └── README.md

## Persistência

O servidor salva em `data/historico.json`:

- Últimas N atualizações de status
- Últimos M eventos semânticos

O arquivo é montado como volume (Docker) ou escrito direto no filesystem (dev).

**Reset de emergência:**

    rm data/historico.json
    docker compose restart

## Variáveis de ambiente

| Variável | Padrão | Descrição |
|----------|--------|-----------|
| `PORT` | `3000` | Porta HTTP |
| `NODE_ENV` | `production` | Ambiente |

## Logs

    docker compose logs -f          # seguir
    docker compose logs --tail 50   # últimas 50 linhas

## Editar o dashboard

O Docker copia `public/` na build. Edições no Mac **não refletem** sem rebuild.

### Solução rápida (após editar)

    docker compose cp public/index.html rack-server:/usr/src/app/public/index.html
    docker compose cp public/style.css  rack-server:/usr/src/app/public/style.css
    docker compose cp public/app.js     rack-server:/usr/src/app/public/app.js
    docker compose cp public/favicon.svg rack-server:/usr/src/app/public/favicon.svg

### Solução definitiva (hot-reload)

Crie `docker-compose.override.yml`:

    services:
      rack-server:
        volumes:
          - ./public:/usr/src/app/public:ro
          - ./server.js:/usr/src/app/server.js:ro

Depois:

    docker compose down
    docker compose up -d

Agora qualquer edição em `public/` reflete na hora — só dar **hard refresh** no navegador (`Cmd + Shift + R`).

## Rebuild completo

    docker compose down
    docker compose build --no-cache
    docker compose up -d

## Notas sobre Docker Compose

- **Nome do serviço:** `rack-server`
- **Nome do container:** `rack-monitor-server`

Ao usar `docker compose cp`, use o **nome do serviço** (`rack-server`), não do container.

Ao usar `docker logs`, use o **nome do container** (`rack-monitor-server`).

## Troubleshooting

Ver [`../docs/TROUBLESHOOTING.md`](../docs/TROUBLESHOOTING.md)
