#pragma once
#include "./SudokuBits.hpp"
#include "./constants.hpp"
#include <bit>
class SudokuMatrix
{
private:
    std::array<char, SudokuSize> m_data;
    SudokuBits m_dataBits;


public:
    static inline constexpr std::size_t SquareIndex(std::size_t row, std::size_t col)
    {
        std::size_t squareRow = row / SudokuSquareSize;
        std::size_t squareCol = col / SudokuSquareSize;
        return squareRow * SudokuSquareSize + squareCol;
    }
    static inline constexpr std::size_t MatrixIndex(const std::size_t row, const std::size_t col)
    {
        return row * SudokuRowSize + col;
    }

    constexpr SudokuMatrix() : m_dataBits({}), m_data({})
    {
        m_data.fill(0);
    }

    constexpr SudokuMatrix(const std::array<char, SudokuSize> &data) : m_data(data), m_dataBits({})
    {
        // Inicializa os bitsets para marcar os valores presentes na matriz inicial
        for (std::size_t row = 0; row < SudokuRowSize; ++row)
        {
            for (std::size_t col = 0; col < SudokuColSize; ++col)
            {
                char value = data[MatrixIndex(row, col)];
                if (value == 0)
                {
                    continue;
                }
                std::size_t squareIndex = SquareIndex(row, col);
                // Marca os valores nos bitsets correspondentes
                m_dataBits.SetValue(row, col, squareIndex, value);
            }
        }
    }

    constexpr SudokuMatrix(const SudokuMatrix &other)
        : m_data(other.m_data),
        m_dataBits(other.m_dataBits)
    {
    }

    constexpr SudokuMatrix(SudokuMatrix &&other) noexcept
        : m_data(std::move(other.m_data)),
          m_dataBits(std::move(other.m_dataBits))
    {
    }

    constexpr SudokuMatrix &operator=(const SudokuMatrix &other)
    {
        if (this == &other)
        {
            return *this;
        }
        m_data = other.m_data;
        m_dataBits = other.m_dataBits;
        return *this;
    }

    constexpr SudokuMatrix &operator=(SudokuMatrix &&other) noexcept
    {
        if (this != &other) // Evita a auto-atribuição
        {
            m_data = std::move(other.m_data);
            m_dataBits = std::move(other.m_dataBits);
        }
        return *this;
    }

    inline constexpr char GetValue(std::size_t row, std::size_t col) const
    {
        return m_data[MatrixIndex(row, col)];
    }

    inline constexpr char GetValue(std::size_t index) const
    {
        return m_data[index];
    }

    inline constexpr void SetValue(std::size_t row, std::size_t col, std::size_t index, std::size_t squareIndex, char value)
    {
        // Remove o valor anterior, se houver
        char &oldValue = m_data[index];
        if (oldValue != 0)
        {
            m_dataBits.ResetValue(row, col, squareIndex, oldValue);
        }

        // Define o novo valor
        oldValue = value;

        // Atualiza os bitsets se o valor não for zero
        if (value != 0)
        {
            m_dataBits.SetValue(row, col, squareIndex, value);
        }
    }

    inline constexpr void SetValue(std::size_t row, std::size_t col, char value)
    {
        std::size_t index = MatrixIndex(row, col);
        std::size_t squareIndex = SquareIndex(row, col);
        return SetValue(row, col, index, squareIndex, value);
    }

    inline constexpr bool IsValidPlay(char value, std::size_t row, std::size_t col, std::size_t squareIndex) const
    {
        return !m_dataBits.Test(row, col, squareIndex, value);
    }

    inline constexpr bool IsValidPlay(char value, std::size_t row, std::size_t col) const
    {
        return IsValidPlay(value, row, col, SquareIndex(row, col));
    }

    struct PossibleValuesIterator
    {
    private:
        std::uint16_t m_flag;

    public:
        constexpr PossibleValuesIterator(std::uint16_t flag) : m_flag(flag) {}
        // Iterator functions
        inline constexpr PossibleValuesIterator &operator++()
        {
            // Remove the least significant set bit
            m_flag &= (m_flag - 1);
            return *this;
        }
        inline constexpr char operator*() const
        {
            // Get the least significant set bit
            std::uint16_t newValue = m_flag & -static_cast<std::int16_t>(m_flag);
            int count = std::countr_zero(newValue);
            return static_cast<char>(count + 1);
        }
        inline constexpr bool operator!=(const PossibleValuesIterator &other) const
        {
            return m_flag != other.m_flag;
        }
        inline constexpr bool operator==(const std::uint16_t value) const
        {
            return m_flag == value;
        }
        inline constexpr PossibleValuesIterator begin()
        {
            return {m_flag};
        }
        static inline constexpr PossibleValuesIterator end()
        {
            return {0};
        }
        inline constexpr int Count() const
        {
            return std::popcount(m_flag);
        }
    };

    inline constexpr PossibleValuesIterator GetPossibleValues(std::size_t row, std::size_t col, std::size_t squareIndex) const
    {
        return {m_dataBits.GetAvailableValues(row, col, squareIndex)};
    }

    inline constexpr PossibleValuesIterator GetPossibleValues(std::size_t row, std::size_t col) const
    {
        return GetPossibleValues(row, col, SquareIndex(row, col));
    }

    inline constexpr void RemoveValue(std::size_t row, std::size_t col, std::size_t index, std::size_t squareIndex)
    {
        SetValue(row, col, index, squareIndex, 0); // Define o valor como zero, removendo-o
    }

    inline constexpr void RemoveValue(std::size_t row, std::size_t col, std::size_t index)
    {
        std::size_t squareIndex = SquareIndex(row, col);
        SetValue(row, col, index, squareIndex, 0); // Define o valor como zero, removendo-o
    }

    inline constexpr void RemoveValue(std::size_t row, std::size_t col)
    {
        SetValue(row, col, 0); // Define o valor como zero, removendo-o
    }

};