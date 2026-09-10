#include <gtest/gtest.h>

#include <set>
#include <string>

#include "code_generator.hpp"

// Um "UrlStore" falso em memoria, so pra controlar exatamente
// quais codigos "ja existem" durante os testes (permite forcar colisao).
class FakeStore {
public:
    bool exists(const std::string& code) const {
        return codes_.count(code) > 0;
    }
    void insert(const std::string& code) {
        codes_.insert(code);
    }

private:
    std::set<std::string> codes_;
};

// Caso principal: gera um codigo com o tamanho esperado (RNF03: 6-8 chars)
TEST(CodeGenerator, GeraCodigoComTamanhoEsperado) {
    FakeStore store;
    auto exists = [&](const std::string& c) { return store.exists(c); };

    std::string code = generateShortCode("https://exemplo.com/pagina", exists, 7);

    EXPECT_EQ(code.size(), 7u);
}

// Caso principal: o codigo so contem caracteres alfanumericos
TEST(CodeGenerator, CodigoEhAlfanumerico) {
    FakeStore store;
    auto exists = [&](const std::string& c) { return store.exists(c); };

    std::string code = generateShortCode("https://exemplo.com/outra-pagina", exists);

    for (char c : code) {
        EXPECT_TRUE(std::isalnum(static_cast<unsigned char>(c)));
    }
}

// RN03: a mesma URL enviada duas vezes gera codigos distintos
// (aqui simulado registrando o primeiro codigo no store antes de gerar o segundo)
TEST(CodeGenerator, MesmaUrlDuasVezesGeraCodigosDistintos) {
    FakeStore store;
    auto exists = [&](const std::string& c) { return store.exists(c); };

    std::string url = "https://exemplo.com/pagina-repetida";
    std::string code1 = generateShortCode(url, exists);
    store.insert(code1);
    std::string code2 = generateShortCode(url, exists);

    EXPECT_NE(code1, code2);
}

// Caso de borda: colisao simulada -- forca o "exists" a dizer que
// os primeiros codigos gerados ja existem, e confirma que o gerador
// tenta novamente ate achar um livre.
TEST(CodeGenerator, ResolveColisaoTentandoNovamente) {
    int callCount = 0;
    auto exists = [&](const std::string&) {
        ++callCount;
        // As primeiras 3 tentativas "colidem"; a 4a jah eh aceita.
        return callCount <= 3;
    };

    std::string code = generateShortCode("https://exemplo.com/colisao", exists);

    EXPECT_FALSE(code.empty());
    EXPECT_GT(callCount, 3);
}

// Caso de borda: colisao sempre presente -- deve lancar excecao
// apos esgotar maxAttempts, em vez de entrar em loop infinito ou
// devolver um codigo duplicado.
TEST(CodeGenerator, LancaExcecaoQuandoNuncaConsegueGerarCodigoUnico) {
    auto sempreColide = [](const std::string&) { return true; };

    EXPECT_THROW(
        generateShortCode("https://exemplo.com/sempre-colide", sempreColide, 7, 5),
        std::runtime_error
    );
}

// Caso de borda: URL vazia -- o CodeGenerator ainda deve gerar um codigo
// valido (validacao de URL vazia/malformada eh responsabilidade da API
// Layer, nao do CodeGenerator -- ver secao 6 do spec.md).
TEST(CodeGenerator, UrlVaziaAindaGeraCodigoValido) {
    FakeStore store;
    auto exists = [&](const std::string& c) { return store.exists(c); };

    std::string code = generateShortCode("", exists);

    EXPECT_EQ(code.size(), 7u);
}
