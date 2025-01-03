#pragma once
#include <array>
#include <bit>
#include <bitset>
#include <boost/dynamic_bitset.hpp>

template <std::size_t N>
struct BitSetIterator
{
public:
    using FlagType = std::conditional_t<(N * N <= 8), std::uint8_t,
                                        std::conditional_t<(N * N <= 16), std::uint16_t,
                                                           std::conditional_t<(N * N <= 32), std::uint32_t, std::uint64_t>>>;
    using DataType = std::conditional_t<(N * N <= std::numeric_limits<std::uint8_t>::max()), std::uint8_t,
                                        std::conditional_t<(N * N <= std::numeric_limits<std::uint16_t>::max()), std::uint16_t,
                                                           std::conditional_t<(N * N <= std::numeric_limits<std::uint32_t>::max()), std::uint32_t, std::uint64_t>>>;

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
    inline constexpr DataType operator*() const
    {
        // Get the least significant set bit
        FlagType newValue = m_flag & -static_cast<std::make_signed_t<FlagType>>(m_flag);
        int count = std::countr_zero(newValue);
        return static_cast<DataType>(count + 1);
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

struct DynamicBitSetIterator
{
public:
    using DataType = std::uint8_t;
private:
    using size_type = boost::dynamic_bitset<>::size_type;
    boost::dynamic_bitset<> m_bitset;
    size_type m_index;
    size_type m_count;

public:
    DynamicBitSetIterator(const boost::dynamic_bitset<> &bitset) : m_bitset(bitset), m_index(0), m_count(bitset.count())
    {
        m_index = m_bitset.find_first();
    }
    inline DynamicBitSetIterator &operator++()
    {
        m_index = m_bitset.find_next(m_index);
        m_count--;
        return *this;
    }
    inline DataType operator*() const
    {
        return static_cast<DataType>(m_index + 1);
    }
    inline bool operator!=(const DynamicBitSetIterator &other) const
    {
        return m_index != other.m_index;
    }
    inline DynamicBitSetIterator begin()
    {
        return {m_bitset};
    }
    static inline DynamicBitSetIterator end()
    {
        return {boost::dynamic_bitset<>()};
    }
    inline std::size_t Count() const
    {
        return m_count;
    }
};

template <std::size_t N>
struct SudokuBits
{
public:
    using FlagType = std::bitset<N * N>;
    using DataType = typename BitSetIterator<N>::DataType;
private:
    std::array<FlagType, N * N * 3> m_bits;
    static constexpr std::size_t size = N * N;
    static constexpr FlagType AllBitsSet = (1ULL << size) - 1;

public:
    inline constexpr void SetValue(std::size_t row, std::size_t col, std::size_t square, DataType value)
    {
        FlagType mask = 1ULL << (value - 1);
        m_bits[row] |= mask;
        m_bits[size + col] |= mask;
        m_bits[size * 2 + square] |= mask;
    }

    inline constexpr void ResetValue(std::size_t row, std::size_t col, std::size_t square, DataType value)
    {
        FlagType mask = ~(1 << (value - 1));
        m_bits[row] &= mask;
        m_bits[size + col] &= mask;
        m_bits[size * 2 + square] &= mask;
    }

    inline constexpr bool Test(std::size_t row, std::size_t col, std::size_t square, DataType value) const
    {
        FlagType mask = 1ULL << (value - 1);
        return (m_bits[row] & mask) != 0 && (m_bits[size + col] & mask) != 0 && (m_bits[size * 2 + square] & mask) != 0;
    }

    inline constexpr FlagType GetAvailableValues(std::size_t row, std::size_t col, std::size_t square) const
    {
        FlagType usedBits = m_bits[row] | m_bits[size + col] | m_bits[size * 2 + square];
        return static_cast<FlagType>(~usedBits & AllBitsSet);
    }

    inline constexpr const std::array<FlagType, N * N * 3> &GetBits() const
    {
        return m_bits;
    }
};

struct SudokuDynamicBits
{
public:
    using DataType = typename DynamicBitSetIterator::DataType;
private:
    boost::dynamic_bitset<> m_allBitsSet;
    std::vector<boost::dynamic_bitset<>> m_bits;
    std::size_t m_size;

public:
    SudokuDynamicBits(std::size_t size) : m_allBitsSet(size * size), m_bits(size * size * 3, boost::dynamic_bitset<>(size * size)), m_size(size * size)
    {
        m_allBitsSet.set();
    }
    inline void SetValue(std::size_t row, std::size_t col, std::size_t square, DataType value)
    {
        assert(value >= 1 && value <= m_size);
        std::size_t index = static_cast<std::size_t>(value - 1);
        m_bits[row].set(index);
        m_bits[m_size + col].set(index);
        m_bits[m_size * 2 + square].set(index);
    }
    inline void ResetValue(std::size_t row, std::size_t col, std::size_t square, DataType value)
    {
        assert(value >= 1 && value <= m_size);
        std::size_t index = static_cast<std::size_t>(value - 1);
        m_bits[row].reset(index);
        m_bits[m_size + col].reset(index);
        m_bits[m_size * 2 + square].reset(index);
    }
    inline bool Test(std::size_t row, std::size_t col, std::size_t square, DataType value) const
    {
        assert(value >= 1 && value <= m_size);
        std::size_t index = static_cast<std::size_t>(value - 1);
        return m_bits[row].test(index) && m_bits[m_size + col].test(index) && m_bits[m_size * 2 + square].test(index);
    }
    inline boost::dynamic_bitset<> GetAvailableValues(std::size_t row, std::size_t col, std::size_t square) const
    {
        boost::dynamic_bitset<> usedBits = m_bits[row] | m_bits[m_size + col] | m_bits[m_size * 2 + square];
        return ~usedBits & m_allBitsSet;
    }
    inline const std::vector<boost::dynamic_bitset<>> &GetBits() const
    {
        return m_bits;
    }
};