#include <iostream>
#include <string>
#include <optional>
#include "url_store.hpp"
#include "click_tracker.hpp"
#include "redirect_service.hpp"

// Criamos uma classe concreta (Mock) para implementar a interface abstrata UrlStore
class MockUrlStore : public UrlStore {
public:
    bool insert(const std::string& code, const UrlRecord& record) override { return true; }
    std::optional<UrlRecord> get(const std::string& code) const override { return std::nullopt; }
    bool exists(const std::string& code) const override { return false; }
};

int main() {
    std::cout << "Iniciando o Servidor do Encurtador de URLs...\n";

    // 1. Inicializar os componentes (usando o Mock em vez da interface)
    MockUrlStore store;
    ClickTracker tracker;
    
    // 2. Integrar com o serviço de redirecionamento
    RedirectService redirect_service(store, tracker);

    std::cout << "Servidor a correr e componentes integrados com sucesso!\n";
    
    return 0;
}