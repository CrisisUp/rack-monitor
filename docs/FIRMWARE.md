# 🔧 Firmware

## Comandos do Serial Monitor

| Comando | Descrição |
| --- | --- |
| HELP | Lista comandos |
| STATUS | Status completo |
| ARMAR | Força modo ARMADO |
| DESARMAR | Força modo DESARMADO |
| TESTE | Modo teste |
| TESTE_HTTPS | Testa HTTPS |
| TESTE_SRV | Testa servidor |
| DIAG | Diagnóstico de rede |
| RESET | Zera contadores |

## Modo Vigilante

- Cartão armado → desarma (manutenção)
- Cartão desarmado → arma (se porta fechada)
- Rearme com porta aberta → BLOQUEADO
- Timeout de 30 min → rearme automático

## Cartões Autorizados

CARTAO_AUTORIZADO_1 = "01020304"
CARTAO_AUTORIZADO_2 = "55667788"
