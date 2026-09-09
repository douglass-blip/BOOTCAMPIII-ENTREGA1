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
| RNF01 | Geração de código curto deve ocorrer em tempo O(1) amortizado |
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
2. **UrlStore** — interface de persistência (CRUD de URL curta ↔ longa + metadados). Implementação inicial em memória, com interface que permite trocar por um banco depois.
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

*(Seção viva — registrar aqui qualquer ajuste feito na especificação após testes ou revisão do grupo, com data e motivo. Exemplo de formato abaixo.)*

| Data | Alteração | Motivo |
|------|-----------|--------|
| — | — | — |
