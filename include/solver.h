#ifndef SOLVER_H
#define SOLVER_H


#include "problem.h"
#include "config.h"
#include "solution.h"


template <typename SolutionType, typename ConfigType, typename ProblemType> class Solver {
    public:
        virtual SolutionType Solve(const ConfigType &config, const ProblemType &problem) = 0;
};

class SpecificSolver : public Solver<SpecificSolution, SpecificConfig, Problem> {
    public:
        SpecificSolution Solve( const SpecificConfig &config,const Problem &problem) override;
};



///  Main function for solving the problem.
/// problem The problem instance.
/// return The solution to the problem.

#endif