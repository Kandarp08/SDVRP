#ifndef INTER_OPERATOR_H
#define INTER_OPERATOR_H

#include "problem.h"
#include "solution.h"
#include "route_context.h"
#include "cache.h"
#include <vector>

  // Base class for inter-operators
  class InterOperator {
  public:

    virtual ~InterOperator() = default;

    /*A vector of route indices representing the modified routes. Empty if the operator
      fails to optimize the solution. */
    virtual std::vector<Node> operator()(const Problem &problem, SpecificSolution &solution,
                                         RouteContext &context, 
                                         CacheMap &cache_map) const = 0;
  };

  // Inter-operator that performs a Swap(num_x, num_y) operation.
  template <int num_x, int num_y> class Swap : public InterOperator {
  public:
    std::vector<Node> operator()(const Problem &problem, SpecificSolution &solution, RouteContext &context,
                                 CacheMap &cache_map) const override;
  };

  // Inter-operator that performs a Relocate operation.
  class Relocate : public InterOperator {
  public:
    std::vector<Node> operator()(const Problem &problem, SpecificSolution &solution, RouteContext &context,
                                 CacheMap &cache_map) const;
  };

  // Inter-operator that performs a Swap* operation.
  class SwapStar : public InterOperator {
  public:
    std::vector<Node> operator()(const Problem &problem, SpecificSolution &solution, RouteContext &context,
                                 CacheMap &cache_map) const;
  };

  /**
   * @class Cross
   * @brief Inter-operator that performs a Cross operation.
   */
  class Cross : public InterOperator {
  public:
    std::vector<Node> operator()(const Problem &problem, SpecificSolution &solution, RouteContext &context,
                                 CacheMap &cache_map) const;
  };

  // Inter-operator that performs a SD-Swap* operation.
  class SdSwapStar : public InterOperator {
  public:
    std::vector<Node> operator()(const Problem &problem, SpecificSolution &solution, RouteContext &context,
                                 CacheMap &cache_map) const;
  };

  // Inter-operator that performs a SD-Swap(1, 1) operation.
  class SdSwapOneOne : public InterOperator {
  public:
    std::vector<Node> operator()(const Problem &problem, SpecificSolution &solution, RouteContext &context,
                                 CacheMap &cache_map) const;
  };

  // Inter-operator that performs a SD-Swap(2, 1) operation.
  class SdSwapTwoOne : public InterOperator {
  public:
    std::vector<Node> operator()(const Problem &problem, SpecificSolution &solution, RouteContext &context,
                                CacheMap &cache_map) const;
  };

#endif