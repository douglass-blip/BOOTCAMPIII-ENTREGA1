#include "code_generator.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace {

constexpr std::array<char, 62> kAlphabet = {
    '0','1','2','3','4','5','6','7','8','9',
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
};

// Converte um valor de hash (64 bits) em uma string base62 de tamanho fixo.
std::string toBase62(uint64_t value, int length) {
    std::string result(length, '0');
    for (int i = length - 1; i >= 0; --i) {
        result[i] = kAlphabet[value % kAlphabet.size()];
        value /= kAlphabet.size();
    }
    return result;
}

} // namespace

std::string generateShortCode(
    const std::string& url,
    const ExistsFn& codeExists,
    int length,
    int maxAttempts
) {
    std::hash<std::string> hasher;

    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        // O salt (numero da tentativa) garante um hash diferente
        // a cada nova tentativa apos uma colisao.
        std::string salted = url + "#" + std::to_string(attempt);
        uint64_t hashValue = hasher(salted);

        std::string code = toBase62(hashValue, length);

        if (!codeExists(code)) {
            return code;
        }
    }

    throw std::runtime_error(
        "generateShortCode: nao foi possivel gerar um codigo unico apos "
        + std::to_string(maxAttempts) + " tentativas"
    );
}
