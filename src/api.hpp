#pragma once

#include <string>

#include "httplib.h"

class RedirectService;

void registerApiRoutes(
    httplib::Server& server,
    RedirectService& service,
    std::string public_base_url);
