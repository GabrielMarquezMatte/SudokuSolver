#include <random>
#include <pcg_random.hpp>
#include <benchmark/benchmark.h>
#include "../include/SudokuMatrix.hpp"
#include "../include/SudokuUtilities.hpp"
#include "../include/solvers/BackTracking.hpp"
#include "../include/solvers/DlxSolver.hpp"

template <std::size_t N>
static void BM_CreateBoard(benchmark::State &state)
{
    pcg64 rng(1);
    float probability = static_cast<float>(state.range(0)) / 100.0f;
    for (auto _ : state)
    {
        SudokuMatrix board = CreateBoard<N>(probability, rng);
        benchmark::DoNotOptimize(board);
    }
}

BENCHMARK(BM_CreateBoard<3>)->DenseRange(10, 90, 20);
BENCHMARK(BM_CreateBoard<4>)->DenseRange(10, 90, 20);
BENCHMARK(BM_CreateBoard<5>)->DenseRange(10, 90, 20);

template <std::size_t N>
static void BM_CreateDynamicBoard(benchmark::State &state)
{
    pcg64 rng(1);
    constexpr std::size_t size = N;
    float probability = static_cast<float>(state.range(0)) / 100.0f;
    for (auto _ : state)
    {
        DynamicSudokuMatrix board = CreateBoard(size, probability, rng);
        benchmark::DoNotOptimize(board);
    }
}

BENCHMARK(BM_CreateDynamicBoard<3>)->DenseRange(10, 90, 20);
BENCHMARK(BM_CreateDynamicBoard<4>)->DenseRange(10, 90, 20);
BENCHMARK(BM_CreateDynamicBoard<5>)->DenseRange(10, 90, 20);

template <std::size_t N, template <std::size_t> class Solver, typename std::enable_if<std::is_base_of<ISolver<N>, Solver<N>>::value>::type * = nullptr>
static void BM_SolverStatic(benchmark::State &state)
{
    static_assert(N == 3, "This benchmark is only for 3x3 sudoku boards");
    static constexpr std::array<typename Solver<N>::DataType, 81> sudokuGame = {
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
        Solver<N> solver{sudokuGame};
        if constexpr (std::is_same_v<Solver<N>, DLXSolver<N>>)
        {
            while (solver.Advance(false))
            {
                index++;
            }
        }
        else
        {
            while (solver.Advance())
            {
                index++;
            }
        }
    }
    state.SetItemsProcessed(index);
}

BENCHMARK(BM_SolverStatic<3, BackTrackingSolver>);
BENCHMARK(BM_SolverStatic<3, DLXSolver>);

template <class Solver, typename std::enable_if<std::is_base_of<IDynamicSolver, Solver>::value>::type * = nullptr>
static void BM_DynamicSolverStatic(benchmark::State &state)
{
    std::vector<DynamicSudokuMatrix::DataType> sudokuGame = {
        5, 3, 0, 0, 7, 0, 0, 0, 0,
        6, 0, 0, 1, 9, 5, 0, 0, 0,
        0, 9, 8, 0, 0, 0, 0, 6, 0,

        8, 0, 0, 0, 6, 0, 0, 0, 3,
        4, 0, 0, 8, 0, 3, 0, 0, 1,
        7, 0, 0, 0, 2, 0, 0, 0, 6,

        0, 6, 0, 0, 0, 0, 2, 8, 0,
        0, 0, 0, 4, 1, 9, 0, 0, 5,
        0, 0, 0, 0, 8, 0, 0, 7, 9};
    DynamicSudokuMatrix sudokuGameMatrix{std::move(sudokuGame), 3};
    std::int64_t index = 0;
    for (auto _ : state)
    {
        Solver solver{sudokuGameMatrix};
        while (solver.Advance())
        {
            index++;
        }
    }
    state.SetItemsProcessed(index);
}

BENCHMARK(BM_DynamicSolverStatic<DynamicBackTrackingSolver>);

template <std::size_t N, template <std::size_t> class Solver, typename std::enable_if<std::is_base_of<ISolver<N>, Solver<N>>::value>::type * = nullptr>
static void BM_SolverRandom(benchmark::State &state)
{
    pcg64 rng(1);
    float probability = static_cast<float>(state.range(0)) / 100.0f;
    SudokuMatrix<N> sudokuGame = CreateBoard<N>(probability, rng);
    std::int64_t index = 0;
    for (auto _ : state)
    {
        Solver<N> solver{sudokuGame};
        std::size_t innerIndex = 0;
        if constexpr (std::is_same_v<Solver<N>, DLXSolver<N>>)
        {
            while (solver.Advance(false))
            {
                innerIndex++;
                if (innerIndex == 10'000'000)
                {
                    index += innerIndex;
                    break;
                }
            }
        }
        else
        {
            while (solver.Advance())
            {
                innerIndex++;
                if (innerIndex == 10'000'000)
                {
                    index += innerIndex;
                    break;
                }
            }
        }
        index += innerIndex;
    }
    state.SetItemsProcessed(index);
}

BENCHMARK(BM_SolverRandom<3, DLXSolver>)->DenseRange(30, 70, 10);
BENCHMARK(BM_SolverRandom<4, DLXSolver>)->DenseRange(30, 70, 10);
BENCHMARK(BM_SolverRandom<5, DLXSolver>)->DenseRange(30, 70, 10);
BENCHMARK(BM_SolverRandom<3, BackTrackingSolver>)->DenseRange(30, 50, 5);
BENCHMARK(BM_SolverRandom<4, BackTrackingSolver>)->DenseRange(30, 50, 5);
BENCHMARK(BM_SolverRandom<5, BackTrackingSolver>)->DenseRange(30, 50, 5);

template <std::size_t N, class Solver, typename std::enable_if<std::is_base_of<IDynamicSolver, Solver>::value>::type * = nullptr>
static void BM_DynamicSolverRandom(benchmark::State &state)
{
    pcg64 rng(1);
    float probability = static_cast<float>(state.range(0)) / 100.0f;
    DynamicSudokuMatrix sudokuGame = CreateBoard(N, probability, rng);
    std::int64_t index = 0;
    for (auto _ : state)
    {
        Solver solver{sudokuGame};
        std::size_t innerIndex = 0;
        while (solver.Advance())
        {
            innerIndex++;
            if (innerIndex == 10'000'000)
            {
                index += innerIndex;
                state.SetItemsProcessed(index);
                return;
            }
        }
        index += innerIndex;
    }
    state.SetItemsProcessed(index);
}

BENCHMARK(BM_DynamicSolverRandom<3, DynamicBackTrackingSolver>)->DenseRange(30, 50, 5);
BENCHMARK(BM_DynamicSolverRandom<4, DynamicBackTrackingSolver>)->DenseRange(30, 50, 5);
BENCHMARK(BM_DynamicSolverRandom<5, DynamicBackTrackingSolver>)->DenseRange(30, 50, 5);

BENCHMARK_MAIN();