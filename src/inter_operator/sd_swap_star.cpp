#include "../../include/inter_operator.h"
#include <vector>
#include "../../include/base_cache.h"
#include "../../include/base_star.h"
#include "../../include/route_head_guard.h"

  // Structure representing a Split Delivery Swap Star Move
  // This structure captures the details of a potential route modification 
  // involving swapping nodes between two different routes
  struct SdSwapStarMove {
    bool swapped;
    Node route_x, route_y;
    Node node_x, predecessor_x, successor_x;
    Node node_y, predecessor_y, successor_y;
    int split_load;
  };

  // Executes the Split Delivery Swap Star Move by modifying the solution
  // This function physically performs the node swapping between routes

  void DoSdSwapStar(SdSwapStarMove &move, SpecificSolution &solution, RouteContext &context) {
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

  // Inner function to evaluate a potential Split Delivery Swap Star Move
  // This function calculates the cost delta and finds the best insertion point
  void SdSwapStarInner(const Problem &problem, const SpecificSolution &solution,
                       [[maybe_unused]] const RouteContext &context, bool swapped, Node route_x,
                       Node route_y, Node node_x, Node node_y, int split_load,
                       BaseCache<SdSwapStarMove> &cache, StarCaches &star_caches) {
    auto &&insertion_x = star_caches.Get(route_y, solution.Customer(node_x));
    auto &&insertion_y = star_caches.Get(route_x, solution.Customer(node_y));
    Node predecessor_y = solution.Predecessor(node_y);
    Node successor_y = solution.Successor(node_y);
    int delta = -CalcDelta(problem, solution, node_y, predecessor_y, successor_y);
    int delta_x = CalcDelta(problem, solution, node_x, predecessor_y, successor_y);
    auto best_insertion_y = insertion_y.FindBest();
    auto best_insertion_x = insertion_x.FindBestWithoutNode(node_y);
    if (best_insertion_x && best_insertion_x->delta.value < delta_x) {
      delta_x = best_insertion_x->delta.value;
      predecessor_y = best_insertion_x->predecessor;
      successor_y = best_insertion_x->successor;
    }
    delta += delta_x + best_insertion_y->delta.value;
    if (cache.delta.Update(delta)) {
      cache.move = {swapped,
                    route_x,
                    route_y,
                    node_x,
                    predecessor_y,
                    successor_y,
                    node_y,
                    best_insertion_y->predecessor,
                    best_insertion_y->successor,
                    split_load};
    }
  }

  // Overloaded inner function to iterate through nodes in two routes
  void SdSwapStarInner(const Problem &problem, const SpecificSolution &solution,
                       const RouteContext &context, Node route_x, Node route_y,
                       BaseCache<SdSwapStarMove> &cache, StarCaches &star_caches) {
    star_caches.Preprocess(problem, solution, context, route_x);
    star_caches.Preprocess(problem, solution, context, route_y);
    Node node_x = context.Head(route_x);
    while (node_x) {
      int load_x = solution.Load(node_x);
      Node node_y = context.Head(route_y);
      while (node_y) {
        int load_y = solution.Load(node_y);
        if (load_x > load_y) {
          SdSwapStarInner(problem, solution, context, false, route_x, route_y, node_x, node_y,
                          load_x - load_y, cache, star_caches);
        } else if (load_y > load_x) {
          SdSwapStarInner(problem, solution, context, true, route_y, route_x, node_y, node_x,
                          load_y - load_x, cache, star_caches);
        }
        node_y = solution.Successor(node_y);
      }
      node_x = solution.Successor(node_x);
    }
  }

  // Main operator function implementing the Split Delivery Swap Star Move
  // This function finds and applies the best route modification
  std::vector<Node> SdSwapStar::operator()(const Problem &problem,
                                                           SpecificSolution &solution,
                                                           RouteContext &context,
                                                           CacheMap &cache_map) const {
    auto &caches = cache_map.Get<InterRouteCache<SdSwapStarMove>>(solution, context);
    auto &star_caches = cache_map.Get<StarCaches>(solution, context);
    SdSwapStarMove best_move{};
    Delta<int> best_delta{};
    for (Node route_x = 0; route_x < context.NumRoutes(); ++route_x) {
      for (Node route_y = route_x + 1; route_y < context.NumRoutes(); ++route_y) {
        auto &cache = caches.Get(route_x, route_y);
        if (!cache.TryReuse()) {
          SdSwapStarInner(problem, solution, context, route_x, route_y, cache, star_caches);
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
      DoSdSwapStar(best_move, solution, context);
      return {best_move.route_x, best_move.route_y};
    }
    // Return empty vector if no improvement found
    return {};
  }