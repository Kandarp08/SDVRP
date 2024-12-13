#include "../../include/inter_operator.h"

#include "../../include/base_cache.h"

  // Struct to represent a swap move operation between routes
  template <int, int> struct SwapMove {
    Node route_x, route_y;
    int direction_x, direction_y;
    Node left_x, left_y;
    Node right_x, right_y;
  };
  // Function to insert a segment into a route, with optional reversal
  void SegmentInsertion(SpecificSolution &solution, RouteContext &context, Node left, Node right,
                        Node predecessor, Node successor, Node route_index, int direction) {
    if (direction) {
      solution.ReversedLink(left, right, predecessor, successor);
    } else {
      solution.Link(predecessor, left);
      solution.Link(right, successor);
    }
    if (predecessor == 0) {
      context.SetHead(route_index, direction ? right : left);
    }
  }

  // Template function to perform a swap move between routes
  template <int num_x, int num_y>
  void DoSwap(SwapMove<num_x, num_y> &move, SpecificSolution &solution, RouteContext &context) {
    if (move.direction_y == -1) {
      Node predecessor = solution.Predecessor(move.left_x);
      Node successor = solution.Successor(move.right_x);
      solution.Link(predecessor, successor);
      if (predecessor == 0) {
        context.SetHead(move.route_x, successor);
      }
      SegmentInsertion(solution, context, move.left_x, move.right_x, move.left_y, move.right_y,
                       move.route_y, move.direction_x);
    } else {
      Node predecessor_x = solution.Predecessor(move.left_x);
      Node successor_x = solution.Successor(move.right_x);
      Node predecessor_y = solution.Predecessor(move.left_y);
      Node successor_y = solution.Successor(move.right_y);
      SegmentInsertion(solution, context, move.left_x, move.right_x, predecessor_y, successor_y,
                       move.route_y, move.direction_x);
      SegmentInsertion(solution, context, move.left_y, move.right_y, predecessor_x, successor_x,
                       move.route_x, move.direction_y);
    }
  }

  // Function to update and evaluate a potential shift move between routes
  template <int num_x, int num_y>
  void UpdateShift(const Problem &problem, const SpecificSolution &solution, Node route_x, Node route_y,
                   Node left, Node right, Node predecessor, Node successor, Node base_x,
                   BaseCache<SwapMove<num_x, num_y>> &cache) {
    Node customer_left = solution.Customer(left);
    Node customer_predecessor = solution.Customer(predecessor);
    Node customer_right = solution.Customer(right);
    Node customer_successor = solution.Customer(successor);
    int d1 = problem.distance_matrix[customer_left][customer_predecessor]
             + problem.distance_matrix[customer_right][customer_successor];
    int d2 = problem.distance_matrix[customer_left][customer_successor]
             + problem.distance_matrix[customer_right][customer_predecessor];
    int direction = d1 >= d2;
    int delta = base_x + (direction ? d2 : d1)
                - problem.distance_matrix[customer_predecessor][customer_successor];
    if (cache.delta.Update(delta)) {
      cache.move = {route_x, route_y, direction, -1, left, predecessor, right, successor};
    }
  }

  // Function to update and evaluate a potential swap move between routes
  template <int num_x, int num_y>
  void UpdateSwap(const Problem &problem, const SpecificSolution &solution, Node route_x, Node route_y,
                  Node left_x, Node right_x, Node left_y, Node right_y, int base_x,
                  BaseCache<SwapMove<num_x, num_y>> &cache) {
    Node customer_left_x = solution.Customer(left_x);
    Node customer_right_x = solution.Customer(right_x);
    Node customer_left_y = solution.Customer(left_y);
    Node customer_right_y = solution.Customer(right_y);
    Node predecessor_x = solution.Customer(solution.Predecessor(left_x));
    Node successor_x = solution.Customer(solution.Successor(right_x));
    Node predecessor_y = solution.Customer(solution.Predecessor(left_y));
    Node successor_y = solution.Customer(solution.Successor(right_y));
    int d1 = problem.distance_matrix[customer_left_x][predecessor_y]
             + problem.distance_matrix[customer_right_x][successor_y];
    int d2 = problem.distance_matrix[customer_left_x][successor_y]
             + problem.distance_matrix[customer_right_x][predecessor_y];
    int d3 = problem.distance_matrix[customer_left_y][predecessor_x]
             + problem.distance_matrix[customer_right_y][successor_x];
    int d4 = problem.distance_matrix[customer_left_y][successor_x]
             + problem.distance_matrix[customer_right_y][predecessor_x];
    int direction_x = d1 >= d2;
    int direction_y = d3 >= d4;
    int delta = base_x + (direction_x ? d2 : d1) + (direction_y ? d4 : d3)
                - problem.distance_matrix[customer_left_y][predecessor_y]
                - problem.distance_matrix[customer_right_y][successor_y];
    if (cache.delta.Update(delta)) {
      cache.move = {route_x, route_y, direction_x, direction_y, left_x, left_y, right_x, right_y};
    }
  }

  // Core function to explore swap moves within and between routes
  template <int num_x, int num_y>
  void SwapInner(const Problem &problem, SpecificSolution &solution, RouteContext &context, Node route_x,
                 Node route_y, BaseCache<SwapMove<num_x, num_y>> &cache) {
    Node left_x = context.Head(route_x);
    int load_x = solution.Load(left_x);
    Node right_x = left_x;
    for (int i = 1; right_x && i < num_x; ++i) {
      right_x = solution.Successor(right_x);
      load_x += solution.Load(right_x);
    }
    while (right_x) {
      int base_x = -problem.distance_matrix[solution.Customer(left_x)]
                                           [solution.Customer(solution.Predecessor(left_x))]
                   - problem.distance_matrix[solution.Customer(right_x)]
                                            [solution.Customer(solution.Successor(right_x))];
      if (num_y == 0) {
        base_x += problem.distance_matrix[solution.Customer(solution.Predecessor(left_x))]
                                         [solution.Customer(solution.Successor(right_x))];
      }
      int load_y_lower = -problem.capacity + context.Load(route_y) + load_x;
      if (num_y == 0) {
        if (load_y_lower <= 0) {
          Node predecessor = 0;
          Node successor = context.Head(route_y);
          while (true) {
            UpdateShift(problem, solution, route_x, route_y, left_x, right_x, predecessor,
                        successor, base_x, cache);
            if (!successor) {
              break;
            }
            predecessor = successor;
            successor = solution.Successor(successor);
          }
        }
      } else {
        int load_y_upper = problem.capacity - context.Load(route_x) + load_x;
        Node left_y = context.Head(route_y);
        int load_y = solution.Load(left_y);
        Node right_y = left_y;
        for (int i = 1; right_y && i < num_y; ++i) {
          right_y = solution.Successor(right_y);
          load_y += solution.Load(right_y);
        }
        while (right_y) {
          if (load_y >= load_y_lower && load_y <= load_y_upper) {
            UpdateSwap(problem, solution, route_x, route_y, left_x, right_x, left_y, right_y,
                       base_x, cache);
          }
          load_y -= solution.Load(left_y);
          left_y = solution.Successor(left_y);
          right_y = solution.Successor(right_y);
          load_y += solution.Load(right_y);
        }
      }
      load_x -= solution.Load(left_x);
      left_x = solution.Successor(left_x);
      right_x = solution.Successor(right_x);
      load_x += solution.Load(right_x);
    }
  }

  // Operator overload to perform swap moves between routes
  template <int num_x, int num_y> std::vector<Node> Swap<num_x, num_y>::operator()(
      const Problem &problem, SpecificSolution &solution, RouteContext &context,
      CacheMap &cache_map) const {
    auto &caches = cache_map.Get<InterRouteCache<SwapMove<num_x, num_y>>>(solution, context);
    SwapMove<num_x, num_y> best_move{};
    Delta<int> best_delta{};
    for (Node route_x = 0; route_x < context.NumRoutes(); ++route_x) {
      for (Node route_y = num_x != num_y ? 0 : route_x + 1; route_y < context.NumRoutes();
           ++route_y) {
        if (num_x != num_y && route_x == route_y) {
          continue;
        }
        auto &cache = caches.Get(route_x, route_y);
        if (!cache.TryReuse()) {
          SwapInner<num_x, num_y>(problem, solution, context, route_x, route_y, cache);
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
      DoSwap(best_move, solution, context);
      return {best_move.route_x, best_move.route_y};
    }
    return {};
  }

  // Explicit template instantiations for different segment swap configurations
  template class Swap<1, 0>;
  template class Swap<2, 0>;
  template class Swap<1, 1>;
  template class Swap<2, 1>;
  template class Swap<2, 2>;