cmake_minimum_required(VERSION 3.16)
project(url_shortener CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Flags de correção/depuração (recomendado durante desenvolvimento)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra)

# Biblioteca com o código dos componentes (CodeGenerator, UrlStore, etc.)
add_library(url_shortener_lib
    src/code_generator.cpp
)
target_include_directories(url_shortener_lib PUBLIC src)

# Executável principal (placeholder por enquanto)
add_executable(url_shortener src/main.cpp)
target_link_libraries(url_shortener PRIVATE url_shortener_lib)

# Testes (habilitado quando o harness for adicionado no item 4)
enable_testing()
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()
