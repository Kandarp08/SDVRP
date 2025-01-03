#ifndef BASE_STAR_H
#define BASE_STAR_H

#include "inter_operator.h"
#include <limits>
#include "base_cache.h"

// Represents an insertion with delta and its position
struct Insertion {
  Delta<int> delta;
  Node predecessor{}, successor{};
};

// Stores the best insertions, up to a fixed number
template <int num> struct BestInsertion {
  Insertion insertions[num];

  // Resets all insertions to default state
  void Reset() {
    for (auto &insertion : insertions) {
      insertion.delta = Delta(std::numeric_limits<int>::max(), -1);
    }
  }

  // Adds a new insertion if it's better than existing ones
  void Add(int delta, Node predecessor, Node successor) {
    for (int i = 0; i < num; ++i) {
      if (insertions[i].delta.value == std::numeric_limits<int>::max()) {
        insertions[i] = {{delta, 1}, predecessor, successor};
        return;
      } else if (delta < insertions[i].delta.value) {
        for (int j = num - 1; j > i; --j) {
          insertions[j] = insertions[j - 1];
        }
        insertions[i] = {{delta, 1}, predecessor, successor};
        return;
      } else if (delta == insertions[i].delta.value && insertions[i].delta.counter != -1) {
        if ((rand() % (insertions[i].delta.counter + 1) + 1) == 1) {
          for (int j = num - 1; j > i; --j) {
            insertions[j] = insertions[j - 1];
          }
          ++insertions[i].delta.counter;
          insertions[i].predecessor = predecessor;
          insertions[i].successor = successor;
          break;
        } else {
          ++insertions[i].delta.counter;
        }
      }
    }
  }

  // Finds the best insertion
  const Insertion *FindBest() const { return insertions; }

  // Finds the best insertion without a specific node
  const Insertion *FindBestWithoutNode(Node node_index) const {
    for (auto &insertion : insertions) {
      if (insertion.delta.counter > 0 && insertion.predecessor != node_index
          && insertion.successor != node_index) {
        return &insertion;
      }
    }
    return nullptr;
  }
};

// Manages star-related caches
class StarCaches : public Cache {
public:
  // Resets caches based on solution and context
  void Reset(const SpecificSolution &solution, const RouteContext &context) {
    caches_.resize(context.NumRoutes());
    for (Node route_index = 0;
          static_cast<size_t>(route_index) < std::min(routes_.size(), caches_.size());
          ++route_index) {
      bool same_route = false;
      if (route_index < context.NumRoutes()) {
        same_route = true;
        Node head = context.Head(route_index);
        for (Node node : routes_[route_index]) {
          if (head != node) {
            same_route = false;
            break;
          }
          head = solution.Successor(head);
        }
        if (head != 0) {
          same_route = false;
        }
      }
      if (!same_route) {
        caches_[route_index].clear();
      }
    }
  }

  // Adds a route
  void AddRoute(Node route_index) override { caches_.resize(route_index + 1); }

  // Removes a route
  void RemoveRoute(Node route_index) override { caches_[route_index].clear(); }

  // Moves a route from source to destination
  void MoveRoute(Node dest_route_index, Node src_route_index) override {
    caches_[dest_route_index].swap(caches_[src_route_index]);
  }

  // Prepares caches for a specific route
  void Preprocess(const Problem &problem, const SpecificSolution &solution, const RouteContext &context,
                  Node route) {
    auto &&insertions = caches_[route];
    if (!insertions.empty()) {
      return;
    }
    insertions.resize(problem.num_customers);
    for (Node customer = 1; customer < problem.num_customers; ++customer) {
      insertions[customer].Reset();
    }
    Node predecessor = 0;
    Node successor = context.Head(route);
    while (true) {
      Node predecessor_customer = solution.Customer(predecessor);
      Node successor_customer = solution.Customer(successor);
      auto &&predecessor_distances = problem.distance_matrix[predecessor_customer];
      auto &&successor_distances = problem.distance_matrix[successor_customer];
      auto distance = problem.distance_matrix[predecessor_customer][successor_customer];
      for (Node customer = 1; customer < problem.num_customers; ++customer) {
        int delta = predecessor_distances[customer] + successor_distances[customer] - distance;
        insertions[customer].Add(delta, predecessor, successor);
      }
      if (!successor) {
        break;
      }
      predecessor = successor;
      successor = solution.Successor(successor);
    }
  }

  // Saves routes from solution and context
  void Save(const SpecificSolution &solution, const RouteContext &context) {
    routes_.resize(context.NumRoutes());
    for (Node route_index = 0; static_cast<size_t>(route_index) < routes_.size(); ++route_index) {
      auto &route = routes_[route_index];
      route.clear();
      for (Node node = context.Head(route_index); node; node = solution.Successor(node)) {
        route.push_back(node);
      }
    }
  }

  // Gets the best insertion for a route and customer
  BestInsertion<3> &Get(Node route_index, Node customer) {
    return caches_[route_index][customer];
  }

private:
  std::vector<std::vector<BestInsertion<3>>> caches_; // Route caches
  std::vector<std::vector<Node>> routes_; // Routes data
};

// Calculates delta for inserting a node
inline int CalcDelta(const Problem &problem, const SpecificSolution &solution, Node node_index,
                      Node predecessor, Node successor) {
  return problem.distance_matrix[solution.Customer(node_index)][solution.Customer(predecessor)]
          + problem.distance_matrix[solution.Customer(node_index)][solution.Customer(successor)]
          - problem.distance_matrix[solution.Customer(predecessor)][solution.Customer(successor)];
}

#endif
