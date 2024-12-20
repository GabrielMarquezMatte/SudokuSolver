#pragma once
#include <vector>
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
struct Placement
{
    int row;
    int col;
    int digit;
};

template <std::size_t N>
class DLXSolver : ISolver<N>
{
private:
    SudokuMatrix<N> m_data;
    DLXColumn *m_header = nullptr;

    // A stack to keep track of chosen rows while solving
    std::vector<DLXNode *> m_solutionStack;

    AdvanceResult m_currentState = AdvanceResult::Continue;
    bool m_solved = false;

    // Internal helper functions:

    // Cover a column and all rows intersecting it
    inline constexpr void CoverColumn(DLXColumn *c)
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
    inline constexpr void UncoverColumn(DLXColumn *c)
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

    // Choose the next column (e.g., the one with the smallest size)
    inline constexpr DLXColumn *ChooseColumn()
    {
        DLXColumn *best = nullptr;
        std::size_t minSize = std::numeric_limits<std::size_t>::max();
        for (DLXColumn *c = (DLXColumn *)m_header->right; c != m_header; c = (DLXColumn *)c->right)
        {
            if (c->size < minSize)
            {
                minSize = c->size;
                best = c;
            }
        }
        return best;
    }

    Placement DecodePlacement(DLXNode *rowNode)
    {
        constexpr std::size_t size = N * N;
        constexpr std::size_t sizeSquared = size * size;
        constexpr std::size_t doubleSizeSquared = 2 * sizeSquared;
        constexpr int sizeInt = static_cast<int>(size);
        constexpr int sizeSquaredInt = sizeInt * sizeInt;

        std::vector<std::size_t> colIndices;
        DLXNode *cur = rowNode;
        do
        {
            DLXColumn *col = static_cast<DLXColumn *>(cur->column);
            colIndices.push_back(col->index);
            cur = cur->right;
        } while (cur != rowNode);

        // Find cell and row columns
        int cellColIndex = -1;
        int rowColIndex = -1;
        for (auto ci : colIndices)
        {
            if (ci < sizeSquared)
            {
                cellColIndex = (int)ci;
            }
            else if (ci >= sizeSquared && ci < doubleSizeSquared)
            {
                rowColIndex = (int)ci;
            }
        }

        int r = cellColIndex / sizeInt;
        int c = cellColIndex % sizeInt;
        int d = (rowColIndex - sizeSquaredInt) % sizeInt + 1;

        return {r, c, d};
    }

    void FinalizeSolution()
    {
        constexpr std::size_t size = N * N;
        constexpr std::size_t sizeSquared = size * size;
        constexpr std::size_t doubleSizeSquared = 2 * sizeSquared;
        for (auto rowNode : m_solutionStack)
        {
            // Gather all column indices of this row
            std::vector<std::size_t> colIndices;
            DLXNode *cur = rowNode;
            do
            {
                DLXColumn *col = static_cast<DLXColumn *>(cur->column);
                std::size_t colIndex = col->index;
                colIndices.push_back(colIndex);
                cur = cur->right;
            } while (cur != rowNode);

            // Now find the cellCol and rowCol
            int cellColIndex = -1;
            int rowColIndex = -1;
            for (std::size_t ci : colIndices)
            {
                if (ci < sizeSquared)
                {
                    cellColIndex = static_cast<int>(ci); // cell constraint col
                }
                else if (ci >= sizeSquared && ci < doubleSizeSquared)
                {
                    rowColIndex = static_cast<int>(ci); // row-digit constraint col
                }
            }

            // Decode r, c from cellColIndex
            int r = cellColIndex / size;
            int c = cellColIndex % size;

            // Decode d from rowColIndex
            int d = (rowColIndex - static_cast<int>(sizeSquared)) % size + 1;

            // Set the value in the SudokuMatrix
            m_data.SetValue(r, c, static_cast<char>(d));
        }
    }

public:
    DLXSolver(const SudokuMatrix<N> &data) : m_data(data)
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
        constexpr std::size_t totalCols = 4 * size * size;

        // Create column headers + header node
        m_header = new DLXColumn;
        m_header->left = m_header->right = m_header;
        m_header->up = m_header->down = m_header;
        m_header->column = m_header;
        m_header->size = 0; // not really used for header

        // Create an array of column headers
        std::vector<DLXColumn *> columns(totalCols, nullptr);
        for (std::size_t i = 0; i < totalCols; i++)
        {
            DLXColumn *col = new DLXColumn;
            col->column = col;
            col->size = 0;
            col->index = i;
            // Insert col right of m_header
            col->right = m_header;
            col->left = m_header->left;
            m_header->left->right = col;
            m_header->left = col;
            col->up = col;
            col->down = col;
            columns[i] = col;
        }

