#include <iostream>
#include <vector>
#include <array>
#include <span>
#include <cmath>
#include <random>
#include <chrono>
#include <bitset>
#include <pcg_random.hpp>

static constexpr std::size_t SudokuSquareSize = 3;
static constexpr std::size_t SudokuRowSize = SudokuSquareSize * SudokuSquareSize;
static constexpr std::size_t SudokuColSize = SudokuSquareSize * SudokuSquareSize;
static constexpr std::size_t SudokuSize = SudokuColSize * SudokuRowSize;
static constexpr std::size_t CellSize = 80;

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

    inline constexpr char GetValue(std::size_t row, std::size_t col, std::size_t square) const
    {
        std::uint16_t usedBits = m_bits[row] & m_bits[9 + col] & m_bits[18 + square];
        return static_cast<std::uint16_t>(~usedBits & 0x1FF);
    }
};

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

    constexpr SudokuMatrix() : m_dataBits({})
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

SudokuMatrix CreateBoard(const float probabilityOfFilled, pcg64 &randomDevice)
{
    SudokuMatrix board = {};
    // Gerador de números aleatórios
    std::uniform_real_distribution<float> probabilityDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> valueDist(1, 9);

    // Preenche aleatoriamente algumas células do tabuleiro
    for (std::size_t row = 0; row < SudokuRowSize; ++row)
    {
        for (std::size_t col = 0; col < SudokuColSize; ++col)
        {
            if (probabilityDist(randomDevice) >= probabilityOfFilled)
            {
                continue;
            }
            for (int attempts = 0; attempts < 10; ++attempts) // Limita as tentativas para evitar loops infinitos
            {
                char value = static_cast<char>(valueDist(randomDevice));
                if (board.IsValidPlay(value, row, col))
                {
                    board.SetValue(row, col, value);
                }
            }
        }
    }

    return board;
}

#define DO_BENCHMARK 0
#if DO_BENCHMARK
#include <benchmark/benchmark.h>
static void BM_Solver(benchmark::State &state)
{
    // std::random_device device;
    // pcg64 rng{device()};
    // static constexpr float probability = 0.2f;
    // SudokuMatrix sudokuGame = CreateBoard(probability, rng);
    static constexpr std::array<char, SudokuSize> sudokuGame = {
        5, 3, 0, 0, 7, 0, 0, 0, 0,
        6, 0, 0, 1, 9, 5, 0, 0, 0,
        0, 9, 8, 0, 0, 0, 0, 6, 0,

        8, 0, 0, 0, 6, 0, 0, 0, 3,
        4, 0, 0, 8, 0, 3, 0, 0, 1,
        7, 0, 0, 0, 2, 0, 0, 0, 6,

        0, 6, 0, 0, 0, 0, 2, 8, 0,
        0, 0, 0, 4, 1, 9, 0, 0, 5,
        0, 0, 0, 0, 8, 0, 0, 7, 9};
    std::int64_t index = 0;
    for (auto _ : state)
    {
        BackTrackingSolver solver{sudokuGame};
        while (solver.Advance())
        {
            index++;
        }
    }
    state.SetItemsProcessed(index);
}

BENCHMARK(BM_Solver);

BENCHMARK_MAIN();
// int main()
// {
//     static constexpr std::array<char, SudokuSize> sudokuGame = {
//         5, 3, 0, 0, 7, 0, 0, 0, 0,
//         6, 0, 0, 1, 9, 5, 0, 0, 0,
//         0, 9, 8, 0, 0, 0, 0, 6, 0,

//         8, 0, 0, 0, 6, 0, 0, 0, 3,
//         4, 0, 0, 8, 0, 3, 0, 0, 1,
//         7, 0, 0, 0, 2, 0, 0, 0, 6,

//         0, 6, 0, 0, 0, 0, 2, 8, 0,
//         0, 0, 0, 4, 1, 9, 0, 0, 5,
//         0, 0, 0, 0, 8, 0, 0, 7, 9};
//     std::int64_t index = 0;
//     for (std::size_t i = 0; i < 1000; i++)
//     {
//         BackTrackingSolver solver{sudokuGame};
//         while (solver.Advance())
//         {
//             index++;
//         }
//     }
// }
#else
#include <SFML/Graphics.hpp>

