#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "url_store.hpp"

using Clock = std::chrono::system_clock;

TEST(UrlStore, InsereERecuperaRegistro) {
    InMemoryUrlStore store;
    UrlRecord record{"https://exemplo.com/pagina", Clock::now(), std::nullopt};

    ASSERT_TRUE(store.insert("abc1234", record));

    auto found = store.get("abc1234");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->url, "https://exemplo.com/pagina");
}

// Caso de borda: codigo inexistente
TEST(UrlStore, GetDeCodigoInexistenteRetornaNullopt) {
    InMemoryUrlStore store;

    auto found = store.get("naoexiste");

    EXPECT_FALSE(found.has_value());
}

TEST(UrlStore, ExistsReflectsCorretamente) {
    InMemoryUrlStore store;
    UrlRecord record{"https://exemplo.com", Clock::now(), std::nullopt};

    EXPECT_FALSE(store.exists("codigo1"));
    store.insert("codigo1", record);
    EXPECT_TRUE(store.exists("codigo1"));
}

// RN01: codigo, uma vez gerado, e imutavel -- insert duplicado deve falhar
// e NAO deve sobrescrever o registro original.
TEST(UrlStore, InsertDuplicadoFalhaENaoSobrescreve) {
    InMemoryUrlStore store;
    UrlRecord original{"https://exemplo.com/original", Clock::now(), std::nullopt};
    UrlRecord tentativaSobrescrita{"https://exemplo.com/outra", Clock::now(), std::nullopt};

    ASSERT_TRUE(store.insert("dup1234", original));
    bool segundoInsert = store.insert("dup1234", tentativaSobrescrita);

    EXPECT_FALSE(segundoInsert);
    auto found = store.get("dup1234");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->url, "https://exemplo.com/original");
}

// Caso de borda: expiresAt opcional deve ser preservado corretamente,
// tanto quando presente quanto quando ausente.
TEST(UrlStore, PreservaExpiresAtOpcional) {
    InMemoryUrlStore store;

    UrlRecord semExpiracao{"https://exemplo.com/sem-exp", Clock::now(), std::nullopt};
    store.insert("sem-exp", semExpiracao);
    EXPECT_FALSE(store.get("sem-exp")->expiresAt.has_value());

    auto expira = Clock::now() + std::chrono::hours(24);
    UrlRecord comExpiracao{"https://exemplo.com/com-exp", Clock::now(), expira};
    store.insert("com-exp", comExpiracao);
    ASSERT_TRUE(store.get("com-exp")->expiresAt.has_value());
    EXPECT_EQ(store.get("com-exp")->expiresAt.value(), expira);
}

// Integracao: UrlStore::exists usado como callback do CodeGenerator
// (garante que a interface e compativel com o uso real, nao so isolado).
#include "code_generator.hpp"

TEST(UrlStoreIntegration, ExistsFuncionaComoCallbackDoCodeGenerator) {
    InMemoryUrlStore store;
    auto existsFn = [&store](const std::string& c) { return store.exists(c); };

    std::string code = generateShortCode("https://exemplo.com/integracao", existsFn);
    EXPECT_FALSE(code.empty());

    UrlRecord record{"https://exemplo.com/integracao", Clock::now(), std::nullopt};
    ASSERT_TRUE(store.insert(code, record));

    // Gerar de novo pra mesma URL agora deve evitar o codigo ja inserido
    std::string code2 = generateShortCode("https://exemplo.com/integracao", existsFn);
    EXPECT_NE(code, code2);
}

TEST(UrlStore, ConcurrentDuplicateInsertHasSingleWinner) {
    InMemoryUrlStore store;
    const UrlRecord record{
        "https://example.com/concurrent",
        Clock::now(),
        std::nullopt
    };
    std::atomic<int> successful_inserts{0};
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&] {
            if (store.insert("same123", record)) {
                ++successful_inserts;
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successful_inserts.load(), 1);
    EXPECT_EQ(store.get("same123")->url, record.url);
}
