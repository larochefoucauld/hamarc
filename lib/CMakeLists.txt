add_subdirectory(src)
target_include_directories(ArgParser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/argparser)
target_include_directories(HamArc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/hamarc)
