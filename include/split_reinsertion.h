#ifndef SPLIT_REINSERTION_H
#define SPLIT_REINSERTION_H

#include <../include/problem.h>
#include <../include/solution.h>
#include "../include/route_context.h"

void SplitReinsertion(const Problem& problem, Node customer, int demand, double blinkRate,
                        SpecificSolution &solution, RouteContext &context);


#endif