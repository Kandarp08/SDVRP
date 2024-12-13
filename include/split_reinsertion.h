#ifndef SPLIT_REINSERTION_H
#define SPLIT_REINSERTION_H

#include "problem.h"
#include "solution.h"
#include "route_context.h"

// Perform split reinsertion to reallocate customer demand across routes.
void SplitReinsertion(const Problem& problem, Node customer, int demand, double blinkRate,
                        SpecificSolution &solution, RouteContext &context);


#endif