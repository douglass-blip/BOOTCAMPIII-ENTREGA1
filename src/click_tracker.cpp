#include "click_tracker.hpp"

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <utility>

#include <nlohmann/json.hpp>

namespace {

using Json = nlohmann::json;

std::int64_t toMilliseconds(ClickTracker::Clock::time_point time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        time.time_since_epoch()).count();
}

ClickTracker::Clock::time_point fromMilliseconds(std::int64_t milliseconds) {
    return ClickTracker::Clock::time_point(
        std::chrono::duration_cast<ClickTracker::Clock::duration>(
            std::chrono::milliseconds(milliseconds)));
}

} // namespace

ClickTracker::ClickTracker(std::filesystem::path persistence_file)
    : persistence_file_(std::move(persistence_file)) {
    if (!persistence_file_.empty() && !persistence_file_.parent_path().empty()) {
        std::filesystem::create_directories(persistence_file_.parent_path());
    }
    load();
}

void ClickTracker::record(const std::string& code, Clock::time_point when) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!persistence_file_.empty()) {
        std::ofstream output(persistence_file_, std::ios::binary | std::ios::app);
        if (!output) {
            throw std::runtime_error(
                "cannot append click history: " + persistence_file_.string());
        }
        output << Json{{"code", code}, {"timestamp_ms", toMilliseconds(when)}}.dump()
               << '\n';
        output.flush();
        if (!output) {
            throw std::runtime_error(
                "cannot persist click: " + persistence_file_.string());
        }
    }
    clicks_[code].push_back(when);
}

std::size_t ClickTracker::count(const std::string& code) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = clicks_.find(code);
    return it == clicks_.end() ? 0 : it->second.size();
}

std::vector<ClickTracker::Clock::time_point> ClickTracker::history(const std::string& code) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = clicks_.find(code);
    if (it == clicks_.end()) {
        return {};
    }
    // Devolve uma COPIA: o chamador nunca enxerga o vetor interno sem o lock.
    return it->second;
}

void ClickTracker::load() {
    if (persistence_file_.empty()) {
        return;
    }

    std::ifstream input(persistence_file_, std::ios::binary);
    if (!input) {
        if (std::filesystem::exists(persistence_file_)) {
            throw std::runtime_error(
                "cannot open click history: " + persistence_file_.string());
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
                !value["code"].is_string() || !value.contains("timestamp_ms") ||
                !value["timestamp_ms"].is_number_integer()) {
                throw std::runtime_error("invalid click record");
            }
            clicks_[value["code"].get<std::string>()].push_back(
                fromMilliseconds(value["timestamp_ms"].get<std::int64_t>()));
        } catch (const std::exception& error) {
            throw std::runtime_error(
                "invalid click history at line " + std::to_string(line_number) +
                ": " + error.what());
        }
    }
    if (input.bad()) {
        throw std::runtime_error(
            "cannot read click history: " + persistence_file_.string());
    }
}
