#include <random>
#include <pcg_random.hpp>
#include "./SudokuMatrix.hpp"
SudokuMatrix CreateBoard(const float probabilityOfFilled, pcg64 &randomDevice)
{
    SudokuMatrix board = {};
    // Gerador de números aleatórios
    std::uniform_real_distribution<float> probabilityDist(0.0f, 1.0f);

    // Preenche aleatoriamente algumas células do tabuleiro
    for (std::size_t row = 0; row < SudokuRowSize; ++row)
    {
        for (std::size_t col = 0; col < SudokuColSize; ++col)
        {
            if (probabilityDist(randomDevice) >= probabilityOfFilled)
            {
                continue;
            }
            SudokuMatrix::PossibleValuesIterator possibleValues = board.GetPossibleValues(row, col);
            int count = possibleValues.Count();
            if (count == 0)
            {
                continue;
            }
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

SudokuMatrix CreateBoard2(const float probabilityOfFilled, pcg64 &randomDevice)
{
    SudokuMatrix board = {};
    // Gerador de números aleatórios
    std::uniform_real_distribution<float> probabilityDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> valueDist(1, 9);

    // Preenche aleatoriamente algumas células do tabuleiro
    for (std::size_t row = 0; row < SudokuRowSize; ++row)
    {
        for (std::size_t col = 0; col < SudokuColSize; ++col)
        {
            if (probabilityDist(randomDevice) >= probabilityOfFilled)
            {
                continue;
            }
            for (int attempts = 0; attempts < 10; ++attempts) // Limita as tentativas para evitar loops infinitos
            {
                char value = static_cast<char>(valueDist(randomDevice));
                if (board.IsValidPlay(value, row, col))
                {
                    board.SetValue(row, col, value);
                }
            }
        }
    }

    return board;
}