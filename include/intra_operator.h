#ifndef INTRA_OPERATOR_H
#define INTRA_OPERATOR_H

#include "problem.h"
#include "solution.h"
#include "route_context.h"//
#include <vector>

class IntraOperator {
public:
  virtual ~IntraOperator() = default;
  virtual bool operator()(const Problem &problem, Node route_index, SpecificSolution &solution,
                          RouteContext &context) const = 0;
};

class Exchange : public IntraOperator {
public:
  bool operator()(const Problem &problem, Node route_index, SpecificSolution &solution,
                  RouteContext &context) const;
};
template <int num> class OrOpt : public IntraOperator {
public:
  bool operator()(const Problem &problem, Node route_index, SpecificSolution &solution,
                  RouteContext &context) const override;
};

#endif