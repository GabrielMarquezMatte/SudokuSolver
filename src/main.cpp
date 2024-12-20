#include <cmath>
#include <random>
#include <chrono>
#include <iostream>
#include <pcg_random.hpp>
#include <SFML/Graphics.hpp>
#include "../include/solvers/BackTracking.hpp"
#include "../include/solvers/DlxSolver.hpp"
#include "../include/SudokuUtilities.hpp"

static constexpr std::size_t CellSize = 48;

template <std::size_t N>
static void DrawLines(sf::RenderWindow &window)
{
    constexpr std::size_t Size = N * N;
    for (std::size_t i = 0; i <= Size; ++i)
    {
        sf::Vertex line[] =
            {
                sf::Vertex(sf::Vector2f(0, static_cast<float>(i * CellSize)), sf::Color::Black),
                sf::Vertex(sf::Vector2f(static_cast<float>(Size * CellSize), static_cast<float>(i * CellSize)), sf::Color::Black)};
        window.draw(line, 2, sf::Lines);
    }

    for (std::size_t j = 0; j <= Size; ++j)
    {
        sf::Vertex line[] =
            {
                sf::Vertex(sf::Vector2f(static_cast<float>(j * CellSize), 0), sf::Color::Black),
                sf::Vertex(sf::Vector2f(static_cast<float>(j * CellSize), static_cast<float>(Size * CellSize)), sf::Color::Black)};
        window.draw(line, 2, sf::Lines);
    }
}

template <std::size_t N>
static void DrawNumbers(const SudokuMatrix<N> &board, sf::RenderWindow &window, sf::Font &font)
{
    constexpr std::size_t Size = N * N;
    for (std::size_t i = 0; i < Size; ++i) // Corrigido para < em vez de <=
    {
        for (std::size_t j = 0; j < Size; ++j) // Corrigido para < em vez de <=
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

template <std::size_t N, typename Solver = ISolver<N>>
static bool RunClass(const SudokuMatrix<N> &matrix, sf::RenderWindow &window, sf::Font &font)
{
    Solver solver{matrix};
    std::size_t index = 0;
    while (solver.Advance() && window.isOpen())
    {
        if (index % 1'000 != 0)
        {
            index++;
            continue;
        }
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                return solver.IsSolved();
            }
        }

        const auto &board = solver.GetBoard();
        // Limpa a janela com cor branca
        window.clear(sf::Color::White);

        // Desenha as linhas horizontais e verticais para formar a grade
        DrawLines<N>(window);

        // Desenha os números dentro das células
        DrawNumbers<N>(board, window, font);

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
        }
        if (drawn)
        {
            continue;
        }
        const auto &board = solver.GetBoard();
        // Limpa a janela com cor branca
        window.clear(sf::Color::White);

        // Desenha as linhas horizontais e verticais para formar a grade
        DrawLines<N>(window);

        // Desenha os números dentro das células
        DrawNumbers<N>(board, window, font);

        // Atualiza a janela
        window.display();
        drawn = true;
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
    static constexpr std::size_t size = 3;
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
    std::random_device device;
    pcg64 rng{device()};
    static constexpr float probability = 0.35f;
    SudokuMatrix data = CreateBoard<size>(probability, rng);
    bool canBeSolved = false;
    // static constexpr SudokuMatrix<3> data{sudokuGame};
    using doubleMilliseconds = std::chrono::duration<double, std::milli>;
    std::size_t tries = 0;
    auto startTries = std::chrono::high_resolution_clock::now();
    while (!canBeSolved)
    {
        tries++;
        DLXSolver solver{data};
        std::size_t index = 0;
        auto start = std::chrono::high_resolution_clock::now();
        while (solver.Advance())
        {
            index++;
        }
        auto end = std::chrono::high_resolution_clock::now();
        if (!solver.IsSolved())
        {
            data = CreateBoard<size>(probability, rng);
            continue;
        }
        if (!IsValidSudoku(solver.GetBoard()))
        {
            data = CreateBoard<size>(probability, rng);
            continue;
        }
        std::cout << "Solved in " << index << " tries and " << std::chrono::duration_cast<doubleMilliseconds>(end - start).count() << "ms\n";
        canBeSolved = true;
    }
    auto endTries = std::chrono::high_resolution_clock::now();
    std::cout << "Tries: " << tries << " in " << std::chrono::duration_cast<doubleMilliseconds>(endTries - startTries).count() << "ms\n";
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Sudoku Solver Visualizer");
    RunClass<size, DLXSolver<size>>(data, window, font);
    return 0;
}
