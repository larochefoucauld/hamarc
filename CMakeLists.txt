cmake_minimum_required(VERSION 3.12)

project(
    hamarc
    VERSION 1.0
    DESCRIPTION "Hamming-based error-correcting archiver"
    LANGUAGES CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)
add_subdirectory(src)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/include)

add_subdirectory(tests)
target_include_directories(hamarc_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/include)
