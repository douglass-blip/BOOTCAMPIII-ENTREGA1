#include "api.hpp"

#include <iostream>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "redirect_service.hpp"
#include "time_utils.hpp"

namespace {

using Json = nlohmann::json;

void sendJson(httplib::Response& response, int status, const Json& body) {
    response.status = status;
    response.set_content(body.dump(), "application/json");
}

void sendError(httplib::Response& response, int status, const std::string& message) {
    sendJson(response, status, Json{{"error", message}});
}

} // namespace

void registerApiRoutes(
    httplib::Server& server,
    RedirectService& service,
    std::string public_base_url
) {
    while (!public_base_url.empty() && public_base_url.back() == '/') {
        public_base_url.pop_back();
    }

    server.Post("/shorten", [&service, public_base_url](
        const httplib::Request& request,
        httplib::Response& response
    ) {
        Json body;
        try {
            body = Json::parse(request.body);
        } catch (const Json::parse_error&) {
            sendError(response, 400, "request body must be valid JSON");
            return;
        }

        if (!body.is_object() || !body.contains("url") ||
            !body["url"].is_string()) {
            sendError(response, 400, "url must be a string");
            return;
        }

        std::optional<RedirectService::Clock::time_point> expires_at;
        if (body.contains("expires_at") && !body["expires_at"].is_null()) {
            if (!body["expires_at"].is_string()) {
                sendError(response, 400, "expires_at must be an ISO 8601 string");
                return;
            }
            expires_at = TimeUtils::parseIso8601(
                body["expires_at"].get<std::string>());
            if (!expires_at) {
                sendError(response, 400, "expires_at must be a valid ISO 8601 timestamp");
                return;
            }
        }

        try {
            const std::string code = service.shorten(
                body["url"].get<std::string>(), expires_at);
            sendJson(response, 201, Json{
                {"short_code", code},
                {"short_url", public_base_url + "/" + code}
            });
        } catch (const std::invalid_argument& error) {
            sendError(response, 400, error.what());
        } catch (const std::runtime_error& error) {
            std::cerr << "Short URL creation failed: " << error.what() << '\n';
            sendError(response, 500, "unable to create short URL");
        }
    });

    server.Get(R"(/([A-Za-z0-9]{6,8})/stats)", [&service](
        const httplib::Request& request,
        httplib::Response& response
    ) {
        const auto stats = service.stats(request.matches[1].str());
        if (!stats) {
            sendError(response, 404, "short URL not found");
            return;
        }

        Json expires_at = nullptr;
        if (stats->record.expiresAt) {
            expires_at = TimeUtils::formatIso8601(*stats->record.expiresAt);
        }
        sendJson(response, 200, Json{
            {"short_code", stats->shortCode},
            {"url", stats->record.url},
            {"clicks", stats->clicks},
            {"created_at", TimeUtils::formatIso8601(stats->record.createdAt)},
            {"expires_at", expires_at}
        });
    });

    server.Get(R"(/([A-Za-z0-9]{6,8}))", [&service](
        const httplib::Request& request,
        httplib::Response& response
    ) {
        try {
            const auto url = service.redirect(request.matches[1].str());
            if (!url) {
                sendError(response, 404, "short URL not found");
                return;
            }
            response.status = 302;
            response.set_header("Location", *url);
        } catch (const std::runtime_error& error) {
            std::cerr << "Redirect failed: " << error.what() << '\n';
            sendError(response, 500, "unable to record redirect");
        }
    });
}
