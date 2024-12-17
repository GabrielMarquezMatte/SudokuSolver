#pragma once
#include <array>

template<std::size_t N>
struct SudokuBits
{
private:
    using FlagType = std::conditional_t<(N * N <= 8), std::uint8_t,
            std::conditional_t<(N * N <= 16), std::uint16_t,
                std::conditional_t<(N * N <= 32), std::uint32_t, std::uint64_t>>>;
    std::array<FlagType, N * N * 3> m_bits;
    static constexpr std::size_t size = N * N;
    static constexpr FlagType AllBitsSet = (1 << size) - 1;
public:
    inline constexpr void SetValue(std::size_t row, std::size_t col, std::size_t square, char value)
    {
        FlagType mask = 1 << (value - 1);
        m_bits[row] |= mask;
        m_bits[size + col] |= mask;
        m_bits[size * 2 + square] |= mask;
    }

    inline constexpr void ResetValue(std::size_t row, std::size_t col, std::size_t square, char value)
    {
        FlagType mask = ~(1 << (value - 1));
        m_bits[row] &= mask;
        m_bits[size + col] &= mask;
        m_bits[size * 2 + square] &= mask;
    }

    inline constexpr bool Test(std::size_t row, std::size_t col, std::size_t square, char value) const
    {
        FlagType mask = 1 << (value - 1);
        return (m_bits[row] & mask) != 0 && (m_bits[size + col] & mask) != 0 && (m_bits[size * 2 + square] & mask) != 0;
    }

    inline constexpr FlagType GetAvailableValues(std::size_t row, std::size_t col, std::size_t square) const
    {
        FlagType usedBits = m_bits[row] | m_bits[size + col] | m_bits[size * 2 + square];
        return static_cast<FlagType>(~usedBits & AllBitsSet);
    }
};