        // Function to get box index from (r,c)
        auto boxIndex = [](std::size_t r, std::size_t c)
        {
            return (r / N) * N + (c / N);
        };

        // For each cell and each possible digit, add row nodes if valid
        // If cell is pre-filled with digit d, only that (r,c,d) will be considered.
        // If cell is empty, consider all possible digits that don't violate given constraints.
        for (std::size_t r = 0; r < size; r++)
        {
            for (std::size_t c = 0; c < size; c++)
            {
                char val = m_data.GetValue(r, c);
                std::vector<char> candidates;

                if (val != 0)
                {
                    // Cell is fixed with digit val
                    candidates.push_back(val);
                }
                else
                {
                    // Cell is empty - gather all possible digits
                    // Use SudokuMatrix method or logic to find allowed digits
                    // For demonstration, assume digits 1..size are all possible.
                    // In practice, use m_data.GetPossibleValues(...) or equivalent.
                    auto possibleValues = m_data.GetPossibleValues(r, c);
                    for (auto d : possibleValues)
                    {
                        candidates.push_back(d);
                    }
                }

                for (char d : candidates)
                {
                    // Compute column indices for this candidate (r,c,d)
                    std::size_t cellCol = r * size + c;
                    std::size_t rowCol = size * size + r * size + (d - 1);
                    std::size_t colCol = 2 * size * size + c * size + (d - 1);
                    std::size_t boxCol = 3 * size * size + boxIndex(r, c) * size + (d - 1);

                    // Create 4 nodes for this row
                    DLXNode *n1 = new DLXNode;
                    DLXNode *n2 = new DLXNode;
                    DLXNode *n3 = new DLXNode;
                    DLXNode *n4 = new DLXNode;

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
                    insert_into_column(n1, columns[cellCol]);
                    insert_into_column(n2, columns[rowCol]);
                    insert_into_column(n3, columns[colCol]);
                    insert_into_column(n4, columns[boxCol]);
                }
            }
        }
    }

    ~DLXSolver()
    {
        // Clean up all allocated nodes and columns
        DLXColumn *cur = static_cast<DLXColumn *>(m_header->right);
        while (cur != m_header)
        {
            DLXColumn *next = static_cast<DLXColumn *>(cur->right);
            DLXNode *row = cur->down;
            while (row != cur)
            {
                DLXNode *nextRow = row->down;
                delete row;
                row = nextRow;
            }
            delete cur;
            cur = next;
        }
        delete m_header;
    }

    inline constexpr bool IsSolved() const override { return m_solved; }
    inline constexpr AdvanceResult GetStatus() const override { return m_currentState; }
    inline constexpr const SudokuMatrix<N> &GetBoard() const override { return m_data; }

    bool Advance() override
    {
        if (m_solved || m_currentState == AdvanceResult::Finished)
        {
            m_currentState = AdvanceResult::Finished;
            return false;
        }

        if (m_header->right == m_header)
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
            DLXNode *choice = col->down; // For now, pick the first.
                                         // In a more refined approach, you might iterate through possibilities.
            if (choice == col)
            {
                // No choices in this column
                return BackTrack();
            }

            // Try this row
            ChooseRow(choice);
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
        for (DLXNode *j = lastChoice->left; j != lastChoice; j = j->left)
        {
            UncoverColumn(static_cast<DLXColumn *>(j->column));
        }
        UncoverColumn(static_cast<DLXColumn *>(lastChoice->column));
        auto [rowIndex, colIndex, digit] = DecodePlacement(lastChoice);
        m_data.SetValue(rowIndex, colIndex, 0);

        // Now, try the next choice in the same column if any.
        // If there are no more rows in that column, we must backtrack further.
        DLXColumn *c = static_cast<DLXColumn *>(lastChoice->column);
        DLXNode *nextChoice = lastChoice->down;
        while (nextChoice != c && nextChoice != lastChoice->column)
        {
            nextChoice = nextChoice->down;
        }

        if (nextChoice == c)
        {
            // No more rows to try, backtrack more
            return BackTrack();
        }
        // Try nextChoice
        ChooseRow(nextChoice);
        return Continue();
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

    void ChooseRow(DLXNode *rowNode)
    {
        m_solutionStack.push_back(rowNode);
        // Cover the chosen row’s column first
        CoverColumn(static_cast<DLXColumn *>(rowNode->column));
        // Then cover all other columns in that row
        for (DLXNode *j = rowNode->right; j != rowNode; j = j->right)
        {
            CoverColumn(static_cast<DLXColumn *>(j->column));
        }
        auto [r, c, d] = DecodePlacement(rowNode);
        m_data.SetValue(r, c, (char)d);
    }
};