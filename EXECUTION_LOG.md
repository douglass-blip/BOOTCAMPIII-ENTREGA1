# Registro de execução

## Ambiente desta sessão

- Sistema: Windows
- Compilador: MinGW-w64 GCC 16.2.0
- CMake: 4.4.4
- Framework: GoogleTest v1.15.2, obtido via CMake FetchContent
- Dependências HTTP/JSON: cpp-httplib v0.18.3 e nlohmann/json v3.11.3
- Sanitizers: não usados nesta execução local
- Coverage: GCC instrumentado com gcovr 8.4; 507 de 590 linhas executáveis em `src/` cobertas (85%). O alvo local e a CI geram relatórios HTML/XML/texto e exigem pelo menos 70%.

## Comandos e resultados

Configuração:

```powershell
cmake -S . -B build -G "MinGW Makefiles" `
  -DCMAKE_C_COMPILER="C:/msys64/mingw64/bin/gcc.exe" `
  -DCMAKE_CXX_COMPILER="C:/msys64/mingw64/bin/g++.exe" `
  -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/mingw32-make.exe"
```

`cmake --build build --parallel` concluiu com sucesso. `ctest --test-dir build --output-on-failure --output-log build/test-log.txt` executou 37 testes: **37 passaram, 0 falharam**.

Cobertura incluída:

- geração de códigos, tamanho/alfabeto e resolução de colisões;
- inserção, imutabilidade, persistência/reload e falha explícita para arquivo corrompido;
- registro concorrente e histórico de cliques;
- expiração anterior e no instante exato;
- criação HTTP, redirect `302`, estatísticas, validação de payload e timestamps, códigos inexistentes/ inválidos e URLs expiradas;
- criação concorrente sem reutilização de código e repetição da mesma URL.

Também foi iniciado o executável real e exercitado via HTTP: `POST /shorten` respondeu `201`, `GET /{code}` respondeu `302`, e `GET /{code}/stats` mostrou um clique. Após encerrar e reiniciar o processo com o mesmo diretório de dados, o registro e a contagem foram recuperados; novo redirect elevou o total para dois.

O executável de testes completo foi repetido cinco vezes com ordem aleatória (`--gtest_shuffle`); cada execução passou os 37 testes. `docker compose config --quiet` validou a configuração. O daemon do Docker não estava ativo, então imagem/containers não foram iniciados.

Nesta revisão, o alvo de coverage compilou a instrumentação, executou os 37 testes via CTest e produziu `coverage.txt`, `coverage.xml` e `coverage.html` em `build-coverage/`. O relatório gcovr 8.4 cobriu 507/590 linhas executáveis em `src/` (85%), acima do limite local/CI de 70%. A cobertura por arquivo foi: `api.cpp` 88%, `click_tracker.cpp` 80%, `code_generator.cpp` 95%, `redirect_service.cpp` 78%, `time_utils.cpp` 95% e `url_store.cpp` 86% (headers instrumentados: 100%); o escopo do relatório inclui também os headers.

Após o merge do PR #27 em `develop`, o build local foi executado novamente e o CTest passou 37/37 testes. `docker compose config --profile test --quiet` validou a configuração do serviço de testes. Não foi possível executar `docker compose up` nem o perfil de testes em container: o daemon Linux do Docker não estava disponível (`dockerDesktopLinuxEngine` ausente).

## Histórico remoto observado

A execução do CI 39786466411 no PR #26, em 22/10/2026, falhou durante a compilação com `RedirectService was not declared in this scope` em `src/main.cpp:24`. O job terminou antes de executar os testes.

O arquivo de log herdado registrava seis testes de `CodeGenerator`, execução randomizada e teste de estresse, mas esses resultados não foram reproduzidos nesta sessão e não são apresentados como validação atual.

## Publicação GitHub

- PR [#27](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/pull/27), base `develop`, branch `douglass-blip-finalizar-bootcampiii`.
- PR #27 foi mesclado em `develop` em 2026-10-29. Merge commit: `517f48c2985f53a674e866fca0967d164fa80a56`.
- A CI do PR #27 passou `build-and-test` e `coverage` na execução [40793757408](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/actions/runs/40793757408), no head `9cc9902d6dcfed8f88222d298e13c91bc2c1ab75`. A execução anterior [40793450790](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/actions/runs/40793450790), no head `3bc12949e88c896ba0ccf6a9ff0d0398ffdc2f14`, também passou e produziu o relatório remoto de cobertura de 501/579 linhas executáveis (86%) em `src/`.
- O PR de promoção [#28](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/pull/28), de `develop` para `main`, foi mesclado em 29/10/2026 às 13:30:38Z no commit `d146676c8a6b7d8297b86e13b5e4f6bab1794f15`. Segundo autorização específica informada pelo autor, foi usado `--admin`; a API do GitHub confirma que não há reviews (`reviews: []`) e ainda exibe `REVIEW_REQUIRED`. Não houve revisão por pares. A release [v1.0.0](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/releases/tag/v1.0.0) foi publicada às 13:30:57Z sobre esse commit. No head `edefafbbd7b1600c3f8a6561262f676807d3bd96`, os jobs `build-and-test` e `coverage` passaram nos runs [40882260007](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/actions/runs/40882260007) e [40882266435](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/actions/runs/40882266435); o artefato do run 40882260007 registrou 501/579 linhas (86%). Esta atualização do relatório será enviada em PR de correção separado; não se presume aprovação ou merge.

## Relatório final

O relatório [`docs/RELATORIO_FINAL.pdf`](docs/RELATORIO_FINAL.pdf) e sua fonte [`docs/RELATORIO_FINAL.html`](docs/RELATORIO_FINAL.html) foram montados usando a identificação e estrutura dos PDFs fornecidos pelo autor, além dos resultados verificáveis deste repositório. A comparação anterior com Cursor foi excluída por conflitar com a confirmação do autor; nenhuma comparação experimental ou revisão por pares foi inventada. A revisão inicial do novo rascunho identificou alegações ainda não verificadas naquele momento sobre release v1.0.0 e merge em `main`; posteriormente, PR #28 e a release v1.0.0 foram publicados. A errata distingue essas fotografias e corrige as afirmações que seguem sem evidência (quatro testes, TSan no workflow, revisão por pares e conformidade total). Ver [`docs/CORRECOES_RASCUNHO_FINAL.md`](docs/CORRECOES_RASCUNHO_FINAL.md). A adequação a uma rubrica/template oficial diferente ainda depende de confirmação do curso/autor.
