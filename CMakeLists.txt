cmake_minimum_required(VERSION 3.30.0)
project(SudokuSolver VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG:OFF")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG:OFF")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /LTCG:OFF")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=array-bounds")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mavx2")
endif()

include(${PROJECT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(SFML COMPONENTS system window graphics audio CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS dynamic_bitset)
find_path(PCG_INCLUDE_DIRS "pcg_extras.hpp")

add_subdirectory(tests)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-system sfml-network sfml-graphics sfml-window sfml-audio Boost::dynamic_bitset)
target_include_directories(${PROJECT_NAME} PRIVATE ${PCG_INCLUDE_DIRS})
add_executable(${PROJECT_NAME}_BENCHMARK src/benchmarks.cpp)
target_link_libraries(${PROJECT_NAME}_BENCHMARK PRIVATE benchmark::benchmark benchmark::benchmark_main Boost::dynamic_bitset)
target_include_directories(${PROJECT_NAME}_BENCHMARK PRIVATE ${PCG_INCLUDE_DIRS})