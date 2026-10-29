# Encurtador de URL com Analytics

Encurtador HTTP em C++17: cria códigos curtos, redireciona para a URL original e mantém métricas de acesso. A implementação segue a especificação em [`spec.md`](./spec.md).

## Funcionalidades

- `POST /shorten`: recebe `url` e `expires_at` opcional e retorna `201` com `short_code` e `short_url`.
- `GET /{short_code}`: responde `302` com `Location`; códigos inexistentes ou expirados respondem `404`.
- `GET /{short_code}/stats`: informa URL, contagem de cliques e timestamps.
- Códigos base62 de sete caracteres por padrão; inserções concorrentes não sobrescrevem códigos.
- Registros e cliques persistem em arquivos JSON Lines, recarregados na inicialização.

## Compilar e testar

Requisitos: CMake 3.16+, compilador C++17, Git e acesso à Internet na primeira configuração (CMake baixa GoogleTest, cpp-httplib e nlohmann/json via `FetchContent`).

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Em Linux/macOS, os mesmos comandos funcionam em um shell POSIX. O teste HTTP usa uma porta local dinâmica.

## Executar

```powershell
.\build\url_shortener.exe
```

Por padrão o servidor escuta em `0.0.0.0:8080` e grava `data/urls.jsonl` e `data/clicks.jsonl`. Configure antes de iniciar:

| Variável | Padrão | Descrição |
|---|---|---|
| `URL_SHORTENER_HOST` | `0.0.0.0` | Interface de rede do servidor |
| `URL_SHORTENER_PORT` | `8080` | Porta TCP (1–65535) |
| `URL_SHORTENER_PUBLIC_BASE_URL` | `http://localhost:8080` | Base incluída em `short_url` |
| `URL_SHORTENER_DATA_DIR` | `data` | Diretório dos arquivos persistentes |

Exemplo de requisições:

```sh
curl -i http://localhost:8080/shorten \
  -H 'Content-Type: application/json' \
  -d '{"url":"https://example.com/a-long-page","expires_at":"2030-12-31T23:59:59Z"}'
curl -i http://localhost:8080/Ab3dE91
curl -i http://localhost:8080/Ab3dE91/stats
```

`expires_at` aceita timestamps ISO 8601 com `Z` ou offset (`+/-HH:MM`) e até três casas decimais. Expira no instante indicado (inclusive). Os stats continuam disponíveis para registros expirados; cliques não são incrementados para redirects inválidos ou expirados.

## Docker Compose

```sh
docker compose up --build
docker compose --profile test run --rm tests
```

O serviço salva os dados no volume `url_data`, que sobrevive à recriação do container. `docker compose down -v` também remove esse volume e os dados.

## Arquitetura

```mermaid
flowchart LR
    Client[Cliente HTTP] --> API[API cpp-httplib]
    API --> Service[RedirectService]
    Service --> Generator[CodeGenerator]
    Service --> Store[PersistentUrlStore]
    Service --> Tracker[ClickTracker]
    Store --> URLs[(urls.jsonl)]
    Tracker --> Clicks[(clicks.jsonl)]
```

`InMemoryUrlStore` e `ClickTracker` sem caminho de persistência são usados em testes. O armazenamento de arquivos é thread-safe dentro de um processo; compartilhar os mesmos arquivos entre processos simultâneos não é suportado.

## As-built e cobertura

A aplicação, APIs, persistência em disco e serviços abaixo estão implementados. O resumo verificável dos testes e do estado do CI fica em [`EXECUTION_LOG.md`](./EXECUTION_LOG.md); ocorrências corrigidas estão em [`docs/ERROS.md`](./docs/ERROS.md). A execução local instrumentada desta revisão cobriu 507 de 590 linhas executáveis em `src/` (85%); a execução remota [40871757141](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/actions/runs/40871757141) cobriu 501 de 579 linhas (86%). O job de CI publica relatórios HTML/XML/texto como artefato `coverage-report` e falha se a cobertura de linhas desse escopo ficar abaixo de 70%.

