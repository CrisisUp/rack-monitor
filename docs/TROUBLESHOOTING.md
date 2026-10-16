# 🔧 Troubleshooting

Problemas comuns e soluções, específicos para macOS + Docker + Cloudflare + Wokwi.

---

## Docker

### `Cannot connect to the Docker daemon`

**Causa:** Docker Desktop não está rodando.

**Solução:**

1. Abra o Docker Desktop
2. Aguarde a baleia 🐳 ficar verde no menu bar
3. Tente de novo: `docker compose up -d`

### `port 3000 already in use`

**Causa:** outro processo está escutando na porta 3000.

**Verificar:**

```bash
lsof -iTCP:3000 -sTCP:LISTEN
```

Solução A — matar o processo:

```bash
kill <PID>
```

Solução B — trocar a porta do container:

Edite server/docker-compose.yml:

```yaml
ports:
  - "3001:3000"   # 3001 no host, 3000 no container
```

E ajuste a URL do cloudflared:

```bash
cloudflared tunnel --url http://localhost:3001
```

Container reiniciando em loop

```bash
cd server
docker compose logs --tail 50
```

Procure por stack traces. Causas comuns: porta ocupada, erro de sintaxe em server.js, permissão em data/.

Container com status unhealthy
O healthcheck no Dockerfile está apontando para um path inexistente ou a porta mudou. Verifique:

```bash
docker inspect --format '{{json .State.Health}}' rack-monitor-server | python3 -m json.tool
```

---

## Cloudflare Tunnel

### `command not found: cloudflared`

**Causa:** não instalado.

**Solução:**

```bash
brew install cloudflared
```

Confirme:

```bash
cloudflared --version
```

URL mudou / parou de funcionar
Causa: quick tunnels geram URL nova a cada restart.

Diagnóstico:

```bash
curl -v https://SUA-URL.trycloudflare.com/api/health
```

Se o curl falhar:

Confirme que o cloudflared está rodando em outro terminal

Reinicie o túnel:

```bash
cloudflared tunnel --url http://localhost:3000
```

Copie a nova URL

Atualize no firmware:

Sem recompilar: Serial Monitor → SET URL <https://nova-url.trycloudflare.com>

Recompilando: edite SERVIDOR_URL_PADRAO no sketch

Erro 1033 no navegador

```text
Error 1033 · Cloudflare Tunnel error
Causa: a URL na barra do navegador não corresponde ao túnel ativo. Provavelmente você está usando uma URL antiga.
```

Solução: use a URL que o cloudflared está imprimindo agora.

Erro 530 no ESP32

