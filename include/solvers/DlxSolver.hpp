#pragma once
#include <vector>
#include <span>
#include "./StateMachineStatus.hpp"
#include "./ISolver.hpp"
#include "../SudokuMatrix.hpp"

struct DLXNode
{
    DLXNode *left;
    DLXNode *right;
    DLXNode *up;
    DLXNode *down;
    DLXNode *column;
};

struct DLXColumn : public DLXNode
{
    std::size_t size; // Number of rows currently active in this column
    std::size_t index;
    // Additional metadata can go here
};
template <typename T>
struct Placement
{
    std::size_t row;
    std::size_t col;
    T digit;
};

template <std::size_t N>
class DLXSolver : public ISolver<N>
{
public:
    using DataType = typename SudokuMatrix<N>::DataType;

private:
    SudokuMatrix<N> m_data;
    DLXColumn m_header = {};
    std::vector<DLXNode *> m_solutionStack;
    std::vector<DLXNode> m_nodes;
    std::array<DLXColumn, 4 * N * N * N * N> m_columns;
    AdvanceResult m_currentState = AdvanceResult::Continue;
    bool m_solved = false;
    static inline constexpr void CoverColumn(DLXColumn *c)
    {
        c->right->left = c->left;
        c->left->right = c->right;
        for (DLXNode *i = c->down; i != c; i = i->down)
        {
            for (DLXNode *j = i->right; j != i; j = j->right)
            {
                j->up->down = j->down;
                j->down->up = j->up;
                static_cast<DLXColumn *>(j->column)->size--;
            }
        }
    }
    static inline constexpr void UncoverColumn(DLXColumn *c)
    {
        for (DLXNode *i = c->up; i != c; i = i->up)
        {
            for (DLXNode *j = i->left; j != i; j = j->left)
            {
                static_cast<DLXColumn *>(j->column)->size++;
                j->up->down = j;
                j->down->up = j;
            }
        }
        c->right->left = c;
        c->left->right = c;
    }

    static inline constexpr void CoverRow(DLXNode *rowNode)
    {
        CoverColumn(static_cast<DLXColumn *>(rowNode->column));
        for (DLXNode *j = rowNode->right; j != rowNode; j = j->right)
        {
            CoverColumn(static_cast<DLXColumn *>(j->column));
        }
    }

    static inline constexpr void UncoverRow(DLXNode *rowNode)
    {
        for (DLXNode *j = rowNode->left; j != rowNode; j = j->left)
        {
            UncoverColumn(static_cast<DLXColumn *>(j->column));
        }
        UncoverColumn(static_cast<DLXColumn *>(rowNode->column));
    }

    inline constexpr DLXColumn *ChooseColumn() const noexcept
    {
        DLXColumn *best = nullptr;
        std::size_t minSize = std::numeric_limits<std::size_t>::max();
        for (DLXColumn *c = (DLXColumn *)m_header.right; c != &m_header; c = (DLXColumn *)c->right)
        {
            if (c->size < minSize)
            {
                minSize = c->size;
                best = c;
            }
        }
        return best;
    }

    constexpr std::pair<std::size_t, std::size_t> GetRowColIndices(const std::span<const std::size_t> indices) const noexcept
    {
        constexpr std::size_t size = N * N;
        constexpr std::size_t sizeSquared = size * size;
        constexpr std::size_t doubleSizeSquared = 2 * sizeSquared;
        std::size_t cellColIndex = std::numeric_limits<std::size_t>::min();
        std::size_t rowColIndex = std::numeric_limits<std::size_t>::min();
        for (const std::size_t ci : indices)
        {
            if (ci < sizeSquared)
            {
                cellColIndex = ci;
            }
            else if (ci >= sizeSquared && ci < doubleSizeSquared)
            {
                rowColIndex = ci;
            }
        }
        return {cellColIndex, rowColIndex};
    }

    constexpr Placement<DataType> DecodePlacement(DLXNode *rowNode) const
    {
        static constexpr std::size_t size = N * N;
        static constexpr std::size_t sizeSquared = size * size;

        std::vector<std::size_t> colIndices;
        DLXNode *cur = rowNode;
        do
        {
            DLXColumn *col = static_cast<DLXColumn *>(cur->column);
            colIndices.push_back(col->index);
            cur = cur->right;
        } while (cur != rowNode);

        auto [cellColIndex, rowColIndex] = GetRowColIndices(colIndices);
        std::size_t r = cellColIndex / size;
        std::size_t c = cellColIndex % size;
        DataType d = static_cast<DataType>((rowColIndex - sizeSquared) % size + 1);
        return {r, c, d};
    }

    inline constexpr void FinalizeSolution()
    {
        for (const auto rowNode : m_solutionStack)
        {
            auto [r, c, d] = DecodePlacement(rowNode);
            m_data.SetValue(r, c, d);
        }
    }

    inline constexpr BitSetIterator<N> GetCandidates(std::size_t row, std::size_t column)
    {
        DataType val = m_data.GetValue(row, column);
        if (val != 0)
        {
            std::size_t value = val - 1;
            typename BitSetIterator<N>::FlagType flag;
            flag.set(value);
            return {flag};
        }
        return m_data.GetPossibleValues(row, column);
    }

