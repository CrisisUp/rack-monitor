# 🔧 Troubleshooting

## Docker

### Cannot connect to the Docker daemon

Abra o Docker Desktop e aguarde a baleia ficar verde.

### Container unhealthy

Normal — o healthcheck usa wget, que não existe na imagem Alpine.

### Port 3000 already in use

Edite docker-compose.yml e mude para 3001:3000.

## Cloudflare Tunnel

### URL não funciona

Reinicie o cloudflared. Nova URL será gerada.

### Túnel caiu

Rode novamente o comando do cloudflared.

## Wokwi

### HTTP -1 | connection refused

Rode TESTE_HTTPS no Serial para diagnosticar.

### Rate limit HTTPS

Aumente intervalos no sketch para 20000ms e 15000ms.
