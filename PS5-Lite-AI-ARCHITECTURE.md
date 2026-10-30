# PS5 Lite AI — protótipo inicial

## Arquivos
- `source/main.c`: interface de console e integração com o motor.
- `source/ps5lite_ai.h`: tipos e funções públicas.
- `source/ps5lite_ai.c`: média suavizada e detecção de ciclos lentos.
- `Makefile`: regras de compilação devkitPro/libnx.

## Teste
1. Compile em um ambiente com devkitPro e libnx.
2. Execute o NRO no Nintendo Switch desbloqueado.
3. Pressione A para simular uma pausa de 50 ms.
4. Pressione X para reiniciar as estatísticas; + para sair.

## Limites
Este protótipo usa heurísticas simples, não um modelo de machine learning treinado. Mede apenas o ciclo deste aplicativo. O modo LEVE é um diagnóstico e ainda não altera clocks, GPU, resolução ou configurações de jogos externos. Este projeto não executa jogos de PlayStation 5.
