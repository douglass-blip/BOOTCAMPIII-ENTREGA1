#include "url_store.hpp"

bool InMemoryUrlStore::insert(const std::string& code, const UrlRecord& record) {
    // RN01: nao sobrescreve um codigo ja existente.
    if (records_.count(code) > 0) {
        return false;
    }
    records_.emplace(code, record);
    return true;
}

std::optional<UrlRecord> InMemoryUrlStore::get(const std::string& code) const {
    auto it = records_.find(code);
    if (it == records_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool InMemoryUrlStore::exists(const std::string& code) const {
    return records_.count(code) > 0;
}
