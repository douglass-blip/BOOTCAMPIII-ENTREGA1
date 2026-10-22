#include <iostream>
#include <string>
#include <optional>
#include "url_store.hpp"
#include "click_tracker.hpp"
#include "redirect_service.hpp" // Inclusão obrigatória para corrigir o erro de escopo

// Mock para a base de dados com os nomes dos parâmetros omitidos para evitar o erro [-Wunused-parameter]
class MockUrlStore : public UrlStore {
public:
    bool insert(const std::string&, const UrlRecord&) override { return true; }
    std::optional<UrlRecord> get(const std::string&) const override { return std::nullopt; }
    bool exists(const std::string&) const override { return false; }
};

int main() {
    std::cout << "Iniciando o Servidor do Encurtador de URLs...\n";

    // 1. Inicializar os componentes
    MockUrlStore store;
    ClickTracker tracker;
    
    // 2. Integrar com o serviço de redirecionamento
    RedirectService redirect_service(store, tracker);

    std::cout << "Servidor a correr e componentes integrados com sucesso!\n";
    
    return 0;
}