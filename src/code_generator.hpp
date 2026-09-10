#pragma once

#include <functional>
#include <string>

// Callback que informa se um codigo curto ja existe no UrlStore.
// Isolado via std::function para que o CodeGenerator seja testavel
// sem depender de uma implementacao real de storage.
using ExistsFn = std::function<bool(const std::string&)>;

// Gera um codigo curto unico para a URL informada.
//
// Estrategia: hash truncado da URL + salt (numero da tentativa),
// convertido para base62. Em caso de colisao (codeExists retorna true),
// tenta novamente incrementando o salt, ate maxAttempts vezes.
//
// Lanca std::runtime_error se nao conseguir gerar um codigo unico
// dentro de maxAttempts tentativas (RNF03: 6-8 caracteres alfanumericos).
std::string generateShortCode(
    const std::string& url,
    const ExistsFn& codeExists,
    int length = 7,
    int maxAttempts = 10
);
