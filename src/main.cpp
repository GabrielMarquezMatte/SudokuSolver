#include <cmath>
#include <random>
#include <chrono>
#include <iostream>
#include <pcg_random.hpp>
#include <SFML/Graphics.hpp>
#include "../include/solvers/BackTracking.hpp"
#include "../include/solvers/DlxSolver.hpp"
#include "../include/SudokuUtilities.hpp"

template <std::size_t N>
static void DrawLines(sf::RenderWindow &window, std::size_t cellSize)
{
    constexpr std::size_t Size = N * N;
    std::size_t resultSize = Size * cellSize;
    float resultSizeFloat = static_cast<float>(resultSize);
    for (std::size_t i = 0; i <= Size; ++i)
    {
        std::size_t dataSize = i * cellSize;
        float dataSizeFloat = static_cast<float>(dataSize);
        sf::Vertex line[] =
            {
                sf::Vertex(sf::Vector2f(0, dataSizeFloat), sf::Color::Black),
                sf::Vertex(sf::Vector2f(resultSizeFloat, dataSizeFloat), sf::Color::Black)};
        window.draw(line, 2, sf::Lines);
    }

    for (std::size_t j = 0; j <= Size; ++j)
    {
        std::size_t dataSize = j * cellSize;
        float dataSizeFloat = static_cast<float>(dataSize);
        sf::Vertex line[] =
            {
                sf::Vertex(sf::Vector2f(dataSizeFloat, 0), sf::Color::Black),
                sf::Vertex(sf::Vector2f(dataSizeFloat, resultSizeFloat), sf::Color::Black)};
        window.draw(line, 2, sf::Lines);
    }
}

template <std::size_t N>
std::string GetStringFromData(typename SudokuMatrix<N>::DataType value)
{
    if (value < 10)
    {
        return std::to_string(value);
    }
    return std::string(1, 'A' + value - 10);
}

template <std::size_t N>
static void DrawNumbers(const SudokuMatrix<N> &board, sf::RenderWindow &window, sf::Font &font, std::size_t cellSize)
{
    constexpr std::size_t Size = N * N;
    for (std::size_t i = 0; i < Size; ++i) // Corrigido para < em vez de <=
    {
        for (std::size_t j = 0; j < Size; ++j) // Corrigido para < em vez de <=
        {
            typename SudokuMatrix<N>::DataType value = board.GetValue(i, j);
            if (value == 0)
            {
                continue;
            }
            sf::Text text;
            text.setFont(font);
            text.setString(GetStringFromData<N>(value));
            text.setCharacterSize(24);
            text.setFillColor(sf::Color::Black);
            sf::FloatRect bounds = text.getLocalBounds();

            // Centralizando o texto dentro da célula
            float textX = j * cellSize + (cellSize - bounds.width) / 2.0f;
            float textY = i * cellSize + (cellSize - bounds.height) / 2.0f - bounds.top;
            text.setPosition(textX, textY);

            window.draw(text);
        }
    }
}

template <std::size_t N, template <std::size_t> class Solver, typename std::enable_if<std::is_base_of<ISolver<N>, Solver<N>>::value>::type * = nullptr>
static bool RunClass(const SudokuMatrix<N> &matrix, sf::RenderWindow &window, sf::Font &font, std::size_t cellSize)
{
    Solver<N> solver{matrix};
    std::size_t index = 0;
    while (solver.Advance() && window.isOpen())
    {
        if constexpr (!std::is_same_v<Solver<N>, DLXSolver<N>>)
        {
            if (index % 1'000 != 0)
            {
                index++;
                continue;
            }
        }
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                return solver.IsSolved();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R)
            {
                return RunClass<N, Solver>(matrix, window, font, cellSize);
            }
        }

        const auto &board = solver.GetBoard();
        // Limpa a janela com cor branca
        window.clear(sf::Color::White);

        // Desenha as linhas horizontais e verticais para formar a grade
        DrawLines<N>(window, cellSize);

        // Desenha os números dentro das células
        DrawNumbers<N>(board, window, font, cellSize);

        // Atualiza a janela
        window.display();
        index++;
    }

    // Mantém a janela aberta até ser fechada pelo usuário
    bool drawn = false;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R)
            {
                return RunClass<N, Solver>(matrix, window, font, cellSize);
            }
        }
        if (drawn)
        {
            continue;
        }
        const auto &board = solver.GetBoard();
        // Limpa a janela com cor branca
        window.clear(sf::Color::White);

        // Desenha as linhas horizontais e verticais para formar a grade
        DrawLines<N>(window, cellSize);

        // Desenha os números dentro das células
        DrawNumbers<N>(board, window, font, cellSize);

        // Atualiza a janela
        window.display();
        drawn = true;
    }

    return solver.IsSolved();
}

