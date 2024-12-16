#pragma once
#include "../SudokuMatrix.hpp"
enum class AdvanceResult
{
    Continue,
    BackTracking,
    Finished
};
class BackTrackingSolver
{
private:
    SudokuMatrix m_data;
    std::size_t m_currentRow = 0;
    std::size_t m_currentCol = 0;
    AdvanceResult m_currentState = AdvanceResult::Continue;
    bool m_solved = false;
    inline constexpr bool AdvanceToNextCell()
    {
        if (m_currentCol == SudokuColSize - 1 && m_currentRow == SudokuRowSize - 1)
        {
            m_currentState = AdvanceResult::Finished;
            m_solved = true;
            return false;
        }
        if (++m_currentCol == SudokuColSize)
        {
            m_currentCol = 0;
            m_currentRow++;
        }
        return true;
    }
    inline constexpr bool RetreatToPreviousCell()
    {
        if (m_currentCol == 0 && m_currentRow == 0)
        {
            m_currentState = AdvanceResult::Finished;
            m_solved = false;
            return false;
        }
        if (m_currentCol == 0 && m_currentRow != 0)
        {
            m_currentCol = SudokuColSize - 1;
            m_currentRow--;
            return true;
        }
        if (m_currentCol != 0)
        {
            m_currentCol--;
        }
        return true;
    }

    inline constexpr bool Continue()
    {
        m_currentState = AdvanceResult::Continue;
        return AdvanceToNextCell();
    }

    inline constexpr bool BackTrack()
    {
        m_currentState = AdvanceResult::BackTracking;
        return RetreatToPreviousCell();
    }

public:
    constexpr BackTrackingSolver() : m_data{} {}
    constexpr BackTrackingSolver(const SudokuMatrix &data) : m_data(data) {}
    constexpr BackTrackingSolver(SudokuMatrix &&data) : m_data(std::move(data)) {}
    constexpr bool Advance()
    {
        if (m_solved)
        {
            m_currentState = AdvanceResult::Finished;
            return false;
        }
        if (m_currentRow == SudokuRowSize)
        {
            m_solved = true;
            m_currentState = AdvanceResult::Finished;
            return false;
        }
        std::size_t index = SudokuMatrix::MatrixIndex(m_currentRow, m_currentCol);
        if (m_currentState == AdvanceResult::BackTracking)
        {
            char value = m_data.GetValue(index) + 1;
            std::size_t squareIndex = SudokuMatrix::SquareIndex(m_currentRow, m_currentCol);
            m_data.RemoveValue(m_currentRow, m_currentCol, index, squareIndex);
            auto possibleValues = m_data.GetPossibleValues(m_currentRow, m_currentCol, squareIndex);
            for (auto possibility : possibleValues)
            {
                if (possibility >= value)
                {
                    m_data.SetValue(m_currentRow, m_currentCol, index, squareIndex, possibility);
                    return Continue();
                }
            }
            return BackTrack();
        }
        char inSpot = m_data.GetValue(index);
        if (inSpot != 0)
        {
            return Continue();
        }
        std::size_t squareIndex = SudokuMatrix::SquareIndex(m_currentRow, m_currentCol);
        auto possibleValues = m_data.GetPossibleValues(m_currentRow, m_currentCol, squareIndex);
        if (possibleValues == 0)
        {
            return BackTrack();
        }
        m_data.SetValue(m_currentRow, m_currentCol, index, squareIndex, *possibleValues);
        return Continue();
    }
    constexpr bool Retreat()
    {
        // Se já estamos no início da resolução, não podemos retroceder mais
        if (m_currentRow == 0 && m_currentCol == 0 && m_data.GetValue(m_currentRow, m_currentCol) == 0)
        {
            return false; // Não há mais nada para retroceder
        }

        m_data.RemoveValue(m_currentRow, m_currentCol);
        // Retrocede para a célula anterior
        RetreatToPreviousCell();
        if (m_currentRow == 0 && m_currentCol == 0)
        {
            return false;
        }
        m_currentState = AdvanceResult::BackTracking;
        return true; // Ainda há células para retroceder
    }
    inline constexpr const AdvanceResult GetStatus() const
    {
        return m_currentState;
    }
    inline constexpr const SudokuMatrix &GetBoard() const
    {
        return m_data;
    }
    inline constexpr bool IsSolved() const
    {
        return m_solved;
    }
};