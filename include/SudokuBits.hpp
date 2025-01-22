#pragma once
#include <array>
#include <bit>
#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <algorithm> // for std::fill_n, etc.
#include <immintrin.h>

template <std::size_t BITS>
class FastBitset
{
    static constexpr std::size_t BITS_PER_CHUNK = 64;
    static constexpr std::size_t NUM_CHUNKS =
        (BITS + BITS_PER_CHUNK - 1) / BITS_PER_CHUNK; // ceiling division

    std::array<std::uint64_t, NUM_CHUNKS> m_data{};

    static inline int lzcnt_si256(const __m256i vec)
    {
        __m256i vnonzero = _mm256_cmpeq_epi32(vec, _mm256_setzero_si256());
        std::uint32_t nzmask = ~_mm256_movemask_epi8(vnonzero); //  1 for bytes that are part of non-zero dwords
        // nzmask |= 0xf;  // branchless clamp to last elem
        if (nzmask == 0) // all 32 bits came from vpmovmskb, so NOT didn't introduce any constant 1s
        {
            return 256; // don't access outside the array
        }
        alignas(32) std::uint32_t elems[8];
        _mm256_storeu_si256((__m256i *)elems, vec);
        unsigned int lzbytes = _lzcnt_u32(nzmask); // bytes above the dword containing the nonzero bit.
        unsigned char *end_elem = 28 + (unsigned char *)elems;
        uint32_t *nz_elem = (uint32_t *)(end_elem - lzbytes); // idx = 31-(lzcnt+3) = 28-lzcnt
        return 8 * lzbytes + _lzcnt_u32(*nz_elem);
    }

public:
    // Default constructor: all bits off
    constexpr FastBitset() = default;

    // Construct from uint64_t (for small sets) or from an initializer:
    inline constexpr explicit FastBitset(std::uint64_t value)
    {
        m_data[0] = value;
        for (std::size_t i = 1; i < NUM_CHUNKS; ++i)
            m_data[i] = 0ULL;
    }

    // Set bit 'pos' to 1 (if 'val' is true) or 0 (if 'val' is false)
    inline constexpr void set(std::size_t pos, bool val = true)
    {
        if (pos >= BITS)
            return; // out of range
        const std::size_t chunkIndex = pos / BITS_PER_CHUNK;
        const std::size_t bitIndex = pos % BITS_PER_CHUNK;
        std::uint64_t mask = (std::uint64_t{1} << bitIndex);
        if (val)
        {
            m_data[chunkIndex] |= mask;
            return;
        }
        m_data[chunkIndex] &= ~mask;
    }

    // Reset a single bit (turn off)
    inline constexpr void reset(std::size_t pos)
    {
        if (pos >= BITS)
            return;
        const std::size_t chunkIndex = pos / BITS_PER_CHUNK;
        const std::size_t bitIndex = pos % BITS_PER_CHUNK;
        m_data[chunkIndex] &= ~(std::uint64_t{1} << bitIndex);
    }

    // Reset all bits
    inline constexpr void reset()
    {
        for (auto &chunk : m_data)
            chunk = 0ULL;
    }

    // Return whether bit 'pos' is set
    inline constexpr bool test(std::size_t pos) const
    {
        if (pos >= BITS)
            return false;
        const std::size_t chunkIndex = pos / BITS_PER_CHUNK;
        const std::size_t bitIndex = pos % BITS_PER_CHUNK;
        return (m_data[chunkIndex] & (std::uint64_t{1} << bitIndex)) != 0ULL;
    }

    // Return total number of set bits
    inline constexpr int count() const
    {
        int result = 0;
        for (const std::uint64_t chunk : m_data)
        {
            result += std::popcount(chunk);
        }
        return result;
    }

    // Is there any bit set?
    inline constexpr bool any() const
    {
        for (const std::uint64_t chunk : m_data)
            if (chunk != 0ULL)
                return true;
        return false;
    }

    // Are all bits off?
    inline constexpr bool none() const
    {
        return !any();
    }

    // Operator== for comparing two FastBitset<BITS>
    inline friend constexpr bool operator==(FastBitset const &lhs, FastBitset const &rhs)
    {
        return lhs.m_data == rhs.m_data;
    }

    // Operator!=
    inline friend constexpr bool operator!=(FastBitset const &lhs, FastBitset const &rhs)
    {
        return !(lhs == rhs);
    }