    inline constexpr DLXColumn *InitializeColumn(DLXColumn *header, std::size_t index)
    {
        DLXColumn &col = m_columns[index];
        DLXColumn *ptr = &col;
        col.column = ptr;
        col.size = 0;
        col.index = index;
        col.right = header;
        col.left = header->left;
        header->left->right = ptr;
        header->left = ptr;
        col.up = ptr;
        col.down = ptr;
        return ptr;
    }

public:
    constexpr DLXSolver(const SudokuMatrix<N> &data) : m_data(data)
    {
        constexpr std::size_t size = N * N;
        constexpr std::size_t squaredSize = size * size;
        constexpr std::size_t totalCols = 4 * size * size;
        m_nodes.reserve(squaredSize * squaredSize);
        m_solutionStack.reserve(squaredSize);
        m_header.left = m_header.right = &m_header;
        m_header.up = m_header.down = &m_header;
        m_header.column = &m_header;
        m_header.size = 0;
        for (std::size_t i = 0; i < totalCols; i++)
        {
            InitializeColumn(&m_header, i);
        }
        auto boxIndex = [](std::size_t r, std::size_t c)
        {
            return (r / N) * N + (c / N);
        };
        for (std::size_t row = 0; row < size; row++)
        {
            for (std::size_t column = 0; column < size; column++)
            {
                for (DataType d : GetCandidates(row, column))
                {
                    std::size_t cellCol = row * size + column;
                    std::size_t rowCol = squaredSize + row * size + (d - 1);
                    std::size_t colCol = 2 * squaredSize + column * size + (d - 1);
                    std::size_t boxCol = 3 * squaredSize + boxIndex(row, column) * size + (d - 1);

                    DLXNode *n1 = &m_nodes.emplace_back();
                    DLXNode *n2 = &m_nodes.emplace_back();
                    DLXNode *n3 = &m_nodes.emplace_back();
                    DLXNode *n4 = &m_nodes.emplace_back();

                    n1->right = n2;
                    n2->right = n3;
                    n3->right = n4;
                    n4->right = n1;
                    n1->left = n4;
                    n2->left = n1;
                    n3->left = n2;
                    n4->left = n3;
                    auto insert_into_column = [](DLXNode *node, DLXColumn *col)
                    {
                        node->column = col;
                        node->up = col->up;
                        node->down = col;
                        col->up->down = node;
                        col->up = node;
                        col->size++;
                    };
                    insert_into_column(n1, &m_columns[cellCol]);
                    insert_into_column(n2, &m_columns[rowCol]);
                    insert_into_column(n3, &m_columns[colCol]);
                    insert_into_column(n4, &m_columns[boxCol]);
                }
            }
        }
    }

    inline constexpr bool IsSolved() const noexcept override { return m_solved; }
    inline constexpr AdvanceResult GetStatus() const noexcept override { return m_currentState; }
    inline constexpr const SudokuMatrix<N> &GetBoard() const noexcept override { return m_data; }

    constexpr bool Advance(bool insertEveryStep)
    {
        if (m_solved || m_currentState == AdvanceResult::Finished)
        {
            m_currentState = AdvanceResult::Finished;
            return false;
        }

        if (m_header.right == &m_header)
        {
            m_solved = true;
            m_currentState = AdvanceResult::Finished;
            FinalizeSolution();
            return false;
        }

        if (m_currentState == AdvanceResult::Continue)
        {
            DLXColumn *col = ChooseColumn();
            if (!col || col->size == 0)
            {
                return BackTrack();
            }
            DLXNode *choice = col->down;
            if (choice == col)
            {
                return BackTrack();
            }
            ChooseRow(choice, insertEveryStep);
            return Continue();
        }
        if (m_solutionStack.empty())
        {
            m_currentState = AdvanceResult::Finished;
            return false;
        }

        DLXNode *lastChoice = m_solutionStack.back();
        m_solutionStack.pop_back();
        UncoverRow(lastChoice);
        DLXColumn *c = static_cast<DLXColumn *>(lastChoice->column);
        DLXNode *nextChoice = lastChoice->down;
        if (nextChoice == c)
        {
            return BackTrack();
        }
        ChooseRow(nextChoice, insertEveryStep);
        return Continue();
    }
    inline constexpr bool Advance() override
    {
        return Advance(true);
    }

private:
    inline constexpr bool Continue()
    {
        m_currentState = AdvanceResult::Continue;
        return true;
    }

    inline constexpr bool BackTrack()
    {
        m_currentState = AdvanceResult::BackTracking;
        return true;
    }

    inline constexpr void ChooseRow(DLXNode *rowNode, bool insertValue)
    {
        m_solutionStack.push_back(rowNode);
        CoverRow(rowNode);
        if (insertValue)
        {
            auto [r, c, d] = DecodePlacement(rowNode);
            m_data.SetValue(r, c, d);
        }
    }
};