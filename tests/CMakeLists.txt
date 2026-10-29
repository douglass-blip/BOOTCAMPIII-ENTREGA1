include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(run_tests
    test_code_generator.cpp
    test_url_store.cpp
    test_click_tracker.cpp
    test_redirect_service.cpp
    test_persistence.cpp
    test_api.cpp
    test_time_utils.cpp
)

target_link_libraries(run_tests
    PRIVATE
    url_shortener_api
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)
