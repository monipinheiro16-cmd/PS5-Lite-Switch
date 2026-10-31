# PS5 Lite Switch — arquitetura

Aplicativo homebrew para Nintendo Switch (devkitPro/libnx). **Nao executa jogos de PS5**:
e um navegador de arquivos com diagnostico simples e um esqueleto de nucleo de emulacao.

## Arquivos (pasta `source/`)
- `main.cpp`: unico ponto de entrada. Interface de console, navegador de arquivos, importacao, execucao de NRO e integracao com os demais modulos.
- `ps5lite_ai.h/.c`: media movel e deteccao de ciclos lentos (heuristica, nao e ML).
- `ps5lite_optimizer.h/.c`: perfis informativos (Equilibrado, Desempenho, Economia).
- `pkg_loader.h/.c`: identifica `.pkg` pela extensao e le tamanho/nome.
- `pkg_reader.hpp/.cpp`: le os 16 primeiros bytes e mostra o "magic".
- `emulator_core.hpp`: nucleo experimental (ainda sem carregador PS5).
- `adaptive.h/.c` e `resolution_manager.h/.c`: modulos auxiliares (ainda nao ligados ao `main.cpp`).
- `Makefile`: compila todos os `.c` e `.cpp` de `source/`.

## Controles
- Cima/Baixo: selecionar | A: abrir pasta ou verificar arquivo | B: voltar pasta
- X: importar para `imports/` | Y: executar NRO | L/R: trocar perfil
- Menos: nucleo experimental | Mais: sair

## Limites
Os perfis nao alteram hardware, clocks, GPU ou ventoinha. O tempo de ciclo medido e o do proprio aplicativo.
