#ifndef SOLVER_H
#define SOLVER_H

#include "problem.h"
#include "solution.h"


template <typename SolutionType, typename ProblemType> class Solver {
    public:
        virtual SolutionType Solve(const ProblemType &problem) = 0;
};

class SpecificSolver : public Solver<SpecificSolution, Problem> {
    public:
        SpecificSolution Solve(const Problem &problem) override;
};



///  Main function for solving the problem.
/// problem The problem instance.
/// return The solution to the problem.

#endif