    // Return the least significant set bit index, or BITS if none is set.
    constexpr std::size_t findLSB() const
    {
#ifdef __AVX2__
        // 1) Processamos em blocos de 4x64 bits (256 bits) por vez.
        // Cada "for" verifica se 4 blocos são todos zero usando intrínsecos AVX2.
        // Se forem todos zero, pulamos de imediato. Se não, checamos blocos
        // individualmente na sequência.
        constexpr std::size_t STEP = 4; // 4 blocos de 64 bits = 256 bits
        std::size_t i = 0;

        for (; i + STEP <= NUM_CHUNKS; i += STEP)
        {
            // Carrega 256 bits (4 * 64) da memória
            __m256i v = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(&m_data[i]));

            // Se todos bits de v forem zero, avança
            if (_mm256_testz_si256(v, v) != 0)
            {
                continue;
            }
            int tz = lzcnt_si256(v);
            std::size_t bitPos = i * BITS_PER_CHUNK + tz;
            return (bitPos < BITS) ? bitPos : BITS;
        }

        // 2) Resto (caso a quantidade de blocos não seja múltipla de 4)
        for (; i < NUM_CHUNKS; ++i)
        {
            std::uint64_t chunk = m_data[i];
            if (chunk != 0ULL)
            {
                unsigned tz = std::countr_zero(chunk);
                std::size_t bitPos = i * BITS_PER_CHUNK + tz;
                return (bitPos < BITS) ? bitPos : BITS;
            }
        }

        return BITS; // Não encontrou nenhum bit set

#else
        // Fallback escalar se não houver suporte a AVX2
        for (std::size_t chunkIndex = 0; chunkIndex < NUM_CHUNKS; ++chunkIndex)
        {
            std::uint64_t chunk = m_data[chunkIndex];
            if (chunk == 0ULL)
            {
                continue;
            }
            const unsigned tz = std::countr_zero(chunk);
            const std::size_t bitPos = chunkIndex * BITS_PER_CHUNK + tz;
            return (bitPos < BITS) ? bitPos : BITS;
        }
        return BITS;
#endif
    }
};

template <std::size_t N>
struct BitSetIterator
{
public:
    using FlagType = FastBitset<N * N>;
    using DataType = std::conditional_t<(N * N <= std::numeric_limits<std::uint8_t>::max()), std::uint8_t,
                                        std::conditional_t<(N * N <= std::numeric_limits<std::uint16_t>::max()), std::uint16_t,
                                                           std::conditional_t<(N * N <= std::numeric_limits<std::uint32_t>::max()), std::uint32_t, std::uint64_t>>>;

private:
    FlagType m_flag;

public:
    constexpr BitSetIterator() : m_flag() {}
    constexpr BitSetIterator(const FlagType &flag) : m_flag(flag) {}

    // ++ removes the least significant set bit.
    inline constexpr BitSetIterator &operator++()
    {
        std::size_t idx = m_flag.findLSB();
        if (idx < N * N)
        {
            // Reset that bit.
            m_flag.reset(idx);
        }
        return *this;
    }

    // * returns the 1-based index of the least significant set bit.
    inline constexpr DataType operator*() const
    {
        std::size_t idx = m_flag.findLSB();
        // Original code adds 1 to the 0-based position:
        return static_cast<DataType>(idx + 1);
    }

    // Compare iterators by comparing the underlying bitset.
    inline constexpr bool operator!=(const BitSetIterator &other) const
    {
        return m_flag != other.m_flag;
    }

    // Compare against a FlagType (bitset). Equivalent to `iterator.m_flag == value`.
    inline constexpr bool operator==(const FlagType &value) const
    {
        return m_flag == value;
    }

    // begin() returns an iterator starting with the same bitset.
    inline constexpr BitSetIterator begin() const
    {
        return BitSetIterator(m_flag);
    }

    // end() returns an iterator with an empty bitset (no set bits).
    static inline constexpr BitSetIterator end()
    {
        return BitSetIterator(FlagType{});
    }

    // Returns the total number of set bits.
    inline constexpr int Count() const
    {
        return m_flag.count();
    }

    inline constexpr bool Any() const
    {
        return m_flag.any();
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
    static constexpr FlagType AllBitsSet = []()
    {
        FlagType bits;
        bits.set();
        return bits;
    }();

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