cmake_minimum_required(VERSION 3.16)
project(url_shortener CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

set(HTTPLIB_COMPILE OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
)
FetchContent_MakeAvailable(httplib)

add_library(url_shortener_lib
    src/code_generator.cpp
    src/url_store.cpp
    src/click_tracker.cpp
    src/redirect_service.cpp
    src/time_utils.cpp
)
find_package(Threads REQUIRED)
target_include_directories(url_shortener_lib PUBLIC src)
target_link_libraries(
    url_shortener_lib
    PUBLIC Threads::Threads nlohmann_json::nlohmann_json)

add_library(url_shortener_api src/api.cpp)
target_include_directories(url_shortener_api PUBLIC src)
target_link_libraries(
    url_shortener_api
    PUBLIC url_shortener_lib httplib::httplib)
if(WIN32)
    target_link_libraries(url_shortener_api PUBLIC ws2_32)
endif()

add_executable(url_shortener src/main.cpp)
target_link_libraries(url_shortener PRIVATE url_shortener_api)

enable_testing()
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()
