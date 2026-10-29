cmake_minimum_required(VERSION 3.16)
project(url_shortener CXX)

option(URL_SHORTENER_ENABLE_COVERAGE "Enable gcovr line coverage reports" OFF)

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

if(URL_SHORTENER_ENABLE_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Coverage is supported only with GCC or Clang")
    endif()

    find_program(GCOVR_EXECUTABLE gcovr REQUIRED)

    function(enable_target_coverage target)
        target_compile_options(${target} PRIVATE --coverage -O0 -g)
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "STATIC_LIBRARY")
            target_link_options(${target} PRIVATE --coverage)
        endif()
    endfunction()

    enable_target_coverage(url_shortener_lib)
    enable_target_coverage(url_shortener_api)
    enable_target_coverage(url_shortener)
    enable_target_coverage(run_tests)

    add_custom_target(coverage
        COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${PROJECT_BINARY_DIR} --output-on-failure
        COMMAND ${GCOVR_EXECUTABLE}
            --root ${PROJECT_SOURCE_DIR}
            --object-directory ${PROJECT_BINARY_DIR}
            --filter ${PROJECT_SOURCE_DIR}/src/
            --txt ${PROJECT_BINARY_DIR}/coverage.txt
            --xml-pretty
            --xml ${PROJECT_BINARY_DIR}/coverage.xml
            --html-details ${PROJECT_BINARY_DIR}/coverage.html
            --fail-under-line 70
        DEPENDS run_tests
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        VERBATIM
    )
endif()
