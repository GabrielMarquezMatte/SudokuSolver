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
using CellType = std::bitset<SudokuColSize>;

class SudokuMatrix
{
private:
    std::array<char, SudokuSize> m_data;
    std::array<CellType, SudokuRowSize> m_rowBits;
    std::array<CellType, SudokuColSize> m_colBits;
    std::array<CellType, SudokuSquareSize * SudokuSquareSize> m_squareBits;

    static inline constexpr std::size_t MatrixIndex(const std::size_t row, const std::size_t col)
    {
        return row * SudokuRowSize + col;
    }

    static inline constexpr std::size_t SquareIndex(std::size_t row, std::size_t col)
    {
        std::size_t squareRow = row / SudokuSquareSize;
        std::size_t squareCol = col / SudokuSquareSize;
        return squareRow * SudokuSquareSize + squareCol;
    }

public:
    constexpr SudokuMatrix()
    {
        m_data.fill(0);
        m_rowBits.fill(CellType());
        m_colBits.fill(CellType());
        m_squareBits.fill(CellType());
    }

    constexpr SudokuMatrix(const std::array<char, SudokuSize> &data) : m_data(data)
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
                std::size_t bitIndex = value - 1; // Convertendo valor (1-9) para índice (0-8)
                std::size_t squareIndex = SquareIndex(row, col);

