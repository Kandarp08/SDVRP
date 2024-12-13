#ifndef REPAIR_H
#define REPAIR_H

#include "problem.h"
#include "solution.h"

#include "route_context.h"

// Repairs a route by merging duplicate customers and optimizing node placements.
void Repair(const Problem& problem, Node route_index, SpecificSolution &solution, RouteContext &context);

#endif