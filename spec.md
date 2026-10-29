# Especificação Técnica — Encurtador de URL com Analytics

## 1. Visão Geral do Problema

Sistema que recebe uma URL longa e gera um código curto único que redireciona para ela. Além do redirecionamento, o sistema registra e expõe métricas de acesso (analytics) por código encurtado.

O problema é intencionalmente pequeno, mas permite decomposição real em componentes independentes e testáveis, exigidos pelo fluxo SDD.

## 2. Requisitos Funcionais (RF)

| ID | Descrição |
|----|-----------|
| RF01 | O sistema deve receber uma URL longa e retornar um código curto único associado a ela |
| RF02 | O sistema deve redirecionar um código curto para a URL longa original |
| RF03 | O sistema deve permitir definir uma data de expiração opcional para cada URL encurtada |
| RF04 | O sistema deve registrar cada acesso a um código curto (timestamp) |
| RF05 | O sistema deve expor a contagem total de cliques de um código curto |
| RF06 | O sistema deve retornar erro apropriado ao tentar acessar um código inexistente ou expirado |
| RF07 | O sistema não deve gerar códigos duplicados para URLs diferentes |

## 3. Requisitos Não-Funcionais (RNF)

| ID | Descrição |
|----|-----------|
| RNF01 | Geração/reserva de código deve ocorrer em tempo O(1) amortizado em relação à quantidade de códigos armazenados |
| RNF02 | Sistema deve suportar operações concorrentes sem corromper contadores de clique |
| RNF03 | Código curto deve ter entre 6 e 8 caracteres alfanuméricos |
| RNF04 | Persistência deve sobreviver a reinício do processo (não pode ser apenas em memória volátil sem opção de dump/reload) |

## 4. Regras de Negócio

- RN01: Um código curto, uma vez gerado, é imutável — não pode apontar para outra URL depois de criado.
- RN02: URLs expiradas retornam erro de "não encontrado" (não distinguir de código inexistente, por segurança).
- RN03: A mesma URL longa enviada duas vezes gera dois códigos curtos distintos (sem deduplicação de entrada — simplifica o design).
- RN04: Um clique só é contado em caso de redirecionamento bem-sucedido (não conta em URL expirada/inexistente).

## 5. Contratos de Entrada/Saída

### `POST /shorten`
**Entrada:**
```json
{ "url": "https://exemplo.com/pagina-longa", "expires_at": "2026-12-31T23:59:59Z" }
```
`expires_at` é opcional.

**Saída (201):**
```json
{ "short_code": "aZ3kT9", "short_url": "https://short.ly/aZ3kT9" }
```

**Erros:** 400 se `url` ausente ou malformada.

### `GET /{short_code}`
**Saída (302):** redireciona para a URL original via header `Location`.

**Erros:** 404 se código inexistente ou expirado.

### `GET /{short_code}/stats`
**Saída (200):**
```json
{ "short_code": "aZ3kT9", "url": "https://exemplo.com/pagina-longa", "clicks": 42, "created_at": "...", "expires_at": "..." }
```

**Erros:** 404 se código inexistente.

## 6. Decomposição em Unidades (Componentes)

1. **CodeGenerator** — gera código curto único (ex: base62 sobre contador ou hash truncado + checagem de colisão). Testável isoladamente sem storage real.
2. **UrlStore** — interface de persistência (CRUD de URL curta ↔ longa + metadados). Há uma implementação em memória para testes e uma implementação JSON Lines para sobreviver a reinícios.
3. **ClickTracker** — responsável por registrar e contar cliques de forma thread-safe (contador atômico ou lock).
4. **RedirectService** — orquestra: valida existência/expiração do código, aciona ClickTracker, retorna URL de destino.
5. **API Layer** — camada HTTP fina que expõe os 3 endpoints, delega tudo para os serviços acima.

Cada componente deve ser testável isoladamente (sem subir a API inteira), o que é o requisito central do harness de testes.

## 7. Casos de Borda a Cobrir nos Testes

- URL malformada ou vazia no `/shorten`
- Código curto inexistente no `/{short_code}` e no `/stats`
- Código expirado (antes e exatamente no instante de expiração)
- Requisições concorrentes de clique no mesmo código (corrida de contador)
- Geração de código com colisão simulada (forçar colisão e checar que o gerador resolve)
- Código curto com caracteres inválidos/formato errado na URL de requisição

## 8. Refinamento por Feedback

Esta tabela registra clarificações adotadas na implementação local; não representa revisão ou aprovação de colegas.

| Data | Alteração | Motivo |
|------|-----------|--------|
| 2026-10-28 | Expiração é inclusiva: um link não redireciona quando `now >= expires_at`; a consulta de stats continua disponível para links expirados. | Definir resultado determinístico para o caso no instante exato e separar métricas da disponibilidade do redirect. |
| 2026-10-28 | `expires_at` aceita ISO 8601 com `Z` ou offset `+/-HH:MM`, opcionalmente com até milissegundos; timestamps inválidos recebem HTTP 400. | Validar o contrato temporal sem depender do timezone da máquina. |
| 2026-10-28 | URLs de entrada devem ser absolutas e usar `http` ou `https`, com host válido; formato vazio/malformado recebe HTTP 400. | Restringir destinos aos esquemas web cobertos pelo redirect. |
| 2026-10-28 | Registros e eventos de clique são persistidos em arquivos JSON Lines, recarregados na inicialização; linhas corrompidas interrompem o startup com erro. | Atender RNF04 sem perder contagem de analytics; evitar recuperação silenciosa e perda de dados. |
| 2026-10-28 | RNF01 mede a busca/reserva em relação ao número de registros; o cálculo do hash continua proporcional ao tamanho da URL de entrada. | Tornar explícito o custo inevitável de ler a entrada ao hasheá-la. |
| 2026-10-28 | Persistência em arquivo suporta um processo por diretório de dados; não oferece coordenação entre processos. | Delimitar a implementação local e evitar prometer concorrência distribuída não testada. |
