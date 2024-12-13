#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H

#include "solution.h"
#include "problem.h"

// Constructs an initial feasible solution
SpecificSolution Construct(const Problem& problem);

Node CalcFleetLowerBound(const Problem &problem);

#endif