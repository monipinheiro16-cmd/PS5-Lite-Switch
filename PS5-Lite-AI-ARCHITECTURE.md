# PS5 Lite AI — protótipo inicial

## Arquivos
- `source/main.c`: interface de console, gerenciamento de arquivos e integração com os módulos de IA e otimização.
- `source/ps5lite_ai.h`: tipos e funções públicas da IA do aplicativo.
- `source/ps5lite_ai.c`: média suavizada e detecção de ciclos lentos.
- `source/ps5lite_optimizer.h`: tipos e API do perfil de otimização.
- `source/ps5lite_optimizer.c`: descrições e rotação dos perfis de operação.
- `Makefile`: regras de compilação para o devkitPro/libnx.

## Teste
1. Compile em um ambiente com devkitPro e libnx.
2. Execute o NRO no Nintendo Switch desbloqueado.
3. Pressione A para verificar a compatibilidade do arquivo selecionado.
4. Pressione X para importar um arquivo para a pasta `imports`.
5. Pressione L/R para alternar o perfil de otimização.
6. Pressione Y para executar um NRO reconhecido.
7. Pressione + para sair.

## Limites
Este protótipo usa heurísticas simples, não um modelo de machine learning treinado. Mede apenas o ciclo deste aplicativo e, por design, os perfis de otimização são informativos e não alteram hardware, clocks, GPU ou ventoinha. A aplicação serve como base para diagnóstico e demonstração de comportamento.

## Estado do projeto
O repositório foi ajustado para manter a estrutura de compilação consistente, incorporar os módulos de IA e otimização à interface principal e reduzir erros de configuração e integração.
