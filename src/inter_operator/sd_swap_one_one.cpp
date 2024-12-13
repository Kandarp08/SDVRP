#include "../../include/inter_operator.h"
#include "../../include/base_cache.h"
#include "../../include/base_star.h"
#include "../../include/route_head_guard.h"

// Structure representing a Split Delivery Swap One-One Move
// This structure captures the details of a potential route modification 
// involving swapping a single node between two different routes
struct SdSwapOneOneMove {
  bool swapped;
  Node route_x, route_y;
  Node node_x, predecessor_x, successor_x;
  Node node_y, predecessor_y, successor_y;
  int split_load;
};

// Executes the Split Delivery Swap One-One Move by modifying the solution
// This function physically performs the node swapping between routes
void DoSdSwapOneOne(const SdSwapOneOneMove &move, SpecificSolution &solution, RouteContext &context) {
  Node predecessor_y = solution.Predecessor(move.node_y);
  Node successor_y = solution.Successor(move.node_y);
  {
    RouteHeadGuard guard(solution, context, move.route_x);
    solution.SetLoad(move.node_x, move.split_load);
    solution.Link(move.predecessor_y, move.node_y);
    solution.Link(move.node_y, move.successor_y);
  }
  {
    RouteHeadGuard guard(solution, context, move.route_y);
    solution.Link(predecessor_y, successor_y);
    solution.Insert(solution.Customer(move.node_x), solution.Load(move.node_y),
                    move.predecessor_x, move.successor_x);
  }
}

// Inner function to evaluate a potential Split Delivery Swap One-One Move
// This function calculates the cost delta and finds the best insertion point
void SdSwapOneOneInner(const Problem &problem, const SpecificSolution &solution,
                        [[maybe_unused]] const RouteContext &context, bool swapped, Node route_x,
                        Node route_y, Node node_x, Node node_y, int split_load,
                        BaseCache<SdSwapOneOneMove> &cache) {
  Node predecessor_x = solution.Predecessor(node_x);
  Node successor_x = solution.Successor(node_x);
  Node predecessor_y = solution.Predecessor(node_y);
  Node successor_y = solution.Successor(node_y);
  int delta = -CalcDelta(problem, solution, node_y, predecessor_y, successor_y);
  int delta_x = CalcDelta(problem, solution, node_x, predecessor_y, successor_y);
  int before = CalcDelta(problem, solution, node_y, predecessor_x, node_x);
  int after = CalcDelta(problem, solution, node_y, node_x, successor_x);
  int delta_y;
  Node predecessor;
  Node successor;
  if (before <= after) {
    predecessor = predecessor_x;
    successor = node_x;
    delta_y = before;
  } else {
    predecessor = node_x;
    successor = successor_x;
    delta_y = after;
  }
  delta += delta_x + delta_y;
  if (cache.delta.Update(delta)) {
    cache.move = {swapped,     route_x, route_y,     node_x,    predecessor_y,
                  successor_y, node_y,  predecessor, successor, split_load};
  }
}

// Overloaded inner function to iterate through nodes in two routes
void SdSwapOneOneInner(const Problem &problem, const SpecificSolution &solution,
                        const RouteContext &context, Node route_x, Node route_y,
                        BaseCache<SdSwapOneOneMove> &cache) {
  for (Node node_x = context.Head(route_x); node_x; node_x = solution.Successor(node_x)) {
    int load_x = solution.Load(node_x);
    for (Node node_y = context.Head(route_y); node_y; node_y = solution.Successor(node_y)) {
      int load_y = solution.Load(node_y);
      if (load_x > load_y) {
        SdSwapOneOneInner(problem, solution, context, false, route_x, route_y, node_x, node_y,
                          load_x - load_y, cache);
      } else if (load_y > load_x) {
        SdSwapOneOneInner(problem, solution, context, true, route_y, route_x, node_y, node_x,
                          load_y - load_x, cache);
      }
    }
  }
}

// Main operator function implementing the Split Delivery Swap One-One Move
// This function finds and applies the best route modification
std::vector<Node> SdSwapOneOne::operator()(const Problem &problem,
                                                            SpecificSolution &solution,
                                                            RouteContext &context,
                                                            CacheMap &cache_map) const {
  auto &caches = cache_map.Get<InterRouteCache<SdSwapOneOneMove>>(solution, context);
  SdSwapOneOneMove best_move{};
  Delta<int> best_delta{};
  for (Node route_x = 0; route_x < context.NumRoutes(); ++route_x) {
    for (Node route_y = route_x + 1; route_y < context.NumRoutes(); ++route_y) {
      auto &cache = caches.Get(route_x, route_y);
      if (!cache.TryReuse()) {
        SdSwapOneOneInner(problem, solution, context, route_x, route_y, cache);
      } else {
        if (!cache.move.swapped) {
          cache.move.route_x = route_x;
          cache.move.route_y = route_y;
        } else {
          cache.move.route_x = route_y;
          cache.move.route_y = route_x;
        }
      }
      if (best_delta.Update(cache.delta)) {
        best_move = cache.move;
      }
    }
  }

  // Apply the best move if it improves the solution
  if (best_delta.value < 0) {
    DoSdSwapOneOne(best_move, solution, context);
    return {best_move.route_x, best_move.route_y};
  }
  // Return empty vector if no improvement found
  return {};
}