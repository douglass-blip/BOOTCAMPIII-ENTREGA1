#pragma once

#include <chrono>
#include <mutex>
#include <optional>
#include <string>

#include "click_tracker.hpp"
#include "url_store.hpp"

struct UrlStats {
    std::string shortCode;
    UrlRecord record;
    std::size_t clicks;
};

class RedirectService {
public:
    using Clock = std::chrono::system_clock;

    RedirectService(UrlStore& store, ClickTracker& tracker);

    std::string shorten(
        const std::string& url,
        std::optional<Clock::time_point> expires_at = std::nullopt,
        Clock::time_point now = Clock::now());

    std::optional<std::string> redirect(
        const std::string& code,
        Clock::time_point now = Clock::now());

    std::optional<UrlStats> stats(const std::string& code) const;

    static bool isValidUrl(const std::string& url);
    static bool isValidCode(const std::string& code);

private:
    UrlStore& store_;
    ClickTracker& tracker_;
    std::mutex creation_mutex_;
};