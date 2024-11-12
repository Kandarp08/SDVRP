#include "split_reinsertion.h"
#include "../include/utils.h"
#include <algorithm>
#include <vector>

struct SplitReinsertionMove {
    InsertionWithCost<int> insertion;
    int residual;
    SplitReinsertionMove(const InsertionWithCost<int> &insertion, int residual)
        : insertion(insertion), residual(residual) {}
};

void SplitReinsertion(const Problem& problem, Node customer, int demand, double blink_rate,
                      SpecificSolution& solution, RouteContext& context) {
    auto func = [&](Node predecessor, Node successor, Node customer) {
        Node preCustomer = solution.Customer(predecessor);
        Node sucCustomer = solution.Customer(successor);
        return problem.distance_matrix[customer][preCustomer]
               + problem.distance_matrix[customer][sucCustomer]
               - problem.distance_matrix[preCustomer][sucCustomer];
    };

    std::vector<SplitReinsertionMove> moves;
    moves.reserve(context.NumRoutes());
    int sumResidual = 0;

    for (Node routeIndex = 0; routeIndex < context.NumRoutes(); ++routeIndex) {
        int residual = std::min(demand, problem.capacity - context.Load(routeIndex));
        if (residual > 0) {
            auto insertion = CalcBestInsertion(solution, func, context, routeIndex, customer);
            moves.emplace_back(insertion, residual);
            sumResidual += residual;
        }
    }

    if (sumResidual < demand) {
        return;
    }

    std::stable_sort(moves.begin(), moves.end(),
                     [](const SplitReinsertionMove& lhs, const SplitReinsertionMove& rhs) {
                         return lhs.insertion.cost.value * rhs.residual
                                < rhs.insertion.cost.value * lhs.residual;
                     });

    for (const auto& move : moves) {
        sumResidual -= move.residual;
      if (sumResidual >= demand && rand() < blink_rate) {
        continue;
      }
      int load = std::min(demand, move.residual);
      Node nodeIndex = solution.Insert(customer, load, move.insertion.predecessor, move.insertion.successor);
      if (move.insertion.predecessor == 0) {
           context.SetHead(move.insertion.route_index, nodeIndex);
      }
      context.UpdateRouteContext(solution, move.insertion.route_index, move.insertion.predecessor);
      demand -= load;
      if (demand == 0) {
          break;
      }
  }
}

