#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H

#include "solution.h"
#include "problem.h"

SpecificSolution Construct(const Problem& problem);

Node CalcFleetLowerBound(const Problem &problem);

#endif