#pragma once
#include <array>
struct SudokuBits
{
private:
    std::array<std::uint16_t, 27> m_bits;
public:
    inline constexpr void SetValue(std::size_t row, std::size_t col, std::size_t square, char value)
    {
        std::uint16_t mask = 1 << (value - 1);
        m_bits[row] |= mask;
        m_bits[9 + col] |= mask;
        m_bits[18 + square] |= mask;
    }

    inline constexpr void ResetValue(std::size_t row, std::size_t col, std::size_t square, char value)
    {
        std::uint16_t mask = ~(1 << (value - 1));
        m_bits[row] &= mask;
        m_bits[9 + col] &= mask;
        m_bits[18 + square] &= mask;
    }

    inline constexpr bool Test(std::size_t row, std::size_t col, std::size_t square, char value) const
    {
        std::uint16_t mask = 1 << (value - 1);
        return (m_bits[row] & mask) != 0 && (m_bits[9 + col] & mask) != 0 && (m_bits[18 + square] & mask) != 0;
    }

    inline constexpr std::uint16_t GetAvailableValues(std::size_t row, std::size_t col, std::size_t square) const
    {
        std::uint16_t usedBits = m_bits[row] | m_bits[9 + col] | m_bits[18 + square];
        return static_cast<std::uint16_t>(~usedBits & 0x1FF);
    }
};