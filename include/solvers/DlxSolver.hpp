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

    // A stack to keep track of chosen rows while solving
    std::vector<DLXNode *> m_solutionStack;
    std::vector<DLXNode> m_nodes;
    std::array<DLXColumn, 4 * N * N * N * N> m_columns;

    AdvanceResult m_currentState = AdvanceResult::Continue;
    bool m_solved = false;

    // Internal helper functions:

    // Cover a column and all rows intersecting it
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

    // Uncover a column
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
        // Cobre a coluna principal
        CoverColumn(static_cast<DLXColumn *>(rowNode->column));
        // Cobre as demais colunas que este row cobre
        for (DLXNode *j = rowNode->right; j != rowNode; j = j->right)
        {
            CoverColumn(static_cast<DLXColumn *>(j->column));
        }
    }

    static inline constexpr void UncoverRow(DLXNode *rowNode)
    {
        // Descobre as colunas na ordem inversa da cobertura
        for (DLXNode *j = rowNode->left; j != rowNode; j = j->left)
        {
            UncoverColumn(static_cast<DLXColumn *>(j->column));
        }
        UncoverColumn(static_cast<DLXColumn *>(rowNode->column));
    }

    // Choose the next column (e.g., the one with the smallest size)
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

        // Find cell and row columns
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
        // Construct the exact cover matrix and initialize DLX structure.
        // Build all columns and rows corresponding to Sudoku constraints:
        // - One column for each cell-position possibility (N*N*N)
        // - One column for each row+value constraint
        // - One column for each column+value constraint
        // - One column for each box+value constraint
        // Insert nodes accordingly.
        // Set m_header to the main header node of the DLX structure.
        constexpr std::size_t size = N * N;
        constexpr std::size_t squaredSize = size * size;
        constexpr std::size_t totalCols = 4 * size * size;
        m_nodes.reserve(squaredSize * squaredSize);
        m_solutionStack.reserve(squaredSize);

        // Create column headers + header node
        m_header.left = m_header.right = &m_header;
        m_header.up = m_header.down = &m_header;
        m_header.column = &m_header;
        m_header.size = 0; // not really used for header

        // Create an array of column headers
        for (std::size_t i = 0; i < totalCols; i++)
        {
            InitializeColumn(&m_header, i);
        }

        // Function to get box index from (r,c)
        auto boxIndex = [](std::size_t r, std::size_t c)
        {
            return (r / N) * N + (c / N);
        };

        // For each cell and each possible digit, add row nodes if valid
        // If cell is pre-filled with digit d, only that (r,c,d) will be considered.
        // If cell is empty, consider all possible digits that don't violate given constraints.
        for (std::size_t row = 0; row < size; row++)
        {
            for (std::size_t column = 0; column < size; column++)
            {
                for (DataType d : GetCandidates(row, column))
                {
                    // Compute column indices for this candidate (r,c,d)
                    std::size_t cellCol = row * size + column;
                    std::size_t rowCol = squaredSize + row * size + (d - 1);
                    std::size_t colCol = 2 * squaredSize + column * size + (d - 1);
                    std::size_t boxCol = 3 * squaredSize + boxIndex(row, column) * size + (d - 1);

                    // Create 4 nodes for this row
                    DLXNode *n1 = &m_nodes.emplace_back();
                    DLXNode *n2 = &m_nodes.emplace_back();
                    DLXNode *n3 = &m_nodes.emplace_back();
                    DLXNode *n4 = &m_nodes.emplace_back();

                    // Link them horizontally
                    n1->right = n2;
                    n2->right = n3;
                    n3->right = n4;
                    n4->right = n1;
                    n1->left = n4;
                    n2->left = n1;
                    n3->left = n2;
                    n4->left = n3;

                    // Helper lambda to insert a node into a column
                    auto insert_into_column = [](DLXNode *node, DLXColumn *col)
                    {
                        node->column = col;
                        node->up = col->up;
                        node->down = col;
                        col->up->down = node;
                        col->up = node;
                        col->size++;
                    };

                    // Insert each node into corresponding column
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
            // No columns left: solution found
            m_solved = true;
            m_currentState = AdvanceResult::Finished;
            // Use m_solutionStack to fill in m_data if needed
            FinalizeSolution();
            return false;
        }

        // If we are backtracking, we should pick up where we left off.
        // Otherwise, we choose a column and try a row.
        if (m_currentState == AdvanceResult::Continue)
        {
            // Normal continuation
            DLXColumn *col = ChooseColumn();
            if (!col || col->size == 0)
            {
                // Dead end
                return BackTrack();
            }

            // Pick a row in this column to try
            DLXNode *choice = col->down;
            if (choice == col)
            {
                // No choices in this column
                return BackTrack();
            }

            // Try this row
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

        // Uncover columns in reverse order of covering:
        // We covered lastChoice->column first, then the others.
        // So we uncover in reverse:
        UncoverRow(lastChoice);

        // Now, try the next choice in the same column if any.
        // If there are no more rows in that column, we must backtrack further.
        DLXColumn *c = static_cast<DLXColumn *>(lastChoice->column);
        DLXNode *nextChoice = lastChoice->down;
        if (nextChoice == c)
        {
            // No more rows to try, backtrack more
            return BackTrack();
        }
        // Try nextChoice
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