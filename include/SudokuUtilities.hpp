#pragma once
#include <random>
#include <pcg_random.hpp>
#include "./SudokuMatrix.hpp"

template <std::size_t N>
SudokuMatrix<N> CreateBoard(const float probabilityOfFilled, pcg64 &randomDevice)
{
    SudokuMatrix<N> board = {};
    // Gerador de números aleatórios
    std::uniform_real_distribution<float> probabilityDist(0.0f, 1.0f);
    constexpr std::size_t size = N * N;
    // Preenche aleatoriamente algumas células do tabuleiro
    for (std::size_t row = 0; row < size; ++row)
    {
        for (std::size_t col = 0; col < size; ++col)
        {
            if (probabilityDist(randomDevice) >= probabilityOfFilled)
            {
                continue;
            }
            auto possibleValues = board.GetPossibleValues(row, col);
            if (possibleValues == 0)
            {
                continue;
            }
            int count = possibleValues.Count();
            std::uniform_int_distribution<int> indexDist(0, count - 1);
            int index = indexDist(randomDevice);
            for (int i = 0; i < index; ++i)
            {
                ++possibleValues;
            }
            char value = *possibleValues;
            board.SetValue(row, col, value);
        }
    }

    return board;
}

template<std::size_t N>
constexpr bool IsValidSudoku(const SudokuMatrix<N> &board)
{
    constexpr std::size_t size = N * N;
    SudokuMatrix<N> testBoard = {};
    for (std::size_t row = 0; row < size; ++row)
    {
        for (std::size_t col = 0; col < size; ++col)
        {
            char value = board.GetValue(row, col);
            if (value == 0)
            {
                continue;
            }
            if (!testBoard.IsValidPlay(value, row, col))
            {
                return false;
            }
            testBoard.SetValue(row, col, value);
        }
    }
    return true;
}