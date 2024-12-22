#include <random>
#include <pcg_random.hpp>
#include <benchmark/benchmark.h>
#include "../include/SudokuMatrix.hpp"
#include "../include/SudokuUtilities.hpp"
#include "../include/solvers/BackTracking.hpp"
#include "../include/solvers/DlxSolver.hpp"

static void BM_CreateBoard(benchmark::State &state)
{
    pcg64 rng{std::random_device{}()};
    float probability = static_cast<float>(state.range(0)) / 100.0f;
    for (auto _ : state)
    {
        SudokuMatrix board = CreateBoard<3>(probability, rng);
        benchmark::DoNotOptimize(board);
    }
}

BENCHMARK(BM_CreateBoard)->DenseRange(10, 90, 20);

template<typename Solver = BackTrackingSolver<3>>
static void BM_SolverStatic(benchmark::State &state)
{
    static constexpr std::array<char, 81> sudokuGame = {
        5, 3, 0, 0, 7, 0, 0, 0, 0,
        6, 0, 0, 1, 9, 5, 0, 0, 0,
        0, 9, 8, 0, 0, 0, 0, 6, 0,

        8, 0, 0, 0, 6, 0, 0, 0, 3,
        4, 0, 0, 8, 0, 3, 0, 0, 1,
        7, 0, 0, 0, 2, 0, 0, 0, 6,

        0, 6, 0, 0, 0, 0, 2, 8, 0,
        0, 0, 0, 4, 1, 9, 0, 0, 5,
        0, 0, 0, 0, 8, 0, 0, 7, 9};
    std::int64_t index = 0;
    for (auto _ : state)
    {
        Solver solver{sudokuGame};
        while (solver.Advance())
        {
            index++;
        }
    }
    state.SetItemsProcessed(index);
}

BENCHMARK(BM_SolverStatic<BackTrackingSolver<3>>);
BENCHMARK(BM_SolverStatic<DLXSolver<3>>);

template <typename Solver = BackTrackingSolver<3>>
static void BM_SolverRandom(benchmark::State &state)
{
    std::random_device device;
    pcg64 rng{device()};
    float probability = static_cast<float>(state.range(0)) / 100.0f;
    SudokuMatrix<3> sudokuGame = CreateBoard<3>(probability, rng);
    std::int64_t index = 0;
    bool breakAll = false;
    for (auto _ : state)
    {
        Solver solver{sudokuGame};
        std::size_t innerIndex = 0;
        while (solver.Advance())
        {
            innerIndex++;
            if (innerIndex == 10'000'000)
            {
                breakAll = true;
                break;
            }
        }
        index += innerIndex;
        if (breakAll)
        {
            break;
        }
    }
    state.SetItemsProcessed(index);
}

BENCHMARK(BM_SolverRandom<DLXSolver<3>>)->DenseRange(30, 50, 5);
BENCHMARK(BM_SolverRandom<BackTrackingSolver<3>>)->DenseRange(30, 50, 5);

BENCHMARK_MAIN();