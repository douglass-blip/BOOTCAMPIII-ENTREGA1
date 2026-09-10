include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
# Evita que o GoogleTest sobrescreva as flags de compilador do projeto pai
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(run_tests
    test_code_generator.cpp
)

target_link_libraries(run_tests
    PRIVATE
    url_shortener_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)