template <std::size_t N, template <std::size_t> class Solver, typename std::enable_if<std::is_base_of<ISolver<N>, Solver<N>>::value>::type * = nullptr>
SudokuMatrix<N> GetPossibleMatrix(float probability, pcg64 &rng)
{
    while (true)
    {
        SudokuMatrix<N> data = CreateBoard<N>(probability, rng);
        Solver<N> solver{data};
        std::size_t index = 0;
        while (solver.Advance())
        {
            index++;
            if (index % 20'000'000 == 0)
            {
                break;
            }
        }
        if (!solver.IsSolved() || !IsValidSudoku(solver.GetBoard()))
        {
            continue;
        }
        return data;
    }
}

template <std::size_t N, template <std::size_t> class Solver, typename std::enable_if<std::is_base_of<ISolver<N>, Solver<N>>::value>::type * = nullptr>
int Run(const float probability, pcg64 &rng)
{
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        return -1; // Erro ao carregar fonte
    }
    SudokuMatrix<N> data = GetPossibleMatrix<N, Solver>(probability, rng);
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Sudoku Solver Visualizer");
    constexpr std::size_t cellSize = 150 / N;
    RunClass<N, Solver>(data, window, font, cellSize);
    return 0;
}

template <std::size_t N>
int RunForSolver(const float probability, pcg64 &rng, std::string_view userSolver)
{
    if (userSolver == "backtrack")
    {
        return Run<N, BackTrackingSolver>(probability, rng);
    }
    if (userSolver == "dlx")
    {
        return Run<N, DLXSolver>(probability, rng);
    }
    std::cerr << "Valid solvers are 'backtrack' and 'dlx'\n";
    return 1;
}

int main(int argc, char **argv)
{
    // static constexpr std::array<char, 81> sudokuGame = {
    //     5, 3, 0, 0, 7, 0, 0, 0, 0,
    //     6, 0, 0, 1, 9, 5, 0, 0, 0,
    //     0, 9, 8, 0, 0, 0, 0, 6, 0,

    //     8, 0, 0, 0, 6, 0, 0, 0, 3,
    //     4, 0, 0, 8, 0, 3, 0, 0, 1,
    //     7, 0, 0, 0, 2, 0, 0, 0, 6,

    //     0, 6, 0, 0, 0, 0, 2, 8, 0,
    //     0, 0, 0, 4, 1, 9, 0, 0, 5,
    //     0, 0, 0, 0, 8, 0, 0, 7, 9};
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <size> <solver>\n";
        return 1;
    }
    std::size_t userSize = std::stoul(argv[1]);
    std::string_view userSolver = argv[2];
    std::random_device device;
    pcg64 rng{device()};
    static constexpr float probability = 0.3f;
    switch (userSize)
    {
    case 2:
    {
        return RunForSolver<2>(probability, rng, userSolver);
    }
    case 3:
    {
        return RunForSolver<3>(probability, rng, userSolver);
    }
    case 4:
    {
        return RunForSolver<4>(probability, rng, userSolver);
    }
    case 5:
    {
        return RunForSolver<5>(probability, rng, userSolver);
    }
    case 6:
    {
        return RunForSolver<6>(probability, rng, userSolver);
    }
    case 7:
    {
        return RunForSolver<7>(probability, rng, userSolver);
    }
    default:
        std::cerr << "Invalid size\n";
        return 1;
    }
}