                // Marca os valores nos bitsets correspondentes
                m_rowBits[row].set(bitIndex);
                m_colBits[col].set(bitIndex);
                m_squareBits[squareIndex].set(bitIndex);
            }
        }
    }

    constexpr SudokuMatrix(const SudokuMatrix &other)
        : m_data(other.m_data),
          m_rowBits(other.m_rowBits),
          m_colBits(other.m_colBits),
          m_squareBits(other.m_squareBits)
    {
    }

    constexpr SudokuMatrix(SudokuMatrix &&other) noexcept
        : m_data(std::move(other.m_data)),
          m_rowBits(std::move(other.m_rowBits)),
          m_colBits(std::move(other.m_colBits)),
          m_squareBits(std::move(other.m_squareBits))
    {
    }

    constexpr SudokuMatrix &operator=(const SudokuMatrix &other)
    {
        if (this == &other)
        {
            return *this;
        }
        m_data = other.m_data;
        m_rowBits = other.m_rowBits;
        m_colBits = other.m_colBits;
        m_squareBits = other.m_squareBits;
        return *this;
    }

    constexpr SudokuMatrix &operator=(SudokuMatrix &&other) noexcept
    {
        if (this != &other) // Evita a auto-atribuição
        {
            m_data = std::move(other.m_data);
            m_rowBits = std::move(other.m_rowBits);
            m_colBits = std::move(other.m_colBits);
            m_squareBits = std::move(other.m_squareBits);
        }
        return *this;
    }

    inline constexpr char GetValue(std::size_t row, std::size_t col) const
    {
        return m_data[MatrixIndex(row, col)];
    }

    inline constexpr void SetValue(std::size_t row, std::size_t col, char value)
    {
        std::size_t index = MatrixIndex(row, col);
        std::size_t squareIndex = SquareIndex(row, col);

        // Remove o valor anterior, se houver
        char &oldValue = m_data[index];
        if (oldValue != 0)
        {
            m_rowBits[row].reset(oldValue - 1);
            m_colBits[col].reset(oldValue - 1);
            m_squareBits[squareIndex].reset(oldValue - 1);
        }

        // Define o novo valor
        oldValue = value;

        // Atualiza os bitsets se o valor não for zero
        if (value != 0)
        {
            m_rowBits[row].set(value - 1);
            m_colBits[col].set(value - 1);
            m_squareBits[squareIndex].set(value - 1);
        }
    }

    inline constexpr bool IsValidPlay(char value, std::size_t row, std::size_t col) const
    {
        std::size_t bitIndex = value - 1; // O valor é de 1 a 9, mas o índice do bitset é de 0 a 8
        std::size_t squareIndex = SquareIndex(row, col);

        // Verifica se o valor já está presente na linha, coluna ou quadrante
        return !m_rowBits[row].test(bitIndex) &&
               !m_colBits[col].test(bitIndex) &&
               !m_squareBits[squareIndex].test(bitIndex);
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
    std::size_t m_step = 0;
    std::size_t m_currentRow = 0;
    std::size_t m_currentCol = 0;
    AdvanceResult m_currentState = AdvanceResult::Continue;
    bool m_solved = false;
    inline constexpr void AdvanceToNextCell()
    {
        if (++m_currentCol == SudokuColSize)
        {
            m_currentCol = 0;
            m_currentRow++;
        }
    }
    inline constexpr void RetreatToPreviousCell()
    {
        if (m_currentCol == 0 && m_currentRow != 0)
        {
            m_currentCol = SudokuColSize - 1;
            m_currentRow--;
            return;
        }
        if (m_currentCol != 0)
        {
            m_currentCol--;
        }
    }

    inline constexpr void Continue()
    {
        m_currentState = AdvanceResult::Continue;
        AdvanceToNextCell();
    }

    inline constexpr void BackTrack()
    {
        m_currentState = AdvanceResult::BackTracking;
        RetreatToPreviousCell();
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
        if (m_currentRow == 0 && m_currentCol == 0 && m_currentState == AdvanceResult::BackTracking)
        {
            m_solved = false;
            return false;
        }
        if (m_currentRow == SudokuRowSize)
        {
            m_solved = true;
            m_currentState = AdvanceResult::Finished;
            return false;
        }
        if (m_currentState == AdvanceResult::BackTracking)
        {
            char value = m_data.GetValue(m_currentRow, m_currentCol) + 1;
            m_data.RemoveValue(m_currentRow, m_currentCol);
            for (; value <= 9; value++)
            {
                if (m_data.IsValidPlay(value, m_currentRow, m_currentCol))
                {
                    m_data.SetValue(m_currentRow, m_currentCol, value);
                    Continue();
                    return true;
                }
            }
            BackTrack();
            return true;
        }
        char inSpot = m_data.GetValue(m_currentRow, m_currentCol);
        if (inSpot != 0)
        {
            Continue();
            return true;
        }
        for (char value = 1; value <= 9; value++)
        {
            if (m_data.IsValidPlay(value, m_currentRow, m_currentCol))
            {
                m_data.SetValue(m_currentRow, m_currentCol, value);
                Continue();
                return true;
            }
        }
        BackTrack();
        return true;
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
    // SudokuMatrix data = CreateBoard(probability, rng);
    std::array<char, SudokuSize> sudokuGame = {
        5, 3, 0, 0, 7, 0, 0, 0, 0,
        6, 0, 0, 1, 9, 5, 0, 0, 0,
        0, 9, 8, 0, 0, 0, 0, 6, 0,

        8, 0, 0, 0, 6, 0, 0, 0, 3,
        4, 0, 0, 8, 0, 3, 0, 0, 1,
        7, 0, 0, 0, 2, 0, 0, 0, 6,

        0, 6, 0, 0, 0, 0, 2, 8, 0,
        0, 0, 0, 4, 1, 9, 0, 0, 5,
        0, 0, 0, 0, 8, 0, 0, 7, 9};
    BackTrackingSolver solver{sudokuGame};
    for (auto _ : state)
    {
        while (solver.Advance())
            ;
        benchmark::DoNotOptimize(solver);
    }
}

BENCHMARK(BM_Solver);

BENCHMARK_MAIN();
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

static void DrawNumbers(const SudokuMatrix &board, sf::RenderWindow &window, sf::Font& font)
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
    // static constexpr std::array<char, SudokuSize> sudokuGame = {
    //     5, 3, 0, 0, 7, 0, 0, 0, 0,
    //     6, 0, 0, 1, 9, 5, 0, 0, 0,
    //     0, 9, 8, 0, 0, 0, 0, 6, 0,

    //     8, 0, 0, 0, 6, 0, 0, 0, 3,
    //     4, 0, 0, 8, 0, 3, 0, 0, 1,
    //     7, 0, 0, 0, 2, 0, 0, 0, 6,

    //     0, 6, 0, 0, 0, 0, 2, 8, 0,
    //     0, 0, 0, 4, 1, 9, 0, 0, 5,
    //     0, 0, 0, 0, 8, 0, 0, 7, 9};
    std::random_device device;
    pcg64 rng{device()};
    static constexpr float probability = 0.4f;
    SudokuMatrix data = CreateBoard(probability, rng);
    BackTrackingSolver solver{data};
    std::size_t index = 0;
    while (solver.Advance())
    {
        index++;
        if (index % 1000 == 0)
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
