#include "../include/SudokuMatrix.hpp"
#include "../include/solvers/BackTracking.hpp"
#include "../include/solvers/DlxSolver.hpp"
#include "../include/SudokuUtilities.hpp"
#include <gtest/gtest.h>

inline constexpr SudokuMatrix<3> CreateBoard()
{
    SudokuMatrix<3> matrix;
    matrix.SetValue(0, 0, 1);
    matrix.SetValue(0, 1, 2);
    matrix.SetValue(0, 2, 3);
    matrix.SetValue(1, 0, 4);
    matrix.SetValue(1, 1, 5);
    matrix.SetValue(1, 2, 6);
    matrix.SetValue(2, 0, 7);
    matrix.SetValue(2, 1, 8);
    matrix.SetValue(2, 2, 9);
    return matrix;
}

TEST(SudokuMatrix, SetValue)
{
    static constexpr SudokuMatrix<3> matrix = CreateBoard();
    static_assert(matrix.GetValue(0, 0) == 1);
    static_assert(matrix.GetValue(0, 1) == 2);
    static_assert(matrix.GetValue(0, 2) == 3);
    static_assert(matrix.GetValue(1, 0) == 4);
    static_assert(matrix.GetValue(1, 1) == 5);
    static_assert(matrix.GetValue(1, 2) == 6);
    static_assert(matrix.GetValue(2, 0) == 7);
    static_assert(matrix.GetValue(2, 1) == 8);
    static_assert(matrix.GetValue(2, 2) == 9);
}

TEST(SudokuMatrix, RemoveValue)
{
    static constexpr auto getBoard = []() -> SudokuMatrix<3> {
        SudokuMatrix<3> matrix = CreateBoard();
        matrix.RemoveValue(0, 0);
        matrix.RemoveValue(0, 1);
        matrix.RemoveValue(0, 2);
        matrix.RemoveValue(1, 0);
        matrix.RemoveValue(1, 1);
        matrix.RemoveValue(1, 2);
        matrix.RemoveValue(2, 0);
        matrix.RemoveValue(2, 1);
        matrix.RemoveValue(2, 2);
        return matrix;
    };
    static constexpr SudokuMatrix<3> matrix = getBoard();
    static_assert(matrix.GetValue(0, 0) == 0);
    static_assert(matrix.GetValue(0, 1) == 0);
    static_assert(matrix.GetValue(0, 2) == 0);
    static_assert(matrix.GetValue(1, 0) == 0);
    static_assert(matrix.GetValue(1, 1) == 0);
    static_assert(matrix.GetValue(1, 2) == 0);
    static_assert(matrix.GetValue(2, 0) == 0);
    static_assert(matrix.GetValue(2, 1) == 0);
    static_assert(matrix.GetValue(2, 2) == 0);
}

template<typename Solver = BackTrackingSolver<3>>
inline constexpr bool CanBeSolved()
{
    constexpr std::array<char, 81> sudokuGame = {
        5, 3, 0, 0, 7, 0, 0, 0, 0,
        6, 0, 0, 1, 9, 5, 0, 0, 0,
        0, 9, 8, 0, 0, 0, 0, 6, 0,

        8, 0, 0, 0, 6, 0, 0, 0, 3,
        4, 0, 0, 8, 0, 3, 0, 0, 1,
        7, 0, 0, 0, 2, 0, 0, 0, 6,

        0, 6, 0, 0, 0, 0, 2, 8, 0,
        0, 0, 0, 4, 1, 9, 0, 0, 5,
        0, 0, 0, 0, 8, 0, 0, 7, 9};
    Solver solver{sudokuGame};
    while (solver.Advance()) ;
    return solver.IsSolved() && IsValidSudoku(solver.GetBoard());
}

TEST(SudokuMatrix, CanCreateRandomSudoku)
{
    std::random_device device;
    pcg64 rng{device()};
    constexpr float probability = 0.25f;
    SudokuMatrix<3> data = CreateBoard<3>(probability, rng);
    EXPECT_TRUE(IsValidSudoku(data));
}

TEST(SudokuMatrix, CheckBitSetIterator)
{
    static constinit BitSetIterator<3> it{0b101};
    EXPECT_EQ(it.Count(), 2);
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(it.Count(), 0);
}

TEST(SudokuMatrix, SolveSudokuBackTracking)
{
    EXPECT_TRUE(CanBeSolved<BackTrackingSolver<3>>());
}

TEST(SudokuMatrix, SolveSudokuDlx)
{
    EXPECT_TRUE(CanBeSolved<DLXSolver<3>>());
}