#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "click_tracker.hpp"

using Clock = ClickTracker::Clock;

// Caso de borda: codigo que nunca recebeu clique
TEST(ClickTracker, CodigoSemCliquesTemContagemZero) {
    ClickTracker tracker;

    EXPECT_EQ(tracker.count("nunca-clicado"), 0u);
}

// RF04 + RF05: cada clique registrado aumenta a contagem em 1
TEST(ClickTracker, RecordIncrementaContagem) {
    ClickTracker tracker;

    tracker.record("abc1234");
    tracker.record("abc1234");
    tracker.record("abc1234");

    EXPECT_EQ(tracker.count("abc1234"), 3u);
}

// Codigos diferentes tem contadores independentes
TEST(ClickTracker, CodigosTemContadoresIndependentes) {
    ClickTracker tracker;

    tracker.record("aaa1111");
    tracker.record("aaa1111");
    tracker.record("bbb2222");

    EXPECT_EQ(tracker.count("aaa1111"), 2u);
    EXPECT_EQ(tracker.count("bbb2222"), 1u);
    EXPECT_EQ(tracker.count("ccc3333"), 0u);
}

// RF04: o historico guarda os instantes, na ordem de registro
TEST(ClickTracker, HistoricoPreservaInstantesEOrdem) {
    ClickTracker tracker;
    auto t0 = Clock::now();
    auto t1 = t0 + std::chrono::seconds(5);
    auto t2 = t0 + std::chrono::seconds(9);

    tracker.record("abc1234", t0);
    tracker.record("abc1234", t1);
    tracker.record("abc1234", t2);

    auto h = tracker.history("abc1234");
    ASSERT_EQ(h.size(), 3u);
    EXPECT_EQ(h[0], t0);
    EXPECT_EQ(h[1], t1);
    EXPECT_EQ(h[2], t2);
}

// Caso de borda: historico de codigo inexistente e vazio (nao lanca excecao)
TEST(ClickTracker, HistoricoDeCodigoInexistenteEhVazio) {
    ClickTracker tracker;

    EXPECT_TRUE(tracker.history("fantasma").empty());
}

// RNF02: varias threads registrando no MESMO codigo nao perdem cliques
TEST(ClickTracker, ConcorrenciaNoMesmoCodigoNaoPerdeCliques) {
    ClickTracker tracker;
    constexpr int kThreads = 8;
    constexpr int kClicksPerThread = 2000;

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&tracker] {
            for (int j = 0; j < kClicksPerThread; ++j) {
                tracker.record("concorrente");
            }
        });
    }
    for (auto& t : threads) t.join();

    EXPECT_EQ(tracker.count("concorrente"),
              static_cast<std::size_t>(kThreads) * kClicksPerThread);
}

// RNF02: leitura e escrita simultaneas (o ThreadSanitizer acusa corrida de dados)
TEST(ClickTracker, LeituraEEscritaSimultaneasSaoSeguras) {
    ClickTracker tracker;
    std::atomic<bool> parar{false};

    std::thread leitor([&] {
        while (!parar.load()) {
            (void)tracker.count("misto");
            (void)tracker.history("misto");
        }
    });

    std::vector<std::thread> escritores;
    for (int i = 0; i < 4; ++i) {
        escritores.emplace_back([&tracker] {
            for (int j = 0; j < 1000; ++j) tracker.record("misto");
        });
    }
    for (auto& t : escritores) t.join();
    parar.store(true);
    leitor.join();

    EXPECT_EQ(tracker.count("misto"), 4000u);
}
