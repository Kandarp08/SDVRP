#include "../../include/inter_operator.h"
#include "../../include/base_cache.h"

using namespace std;

  // Structure representing a cross move between two routes
  struct CrossMove {
    bool reversed;
    Node route_x, route_y;
    Node left_x, left_y;
  };

  // Performs the actual cross move between two routes
  // Parameters:
  // - move: The CrossMove containing details of the segment exchange
  // - solution: Current solution being modified
  // - context: Route context tracking route-specific information
  void DoCross(const CrossMove &move, SpecificSolution &solution, RouteContext &context) {
    Node right_x = move.left_x ? solution.Successor(move.left_x) : context.Head(move.route_x);
    Node right_y = move.left_y ? solution.Successor(move.left_y) : context.Head(move.route_y);
    if (!move.reversed) {
      solution.Link(move.left_x, right_y);
      solution.Link(move.left_y, right_x);
      if (move.left_x == 0) {
        context.SetHead(move.route_x, right_y);
      }
      if (move.left_y == 0) {
        context.SetHead(move.route_y, right_x);
      }
    } else {
      Node head_y = context.Head(move.route_y);
      if (right_x) {
        Node tail_x = context.Tail(move.route_x);
        solution.ReversedLink(right_x, tail_x, 0, right_y);
        context.SetHead(move.route_y, tail_x);
      } else {
        solution.Link(0, right_y);
        context.SetHead(move.route_y, right_y);
      }
      solution.SetSuccessor(0, context.Head(move.route_x));
      if (move.left_y) {
        solution.ReversedLink(head_y, move.left_y, move.left_x, 0);
      } else {
        solution.Link(move.left_x, 0);
      }
      context.SetHead(move.route_x, solution.Successor(0));
    }
  }

  // Inner function to calculate cross move possibilities between two routes
  // Parameters:
  // - problem: Problem instance with routing constraints
  // - solution: Current solution
  // - context: Route context
  // - route_x: First route index
  // - route_y: Second route index
  // - cache: Cache to store the best move
  void CrossInner(const Problem& problem, const SpecificSolution &solution, const RouteContext &context,
                  Node route_x, Node route_y, BaseCache<CrossMove> &cache) {
    Node left_x = 0;
    do {
      Node successor_x = left_x ? solution.Successor(left_x) : context.Head(route_x);
      Node left_y = 0;
      do {
        Node predecessor_y = left_y;
        Node successor_y = left_y ? solution.Successor(left_y) : context.Head(route_y);
        int predecessor_load_x = context.PreLoad(left_x);
        int successor_load_x = context.Load(route_x) - predecessor_load_x;
        int predecessor_load_y = context.PreLoad(left_y);
        int successor_load_y = context.Load(route_y) - predecessor_load_y;
        int base
            = -problem.distance_matrix[solution.Customer(left_x)][solution.Customer(successor_x)]
              - problem.distance_matrix[solution.Customer(left_y)][solution.Customer(successor_y)];
        for (bool reversed : {false, true}) {
          if (predecessor_load_x + successor_load_y <= problem.capacity
              && successor_load_x + predecessor_load_y <= problem.capacity) {
            int delta
                = base
                  + problem
                        .distance_matrix[solution.Customer(left_x)][solution.Customer(successor_y)]
                  + problem.distance_matrix[solution.Customer(successor_x)]
                                           [solution.Customer(predecessor_y)];
            if (cache.delta.Update(delta)) {
              cache.move = {reversed, route_x, route_y, left_x, left_y};
            }
          }
          std::swap(predecessor_y, successor_y);
          std::swap(predecessor_load_y, successor_load_y);
        }
        left_y = successor_y;
      } while (left_y);
      left_x = successor_x;
    } while (left_x);
  }

  // Operator implementing the cross move for inter-route optimization
  // Parameters:
  // - problem: Problem instance with routing constraints
  // - solution: Current solution to be modified
  // - context: Route context tracking route-specific information
  // - cache_map: Cache management for move calculations
  // Returns: Vector of modified route indices
  vector<Node> Cross::operator()(const Problem& problem, SpecificSolution &solution,
                                                      RouteContext &context,
                                                      CacheMap &cache_map) const {
    auto &caches = cache_map.Get<InterRouteCache<CrossMove>>(solution, context);
    CrossMove best_move{};
    Delta<int> best_delta{};
    for (Node route_x = 0; route_x < context.NumRoutes(); ++route_x) {
      for (Node route_y = route_x + 1; route_y < context.NumRoutes(); ++route_y) {
        auto &cache = caches.Get(route_x, route_y);
        if (!cache.TryReuse()) {
          CrossInner(problem, solution, context, route_x, route_y, cache);
        } else {
          cache.move.route_x = route_x;
          cache.move.route_y = route_y;
        }
        if (best_delta.Update(cache.delta)) {
          best_move = cache.move;
        }
      }
    }
    if (best_delta.value < 0) {
      DoCross(best_move, solution, context);
      return {best_move.route_x, best_move.route_y};
    }
    return {};
  }