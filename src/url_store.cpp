#include "url_store.hpp"

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <utility>

#include <nlohmann/json.hpp>

namespace {

using Json = nlohmann::json;
using Clock = std::chrono::system_clock;

std::int64_t toMilliseconds(Clock::time_point time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        time.time_since_epoch()).count();
}

Clock::time_point fromMilliseconds(std::int64_t milliseconds) {
    return Clock::time_point(std::chrono::duration_cast<Clock::duration>(
        std::chrono::milliseconds(milliseconds)));
}

UrlRecord recordFromJson(const Json& value) {
    if (!value.is_object() || !value.contains("url") ||
        !value["url"].is_string() || !value.contains("created_at_ms") ||
        !value["created_at_ms"].is_number_integer() ||
        !value.contains("expires_at_ms")) {
        throw std::runtime_error("invalid URL record");
    }

    std::optional<Clock::time_point> expires_at;
    if (!value["expires_at_ms"].is_null()) {
        if (!value["expires_at_ms"].is_number_integer()) {
            throw std::runtime_error("invalid expires_at_ms");
        }
        expires_at = fromMilliseconds(value["expires_at_ms"].get<std::int64_t>());
    }

    return {
        value["url"].get<std::string>(),
        fromMilliseconds(value["created_at_ms"].get<std::int64_t>()),
        expires_at
    };
}

Json recordToJson(const std::string& code, const UrlRecord& record) {
    return {
        {"code", code},
        {"url", record.url},
        {"created_at_ms", toMilliseconds(record.createdAt)},
        {"expires_at_ms", record.expiresAt
            ? Json(toMilliseconds(*record.expiresAt))
            : Json(nullptr)}
    };
}

void createParentDirectory(const std::filesystem::path& path) {
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
    }
}

} // namespace

bool InMemoryUrlStore::insert(const std::string& code, const UrlRecord& record) {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.emplace(code, record).second;
}

std::optional<UrlRecord> InMemoryUrlStore::get(const std::string& code) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = records_.find(code);
    if (it == records_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool InMemoryUrlStore::exists(const std::string& code) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.find(code) != records_.end();
}

PersistentUrlStore::PersistentUrlStore(std::filesystem::path file_path)
    : file_path_(std::move(file_path)) {
    createParentDirectory(file_path_);
    load();
}

bool PersistentUrlStore::insert(const std::string& code, const UrlRecord& record) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (records_.find(code) != records_.end()) {
        return false;
    }

    append(code, record);
    records_.emplace(code, record);
    return true;
}

std::optional<UrlRecord> PersistentUrlStore::get(const std::string& code) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = records_.find(code);
    if (it == records_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool PersistentUrlStore::exists(const std::string& code) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.find(code) != records_.end();
}

void PersistentUrlStore::load() {
    std::ifstream input(file_path_, std::ios::binary);
    if (!input) {
        if (std::filesystem::exists(file_path_)) {
            throw std::runtime_error("cannot open URL store: " + file_path_.string());
        }
        return;
    }

    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (line.empty()) {
            continue;
        }

        try {
            const Json value = Json::parse(line);
            if (!value.is_object() || !value.contains("code") ||
                !value["code"].is_string()) {
                throw std::runtime_error("missing code");
            }
            const std::string code = value["code"].get<std::string>();
            if (!records_.emplace(code, recordFromJson(value)).second) {
                throw std::runtime_error("duplicate code");
            }
        } catch (const std::exception& error) {
            throw std::runtime_error(
                "invalid URL store at line " + std::to_string(line_number) +
                ": " + error.what());
        }
    }
    if (input.bad()) {
        throw std::runtime_error("cannot read URL store: " + file_path_.string());
    }
}

void PersistentUrlStore::append(
    const std::string& code,
    const UrlRecord& record
) const {
    std::ofstream output(file_path_, std::ios::binary | std::ios::app);
    if (!output) {
        throw std::runtime_error("cannot append URL store: " + file_path_.string());
    }
    output << recordToJson(code, record).dump() << '\n';
    output.flush();
    if (!output) {
        throw std::runtime_error("cannot persist URL record: " + file_path_.string());
    }
}
