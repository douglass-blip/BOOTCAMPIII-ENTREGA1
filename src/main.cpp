
#include <iostream>
#include <string>
#include "url_store.hpp"
#include "click_tracker.hpp"
#include "redirect_service.hpp"

// Simulação da API Layer (Ponto de entrada do sistema)
int main() {
    std::cout << "Iniciando o Servidor do Encurtador de URLs...\n";

    // 1. Inicializar os componentes
    UrlStore store;
    ClickTracker tracker;
    RedirectService redirect_service(store, tracker);

    std::cout << "Servidor a correr e componentes integrados com sucesso!\n";
    std::cout << "A escutar pedidos (API Layer pronta) ...\n";

    // A integração está feita para efeitos da Entrega 2.
    return 0;
}