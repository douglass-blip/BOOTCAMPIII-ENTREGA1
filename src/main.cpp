#include <iostream>
#include <string>
#include <optional>
#include "url_store.hpp"
#include "click_tracker.hpp"
#include "redirect_service.hpp" // <- Esta é a linha que o compilador está a pedir

// Mock para a base de dados
class MockUrlStore : public UrlStore {
public:
    bool insert(const std::string&, const UrlRecord&) override { return true; }
    std::optional<UrlRecord> get(const std::string&) const override { return std::nullopt; }
    bool exists(const std::string&) const override { return false; }
};

int main() {
    std::cout << "Iniciando o Servidor do Encurtador de URLs...\n";

    // Inicializar os componentes
    MockUrlStore store;
    ClickTracker tracker;
    
    // Integrar com o serviço de redirecionamento (Linha 24)
    RedirectService redirect_service(store, tracker);

    std::cout << "Servidor a correr e componentes integrados com sucesso!\n";
    
    return 0;
}