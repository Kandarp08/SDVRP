#include "../../include/intra_operator.h"
#include "../../include/delta.h"
#include "../../include/route_context.h"

// Structure representing an exchange move between two nodes in a route
struct ExchangeMove {
  Node node_a;
  Node node_b;
};

// Performs the actual node exchange in a given route
// Parameters:
// - move: The ExchangeMove containing nodes to be swapped
// - route_index: Index of the route where exchange occurs
// - solution: Current solution being modified
// - context: Route context tracking route-specific information
void DoExchange(const ExchangeMove &move, Node route_index, SpecificSolution &solution, RouteContext &context) {
  // Find predecessors and successors of nodes to be exchanged
  Node predecessor_a = solution.Predecessor(move.node_a);
  Node successor_a = solution.Successor(move.node_a);
  Node predecessor_b = solution.Predecessor(move.node_b);
  Node successor_b = solution.Successor(move.node_b);

  //Change the route by swapping the nodes
  solution.Link(predecessor_a, move.node_b);
  solution.Link(move.node_b, successor_a);
  solution.Link(predecessor_b, move.node_a);
  solution.Link(move.node_a, successor_b);

  // Update route head if the first node was the original head
  if (!predecessor_a)     
    context.SetHead(route_index, move.node_b);
   // Update route context after the exchange
  context.UpdateRouteContext(solution, route_index, predecessor_a);
}

// Inner function to calculate delta cost of exchanging two nodes
// Parameters:
// - problem: Problem instance with distance matrix
// - solution: Current solution
// - node_a: First node to be considered for exchange
// - node_b: Second node to be considered for exchange
// - best_move: Reference to store the best exchange move found
// - best_delta: Reference to track the best delta (cost improvement)
void ExchangeInner(const Problem &problem, const SpecificSolution &solution, Node node_a, Node node_b,
                    ExchangeMove &best_move, Delta<int> &best_delta) {
  Node predecessor_a = solution.Predecessor(node_a);
  Node successor_a = solution.Successor(node_a);
  Node predecessor_b = solution.Predecessor(node_b);
  Node successor_b = solution.Successor(node_b);

  // Calculate delta cost of exchanging nodes
  // Compares route distances before and after potential exchange

  int delta
      = problem.distance_matrix[solution.Customer(predecessor_a)][solution.Customer(node_b)]
        + problem.distance_matrix[solution.Customer(node_b)][solution.Customer(successor_a)]
        + problem.distance_matrix[solution.Customer(predecessor_b)][solution.Customer(node_a)]
        + problem.distance_matrix[solution.Customer(node_a)][solution.Customer(successor_b)]
        - problem.distance_matrix[solution.Customer(predecessor_a)][solution.Customer(node_a)]
        - problem.distance_matrix[solution.Customer(node_a)][solution.Customer(successor_a)]
        - problem.distance_matrix[solution.Customer(predecessor_b)][solution.Customer(node_b)]
        - problem.distance_matrix[solution.Customer(node_b)][solution.Customer(successor_b)];

  // Update best move if current exchange provides a cost improvement
  if (best_delta.Update(delta)) {
    best_move = {node_a, node_b};
  }
}

// Operator implementing the exchange move for route optimization
// Parameters:
// - problem: Problem instance with routing constraints
// - route_index: Index of the route being optimized
// - solution: Current solution to be modified
// - context: Route context tracking route-specific information
// Returns: Boolean indicating if an improvement was made
bool Exchange::operator()(const Problem &problem, Node route_index,
                                          SpecificSolution &solution, RouteContext &context) const {
  ExchangeMove best_move{};
  Delta<int> best_delta{};

  // Iterate through nodes in the route
  Node node_a = context.Head(route_index);
  while (node_a) {
    Node node_b = solution.Successor(node_a);
    if (node_b) {
      node_b = solution.Successor(node_b);
      while (node_b) {
        // Check potential exchange between nodes
        ExchangeInner(problem, solution, node_a, node_b, best_move, best_delta);
        node_b = solution.Successor(node_b);
      }
    }
    node_a = solution.Successor(node_a);
  }
  // If an improvement is found, perform the exchange
  if (best_delta.value < 0) {
    DoExchange(best_move, route_index, solution, context);
    return true;
  }
  return false;
}