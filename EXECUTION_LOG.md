# Registro de execução

## Ambiente desta sessão

- Sistema: Windows
- Compilador: MinGW-w64 GCC 16.2.0
- CMake: 4.4.4
- Framework: GoogleTest v1.15.2, obtido via CMake FetchContent
- Dependências HTTP/JSON: cpp-httplib v0.18.3 e nlohmann/json v3.11.3
- Sanitizers: não usados nesta execução local

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

## Histórico remoto observado

A execução do CI 39786466411 no PR #26, em 22/10/2026, falhou durante a compilação com `RedirectService was not declared in this scope` em `src/main.cpp:24`. O job terminou antes de executar os testes. Não houve execução remota após as mudanças desta sessão, porque nenhuma alteração foi enviada ao GitHub.

O arquivo de log herdado registrava seis testes de `CodeGenerator`, execução randomizada e teste de estresse, mas esses resultados não foram reproduzidos nesta sessão e não são apresentados como validação atual.
