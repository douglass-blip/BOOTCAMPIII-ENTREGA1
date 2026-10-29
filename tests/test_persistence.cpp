#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "click_tracker.hpp"
#include "url_store.hpp"

namespace {

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        static std::atomic<unsigned long long> sequence{0};
        const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
            ("url-shortener-test-" + std::to_string(nonce) + "-" +
             std::to_string(sequence.fetch_add(1)));
        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }

    const std::filesystem::path& path() const {
        return path_;
    }

private:
    std::filesystem::path path_;
};

} // namespace

TEST(Persistence, UrlRecordsSurviveStoreRecreation) {
    TemporaryDirectory directory;
    const auto file = directory.path() / "urls.jsonl";
    const auto created_at = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(123456));
    const auto expires_at = created_at + std::chrono::hours(2);

    {
        PersistentUrlStore store(file);
        ASSERT_TRUE(store.insert(
            "abc1234",
            {"https://example.com/persisted", created_at, expires_at}));
        EXPECT_FALSE(store.insert(
            "abc1234",
            {"https://example.com/overwrite", created_at, std::nullopt}));
    }

    PersistentUrlStore restored(file);
    const auto record = restored.get("abc1234");
    ASSERT_TRUE(record);
    EXPECT_EQ(record->url, "https://example.com/persisted");
    EXPECT_EQ(record->createdAt, created_at);
    ASSERT_TRUE(record->expiresAt);
    EXPECT_EQ(*record->expiresAt, expires_at);
}

TEST(Persistence, ClickHistorySurvivesTrackerRecreation) {
    TemporaryDirectory directory;
    const auto file = directory.path() / "clicks.jsonl";
    const auto first = ClickTracker::Clock::time_point(
        std::chrono::milliseconds(1234));
    const auto second = first + std::chrono::seconds(3);

    {
        ClickTracker tracker(file);
        tracker.record("abc1234", first);
        tracker.record("abc1234", second);
    }

    ClickTracker restored(file);
    EXPECT_EQ(restored.count("abc1234"), 2u);
    const auto history = restored.history("abc1234");
    ASSERT_EQ(history.size(), 2u);
    EXPECT_EQ(history[0], first);
    EXPECT_EQ(history[1], second);
}

TEST(Persistence, CorruptStoreFailsExplicitly) {
    TemporaryDirectory directory;
    const auto file = directory.path() / "broken.jsonl";
    {
        std::ofstream output(file);
        output << "not json\n";
    }
    EXPECT_THROW(PersistentUrlStore store(file), std::runtime_error);
}
