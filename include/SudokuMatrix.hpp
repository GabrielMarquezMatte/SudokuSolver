#pragma once
#include "./SudokuBits.hpp"
#include <bit>

template <std::size_t N>
struct PossibleValuesIterator
{
private:
    using FlagType = std::conditional_t<(N * N <= 8), std::uint8_t,
                                        std::conditional_t<(N * N <= 16), std::uint16_t,
                                                           std::conditional_t<(N * N <= 32), std::uint32_t, std::uint64_t>>>;
    FlagType m_flag;

public:
    constexpr PossibleValuesIterator(FlagType flag) : m_flag(flag) {}
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
        FlagType newValue = m_flag & -static_cast<std::make_signed_t<FlagType>>(m_flag);
        int count = std::countr_zero(newValue);
        return static_cast<char>(count + 1);
    }
    inline constexpr bool operator!=(const PossibleValuesIterator<N> &other) const
    {
        return m_flag != other.m_flag;
    }
    inline constexpr bool operator==(const FlagType value) const
    {
        return m_flag == value;
    }
    inline constexpr PossibleValuesIterator<N> begin()
    {
        return {m_flag};
    }
    static inline constexpr PossibleValuesIterator<N> end()
    {
        return {0};
    }
    inline constexpr int Count() const
    {
        return std::popcount(m_flag);
    }
};

template <std::size_t N>
class SudokuMatrix
{
private:
    std::array<char, N * N * N * N> m_data;
    SudokuBits<N> m_dataBits;

public:
    static inline constexpr std::size_t SquareIndex(std::size_t row, std::size_t col)
    {
        constexpr std::size_t squareSize = N;
        std::size_t squareRow = row / squareSize;
        std::size_t squareCol = col / squareSize;
        return squareRow * squareSize + squareCol;
    }
    static inline constexpr std::size_t MatrixIndex(const std::size_t row, const std::size_t col)
    {
        constexpr std::size_t rowSize = N * N;
        return row * rowSize + col;
    }

    constexpr SudokuMatrix() : m_dataBits({}), m_data({})
    {
        m_data.fill(0);
    }

    constexpr SudokuMatrix(const std::array<char, N * N * N * N> &data) : m_data(data), m_dataBits({})
    {
        constexpr std::size_t size = N * N;
        // Inicializa os bitsets para marcar os valores presentes na matriz inicial
        for (std::size_t row = 0; row < size; ++row)
        {
            for (std::size_t col = 0; col < size; ++col)
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

    constexpr SudokuMatrix(const SudokuMatrix<N> &other)
        : m_data(other.m_data),
          m_dataBits(other.m_dataBits)
    {
    }

    constexpr SudokuMatrix(SudokuMatrix<N> &&other) noexcept
        : m_data(std::move(other.m_data)),
          m_dataBits(std::move(other.m_dataBits))
    {
    }

    constexpr SudokuMatrix<N> &operator=(const SudokuMatrix<N> &other)
    {
        if (this == &other)
        {
            return *this;
        }
        m_data = other.m_data;
        m_dataBits = other.m_dataBits;
        return *this;
    }

    constexpr SudokuMatrix<N> &operator=(SudokuMatrix<N> &&other) noexcept
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

    inline constexpr PossibleValuesIterator<N> GetPossibleValues(std::size_t row, std::size_t col, std::size_t squareIndex) const
    {
        return {m_dataBits.GetAvailableValues(row, col, squareIndex)};
    }

    inline constexpr PossibleValuesIterator<N> GetPossibleValues(std::size_t row, std::size_t col) const
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