static void DrawLines(sf::RenderWindow &window)
{
    for (std::size_t i = 0; i <= SudokuRowSize; ++i)
    {
        sf::Vertex line[] =
            {
                sf::Vertex(sf::Vector2f(0, static_cast<float>(i * CellSize)), sf::Color::Black),
                sf::Vertex(sf::Vector2f(static_cast<float>(SudokuColSize * CellSize), static_cast<float>(i * CellSize)), sf::Color::Black)};
        window.draw(line, 2, sf::Lines);
    }

    for (std::size_t j = 0; j <= SudokuColSize; ++j)
    {
        sf::Vertex line[] =
            {
                sf::Vertex(sf::Vector2f(static_cast<float>(j * CellSize), 0), sf::Color::Black),
                sf::Vertex(sf::Vector2f(static_cast<float>(j * CellSize), static_cast<float>(SudokuRowSize * CellSize)), sf::Color::Black)};
        window.draw(line, 2, sf::Lines);
    }
}

static void DrawNumbers(const SudokuMatrix &board, sf::RenderWindow &window, sf::Font &font)
{
    for (std::size_t i = 0; i < SudokuRowSize; ++i) // Corrigido para < em vez de <=
    {
        for (std::size_t j = 0; j < SudokuColSize; ++j) // Corrigido para < em vez de <=
        {
            char value = board.GetValue(i, j);
            if (value == 0)
            {
                continue;
            }
            sf::Text text;
            text.setFont(font);
            text.setString(std::to_string(value));
            text.setCharacterSize(24);
            text.setFillColor(sf::Color::Black);
            sf::FloatRect bounds = text.getLocalBounds();

            // Centralizando o texto dentro da célula
            float textX = j * CellSize + (CellSize - bounds.width) / 2.0f;
            float textY = i * CellSize + (CellSize - bounds.height) / 2.0f - bounds.top;
            text.setPosition(textX, textY);

            window.draw(text);
        }
    }
}

static bool RunClass(const SudokuMatrix &matrix, sf::RenderWindow &window, sf::Font &font)
{
    BackTrackingSolver solver{matrix};
    std::size_t index = 0;
    while (solver.Advance() || window.isOpen())
    {
        if (index % 100 != 0)
        {
            index++;
            continue;
        }
        const auto &board = solver.GetBoard();
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                return solver.IsSolved();
            }
        }

        // Limpa a janela com cor branca
        window.clear(sf::Color::White);

        // Desenha as linhas horizontais e verticais para formar a grade
        DrawLines(window);

        // Desenha os números dentro das células
        DrawNumbers(board, window, font);

        // Atualiza a janela
        window.display();
        index++;
    }

    // Mantém a janela aberta até ser fechada pelo usuário
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
    }

    return solver.IsSolved();
}

int main(int, char **)
{
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        return -1; // Erro ao carregar fonte
    }
    static constexpr std::array<char, SudokuSize> sudokuGame = {
        5, 3, 0, 0, 7, 0, 0, 0, 0,
        6, 0, 0, 1, 9, 5, 0, 0, 0,
        0, 9, 8, 0, 0, 0, 0, 6, 0,

        8, 0, 0, 0, 6, 0, 0, 0, 3,
        4, 0, 0, 8, 0, 3, 0, 0, 1,
        7, 0, 0, 0, 2, 0, 0, 0, 6,

        0, 6, 0, 0, 0, 0, 2, 8, 0,
        0, 0, 0, 4, 1, 9, 0, 0, 5,
        0, 0, 0, 0, 8, 0, 0, 7, 9};
    std::random_device device;
    pcg64 rng{device()};
    static constexpr float probability = 0.15f;
    SudokuMatrix data = CreateBoard(probability, rng);
    // static constexpr SudokuMatrix data{sudokuGame};
    BackTrackingSolver solver{data};
    std::size_t index = 0;
    while (solver.Advance())
    {
        index++;
        if (index % 100'000 == 0)
        {
            std::cout << "Index: " << index << '\n';
        }
    }
    if (!solver.IsSolved())
    {
        std::cout << "Board cannot be solved\n";
        return 1;
    }
    std::cout << "Solved in " << index << " tries\n";
    sf::RenderWindow window(sf::VideoMode(800, 800), "Sudoku Solver Visualizer");
    RunClass(data, window, font);
    return 0;
}
#endif
