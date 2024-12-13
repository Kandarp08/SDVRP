#ifndef ROUTE_HEAD_GUARD
#define ROUTE_HEAD_GUARD

#include "solution.h"
#include "route_context.h"

// Makes sure that the route starts and ends at the depot (Node 0)
struct RouteHeadGuard {
    SpecificSolution &solution;
    RouteContext &context;
    Node route_index;
    RouteHeadGuard(SpecificSolution &solution, RouteContext &context, Node route_index)
        : solution(solution), context(context), route_index(route_index) {
      solution.SetSuccessor(0, context.Head(route_index));
    }
    ~RouteHeadGuard() { context.SetHead(route_index, solution.Successor(0)); }
  };


#endif