find_package(GTest CONFIG REQUIRED)
include(GoogleTest)
set(TEST_SOURCES
    TestSudokuSolver.cpp
)

enable_testing()

# Create the test executable
add_executable(${PROJECT_NAME}_TEST ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}_TEST GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main Boost::headers)
target_include_directories(${PROJECT_NAME}_TEST PRIVATE ${PCG_INCLUDE_DIRS})

add_test(NAME ${PROJECT_NAME}_TEST COMMAND ${PROJECT_NAME}_TEST)
gtest_discover_tests(${PROJECT_NAME}_TEST)