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

inline DynamicSudokuMatrix CreateDynamicBoard()
{
    DynamicSudokuMatrix matrix(3);
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

TEST(DynamicSudokuMatrix, SetValue)
{
    DynamicSudokuMatrix matrix = CreateDynamicBoard();
    EXPECT_EQ(matrix.GetValue(0, 0), 1);
    EXPECT_EQ(matrix.GetValue(0, 1), 2);
    EXPECT_EQ(matrix.GetValue(0, 2), 3);
    EXPECT_EQ(matrix.GetValue(1, 0), 4);
    EXPECT_EQ(matrix.GetValue(1, 1), 5);
    EXPECT_EQ(matrix.GetValue(1, 2), 6);
    EXPECT_EQ(matrix.GetValue(2, 0), 7);
    EXPECT_EQ(matrix.GetValue(2, 1), 8);
    EXPECT_EQ(matrix.GetValue(2, 2), 9);
}

TEST(SudokuMatrix, Initialization)
{
    static constexpr SudokuMatrix<3> matrix1 = CreateBoard();
    static constexpr std::array<SudokuMatrix<3>::DataType, 81> sudokuGame = {
        1, 2, 3, 0, 0, 0, 0, 0, 0,
        4, 5, 6, 0, 0, 0, 0, 0, 0,
        7, 8, 9, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0};
    static constexpr SudokuMatrix<3> matrix2{sudokuGame};
    static_assert(matrix1 == matrix2);
}

TEST(DynamicSudokuMatrix, Initialization)
{
    DynamicSudokuMatrix matrix1 = CreateDynamicBoard();
    std::vector<DynamicSudokuMatrix::DataType> sudokuGame = {
        1, 2, 3, 0, 0, 0, 0, 0, 0,
        4, 5, 6, 0, 0, 0, 0, 0, 0,
        7, 8, 9, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0};
    DynamicSudokuMatrix matrix2{sudokuGame, 3};
    EXPECT_EQ(matrix1, matrix2);
}

TEST(SudokuMatrix, RemoveValue)
{
    static constexpr auto getBoard = []() -> SudokuMatrix<3>
    {
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

TEST(DynamicSudokuMatrix, RemoveValue)
{
    auto getBoard = []()
    {
        DynamicSudokuMatrix matrix = CreateDynamicBoard();
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
    DynamicSudokuMatrix matrix = getBoard();
    EXPECT_EQ(matrix.GetValue(0, 0), 0);
    EXPECT_EQ(matrix.GetValue(0, 1), 0);
    EXPECT_EQ(matrix.GetValue(0, 2), 0);
    EXPECT_EQ(matrix.GetValue(1, 0), 0);
    EXPECT_EQ(matrix.GetValue(1, 1), 0);
    EXPECT_EQ(matrix.GetValue(1, 2), 0);
    EXPECT_EQ(matrix.GetValue(2, 0), 0);
    EXPECT_EQ(matrix.GetValue(2, 1), 0);
    EXPECT_EQ(matrix.GetValue(2, 2), 0);
}

template<std::size_t N, template<std::size_t> class Solver, typename std::enable_if<std::is_base_of<ISolver<N>, Solver<N>>::value>::type * = nullptr>
inline constexpr bool SolveHardSudoku()
{
    constexpr std::array<typename Solver<N>::DataType, 81> sudokuGame = {
        0,0,0,0,0,0,0,0,0,
        0,9,0,0,1,0,0,3,0,
        0,0,6,0,2,0,7,0,0,
        0,0,0,3,0,4,0,0,0,
        2,1,0,0,0,0,0,9,8,
        0,0,0,0,0,0,0,0,0,
        0,0,2,5,0,6,4,0,0,
        0,8,0,0,0,0,0,1,0,
        0,0,0,0,0,0,0,0,0,
    };
    Solver<N> solver{std::move(sudokuGame)};
    std::size_t index = 0;
    while (solver.Advance())
    {
        index++;
    }
    return solver.IsSolved() && IsValidSudoku(solver.GetBoard());
}

template <std::size_t N, template <std::size_t> class Solver, typename std::enable_if<std::is_base_of<ISolver<N>, Solver<N>>::value>::type * = nullptr>
inline constexpr bool CanBeSolved()
{
    static_assert(N == 3, "Only 3x3 sudoku boards are supported");
    constexpr std::array<typename Solver<N>::DataType, 81> sudokuGame = {
        5, 3, 0, 0, 7, 0, 0, 0, 0,
        6, 0, 0, 1, 9, 5, 0, 0, 0,
        0, 9, 8, 0, 0, 0, 0, 6, 0,

        8, 0, 0, 0, 6, 0, 0, 0, 3,
        4, 0, 0, 8, 0, 3, 0, 0, 1,
        7, 0, 0, 0, 2, 0, 0, 0, 6,

        0, 6, 0, 0, 0, 0, 2, 8, 0,
        0, 0, 0, 4, 1, 9, 0, 0, 5,
        0, 0, 0, 0, 8, 0, 0, 7, 9};
    Solver<N> solver{std::move(sudokuGame)};
    if constexpr (std::is_same_v<Solver<N>, DLXSolver<N>>)
    {
        while (solver.Advance(false));
    }
    else
    {
        while (solver.Advance());
    }
    return solver.IsSolved() && IsValidSudoku(solver.GetBoard());
}

template <class Solver, typename std::enable_if<std::is_base_of<IDynamicSolver, Solver>::value>::type * = nullptr>
inline bool CanBeSolved()
{
    DynamicSudokuMatrix sudokuGameMatrix{{5, 3, 0, 0, 7, 0, 0, 0, 0,
                                          6, 0, 0, 1, 9, 5, 0, 0, 0,
                                          0, 9, 8, 0, 0, 0, 0, 6, 0,

                                          8, 0, 0, 0, 6, 0, 0, 0, 3,
                                          4, 0, 0, 8, 0, 3, 0, 0, 1,
                                          7, 0, 0, 0, 2, 0, 0, 0, 6,

                                          0, 6, 0, 0, 0, 0, 2, 8, 0,
                                          0, 0, 0, 4, 1, 9, 0, 0, 5,
                                          0, 0, 0, 0, 8, 0, 0, 7, 9},
                                         3};
    Solver solver{std::move(sudokuGameMatrix)};
    while (solver.Advance());
    return solver.IsSolved() && IsValidSudoku(solver.GetBoard());
}

TEST(SudokuMatrix, CanCreateRandomSudoku)
{
    std::random_device device;
    pcg64 rng{device()};
    static constexpr float probability = 0.25f;
    SudokuMatrix<3> data = CreateBoard<3>(probability, rng);
    EXPECT_TRUE(IsValidSudoku(data));
}

TEST(DynamicSudokuMatrix, CanCreateRandomSudoku)
{
    std::random_device device;
    pcg64 rng{device()};
    static constexpr float probability = 0.25f;
    DynamicSudokuMatrix data = CreateBoard(3, probability, rng);
    EXPECT_TRUE(IsValidSudoku(data));
}

TEST(SudokuMatrix, CheckBitSetIterator)
{
    static constexpr auto getIterator = [](int iterations)
    {
        BitSetIterator<3> it{0b101};
        for (int i = 0; i < iterations; ++i)
        {
            ++it;
        }
        return it;
    };
    static constexpr auto it = getIterator(0);
    static_assert(it.Count() == 2);
    static_assert(*it == 1);
    static constexpr auto it1 = getIterator(1);
    static_assert(it1.Count() == 1);
    static_assert(*it1 == 3);
    static constexpr auto it2 = getIterator(2);
    static_assert(it2.Count() == 0);
}

TEST(DynamicSudokuMatrix, CheckBitSetIterator)
{
    boost::dynamic_bitset<> bitset{3};
    bitset.set(0);
    bitset.set(2);
    DynamicBitSetIterator it{bitset};
    EXPECT_EQ(it.Count(), 2);
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(it.Count(), 1);
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(it.Count(), 0);
}

TEST(SudokuMatrix, GetPossibleValues)
{
    static constexpr auto getIterator = [](const SudokuMatrix<3> &matrix, int iterations)
    {
        auto it = matrix.GetPossibleValues(0, 3);
        for (int i = 0; i < iterations; ++i)
        {
            ++it;
        }
        return it;
    };
    static constexpr SudokuMatrix<3> matrix = CreateBoard();
    static constexpr auto it = getIterator(matrix, 0);
    static_assert(it.Count() == 6);
    static_assert(*it == 4);
    static constexpr auto it1 = getIterator(matrix, 1);
    static_assert(it1.Count() == 5);
    static_assert(*it1 == 5);
    static constexpr auto it2 = getIterator(matrix, 2);
    static_assert(it2.Count() == 4);
    static_assert(*it2 == 6);
    static constexpr auto it3 = getIterator(matrix, 3);
    static_assert(it3.Count() == 3);
    static_assert(*it3 == 7);
}

TEST(DynamicSudokuMatrix, GetPossibleValues)
{
    DynamicSudokuMatrix matrix = CreateDynamicBoard();
    auto it = matrix.GetPossibleValues(0, 3);
    EXPECT_EQ(it.Count(), 6);
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(it.Count(), 5);
    EXPECT_EQ(*it, 5);
    ++it;
    EXPECT_EQ(it.Count(), 4);
    EXPECT_EQ(*it, 6);
    ++it;
    EXPECT_EQ(it.Count(), 3);
    EXPECT_EQ(*it, 7);
}

TEST(BothSudoku, TestSquareIndexes)
{
    DynamicSudokuMatrix matrix(3);
    for (std::size_t row = 0; row < 9; ++row)
    {
        for (std::size_t col = 0; col < 9; ++col)
        {
            EXPECT_EQ(matrix.SquareIndex(row, col), SudokuMatrix<3>::SquareIndex(row, col));
        }
    }
}

TEST(BothSudoku, TestMatrixIndexes)
{
    DynamicSudokuMatrix matrix(3);
    for (std::size_t row = 0; row < 9; ++row)
    {
        for (std::size_t col = 0; col < 9; ++col)
        {
            EXPECT_EQ(matrix.MatrixIndex(row, col), SudokuMatrix<3>::MatrixIndex(row, col));
        }
    }
}

TEST(BothSudoku, TestSameValues)
{
    static constexpr SudokuMatrix<3> matrix1 = CreateBoard();
    DynamicSudokuMatrix matrix2 = CreateDynamicBoard();
    for (std::size_t row = 0; row < 9; ++row)
    {
        for (std::size_t col = 0; col < 9; ++col)
        {
            EXPECT_EQ(matrix1.GetValue(row, col), matrix2.GetValue(row, col));
        }
    }
}

TEST(BothSudoku, TestSameBitSets)
{
    static constexpr SudokuMatrix<3> matrix1 = CreateBoard();
    DynamicSudokuMatrix matrix2 = CreateDynamicBoard();
    const auto &bits1 = matrix1.GetBits();
    const auto &bits2 = matrix2.GetBits();
    if (bits1.size() != bits2.size())
    {
        FAIL();
    }
    for (std::size_t i = 0; i < bits1.size(); ++i)
    {
        EXPECT_EQ(bits1[i], bits2[i].to_ulong());
    }
}

TEST(SudokuMatrix, SolveSudokuBackTracking)
{
    bool solved = CanBeSolved<3, BackTrackingSolver>();
    EXPECT_TRUE(solved);
}

TEST(DynamicSudokuMatrix, SolveSudokuBackTracking)
{
    EXPECT_TRUE(CanBeSolved<DynamicBackTrackingSolver>());
}

TEST(SudokuMatrix, SolveSudokuDlx)
{
    bool solved = CanBeSolved<3, DLXSolver>();
    EXPECT_TRUE(solved);
}

TEST(SudokuMatrix, SolveHardSudokuBackTracking)
{
    bool solved = SolveHardSudoku<3, BackTrackingSolver>();
    EXPECT_TRUE(solved);
}

TEST(SudokuMatrix, SolveHardSudokuDlx)
{
    bool solved = SolveHardSudoku<3, DLXSolver>();
    EXPECT_TRUE(solved);
}