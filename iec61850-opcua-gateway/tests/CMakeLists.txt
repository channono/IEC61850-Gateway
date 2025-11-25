find_package(GTest REQUIRED)

add_executable(unit_tests
    test_main.cpp
    test_scl_parser.cpp
    test_scd_generator.cpp
    test_sv_capture.cpp
    # Add other test files here
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(unit_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    ${PROJECT_NAME} # Link against main project code
    # We might need to link dependencies explicitly if not transitive
    pugixml::pugixml
    spdlog::spdlog
)
