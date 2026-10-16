#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>

// Registro armazenado para cada codigo curto.
struct UrlRecord {
    std::string url;
    std::chrono::system_clock::time_point createdAt;
    std::optional<std::chrono::system_clock::time_point> expiresAt;
};

// Interface abstrata de persistencia (CRUD de URL curta <-> longa + metadados).
// Permite trocar a implementacao (ex: por um banco de verdade) sem alterar
// quem consome o UrlStore (RedirectService, API Layer, etc.).
class UrlStore {
public:
    virtual ~UrlStore() = default;

    // Insere um novo registro. Retorna false se o codigo ja existir
    // (RN01: um codigo, uma vez gerado, e imutavel -- nao pode ser sobrescrito).
    virtual bool insert(const std::string& code, const UrlRecord& record) = 0;

    // Retorna o registro associado ao codigo, ou nullopt se nao existir.
    // Nao faz nenhuma checagem de expiracao -- isso e responsabilidade
    // de quem consome (RedirectService), conforme RN02.
    virtual std::optional<UrlRecord> get(const std::string& code) const = 0;

    // Verifica existencia do codigo, usado pelo CodeGenerator para
    // resolucao de colisao.
    virtual bool exists(const std::string& code) const = 0;
};

// Implementacao inicial em memoria.
class InMemoryUrlStore : public UrlStore {
public:
    bool insert(const std::string& code, const UrlRecord& record) override;
    std::optional<UrlRecord> get(const std::string& code) const override;
    bool exists(const std::string& code) const override;

private:
    std::unordered_map<std::string, UrlRecord> records_;
};
