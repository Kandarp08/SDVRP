#include "../../include/intra_operator.h"
#include "../../include/delta.h"
#include "../../include/route_context.h"

// Structure representing an Or-opt move in route optimization
struct OrOptMove {
  bool reversed;
  Node head;
  Node tail;
  Node predecessor;
  Node successor;
};

// Performs the actual Or-opt move in a given route
// Parameters:
// - move: The OrOptMove containing details of the segment to be moved
// - route_index: Index of the route where move occurs
// - solution: Current solution being modified
// - context: Route context tracking route-specific information
void DoOrOpt(const OrOptMove &move, Node route_index, SpecificSolution &solution, RouteContext &context) {
  // Find predecessor of head and successor of tail in original route
  Node predecessor_head = solution.Predecessor(move.head);
  Node successor_tail = solution.Successor(move.tail);

  // Temporarily set initial successor to route head
  solution.SetSuccessor(0, context.Head(route_index));
  solution.Link(predecessor_head, successor_tail);

  // Insert segment based on whether it needs to be reversed
  if (!move.reversed) {// Normal insertion
    solution.Link(move.predecessor, move.head);
    solution.Link(move.tail, move.successor);
  } else {// Reversed segment insertion
    solution.ReversedLink(move.head, move.tail, move.predecessor, move.successor);
  }
  // Update route head
  context.SetHead(route_index, solution.Successor(0));
}

// Inner function to calculate delta cost of Or-opt move
// Template parameter num: number of consecutive nodes to move (1, 2 or 3)
// Parameters:
// - problem: Problem instance with distance matrix
// - solution: Current solution
// - head: Start of segment to move
// - tail: End of segment to move
// - predecessor: Potential insertion point predecessor
// - successor: Potential insertion point successor
// - best_move: Reference to store the best Or-opt move found
// - best_delta: Reference to track the best delta (cost improvement)
template <int num> void OrOptInner(const Problem &problem, const SpecificSolution &solution, Node head,
                                    Node tail, Node predecessor, Node successor,
                                    OrOptMove &best_move, Delta<int> &best_delta) {
  // Find predecessor of head and successor of tail in original route
  Node predecessor_head = solution.Predecessor(head);
  Node successor_tail = solution.Successor(tail);

  // Calculate base delta by removing the segment
  int delta
      = problem.distance_matrix[solution.Customer(predecessor_head)]
                                [solution.Customer(successor_tail)]
        - problem.distance_matrix[solution.Customer(predecessor_head)][solution.Customer(head)]
        - problem.distance_matrix[solution.Customer(tail)][solution.Customer(successor_tail)]
        - problem.distance_matrix[solution.Customer(predecessor)][solution.Customer(successor)];
  
  bool reversed = false; // Flag to track if segment needs to be reversed
  
  // Calculate delta for normal insertion
  int insertion_delta
      = problem.distance_matrix[solution.Customer(predecessor)][solution.Customer(head)]
        + problem.distance_matrix[solution.Customer(successor)][solution.Customer(tail)];

  // For moves involving more than one node, check reversed insertion
  if (num > 1) {
    int reversed_delta
        = problem.distance_matrix[solution.Customer(predecessor)][solution.Customer(tail)]
          + problem.distance_matrix[solution.Customer(successor)][solution.Customer(head)];
    // Update if reversed insertion is more beneficial
    if (reversed_delta < insertion_delta) {
      insertion_delta = reversed_delta;
      reversed = true;
    }
  }
  delta += insertion_delta;
  // Update best move if current move provides cost improvement
  if (best_delta.Update(delta)) {
    best_move = {reversed, head, tail, predecessor, successor};
  }
}

// Operator implementing the Or-opt move for route optimization
// Template parameter num: number of consecutive nodes to move (1, 2, or 3)
// Parameters:
// - problem: Problem instance with routing constraints
// - route_index: Index of the route being optimized
// - solution: Current solution to be modified
// - context: Route context tracking route-specific information
// Returns: Boolean indicating if an improvement was made
template <int num>
bool OrOpt<num>::operator()(const Problem &problem, Node route_index,
                                            SpecificSolution &solution, RouteContext &context) const {
  OrOptMove best_move{};
  Delta<int> best_delta{};
  Node head = context.Head(route_index);
  Node tail = head;
  for (Node i = 0; tail && i < num - 1; ++i) {
    tail = solution.Successor(tail);
  }

  // Iterate through all possible segment moves
  while (tail) {
    Node predecessor, successor;
    // Check insertions after current tail
    predecessor = solution.Successor(tail);
    while (predecessor) {
      successor = solution.Successor(predecessor);
      OrOptInner<num>(problem, solution, head, tail, predecessor, successor, best_move,
                      best_delta);
      predecessor = successor;
    }
    // Check insertions before current head
    successor = solution.Predecessor(head);
    while (successor) {
      predecessor = solution.Predecessor(successor);
      OrOptInner<num>(problem, solution, head, tail, predecessor, successor, best_move,
                      best_delta);
      successor = predecessor;
    }
    // Move to next segment
    head = solution.Successor(head);
    tail = solution.Successor(tail);
  }

  // If an improvement is found, perform the Or-opt move
  if (best_delta.value < 0) {
    DoOrOpt(best_move, route_index, solution, context);
    context.UpdateRouteContext(solution, route_index, 0);
    return true;
  }
  return false;
}

template class OrOpt<1>;
template class OrOpt<2>;
template class OrOpt<3>;