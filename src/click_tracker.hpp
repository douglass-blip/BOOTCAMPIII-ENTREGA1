#pragma once

#include <chrono>
#include <cstddef>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// Registra e consulta os acessos (cliques) de cada codigo curto.
// Thread-safe (RNF02): varias threads podem chamar record/count/history
// ao mesmo tempo sem perder cliques nem corromper o estado.
class ClickTracker {
public:
    using Clock = std::chrono::system_clock;

    // Registra um clique no codigo informado (RF04). O instante pode ser
    // passado explicitamente, o que torna os testes deterministicos.
    void record(const std::string& code, Clock::time_point when = Clock::now());

    // Total de cliques registrados para o codigo (RF05). Zero se nunca houve.
    std::size_t count(const std::string& code) const;

    // Instantes dos cliques, na ordem em que foram registrados.
    std::vector<Clock::time_point> history(const std::string& code) const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<Clock::time_point>> clicks_;
};
