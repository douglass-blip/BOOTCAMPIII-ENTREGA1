# Contexto e Regras do Agente de IA — Fluxo SDD

Este documento registra como a ferramenta de IA (Claude) foi orquestrada no desenvolvimento deste projeto, conforme exigido pelo item "Configuração de Agentes de IA" da Entrega 1.

## Ferramenta utilizada

Claude (chat), usado em fluxo de Spec-Driven Development (SDD): a especificação técnica (`spec.md`) foi produzida e refinada antes da implementação, e cada componente é decomposto em unidades testáveis antes de virar código.

## Papel do agente no fluxo

1. **Elaboração da especificação técnica** — geração do documento `spec.md` (requisitos funcionais/não-funcionais, regras de negócio, contratos de entrada/saída, decomposição em componentes) a partir da descrição do problema feita pela equipe.
2. **Decomposição em Issues/tarefas** — apoio na quebra do problema em unidades independentes (CodeGenerator, UrlStore, ClickTracker, RedirectService, API Layer), cada uma virando uma Issue rastreável no GitHub.
3. **Apoio à implementação guiada por commits vinculados a Issues** — cada trecho de código gerado ou revisado pelo agente é commitado em uma branch `feature/*` correspondente a uma Issue específica, com mensagens de commit que referenciam o número da Issue (`(#N)`) e Pull Requests que a fecham (`Closes #N`).
4. **Geração de arquivos de padronização de ambiente** — `Dockerfile` e `CMakeLists.txt` foram elaborados com apoio do agente, buscando garantir build reprodutível.

## Regras e diretrizes usadas nas interações com o agente

- Toda mudança na especificação original deve ser registrada na seção "Refinamento por Feedback" do `spec.md`, com data e motivo.
- Código gerado deve respeitar a decomposição em componentes definida na especificação (seção 6 do `spec.md`) — um componente por arquivo/módulo, testável isoladamente.
- Decisões arquiteturais relevantes tomadas com apoio do agente são registradas como ADRs no `README.md`.
- Todo código segue o fluxo de governança do repositório: branch `feature/*` → commit → Pull Request → revisão → merge em `develop`.

## Prompt-base utilizado para tarefas de codificação

> "Com base na especificação em `spec.md`, implemente o componente [NOME_DO_COMPONENTE] em C++, respeitando o contrato de entrada/saída definido e cobrindo os casos de borda listados na seção 7. Sinalize explicitamente qualquer ponto em que a especificação esteja ambígua antes de prosseguir."
