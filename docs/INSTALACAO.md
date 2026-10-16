# 🚀 Instalação

## Pré-requisitos

- Docker Desktop
- Node.js 20+
- cloudflared
- Conta no Wokwi

## Passo a passo

### 1. Servidor

 cd server
docker compose up --build

### 2. Túnel Cloudflare

Em outro terminal:

`& "$HOME\cloudflared.exe" tunnel --url http://localhost:3000`

Copie a URL gerada.

### 3. Firmware

Atualize firmware/sketch.ino:

const char* SERVIDOR_URL = "<https://SUA-URL.trycloudflare.com>";

### 4. Wokwi

Cole sketch.ino, diagram.json e libraries.txt.

### 5. Dashboard

[http://localhost:3000](http://localhost:3000)
