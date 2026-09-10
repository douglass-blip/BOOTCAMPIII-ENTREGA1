# Relatório de Execução — Test Harness

## Ambiente

- Compilador: GCC 13.3.0
- CMake 3.28
- Framework de testes: GoogleTest v1.15.2 (via CMake FetchContent)
- Flags de build: `-Wall -Wextra`, com sanitizers `-fsanitize=address,undefined` habilitados na verificação local

## Suíte de testes automatizada (`tests/test_code_generator.cpp`)

Cobre os casos de borda definidos na seção 7 do `spec.md` aplicáveis ao componente `CodeGenerator`:

| Teste | O que valida |
|---|---|
| `GeraCodigoComTamanhoEsperado` | Código gerado respeita RNF03 (6-8 caracteres) |
| `CodigoEhAlfanumerico` | Código contém apenas caracteres alfanuméricos |
| `MesmaUrlDuasVezesGeraCodigosDistintos` | RN03 — mesma URL gera códigos diferentes em chamadas distintas |
| `ResolveColisaoTentandoNovamente` | Colisão simulada — gerador tenta novamente até achar código livre |
| `LancaExcecaoQuandoNuncaConsegueGerarCodigoUnico` | Esgotamento de tentativas lança exceção em vez de loop infinito ou código duplicado |
| `UrlVaziaAindaGeraCodigoValido` | Caso de borda de entrada vazia não quebra o gerador |

## Log de execução

```
Running main() from googletest/src/gtest_main.cc
[==========] Running 6 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 6 tests from CodeGenerator
[ RUN      ] CodeGenerator.GeraCodigoComTamanhoEsperado
[       OK ] CodeGenerator.GeraCodigoComTamanhoEsperado (0 ms)
[ RUN      ] CodeGenerator.CodigoEhAlfanumerico
[       OK ] CodeGenerator.CodigoEhAlfanumerico (0 ms)
[ RUN      ] CodeGenerator.MesmaUrlDuasVezesGeraCodigosDistintos
[       OK ] CodeGenerator.MesmaUrlDuasVezesGeraCodigosDistintos (0 ms)
[ RUN      ] CodeGenerator.ResolveColisaoTentandoNovamente
[       OK ] CodeGenerator.ResolveColisaoTentandoNovamente (0 ms)
[ RUN      ] CodeGenerator.LancaExcecaoQuandoNuncaConsegueGerarCodigoUnico
[       OK ] CodeGenerator.LancaExcecaoQuandoNuncaConsegueGerarCodigoUnico (0 ms)
[ RUN      ] CodeGenerator.UrlVaziaAindaGeraCodigoValido
[       OK ] CodeGenerator.UrlVaziaAindaGeraCodigoValido (0 ms)
[----------] 6 tests from CodeGenerator (0 ms total)

[----------] Global test environment tear-down
[==========] 6 tests from 1 test suite ran. (0 ms total)
[  PASSED  ] 6 tests.
```

## Verificações adicionais de robustez

- Suíte executada 5 vezes com ordem de testes randomizada (`--gtest_shuffle`) — resultado estável em todas as execuções.
- Teste de estresse dedicado: geração de 100.000 códigos para 100.000 URLs distintas, checando unicidade em tempo real. Resultado: `OK: 100000 codigos gerados, todos unicos.`
- Todas as execuções acima rodaram com `-fsanitize=address,undefined`, sem nenhuma violação de memória ou comportamento indefinido detectada.

## Como reproduzir localmente

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/tests/run_tests
```
