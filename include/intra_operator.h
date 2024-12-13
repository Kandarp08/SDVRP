#ifndef INTRA_OPERATOR_H
#define INTRA_OPERATOR_H

#include "problem.h"
#include "solution.h"
#include "route_context.h"//
#include <vector>

// Base class for intra-operators
class IntraOperator {
public:
  virtual ~IntraOperator() = default;
  virtual bool operator()(const Problem &problem, Node route_index, SpecificSolution &solution,
                          RouteContext &context) const = 0;
};

// This operator swaps the positions of two nodes within a single route.
class Exchange : public IntraOperator {
public:
  bool operator()(const Problem &problem, Node route_index, SpecificSolution &solution,
                  RouteContext &context) const;
};

// This operator moves consecutive `num` nodes from one position in a route to another.
template <int num> class OrOpt : public IntraOperator {
public:
  bool operator()(const Problem &problem, Node route_index, SpecificSolution &solution,
                  RouteContext &context) const override;
};

#endif