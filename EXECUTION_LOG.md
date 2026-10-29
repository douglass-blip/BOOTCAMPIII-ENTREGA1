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

## Histórico remoto observado

A execução do CI 39786466411 no PR #26, em 22/10/2026, falhou durante a compilação com `RedirectService was not declared in this scope` em `src/main.cpp:24`. O job terminou antes de executar os testes.

O arquivo de log herdado registrava seis testes de `CodeGenerator`, execução randomizada e teste de estresse, mas esses resultados não foram reproduzidos nesta sessão e não são apresentados como validação atual.

## Publicação GitHub

- PR [#27](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/pull/27), base `develop`, branch `douglass-blip-finalizar-bootcampiii`.
- Commit `dc7c5c13d2d6a78024f815f301f2fc122cae293a`.
- CI original do PR #27: [execução 40791512208](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/actions/runs/40791512208), `build-and-test` passou.
- CI após a etapa de cobertura: execução [40793450790](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/actions/runs/40793450790), no commit `3bc12949e88c896ba0ccf6a9ff0d0398ffdc2f14`; `build-and-test` e `coverage` passaram. O artefato remoto reportou 501/579 linhas executáveis (86%) em `src/`; os relatórios estão anexados à execução. Esta validação antecede apenas a atualização documental que registra o resultado.