Para gerar o relatório localmente com GCC ou Clang e `gcovr` instalado:

```sh
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug \
  -DURL_SHORTENER_ENABLE_COVERAGE=ON
cmake --build build-coverage --target coverage --parallel
```

Roteiro da demonstração manual de 3–5 minutos: [`docs/DEMO.md`](./docs/DEMO.md). O roteiro é uma lista de ações verificáveis, não um relato pessoal ou uma gravação já produzida.

## Decisões arquiteturais (ADRs)

### ADR-001: Hash truncado com resolução de colisão

O `CodeGenerator` usa `std::hash` com salt de tentativa, converte para base62 e consulta o `UrlStore` para resolver colisões. A busca é O(1) esperado em relação à quantidade de códigos armazenados, dado o `unordered_map`; calcular o hash ainda depende do tamanho da URL. A biblioteca não garante que `std::hash` gere o mesmo valor entre implementações ou execuções, então não há promessa de códigos estáveis entre versões/toolchains.

### ADR-002: Persistência append-only em JSON Lines

Cada novo registro e clique é acrescentado como uma linha JSON e recarregado no início. Isso mantém o formato simples e permite sobreviver a reinícios sem substituir silenciosamente códigos. Uma linha inválida interrompe a inicialização com erro explícito, em vez de descartar dados. É uma solução local de processo único, não um banco transacional ou armazenamento distribuído.

### ADR-003: HTTP/JSON com dependências header-only via FetchContent

`cpp-httplib` implementa o servidor HTTP e `nlohmann/json` trata os contratos JSON e a persistência. As dependências são fixadas em versões no CMake; não são copiadas para o repositório.

### ADR-004: Semântica de expiração

Um link expira quando `now >= expires_at`. Consultas de analytics ainda retornam os metadados do link expirado, mas o redirect responde `404` e não registra clique.

## Fluxo SDD e uso de IA

O autor confirmou o uso de Claude Code e GitHub Copilot. O registro descritivo e seus limites estão em [`docs/USO_IA.md`](./docs/USO_IA.md); não houve experimento controlado comparando ferramentas ou uso com/sem `AGENTS.md`. Não houve revisão por pares: o usuário informou que fará a entrega sozinho. Nenhuma aprovação ou opinião de colega é alegada. A análise ético-técnica, sem alegações pessoais não verificadas, está em [`docs/ETICA.md`](./docs/ETICA.md).

### Retrospectiva do autor

Relato fornecido pelo autor sobre o ciclo de desenvolvimento: a principal dificuldade foi manter compilação e testes consistentes entre ambientes com e sem Docker e entre CMD e PowerShell, além de uma credencial Git de outra pessoa salva no Windows. Entre os problemas encontrados, o autor cita a substituição do `code_generator.cpp` novo por uma cópia antiga (237 bytes em vez de 1537), a omissão de `url_store.cpp` no `CMakeLists.txt` e um push `403` causado por outra conta autenticada. Como respostas, relata ter passado a tratar o GitHub Actions como fonte de verdade para builds limpos no Ubuntu com ASan, UBSan e ThreadSanitizer, conferir tamanho e conteúdo dos arquivos antes de commits, trabalhar em clones limpos, configurar a identidade Git apenas no repositório e remover a credencial do Windows ao terminar. O aprendizado registrado pelo autor é exigir evidência objetiva (como tamanho do arquivo, log do CI ou captura de tela) e definir o ambiente em arquivos versionados, não na memória. Este parágrafo registra a retrospectiva informada pelo autor; não afirma que todas essas ações foram executadas nesta sessão.

## Pendências que exigem evidência ou ação do autor

- A comparação documentada em `docs/USO_IA.md` é descritiva; se for exigido um experimento controlado, ainda será necessário realizá-lo e registrar condições e resultados reais.
- O PDF de submissão depende do template, critérios e dados de identificação exigidos pelo curso.
- Declarações pessoais sobre dados compartilhados, revisão de sugestões e propriedade intelectual devem ser acrescentadas pelo autor apenas se exigidas e com base em sua experiência real.
