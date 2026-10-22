#include "click_tracker.hpp"

void ClickTracker::record(const std::string& code, Clock::time_point when) {
    std::lock_guard<std::mutex> lock(mutex_);
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
