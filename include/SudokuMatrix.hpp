#pragma once
#include "./SudokuBits.hpp"
#include <cmath>

template <std::size_t N>
class SudokuMatrix
{
public:
    using DataType = typename BitSetIterator<N>::DataType;

private:
    std::array<DataType, N * N * N * N> m_data;
    SudokuBits<N> m_dataBits;

public:
    static inline constexpr std::size_t SquareIndex(const std::size_t row, const std::size_t col)
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

    constexpr SudokuMatrix() : m_data({}), m_dataBits({})
    {
        m_data.fill(0);
    }

    constexpr SudokuMatrix(const std::array<DataType, N * N * N * N> &data) : m_data(data), m_dataBits({})
    {
        constexpr std::size_t size = N * N;
        // Inicializa os bitsets para marcar os valores presentes na matriz inicial
        for (std::size_t row = 0; row < size; ++row)
        {
            for (std::size_t col = 0; col < size; ++col)
            {
                DataType value = data[MatrixIndex(row, col)];
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

    constexpr SudokuMatrix(std::array<DataType, N * N * N * N> &&data) : m_data(std::move(data)), m_dataBits({})
    {
        constexpr std::size_t size = N * N;
        // Inicializa os bitsets para marcar os valores presentes na matriz inicial
        for (std::size_t row = 0; row < size; ++row)
        {
            for (std::size_t col = 0; col < size; ++col)
            {
                DataType value = m_data[MatrixIndex(row, col)];
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

    inline constexpr bool operator==(const SudokuMatrix<N> &other) const noexcept
    {
        return m_data == other.m_data;
    }

    inline constexpr DataType GetValue(std::size_t row, std::size_t col) const
    {
        return m_data[MatrixIndex(row, col)];
    }

    inline constexpr DataType GetValue(std::size_t index) const
    {
        return m_data[index];
    }

    inline constexpr void SetValue(std::size_t row, std::size_t col, std::size_t index, std::size_t squareIndex, DataType value)
    {
        // Remove o valor anterior, se houver
        DataType &oldValue = m_data[index];
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

    inline constexpr void SetValue(std::size_t row, std::size_t col, DataType value)
    {
        std::size_t index = MatrixIndex(row, col);
        std::size_t squareIndex = SquareIndex(row, col);
        return SetValue(row, col, index, squareIndex, value);
    }

    inline constexpr bool IsValidPlay(DataType value, std::size_t row, std::size_t col, std::size_t squareIndex) const
    {
        return !m_dataBits.Test(row, col, squareIndex, value);
    }

    inline constexpr bool IsValidPlay(DataType value, std::size_t row, std::size_t col) const
    {
        return IsValidPlay(value, row, col, SquareIndex(row, col));
    }

    inline constexpr BitSetIterator<N> GetPossibleValues(std::size_t row, std::size_t col, std::size_t squareIndex) const
    {
        return {m_dataBits.GetAvailableValues(row, col, squareIndex)};
    }

    inline constexpr BitSetIterator<N> GetPossibleValues(std::size_t row, std::size_t col) const
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

    inline constexpr const std::array<typename SudokuBits<N>::FlagType, N * N * 3> &GetBits() const noexcept
    {
        return m_dataBits.GetBits();
    }
};

class DynamicSudokuMatrix
{
public:
    using DataType = DynamicBitSetIterator::DataType;
    inline constexpr std::size_t MatrixIndex(const std::size_t row, const std::size_t col) const noexcept
    {
        return row * m_rowSize + col;
    }
    inline constexpr std::size_t SquareIndex(const std::size_t row, const std::size_t col) const noexcept
    {
        std::size_t squareRow = row / m_size;
        std::size_t squareCol = col / m_size;
        return squareRow * m_size + squareCol;
    }

private:
    std::size_t m_rowSize;
    std::size_t m_size;
    std::vector<DataType> m_data;
    SudokuDynamicBits m_dataBits;

    void InitializeData()
    {
        for (std::size_t row = 0; row < m_rowSize; ++row)
        {
            for (std::size_t col = 0; col < m_rowSize; ++col)
            {
                DataType value = m_data[MatrixIndex(row, col)];
                if (value == 0)
                {
                    continue;
                }
                std::size_t squareIndex = SquareIndex(row, col);
                m_dataBits.SetValue(row, col, squareIndex, value);
            }
        }
    }

public:
    DynamicSudokuMatrix(std::size_t size) : m_rowSize(size * size), m_size(size), m_data(size * size * size * size, static_cast<DataType>(0)), m_dataBits(size)
    {
    }

    DynamicSudokuMatrix(std::initializer_list<DataType> data, std::size_t size) : m_rowSize(size * size), m_size(size), m_data(data), m_dataBits(size)
    {
        InitializeData();
    }

    DynamicSudokuMatrix(const std::vector<DataType> &data, std::size_t size) : m_rowSize(size * size), m_size(size), m_data(data), m_dataBits(size)
    {
        InitializeData();
    }

    DynamicSudokuMatrix(std::vector<DataType> &&data, std::size_t size) : m_rowSize(size * size), m_size(size), m_data(std::move(data)), m_dataBits(size)
    {
        InitializeData();
    }

    DynamicSudokuMatrix(const DynamicSudokuMatrix &other)
        : m_rowSize(other.m_rowSize),
          m_size(other.m_size),
          m_data(other.m_data),
          m_dataBits(other.m_dataBits)
    {
    }

    DynamicSudokuMatrix(DynamicSudokuMatrix &&other) noexcept
        : m_rowSize(other.m_rowSize),
          m_size(other.m_size),
          m_data(std::move(other.m_data)),
          m_dataBits(std::move(other.m_dataBits))
    {
    }

    DynamicSudokuMatrix &operator=(const DynamicSudokuMatrix &other)
    {
        if (this == &other)
        {
            return *this;
        }
        m_rowSize = other.m_rowSize;
        m_size = other.m_size;
        m_data = other.m_data;
        m_dataBits = other.m_dataBits;
        return *this;
    }

    DynamicSudokuMatrix &operator=(DynamicSudokuMatrix &&other) noexcept
    {
        if (this != &other) // Evita a auto-atribuição
        {
            m_rowSize = other.m_rowSize;
            m_size = other.m_size;
            m_data = std::move(other.m_data);
            m_dataBits = std::move(other.m_dataBits);
        }
        return *this;
    }

    inline bool operator==(const DynamicSudokuMatrix &other) const noexcept
    {
        return m_data == other.m_data;
    }

    inline DataType GetValue(std::size_t row, std::size_t col) const
    {
        return m_data[MatrixIndex(row, col)];
    }

    inline DataType GetValue(std::size_t index)
    {
        return m_data[index];
    }

    inline void SetValue(std::size_t row, std::size_t col, std::size_t index, std::size_t squareIndex, DataType value)
    {
        DataType &oldValue = m_data[index];
        if (oldValue != 0)
        {
            m_dataBits.ResetValue(row, col, squareIndex, oldValue);
        }
        oldValue = value;
        if (value != 0)
        {
            m_dataBits.SetValue(row, col, squareIndex, value);
        }
    }

    inline void SetValue(std::size_t row, std::size_t col, DataType value)
    {
        std::size_t index = MatrixIndex(row, col);
        std::size_t squareIndex = SquareIndex(row, col);
        return SetValue(row, col, index, squareIndex, value);
    }

    inline bool IsValidPlay(DataType value, std::size_t row, std::size_t col, std::size_t squareIndex) const
    {
        return !m_dataBits.Test(row, col, squareIndex, value);
    }

    inline bool IsValidPlay(DataType value, std::size_t row, std::size_t col) const
    {
        return IsValidPlay(value, row, col, SquareIndex(row, col));
    }

    inline DynamicBitSetIterator GetPossibleValues(std::size_t row, std::size_t col, std::size_t squareIndex) const
    {
        return {m_dataBits.GetAvailableValues(row, col, squareIndex)};
    }

    inline DynamicBitSetIterator GetPossibleValues(std::size_t row, std::size_t col) const
    {
        return GetPossibleValues(row, col, SquareIndex(row, col));
    }

    inline void RemoveValue(std::size_t row, std::size_t col, std::size_t index, std::size_t squareIndex)
    {
        SetValue(row, col, index, squareIndex, 0);
    }

    inline void RemoveValue(std::size_t row, std::size_t col, std::size_t index)
    {
        std::size_t squareIndex = SquareIndex(row, col);
        SetValue(row, col, index, squareIndex, 0);
    }

    inline void RemoveValue(std::size_t row, std::size_t col)
    {
        SetValue(row, col, 0);
    }

    inline std::size_t GetSize() const noexcept
    {
        return m_size;
    }

    inline const std::vector<boost::dynamic_bitset<>> &GetBits() const noexcept
    {
        return m_dataBits.GetBits();
    }
};