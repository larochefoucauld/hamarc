include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    hamarc_tests
    copydata_test.cpp decoder_test.cpp encoder_test.cpp hamarchiver_test.cpp
)

add_subdirectory(lib)
# target_include_directories(hamarc_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(hamarc_tests HamArc FileComparator GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(hamarc_tests)
