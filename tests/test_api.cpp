#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include <nlohmann/json.hpp>

#include "api.hpp"
#include "click_tracker.hpp"
#include "redirect_service.hpp"
#include "time_utils.hpp"
#include "url_store.hpp"

class ApiTest : public testing::Test {
protected:
    void SetUp() override {
        registerApiRoutes(server_, service_, "http://short.test/");
        const int port = server_.bind_to_any_port("127.0.0.1");
        ASSERT_GT(port, 0);

        server_thread_ = std::thread([this] {
            server_.listen_after_bind();
        });
        for (int i = 0; i < 100 && !server_.is_running(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        ASSERT_TRUE(server_.is_running());

        client_ = std::make_unique<httplib::Client>("127.0.0.1", port);
        client_->set_connection_timeout(2, 0);
    }

    void TearDown() override {
        server_.stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    InMemoryUrlStore store_;
    ClickTracker tracker_;
    RedirectService service_{store_, tracker_};
    httplib::Server server_;
    std::thread server_thread_;
    std::unique_ptr<httplib::Client> client_;
};

TEST_F(ApiTest, ShortenRedirectAndStatsFollowTheHttpContract) {
    const auto shortened = client_->Post(
        "/shorten",
        R"({"url":"https://example.com/article","expires_at":"2030-01-02T03:04:05Z"})",
        "application/json");
    ASSERT_TRUE(shortened);
    ASSERT_EQ(shortened->status, 201);

    const auto created = nlohmann::json::parse(shortened->body);
    const std::string code = created.at("short_code").get<std::string>();
    EXPECT_EQ(created.at("short_url"), "http://short.test/" + code);
    EXPECT_TRUE(RedirectService::isValidCode(code));

    const auto redirect = client_->Get("/" + code);
    ASSERT_TRUE(redirect);
    EXPECT_EQ(redirect->status, 302);
    EXPECT_EQ(redirect->get_header_value("Location"), "https://example.com/article");

    const auto stats = client_->Get("/" + code + "/stats");
    ASSERT_TRUE(stats);
    ASSERT_EQ(stats->status, 200);
    const auto body = nlohmann::json::parse(stats->body);
    EXPECT_EQ(body.at("short_code"), code);
    EXPECT_EQ(body.at("url"), "https://example.com/article");
    EXPECT_EQ(body.at("clicks"), 1);
    const auto record = service_.stats(code);
    ASSERT_TRUE(record);
    EXPECT_EQ(body.at("created_at"),
              TimeUtils::formatIso8601(record->record.createdAt));
    EXPECT_EQ(body.at("expires_at"), "2030-01-02T03:04:05Z");
}

TEST_F(ApiTest, RejectsMalformedInputsAndUnknownCodes) {
    const auto missing_url = client_->Post("/shorten", R"({"other":"value"})", "application/json");
    ASSERT_TRUE(missing_url);
    EXPECT_EQ(missing_url->status, 400);

    const auto malformed_json = client_->Post("/shorten", "{", "application/json");
    ASSERT_TRUE(malformed_json);
    EXPECT_EQ(malformed_json->status, 400);

    const auto malformed_url = client_->Post(
        "/shorten", R"({"url":"not a URL"})", "application/json");
    ASSERT_TRUE(malformed_url);
    EXPECT_EQ(malformed_url->status, 400);

    const auto malformed_expiry = client_->Post(
        "/shorten",
        R"({"url":"https://example.com","expires_at":"2030-02-30T00:00:00Z"})",
        "application/json");
    ASSERT_TRUE(malformed_expiry);
    EXPECT_EQ(malformed_expiry->status, 400);

    const auto missing_redirect = client_->Get("/missing1");
    ASSERT_TRUE(missing_redirect);
    EXPECT_EQ(missing_redirect->status, 404);
    const auto missing_stats = client_->Get("/missing1/stats");
    ASSERT_TRUE(missing_stats);
    EXPECT_EQ(missing_stats->status, 404);
    const auto invalid_code = client_->Get("/bad-code/stats");
    ASSERT_TRUE(invalid_code);
    EXPECT_EQ(invalid_code->status, 404);
}

TEST_F(ApiTest, ExpiredUrlsReturnNotFoundWithoutCountingClicks) {
    const auto shortened = client_->Post(
        "/shorten",
        R"({"url":"https://example.com/expired","expires_at":"2000-01-01T00:00:00Z"})",
        "application/json");
    ASSERT_TRUE(shortened);
    ASSERT_EQ(shortened->status, 201);
    const std::string code = nlohmann::json::parse(shortened->body)
        .at("short_code").get<std::string>();

    const auto redirect = client_->Get("/" + code);
    ASSERT_TRUE(redirect);
    EXPECT_EQ(redirect->status, 404);
    EXPECT_EQ(tracker_.count(code), 0u);
}

TEST_F(ApiTest, RepeatedUrlCreatesDistinctCodes) {
    const std::string body = R"({"url":"https://example.com/repeated"})";
    const auto first = client_->Post("/shorten", body, "application/json");
    const auto second = client_->Post("/shorten", body, "application/json");
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_EQ(first->status, 201);
    ASSERT_EQ(second->status, 201);
    EXPECT_NE(
        nlohmann::json::parse(first->body).at("short_code"),
        nlohmann::json::parse(second->body).at("short_code"));
}
