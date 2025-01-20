#pragma once
#include "./StateMachineStatus.hpp"
#include "../SudokuMatrix.hpp"
#include "./ISolver.hpp"

template <std::size_t N>
class BackTrackingSolver : public ISolver<N>
{
public:
    using DataType = typename SudokuMatrix<N>::DataType;
private:
    SudokuMatrix<N> m_data;
    std::size_t m_currentRow = 0;
    std::size_t m_currentCol = 0;
    AdvanceResult m_currentState = AdvanceResult::Continue;
    bool m_solved = false;
    inline constexpr bool AdvanceToNextCell()
    {
        constexpr std::size_t size = N * N;
        if (m_currentCol == size - 1 && m_currentRow == size - 1)
        {
            m_currentState = AdvanceResult::Finished;
            m_solved = true;
            return false;
        }
        if (++m_currentCol == size)
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
            constexpr std::size_t size = N * N;
            m_currentCol = size - 1;
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
    constexpr BackTrackingSolver(const SudokuMatrix<N> &data) : m_data(data) {}
    constexpr BackTrackingSolver(SudokuMatrix<N> &&data) : m_data(std::move(data)) {}
    constexpr bool Advance() override
    {
        if (m_solved)
        {
            m_currentState = AdvanceResult::Finished;
            return false;
        }
        constexpr std::size_t size = N * N;
        if (m_currentRow == size)
        {
            m_solved = true;
            m_currentState = AdvanceResult::Finished;
            return false;
        }
        std::size_t index = SudokuMatrix<N>::MatrixIndex(m_currentRow, m_currentCol);
        if (m_currentState == AdvanceResult::BackTracking)
        {
            DataType value = m_data.GetValue(index) + 1;
            std::size_t squareIndex = SudokuMatrix<N>::SquareIndex(m_currentRow, m_currentCol);
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
        DataType inSpot = m_data.GetValue(index);
        if (inSpot != 0)
        {
            return Continue();
        }
        std::size_t squareIndex = SudokuMatrix<N>::SquareIndex(m_currentRow, m_currentCol);
        auto possibleValues = m_data.GetPossibleValues(m_currentRow, m_currentCol, squareIndex);
        if (!possibleValues.Any())
        {
            return BackTrack();
        }
        m_data.SetValue(m_currentRow, m_currentCol, index, squareIndex, *possibleValues);
        return Continue();
    }
    inline constexpr AdvanceResult GetStatus() const noexcept override
    {
        return m_currentState;
    }
    inline constexpr const SudokuMatrix<N> &GetBoard() const noexcept override
    {
        return m_data;
    }
    inline constexpr bool IsSolved() const noexcept override
    {
        return m_solved;
    }
};

class DynamicBackTrackingSolver : public IDynamicSolver
{
public:
    using DataType = typename DynamicSudokuMatrix::DataType;
private:
    DynamicSudokuMatrix m_data;
    std::size_t m_currentRow = 0;
    std::size_t m_currentCol = 0;
    std::size_t m_squaredSize;
    AdvanceResult m_currentState = AdvanceResult::Continue;
    bool m_solved = false;
    inline bool AdvanceToNextCell()
    {
        std::size_t size = m_squaredSize;
        if (m_currentCol == size - 1 && m_currentRow == size - 1)
        {
            m_currentState = AdvanceResult::Finished;
            m_solved = true;
            return false;
        }
        if (++m_currentCol == size)
        {
            m_currentCol = 0;
            m_currentRow++;
        }
        return true;
    }
    inline bool RetreatToPreviousCell()
    {
        if (m_currentCol == 0 && m_currentRow == 0)
        {
            m_currentState = AdvanceResult::Finished;
            m_solved = false;
            return false;
        }
        if (m_currentCol == 0 && m_currentRow != 0)
        {
            std::size_t size = m_squaredSize;
            m_currentCol = size - 1;
            m_currentRow--;
            return true;
        }
        if (m_currentCol != 0)
        {
            m_currentCol--;
        }
        return true;
    }

    inline bool Continue()
    {
        m_currentState = AdvanceResult::Continue;
        return AdvanceToNextCell();
    }

    inline bool BackTrack()
    {
        m_currentState = AdvanceResult::BackTracking;
        return RetreatToPreviousCell();
    }

public:
    DynamicBackTrackingSolver(std::size_t size) : m_data(size), m_squaredSize(size * size) {}
    DynamicBackTrackingSolver(const DynamicSudokuMatrix &data) : m_data(data), m_squaredSize(m_data.GetSize() * m_data.GetSize()) {}
    DynamicBackTrackingSolver(DynamicSudokuMatrix &&data) : m_data(std::move(data)), m_squaredSize(m_data.GetSize() * m_data.GetSize()) {}
    bool Advance() override
    {
        if (m_solved)
        {
            m_currentState = AdvanceResult::Finished;
            return false;
        }
        std::size_t size = m_squaredSize;
        if (m_currentRow == size)
        {
            m_solved = true;
            m_currentState = AdvanceResult::Finished;
            return false;
        }
        std::size_t index = m_data.MatrixIndex(m_currentRow, m_currentCol);
        if (m_currentState == AdvanceResult::BackTracking)
        {
            DataType value = m_data.GetValue(index) + 1;
            std::size_t squareIndex = m_data.SquareIndex(m_currentRow, m_currentCol);
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
        DataType inSpot = m_data.GetValue(index);
        if (inSpot != 0)
        {
            return Continue();
        }
        std::size_t squareIndex = m_data.SquareIndex(m_currentRow, m_currentCol);
        auto possibleValues = m_data.GetPossibleValues(m_currentRow, m_currentCol, squareIndex);
        if (possibleValues.Count() == 0)
        {
            return BackTrack();
        }
        m_data.SetValue(m_currentRow, m_currentCol, index, squareIndex, *possibleValues);
        return Continue();
    }
    inline constexpr AdvanceResult GetStatus() const noexcept override
    {
        return m_currentState;
    }
    inline constexpr const DynamicSudokuMatrix &GetBoard() const noexcept override
    {
        return m_data;
    }
    inline constexpr bool IsSolved() const noexcept override
    {
        return m_solved;
    }
};