```text
[WARN] [SRV] Falha apos 3 tentativas. HTTP 530
Causa: o cloudflared do seu Mac caiu ou foi fechado. O Cloudflare responde, mas não acha o backend.
````

Solução:

Confirme que o terminal do cloudflared está aberto

Se não estiver, reinicie e pegue a URL nova

SET URL <https://nova-url.trycloudflare.com> no Serial

TESTE_SRV para validar

ERR Failed to dial edge no cloudflared
Causa: problema de rede transitório.

Solução: aguarde 30s; o cloudflared reconecta sozinho. Se persistir, reinicie.

Quick tunnel matado após ~4h
Causa: política do Cloudflare para tunnels sem conta.

Soluções:

Curto prazo: reinicie o cloudflared e atualize a URL via SET URL

Definitivo: crie um túnel nomeado com domínio próprio:

```bash
cloudflared tunnel login
cloudflared tunnel create rack
cloudflared tunnel route dns rack rack.seudominio.com
cloudflared tunnel run rack
```

URL fixa, sem expiração.

---

## Wokwi

### `HTTP -1 | connection refused` frequente

**Causas possíveis:**

1. URL errada ou túnel morto
2. Rate limit do Wokwi em HTTPS
3. Socket TLS zumbi

**Diagnóstico (no Serial Monitor):**
DIAG
TESTE_HTTPS
TESTE_SRV

text

**Interpretação:**

- `TESTE_HTTPS` retorna `-1` nos 3 sites → Wokwi bloqueia HTTPS temporariamente (aguarde)
- `TESTE_SRV` retorna `200` mas POSTs falham → problema com URL ou rate limit
- Tudo `-1` → túnel caiu, reinicie `cloudflared`

**Mitigação (já no firmware v7.5):**

- Keep-alive reduz handshakes
- Retry com backoff
- `RECONECTAR` força nova sessão TLS

### WiFi não conecta

**Sintoma:**
[WIFI] Falha. Ativando Access Point...

text

**Causa:** `Wokwi-GUEST` indisponível.

**Solução:** aguarde e reinicie o Wokwi. O fallback AP permite testar offline.

### Firmware não compila

**Causa:** biblioteca faltando.

**Solução:** verifique `firmware/libraries.txt` e adicione no Library Manager do Wokwi:

- Adafruit GFX Library
- Adafruit SSD1306
- RTClib
- MFRC522

### RFID não lê cartão

**Sintomas:**
[WARN] [RFID] Acesso NEGADO! Cartao nao cadastrado: ABCD1234

text

**Solução:**

1. Aproxime o cartão do leitor
2. Veja o UID no serial
3. Adicione o UID em `CARTAO_AUTORIZADO_1` ou `_2`

### RTC não funciona

**Causa:** DS3231 ausente no Wokwi (esperado).

**Solução:** o firmware usa `millis()` como fallback. Para hora real, use ESP32 físico com DS3231.

---

## Frontend / Dashboard

### Dashboard mostra dados antigos

**Causa:** cache do navegador.
**Solução:** hard refresh (`Cmd + Shift + R`) ou abra em aba anônima.

### Dashboard mostra "Offline"

**Causa A:** o ESP32 parou de enviar status. Confira no serial do Wokwi.
**Causa B:** erro de JS no navegador. Abra o Console (F12) e procure por exceções.

### Botão do dashboard não faz nada

**Causa:** o ESP32 não está chamando `/api/comando` (polling).

**Diagnóstico:**

```bash
cd server
docker compose logs -f
```

# Procure por "GET /api/comando 200"

Se nada aparecer em 15s → ESP32 offline.

---

## Servidor Node.js

### `Error 500` no dashboard

```bash
cd server
docker compose logs --tail 50
```

Causas comuns: JSON corrompido em data/historico.json, erro de sintaxe em server.js.

Reset de emergência:

```bash
rm server/data/historico.json
docker compose restart
```

data/historico.json crescendo demais
O servidor mantém as últimas N entradas automaticamente. Se não estiver truncando, verifique server.js.

Reset manual:

```bash
echo '{"historico":[],"eventos":[]}' > server/data/historico.json
docker compose restart
```

Frontend editado não reflete
Causa: Docker copia public/ na build; edições no Mac não chegam ao container sem rebuild.

Solução rápida:

```bash
cd server
docker compose cp public/index.html rack-server:/usr/src/app/public/index.html
docker compose cp public/style.css  rack-server:/usr/src/app/public/style.css
docker compose cp public/app.js     rack-server:/usr/src/app/public/app.js
```

Solução definitiva: volume no docker-compose.override.yml:

```yaml
services:
  rack-server:
    volumes:
      - ./public:/usr/src/app/public:ro
```

Depois docker compose up -d.

## Rede

### `connection refused` no curl local
Causa: o container não está rodando ou está em outra porta.

Verificar:

```bash
docker compose ps
curl -i http://localhost:3000/api/health
```

### Latência alta no túnel
Causa: roteamento geográfico do Cloudflare.

Solução: force a região de São Paulo:

```bash
cloudflared tunnel --url <http://localhost:3000> --region gru
gru = GRU (São Paulo).
```

---

## Diagnóstico rápido (receita de bolo)

Quando **algo não funciona**, siga essa ordem:

```bash
# 1. Backend local está no ar?
curl -s http://localhost:3000/api/health
```

# 2. Túnel está no ar?

curl -s <https://SUA-URL.trycloudflare.com/api/health>

# 3. cloudflared está rodando?

### (veja o outro terminal; deve mostrar logs contínuos)

# 4. Docker está rodando?

cd server && docker compose ps

# 5. ESP32 está enviando?

### (veja o Serial Monitor do Wokwi)

# 6. Dashboard está atualizando?

### (abra F12 → Network → /api/status a cada 2s)

Em 90% dos casos, o problema é:
Túnel caiu → reinicie cloudflared + SET URL no serial
URL antiga no navegador → use a URL atual
Container parado → docker compose up -d
