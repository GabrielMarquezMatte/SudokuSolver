find_package(GTest CONFIG REQUIRED)
set(TEST_SOURCES
    TestSudokuSolver.cpp
)

enable_testing()
include(GoogleTest)

# Create the test executable
add_executable(${PROJECT_NAME}_TEST ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}_TEST GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main Boost::dynamic_bitset)

gtest_discover_tests(${PROJECT_NAME}_TEST)