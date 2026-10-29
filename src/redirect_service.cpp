#include "redirect_service.hpp"

#include <cctype>
#include <stdexcept>

#include "code_generator.hpp"

namespace {

bool validPort(const std::string& value) {
    if (value.empty()) {
        return false;
    }
    unsigned int port = 0;
    for (unsigned char character : value) {
        if (character < '0' || character > '9') {
            return false;
        }
        port = port * 10 + static_cast<unsigned int>(character - '0');
        if (port > 65535) {
            return false;
        }
    }
    return port > 0;
}

bool validIpv4Address(const std::string& value) {
    std::size_t start = 0;
    for (int octet = 0; octet < 4; ++octet) {
        const std::size_t end = value.find('.', start);
        const std::size_t length = (end == std::string::npos ? value.size() : end) - start;
        if (length == 0 || length > 3 || (octet < 3) != (end != std::string::npos)) {
            return false;
        }
        unsigned int number = 0;
        for (std::size_t i = start; i < start + length; ++i) {
            if (value[i] < '0' || value[i] > '9') {
                return false;
            }
            number = number * 10 + static_cast<unsigned int>(value[i] - '0');
        }
        if (number > 255) {
            return false;
        }
        start += length + 1;
    }
    return start == value.size() + 1;
}

bool addIpv6Groups(
    const std::string& part,
    bool allow_ipv4_tail,
    unsigned int& group_count
) {
    if (part.empty()) {
        return true;
    }
    std::size_t start = 0;
    while (start < part.size()) {
        const std::size_t end = part.find(':', start);
        const std::size_t token_end =
            end == std::string::npos ? part.size() : end;
        const std::string token = part.substr(start, token_end - start);
        if (token.empty()) {
            return false;
        }
        if (token.find('.') != std::string::npos) {
            if (!allow_ipv4_tail || end != std::string::npos ||
                !validIpv4Address(token)) {
                return false;
            }
            group_count += 2;
        } else {
            if (token.size() > 4) {
                return false;
            }
            for (unsigned char character : token) {
                if (!std::isxdigit(character)) {
                    return false;
                }
            }
            ++group_count;
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return true;
}

bool validIpv6Address(const std::string& address) {
    const std::size_t compression = address.find("::");
    if (compression == std::string::npos) {
        unsigned int groups = 0;
        return addIpv6Groups(address, true, groups) && groups == 8;
    }
    if (address.find("::", compression + 2) != std::string::npos) {
        return false;
    }

    unsigned int groups = 0;
    const std::string left = address.substr(0, compression);
    const std::string right = address.substr(compression + 2);
    return addIpv6Groups(left, false, groups) &&
        addIpv6Groups(right, true, groups) && groups < 8;
}

bool validDomainName(const std::string& host) {
    if (host.empty() || host.size() > 253) {
        return false;
    }
    std::size_t label_start = 0;
    for (std::size_t i = 0; i <= host.size(); ++i) {
        if (i != host.size() && host[i] != '.') {
            const unsigned char character = static_cast<unsigned char>(host[i]);
            if (!std::isalnum(character) && character != '-') {
                return false;
            }
            continue;
        }

        const std::size_t label_length = i - label_start;
        if (label_length == 0) {
            if (i != host.size() || i == 0 || host.back() != '.') {
                return false;
            }
        } else if (label_length > 63 || host[label_start] == '-' ||
                   host[i - 1] == '-') {
            return false;
        }
        label_start = i + 1;
    }
    return true;
}

} // namespace

RedirectService::RedirectService(UrlStore& store, ClickTracker& tracker)
    : store_(store), tracker_(tracker) {}

std::string RedirectService::shorten(
    const std::string& url,
    std::optional<Clock::time_point> expires_at,
    Clock::time_point now
) {
    if (!isValidUrl(url)) {
        throw std::invalid_argument("url must be a valid absolute HTTP or HTTPS URL");
    }

    std::lock_guard<std::mutex> creation_lock(creation_mutex_);
    const UrlRecord record{url, now, expires_at};
    for (int attempt = 0; attempt < 100; ++attempt) {
        const std::string code = generateShortCode(
            url,
            [this](const std::string& candidate) {
                return store_.exists(candidate);
            },
            7,
            100);
        if (store_.insert(code, record)) {
            return code;
        }
    }

    throw std::runtime_error("could not reserve a unique short code");
}

std::optional<std::string> RedirectService::redirect(
    const std::string& code,
    Clock::time_point now
) {
    if (!isValidCode(code)) {
        return std::nullopt;
    }

    const auto record = store_.get(code);
    if (!record || (record->expiresAt && now >= *record->expiresAt)) {
        return std::nullopt;
    }

    tracker_.record(code, now);
    return record->url;
}

std::optional<UrlStats> RedirectService::stats(const std::string& code) const {
    if (!isValidCode(code)) {
        return std::nullopt;
    }
    const auto record = store_.get(code);
    if (!record) {
        return std::nullopt;
    }
    return UrlStats{code, *record, tracker_.count(code)};
}

bool RedirectService::isValidUrl(const std::string& url) {
    const auto scheme_end = url.find("://");
    if (scheme_end == std::string::npos) {
        return false;
    }

    const std::string scheme = url.substr(0, scheme_end);
    if (scheme != "http" && scheme != "https") {
        return false;
    }

    const std::size_t authority_start = scheme_end + 3;
    const std::size_t authority_end = url.find_first_of("/?#", authority_start);
    const std::string authority = url.substr(
        authority_start,
        authority_end == std::string::npos
            ? std::string::npos
            : authority_end - authority_start);
    if (authority.empty() || authority.find('\\') != std::string::npos) {
        return false;
    }

    const std::string uri_characters = "-._~!$&'()*+,;=:/?#[]@";
    for (std::size_t i = 0; i < url.size(); ++i) {
        const unsigned char character = static_cast<unsigned char>(url[i]);
        if (character == '%') {
            if (i + 2 >= url.size() ||
                !std::isxdigit(static_cast<unsigned char>(url[i + 1])) ||
                !std::isxdigit(static_cast<unsigned char>(url[i + 2]))) {
                return false;
            }
            i += 2;
        } else if (!std::isalnum(character) &&
                   uri_characters.find(static_cast<char>(character)) ==
                       std::string::npos) {
            return false;
        }
    }

    const std::size_t user_info_end = authority.rfind('@');
    const std::string host_port = user_info_end == std::string::npos
        ? authority
        : authority.substr(user_info_end + 1);
    if (host_port.empty() ||
        (user_info_end != std::string::npos &&
         authority.find('@') != user_info_end)) {
        return false;
    }

    std::string host;
    std::string port;
    if (host_port.front() == '[') {
        const std::size_t closing_bracket = host_port.find(']');
        if (closing_bracket == std::string::npos || closing_bracket == 1) {
            return false;
        }
        host = host_port.substr(1, closing_bracket - 1);
        if (!validIpv6Address(host)) {
            return false;
        }
        if (closing_bracket + 1 < host_port.size()) {
            if (host_port[closing_bracket + 1] != ':') {
                return false;
            }
            port = host_port.substr(closing_bracket + 2);
        }
    } else {
        const std::size_t colon = host_port.find(':');
        if (colon != std::string::npos) {
            if (host_port.find(':', colon + 1) != std::string::npos) {
                return false;
            }
            host = host_port.substr(0, colon);
            port = host_port.substr(colon + 1);
        } else {
            host = host_port;
        }
        if (!validDomainName(host)) {
            return false;
        }
    }
    if ((!port.empty() && !validPort(port)) || host_port.back() == ':') {
        return false;
    }
    return true;
}

bool RedirectService::isValidCode(const std::string& code) {
    if (code.size() < 6 || code.size() > 8) {
        return false;
    }
    for (unsigned char character : code) {
        if (!std::isalnum(character)) {
            return false;
        }
    }
    return true;
}