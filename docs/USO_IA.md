# Uso de assistentes de IA

## Fontes e alcance

O autor confirmou o uso de **Claude Code** e **GitHub Copilot** ao longo do projeto. Os registros históricos no README e em `AGENTS.md` associam Claude ao apoio à elaboração de `spec.md`, à decomposição de componentes e à implementação inicial. Nesta finalização, o histórico da sessão e os commits publicados registram o uso do GitHub Copilot no apoio à implementação, depuração, testes e documentação. Isso descreve evidências de fases específicas e não significa que uma ferramenta tenha sido usada exclusivamente em uma etapa.

Esta descrição combina a confirmação do autor com os registros encontrados no repositório e nesta finalização. A versão intermediária do relatório mencionava Cursor, mas essa referência não foi mantida: ela conflita com a confirmação posterior do autor de que as ferramentas usadas foram Claude Code e GitHub Copilot. Ela não pretende atribuir cada linha ou commit exclusivamente a uma ferramenta, nem é uma medição comparativa de desempenho.

## Comparação descritiva, não experimental

| Aspecto | Claude Code | GitHub Copilot |
|---|---|---|
| Uso registrado | Uso confirmado pelo autor ao longo do projeto; registros históricos associam a ferramenta à especificação, decomposição e implementação inicial. | Uso confirmado pelo autor ao longo do projeto; o histórico desta finalização registra apoio à implementação e validação técnica, com evidências versionadas no repositório. |
| Resultado que pode ser afirmado | Participou do fluxo inicial; não há registro comparável de tempo, taxa de defeitos ou cobertura atribuível à ferramenta. | Foi usado durante a finalização; os testes e o CI medem o estado do repositório, não a contribuição isolada da ferramenta. |
| Limitação da evidência | Não há protocolo controlado, prompts completos pareados ou resultados atribuídos por ferramenta. | Não há protocolo controlado, prompts completos pareados ou resultados atribuídos por ferramenta. |

O projeto não realizou um experimento controlado entre as ferramentas, nem uma execução pareada com e sem `AGENTS.md`. As etapas ocorreram em momentos e contextos diferentes, e o estado atual do código não permite separar causalmente a contribuição de cada assistente. Portanto, não se conclui que uma ferramenta seja mais rápida, correta ou eficiente que a outra. A comparação solicitada como experimento permanece pendente se a disciplina exigir medições controladas.

## Verificação do trabalho

As evidências de build, testes, cobertura e CI são registradas em [`EXECUTION_LOG.md`](../EXECUTION_LOG.md). Elas verificam resultados do projeto, mas não constituem prova de que cada sugestão de IA foi aceita sem revisão nem substituem a avaliação do autor.
