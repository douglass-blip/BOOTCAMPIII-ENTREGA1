# Contexto e Regras de IA — Fluxo SDD

Este documento separa o histórico descrito nos arquivos herdados das atividades realizadas nesta finalização. O conteúdo histórico abaixo é o registro preexistente do projeto; esta sessão não valida detalhes que não estejam nos arquivos e commits disponíveis.

## Registro histórico do repositório

README e versões anteriores deste documento registram uso de Claude (chat) na elaboração inicial de `spec.md`, decomposição de componentes e apoio a implementação. Nesta finalização, GitHub Copilot apoiou a implementação e validação técnica local. Não foi feita comparação experimental entre ferramentas.

## Papel do agente no fluxo

1. **Especificação e decomposição** — os documentos preexistentes descrevem RF/RNF, regras de negócio e componentes testáveis.
2. **Implementação e testes nesta sessão** — foram completados serviço de redirecionamento, API HTTP, persistência local e testes; a execução observada está registrada em `EXECUTION_LOG.md`.
3. **Trabalho individual** — o usuário informou que fará a entrega sozinho. Revisão por pares não se aplica; este documento não declara aprovação de colega.

## Regras e diretrizes usadas nas interações com o agente

- Toda mudança na especificação original deve ser registrada na seção "Refinamento por Feedback" do `spec.md`, com data e motivo.
- Código gerado deve respeitar a decomposição em componentes definida na especificação (seção 6 do `spec.md`) — um componente por arquivo/módulo, testável isoladamente.
- Decisões arquiteturais relevantes tomadas com apoio do agente são registradas como ADRs no `README.md`.
- Esta finalização foi mantida localmente, sem commit, merge, release ou mudanças remotas no GitHub.

## Prompt-base utilizado para tarefas de codificação

> "Com base na especificação em `spec.md`, implemente o componente [NOME_DO_COMPONENTE] em C++, respeitando o contrato de entrada/saída definido e cobrindo os casos de borda listados na seção 7. Sinalize explicitamente qualquer ponto em que a especificação esteja ambígua antes de prosseguir."
