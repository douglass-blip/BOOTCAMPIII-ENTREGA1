# Análise ético-técnica

## Escopo e limites

Esta análise descreve riscos do sistema e do uso de assistentes de IA com base na implementação e nas evidências disponíveis no repositório. Não substitui uma declaração pessoal exigida pelo curso, uma avaliação jurídica, nem afirma práticas que não foram documentadas. O projeto foi realizado individualmente; não houve revisão por pares.

## Confiabilidade e alucinações

Assistentes de IA podem sugerir requisitos, código ou explicações incorretos, incompletos ou incompatíveis com o ambiente. Um código que compila também pode conter falhas semânticas ou casos de borda não cobertos.

Como controles técnicos observáveis neste projeto, a especificação e os contratos dos endpoints orientaram a implementação; há testes unitários e de API executados pelo CTest; o workflow de CI compila com AddressSanitizer e UndefinedBehaviorSanitizer e executa os testes; e há um job separado que produz relatório de cobertura e exige ao menos 70% de linhas em `src/`. A cobertura medida foi 85% localmente e 86% no artefato do GitHub Actions do PR #28. Cobertura e testes demonstram apenas os caminhos exercitados: não provam ausência de defeitos, correção de todos os requisitos ou revisão humana independente.

## Privacidade e dados

O serviço persiste URLs de destino e eventos de clique em arquivos JSON Lines locais. URLs podem conter identificadores, parâmetros ou outros dados sensíveis; timestamps de acesso também podem revelar padrões de uso. A exposição dos arquivos, backups ou diretório montado em container pode, portanto, afetar a privacidade.

Medidas recomendadas para uso e demonstração incluem utilizar URLs sintéticas, não incluir dados pessoais ou segredos em URLs, restringir permissões do diretório de dados, evitar publicar os arquivos persistidos e apagar dados de teste conforme a política aplicável. O material disponível não comprova quais dados foram ou não enviados a cada ferramenta de IA; por isso, esta análise não declara que nenhum dado sensível foi compartilhado. O autor deve preencher qualquer relato pessoal de tratamento de dados solicitado pela disciplina.

## Propriedade intelectual e dependências

O CMake obtém dependências externas por FetchContent e fixa versões de `nlohmann/json`, `cpp-httplib` e GoogleTest. Fixar versões melhora a reprodutibilidade, mas não equivale a uma auditoria de licenças nem comprova a origem ou a licença de cada trecho de código produzido durante o desenvolvimento.

Antes de redistribuir o projeto, devem ser verificadas as licenças e avisos das dependências nas versões efetivamente obtidas, bem como as regras do curso sobre código assistido por IA. Esta análise não faz uma conclusão jurídica nem atribui autoria individual a trechos sem evidência de proveniência.

## Responsabilidade e revisão humana

O autor informou que fará a entrega sozinho e que não haverá revisão de outra pessoa. Não houve aprovação de colega nem revisão por pares, e este documento não as sugere. A CI e os testes são controles automatizados, não substitutos para a responsabilidade do autor de entender o código, verificar os resultados e decidir se o comportamento atende à especificação.

O PR de promoção para `main` está sujeito a uma regra do GitHub que exige aprovação humana. Como o autor não fará revisão por outra pessoa, essa exigência permanece sem atendimento: o PR fica aberto e sem merge. Não foi usado bypass nem criada aprovação em nome de terceiro. A decisão de publicar uma release também permanece condicionada ao merge normal em `main`, portanto não será realizada neste estado.

## Declarações pessoais ainda não comprovadas

O repositório não determina quais conteúdos foram enviados a cada assistente, como o autor avaliou cada sugestão individualmente ou quais conclusões pessoais tirou sobre privacidade e propriedade intelectual. Se a entrega acadêmica exigir essas informações, o autor deve completá-las com base em sua experiência real, sem inferi-las desta análise técnica.
