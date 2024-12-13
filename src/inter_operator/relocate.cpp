#include "../../include/base_cache.h"
#include "../../include/base_star.h"
#include "../../include/route_head_guard.h"

  // Structure representing a relocate move between routes
  struct RelocateMove {
    Node route_x, route_y;
    Node node_x, predecessor_x, successor_x;
  };

  // Performs the actual node relocation between routes
  // Parameters:
  // - move: The RelocateMove containing details of node relocation
  // - solution: Current solution being modified
  // - context: Route context tracking route-specific information
  void DoRelocate(RelocateMove &move, SpecificSolution &solution, RouteContext &context) {
    Node predecessor_x = solution.Predecessor(move.node_x);
    Node successor_x = solution.Successor(move.node_x);
    {
      RouteHeadGuard guard(solution, context, move.route_x);
      solution.Link(predecessor_x, successor_x);
    }
    {
      RouteHeadGuard guard(solution, context, move.route_y);
      solution.Link(move.predecessor_x, move.node_x);
      solution.Link(move.node_x, move.successor_x);
    }
  }
  
  // Inner function to calculate relocate move possibilities between two routes
  // Parameters:
  // - problem: Problem instance with routing constraints
  // - solution: Current solution
  // - context: Route context
  // - route_x: Source route index
  // - route_y: Destination route index
  // - cache: Cache to store the best move
  // - star_caches: Star-based caches for efficient insertion point finding
  void RelocateInner(const Problem &problem, const SpecificSolution &solution, const RouteContext &context,
                     Node route_x, Node route_y, BaseCache<RelocateMove> &cache,
                     StarCaches &star_caches) {
    star_caches.Preprocess(problem, solution, context, route_y);
    Node node_x = context.Head(route_x);
    while (node_x) {
      if (context.Load(route_y) + solution.Load(node_x) <= problem.capacity) {
        auto insertion = star_caches.Get(route_y, solution.Customer(node_x)).FindBest();
        Node predecessor_x = solution.Predecessor(node_x);
        Node successor_x = solution.Successor(node_x);
        int delta = insertion->delta.value
                    - CalcDelta(problem, solution, node_x, predecessor_x, successor_x);
        if (cache.delta.Update(delta)) {
          cache.move = {route_x, route_y, node_x, insertion->predecessor, insertion->successor};
        }
      }
      node_x = solution.Successor(node_x);
    }
  }

  // Operator implementing the relocate move for inter-route optimization
  // Parameters:
  // - problem: Problem instance with routing constraints
  // - solution: Current solution to be modified
  // - context: Route context tracking route-specific information
  // - cache_map: Cache management for move calculations
  // Returns: Vector of modified route indices
  std::vector<Node> Relocate::operator()(const Problem &problem, SpecificSolution &solution,
                                                         RouteContext &context,
                                                         CacheMap &cache_map) const {
    auto &caches = cache_map.Get<InterRouteCache<RelocateMove>>(solution, context);
    auto &star_caches = cache_map.Get<StarCaches>(solution, context);
    RelocateMove best_move{};
    Delta<int> best_delta{};
    for (Node route_x = 0; route_x < context.NumRoutes(); ++route_x) {
      for (Node route_y = 0; route_y < context.NumRoutes(); ++route_y) {
        if (route_x == route_y) {
          continue;
        }
        auto &cache = caches.Get(route_x, route_y);
        if (!cache.TryReuse()) {
          RelocateInner(problem, solution, context, route_x, route_y, cache, star_caches);
        } else {
          cache.move.route_x = route_x;
          cache.move.route_y = route_y;
        }
        if (best_delta.Update(cache.delta)) {
          best_move = cache.move;
        }
      }
    }
    // If an improvement is found, perform the relocate move
    if (best_delta.value < 0) {
      DoRelocate(best_move, solution, context);
      return {best_move.route_x, best_move.route_y};
    }
    return {};
  }