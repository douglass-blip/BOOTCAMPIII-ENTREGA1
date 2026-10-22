#include "click_tracker.hpp"

// STUB: so existe para o harness compilar e os testes falharem (fase vermelha).
void ClickTracker::record(const std::string&, Clock::time_point) {}

std::size_t ClickTracker::count(const std::string&) const { return 0; }

std::vector<ClickTracker::Clock::time_point> ClickTracker::history(const std::string&) const {
    return {};
}
