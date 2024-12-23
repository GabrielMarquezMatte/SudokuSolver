#pragma once
#include <cstddef>
#include "./StateMachineStatus.hpp"
#include "../SudokuMatrix.hpp"
template<std::size_t N>
class ISolver
{
public:
    using DataType = typename SudokuMatrix<N>::DataType;
    virtual bool Advance() = 0;
    virtual AdvanceResult GetStatus() const noexcept = 0;
    virtual const SudokuMatrix<N> &GetBoard() const noexcept = 0;
    virtual bool IsSolved() const noexcept = 0;
    virtual ~ISolver() = default;
};

class IDynamicSolver
{
public:
    using DataType = typename DynamicSudokuMatrix::DataType;
    virtual bool Advance() = 0;
    virtual AdvanceResult GetStatus() const noexcept = 0;
    virtual const DynamicSudokuMatrix &GetBoard() const noexcept = 0;
    virtual bool IsSolved() const noexcept = 0;
    virtual ~IDynamicSolver() = default;
};