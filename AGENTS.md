# Contexto e Regras de IA — Fluxo SDD

Este documento separa o histórico descrito nos arquivos herdados das atividades realizadas nesta finalização. O conteúdo histórico abaixo é o registro preexistente do projeto; esta sessão não valida detalhes que não estejam nos arquivos e commits disponíveis.

## Registro histórico do repositório

O autor confirmou uso de Claude Code e GitHub Copilot. README e versões anteriores deste documento registram o apoio de Claude à elaboração inicial de `spec.md`, decomposição de componentes e implementação. Nesta finalização, GitHub Copilot apoiou implementação, depuração, testes e documentação técnica. Não foi feita comparação experimental controlada entre ferramentas nem execução pareada com/sem `AGENTS.md`; o registro descritivo está em `docs/USO_IA.md`.

## Papel do agente no fluxo

1. **Especificação e decomposição** — os documentos preexistentes descrevem RF/RNF, regras de negócio e componentes testáveis.
2. **Implementação e testes nesta sessão** — foram completados serviço de redirecionamento, API HTTP, persistência local e testes; a execução observada está registrada em `EXECUTION_LOG.md`.
3. **Trabalho individual** — o usuário informou que fará a entrega sozinho. Revisão por pares não se aplica; este documento não declara aprovação de colega.

## Regras e diretrizes usadas nas interações com o agente

- Toda mudança na especificação original deve ser registrada na seção "Refinamento por Feedback" do `spec.md`, com data e motivo.
- Código gerado deve respeitar a decomposição em componentes definida na especificação (seção 6 do `spec.md`) — um componente por arquivo/módulo, testável isoladamente.
- Decisões arquiteturais relevantes tomadas com apoio do agente são registradas como ADRs no `README.md`.
- O trabalho de finalização foi integrado em `develop` pelo PR #27 e promovido a `main` pelo PR #28, merge commit `d146676c8a6b7d8297b86e13b5e4f6bab1794f15`. Por autorização específica do autor, o merge do PR #28 foi executado com `--admin`; o GitHub não lista reviews e manteve `REVIEW_REQUIRED`. Não houve revisão por pares, e o bypass administrativo não deve ser descrito como aprovação humana ou review. A release `v1.0.0` foi publicada em https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/releases/tag/v1.0.0.

## Prompt-base utilizado para tarefas de codificação

> "Com base na especificação em `spec.md`, implemente o componente [NOME_DO_COMPONENTE] em C++, respeitando o contrato de entrada/saída definido e cobrindo os casos de borda listados na seção 7. Sinalize explicitamente qualquer ponto em que a especificação esteja ambígua antes de prosseguir."
