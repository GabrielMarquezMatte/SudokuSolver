#pragma once
#include <array>
#include <bit>

template <std::size_t N>
struct BitSetIterator
{
public:
    using FlagType = std::conditional_t<(N * N <= 8), std::uint8_t,
                                        std::conditional_t<(N * N <= 16), std::uint16_t,
                                                           std::conditional_t<(N * N <= 32), std::uint32_t, std::uint64_t>>>;
private:
    FlagType m_flag;

public:
    constexpr BitSetIterator(FlagType flag) : m_flag(flag) {}
    // Iterator functions
    inline constexpr BitSetIterator &operator++()
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
    inline constexpr bool operator!=(const BitSetIterator<N> &other) const
    {
        return m_flag != other.m_flag;
    }
    inline constexpr bool operator==(const FlagType value) const
    {
        return m_flag == value;
    }
    inline constexpr BitSetIterator<N> begin()
    {
        return {m_flag};
    }
    static inline constexpr BitSetIterator<N> end()
    {
        return {0};
    }
    inline constexpr int Count() const
    {
        return std::popcount(m_flag);
    }
};

template<std::size_t N>
struct SudokuBits
{
private:
    using FlagType = BitSetIterator<N>::FlagType;
    std::array<FlagType, N * N * 3> m_bits;
    static constexpr std::size_t size = N * N;
    static constexpr FlagType AllBitsSet = (1ULL << size) - 1;
public:
    inline constexpr void SetValue(std::size_t row, std::size_t col, std::size_t square, char value)
    {
        FlagType mask = 1ULL << (value - 1);
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
        FlagType mask = 1ULL << (value - 1);
        return (m_bits[row] & mask) != 0 && (m_bits[size + col] & mask) != 0 && (m_bits[size * 2 + square] & mask) != 0;
    }

    inline constexpr FlagType GetAvailableValues(std::size_t row, std::size_t col, std::size_t square) const
    {
        FlagType usedBits = m_bits[row] | m_bits[size + col] | m_bits[size * 2 + square];
        return static_cast<FlagType>(~usedBits & AllBitsSet);
    }
};