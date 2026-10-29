# Correções necessárias no rascunho de entrega final

Revisão do PDF anexado “Douglas Ramos - Entrega Final.pdf”, confrontado com o código em `develop` e o GitHub em 29/10/2026. O arquivo anexado é um rascunho de referência; não foi alterado nem tratado como evidência de release.

| Afirmação no rascunho | Estado verificado | Correção recomendada |
|---|---|---|
| “Release v1.0.0” publicada e links para release/tag | Não há tag nem release no repositório. PR #28 de `develop` para `main` está aberto e bloqueado por `REVIEW_REQUIRED`. | Remover o selo/link de release. Informar que publicação aguarda aprovação humana e merge normal. |
| Implementação consolidada na branch `main` | `main` aponta para `8e1c0a261d4d3884696d51cc493c740b62b40922`, o commit inicial. A implementação e a documentação atualizada estão em `develop`, no commit atual `b8b136e870c6fa76ed1c699050542f1f9b2dd23c`. | Distinguir `develop` de `main`; não afirmar promoção concluída. |
| Quatro testes, 100% aprovados, log CTest antigo | O harness atual registra 37 testes CTest; 37/37 passaram localmente. `build-and-test` e `coverage` passaram no head atual do PR #28, run [40875606477](https://github.com/douglass-blip/BOOTCAMPIII-ENTREGA1/actions/runs/40875606477). | Substituir o log antigo pelo resultado atual e identificar o run/SHA. |
| Coverage/Tsan como validação dos checks atuais | O relatório atual de CI executa ASan e UBSan no job de build/teste. O workflow não configura ThreadSanitizer. O artefato de coverage verificado no head anterior, cujo delta até o atual é somente documental, marcou 501/579 linhas (86%); local: 507/590 (85%). | Não atribuir TSan ao workflow atual. Só relatar uma execução histórica se houver link/log verificável, com data e escopo claros. |
| `UrlStore` descrito como armazenamento somente em memória | A aplicação usa persistência JSON Lines append-only e reload; implementações em memória são usadas em testes. | Atualizar a descrição arquitetural para refletir persistência local e limites de processo único. |
| Revisão formal por pares concluída | O autor informou que fará a entrega sozinho e que não haverá revisão de outra pessoa. PR #28 requer aprovação humana para proteger `main`, e seguirá aberto sem merge. | Remover alegações de revisão por pares. Não solicitar revisão nem contornar a proteção de `main`. |
| “Projeto concluído de acordo com todas as rubricas” / “Entrega 3” | A rubrica oficial e o número da entrega não foram fornecidos; o PDF intermediário identificava a Entrega 2. | Confirmar com o curso o número/critério correto e evitar afirmar conformidade total sem checklist oficial. |

O relatório revisado em [`RELATORIO_FINAL.pdf`](RELATORIO_FINAL.pdf), com fonte HTML em [`RELATORIO_FINAL.html`](RELATORIO_FINAL.html), já registra os fatos disponíveis e inclui esta errata. O autor deve validar o número da entrega e quaisquer critérios específicos da disciplina antes de submeter.
