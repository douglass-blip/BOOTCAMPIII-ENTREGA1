#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "api.hpp"
#include "click_tracker.hpp"
#include "redirect_service.hpp"
#include "url_store.hpp"

namespace {

const char* environmentValue(const char* name, const char* fallback) {
    const char* value = std::getenv(name);
    return value == nullptr || *value == '\0' ? fallback : value;
}

bool parsePort(const char* value, int& port) {
    errno = 0;
    char* end = nullptr;
    const long parsed = std::strtol(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0' || parsed < 1 || parsed > 65535) {
        return false;
    }
    port = static_cast<int>(parsed);
    return true;
}

} // namespace

int main() {
    int port = 8080;
    if (!parsePort(environmentValue("URL_SHORTENER_PORT", "8080"), port)) {
        std::cerr << "URL_SHORTENER_PORT must be an integer from 1 to 65535\n";
        return 2;
    }

    const std::filesystem::path data_directory =
        environmentValue("URL_SHORTENER_DATA_DIR", "data");
    PersistentUrlStore store(data_directory / "urls.jsonl");
    ClickTracker tracker(data_directory / "clicks.jsonl");
    RedirectService redirect_service(store, tracker);

    httplib::Server server;
    registerApiRoutes(
        server,
        redirect_service,
        environmentValue(
            "URL_SHORTENER_PUBLIC_BASE_URL",
            "http://localhost:8080"));

    const std::string host = environmentValue("URL_SHORTENER_HOST", "0.0.0.0");
    std::cout << "URL shortener listening on " << host << ':' << port << '\n';
    if (!server.listen(host.c_str(), port)) {
        std::cerr << "Unable to start HTTP server on " << host << ':' << port << '\n';
        return 1;
    }
    return 0;
}