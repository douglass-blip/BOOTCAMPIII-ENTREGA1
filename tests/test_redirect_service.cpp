#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

#include "redirect_service.hpp"

TEST(RedirectService, ShortensUrlAndReturnsDifferentCodeForRepeatedInput) {
    InMemoryUrlStore store;
    ClickTracker tracker;
    RedirectService service(store, tracker);

    const std::string url = "https://example.com/a-long/path";
    const std::string first = service.shorten(url);
    const std::string second = service.shorten(url);

    EXPECT_TRUE(RedirectService::isValidCode(first));
    EXPECT_TRUE(RedirectService::isValidCode(second));
    EXPECT_NE(first, second);
    EXPECT_EQ(service.stats(first)->record.url, url);
}

TEST(RedirectService, ExpirationIsInclusiveAndExpiredLinksDoNotCountClicks) {
    InMemoryUrlStore store;
    ClickTracker tracker;
    RedirectService service(store, tracker);
    const auto now = RedirectService::Clock::time_point(std::chrono::seconds(100));
    const auto expires_at = now + std::chrono::seconds(10);
    const std::string code = service.shorten(
        "https://example.com/temporary", expires_at, now);

    ASSERT_TRUE(service.redirect(code, expires_at - std::chrono::milliseconds(1)));
    EXPECT_FALSE(service.redirect(code, expires_at));
    EXPECT_EQ(tracker.count(code), 1u);
}

TEST(RedirectService, RejectsInvalidUrlsAndCodeFormats) {
    InMemoryUrlStore store;
    ClickTracker tracker;
    RedirectService service(store, tracker);

    EXPECT_THROW(service.shorten(""), std::invalid_argument);
    EXPECT_THROW(service.shorten("ftp://example.com/file"), std::invalid_argument);
    EXPECT_THROW(service.shorten("https:///missing-host"), std::invalid_argument);
    EXPECT_THROW(service.shorten("https://example..com"), std::invalid_argument);
    EXPECT_THROW(service.shorten("https://example.com:"), std::invalid_argument);
    EXPECT_THROW(service.shorten("https://example.com:70000"), std::invalid_argument);
    EXPECT_THROW(service.shorten("https://example.com/%ZZ"), std::invalid_argument);
    EXPECT_THROW(service.shorten("https://example.com/path with space"), std::invalid_argument);
    EXPECT_THROW(service.shorten("https://[:::1]/"), std::invalid_argument);
    EXPECT_FALSE(service.redirect("bad/code"));
    EXPECT_FALSE(service.stats("short"));
}

TEST(RedirectService, AcceptsIpv6UrlAuthority) {
    InMemoryUrlStore store;
    ClickTracker tracker;
    RedirectService service(store, tracker);

    EXPECT_NO_THROW(service.shorten("https://[2001:db8::1]:443/path"));
}

TEST(RedirectService, ConcurrentShorteningReservesUniqueCodes) {
    InMemoryUrlStore store;
    ClickTracker tracker;
    RedirectService service(store, tracker);
    constexpr int kThreads = 8;
    std::vector<std::string> codes(kThreads);
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&service, &codes, i] {
            codes[static_cast<std::size_t>(i)] =
                service.shorten("https://example.com/concurrent");
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    std::sort(codes.begin(), codes.end());
    EXPECT_EQ(std::unique(codes.begin(), codes.end()), codes.end());
}

TEST(RedirectService, MissingAndExpiredDestinationsDoNotIncrementClicks) {
    InMemoryUrlStore store;
    ClickTracker tracker;
    RedirectService service(store, tracker);

    EXPECT_FALSE(service.redirect("missing1"));
    EXPECT_EQ(tracker.count("missing1"), 0u);

    const auto now = RedirectService::Clock::time_point(std::chrono::seconds(500));
    const std::string expired_code = service.shorten(
        "https://example.com/expired", now, now - std::chrono::seconds(1));
    EXPECT_FALSE(service.redirect(expired_code, now));
    EXPECT_EQ(tracker.count(expired_code), 0u);
}
