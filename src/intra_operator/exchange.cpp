#include "../../include/intra_operator.h"
#include "../../include/delta.h"
#include "../../include/route_context.h"


struct ExchangeMove {
  Node node_a;
  Node node_b;
};

void DoExchange(const ExchangeMove &move, Node route_index, SpecificSolution &solution, RouteContext &context) {
  Node predecessor_a = solution.Predecessor(move.node_a);
  Node successor_a = solution.Successor(move.node_a);
  Node predecessor_b = solution.Predecessor(move.node_b);
  Node successor_b = solution.Successor(move.node_b);
  solution.Link(predecessor_a, move.node_b);
  solution.Link(move.node_b, successor_a);
  solution.Link(predecessor_b, move.node_a);
  solution.Link(move.node_a, successor_b);
  if (!predecessor_a)     
    context.SetHead(route_index, move.node_b);
  context.UpdateRouteContext(solution, route_index, predecessor_a);
}

void ExchangeInner(const Problem &problem, const SpecificSolution &solution, Node node_a, Node node_b,
                    ExchangeMove &best_move, Delta<int> &best_delta) {
  Node predecessor_a = solution.Predecessor(node_a);
  Node successor_a = solution.Successor(node_a);
  Node predecessor_b = solution.Predecessor(node_b);
  Node successor_b = solution.Successor(node_b);
  int delta
      = problem.distance_matrix[solution.Customer(predecessor_a)][solution.Customer(node_b)]
        + problem.distance_matrix[solution.Customer(node_b)][solution.Customer(successor_a)]
        + problem.distance_matrix[solution.Customer(predecessor_b)][solution.Customer(node_a)]
        + problem.distance_matrix[solution.Customer(node_a)][solution.Customer(successor_b)]
        - problem.distance_matrix[solution.Customer(predecessor_a)][solution.Customer(node_a)]
        - problem.distance_matrix[solution.Customer(node_a)][solution.Customer(successor_a)]
        - problem.distance_matrix[solution.Customer(predecessor_b)][solution.Customer(node_b)]
        - problem.distance_matrix[solution.Customer(node_b)][solution.Customer(successor_b)];
  if (best_delta.Update(delta)) {
    best_move = {node_a, node_b};
  }
}

bool Exchange::operator()(const Problem &problem, Node route_index,
                                          SpecificSolution &solution, RouteContext &context) const {
  ExchangeMove best_move{};
  Delta<int> best_delta{};
  Node node_a = context.Head(route_index);
  while (node_a) {
    Node node_b = solution.Successor(node_a);
    if (node_b) {
      node_b = solution.Successor(node_b);
      while (node_b) {
        ExchangeInner(problem, solution, node_a, node_b, best_move, best_delta);
        node_b = solution.Successor(node_b);
      }
    }
    node_a = solution.Successor(node_a);
  }
  if (best_delta.value < 0) {
    DoExchange(best_move, route_index, solution, context);
    return true;
  }
  return false;
}