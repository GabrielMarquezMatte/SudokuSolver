#pragma once
#include <cstddef>
#include "./StateMachineStatus.hpp"
template<std::size_t N>
class ISolver
{
public:
    virtual bool Advance() = 0;
    virtual AdvanceResult GetStatus() const = 0;
    virtual const SudokuMatrix<N> &GetBoard() const = 0;
    virtual bool IsSolved() const = 0;
    virtual ~ISolver() = default;
};