# Roteiro de demonstração

Este roteiro foi preparado para uma apresentação manual de 3–5 minutos. Não é uma gravação nem um relato de experiência pessoal. Use o build local, mostre resultados reais e grave a sua própria execução se a entrega do curso exigir vídeo.

## Antes de gravar

1. Compile o projeto e execute `ctest --test-dir build --output-on-failure`.
2. Inicie o servidor e configure `URL_SHORTENER_DATA_DIR` para uma pasta temporária dedicada à demonstração.
3. Tenha `curl` disponível e verifique qual URL/porta pública local o servidor está usando.
4. Não mostre tokens, credenciais, dados pessoais ou conteúdo de diretórios fora do projeto.

## Sequência sugerida

1. **Contexto e arquitetura (30–45 s):** apresente a especificação e a separação API → `RedirectService` → gerador, storage e tracker. Use o diagrama Mermaid do README.
2. **Criar link (45–60 s):** faça `POST /shorten` com uma URL HTTP/HTTPS válida e mostre `201`, `short_code` e `short_url`. Repita a mesma URL e observe que o segundo código é diferente.
3. **Redirect e analytics (45–60 s):** solicite `GET /{short_code}` sem seguir redirects automaticamente; mostre `302` e `Location`. Consulte `GET /{short_code}/stats` antes e depois do redirect para mostrar a contagem real.
4. **Expiração e validação (45–60 s):** crie um link já expirado e mostre que o redirect retorna `404` sem incrementar cliques; mostre uma entrada JSON inválida retornando `400`.
5. **Persistência e testes (45–60 s):** encerre e reinicie o processo com o mesmo diretório de dados; consulte as stats e confirme, com a resposta observada, que URL e cliques foram recuperados. Mostre o resumo atual do CTest.

## Registro para preencher após a gravação

- Data e ambiente usados:
- Resultado observado em cada etapa:
- Problema inesperado e como foi resolvido (se houver):
- Localização/link da gravação final:

Preencha somente com observações da sua própria execução. Não transforme este roteiro em afirmação de que uma gravação ou demonstração já ocorreu.
