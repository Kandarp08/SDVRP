#ifndef BASE_CACHE_H
#define BASE_CACHE_H

#include <algorithm>
#include <vector>
#include "cache.h"
#include "delta.h"
#include "route_context.h"

// Generic base cache template
template <class T> struct BaseCache {
  bool invalidated = true; // Indicates whether the cache is valid
  Delta<int> delta;        // Tracks changes
  T move;                  // Represents the move data

  // Attempts to reuse the cache if valid
  bool TryReuse() {
    if (!invalidated) {
      return true; // Cache is valid, reuse it
    }
    invalidated = false; // Mark cache as valid for next use
    delta = Delta<int>(); // Reset the delta
    return false; // Cache wasn't reusable
  }
};

// Specialized inter-route cache for caching relationships between routes
template <class T> class InterRouteCache : public Cache {
public:
  // Resets the entire cache based on the solution and context
  void Reset([[maybe_unused]] const SpecificSolution &solution, const RouteContext &context) override {
    max_index_ = context.NumRoutes(); // Initialize max index to the number of routes
    matrix_.resize(context.NumRoutes()); // Resize the matrix
    route_index_mappings_.resize(context.NumRoutes()); // Map each route to itself
    route_pool_.clear(); // Clear active routes
    unused_indices_.clear(); // Clear unused indices
    for (Node i = 0; i < context.NumRoutes(); ++i) {
      matrix_[i].resize(context.NumRoutes()); // Create a matrix row for each route
      route_index_mappings_[i] = i; // Map route index to itself
      route_pool_.emplace_back(i); // Add route to active pool
      for (Node j = 0; j < context.NumRoutes(); ++j) {
        matrix_[i][j].invalidated = true; // Mark all entries as invalidated
      }
    }
  }

  // Adds a new route to the cache
  void AddRoute(Node route_index) override {
    Node index;
    if (unused_indices_.empty()) { // Check if there are unused slots
      index = max_index_++; // Create a new index
      route_index_mappings_.resize(max_index_); // Resize mapping
      matrix_.resize(max_index_); // Resize matrix
      for (Node i = 0; i < max_index_; ++i) {
        matrix_[i].resize(max_index_); // Adjust all rows to new size
      }
    } else {
      index = unused_indices_.back(); // Reuse an unused index
      unused_indices_.pop_back();
    }
    route_index_mappings_[route_index] = index; // Map the route
    route_pool_.emplace_back(index); // Add to active routes
    for (Node other : route_pool_) {
      matrix_[index][other].invalidated = true; // Invalidate new connections
      matrix_[other][index].invalidated = true; // Invalidate reverse connections
    }
  }

  // Removes a route from the cache
  void RemoveRoute(Node route_index) override {
    Node index = route_index_mappings_[route_index]; // Find mapped index
    route_pool_.erase(std::find(route_pool_.begin(), route_pool_.end(), index)); // Remove from active routes
    unused_indices_.emplace_back(index); // Add to unused slots
  }

  // Moves data from one route to another
  void MoveRoute(Node dest_route_index, Node src_route_index) override {
    route_index_mappings_[dest_route_index] = route_index_mappings_[src_route_index]; // Copy mapping
  }

  // Saves the cache state (currently a placeholder)
  void Save([[maybe_unused]] const SpecificSolution &solution,
            [[maybe_unused]] const RouteContext &context) override {}

  // Accesses the cache for two specific routes
  BaseCache<T> &Get(Node route_a, Node route_b) {
    return matrix_[route_index_mappings_[route_a]][route_index_mappings_[route_b]];
  }

private:
  std::vector<std::vector<BaseCache<T>>> matrix_; // Cache matrix
  std::vector<Node> route_index_mappings_; // Maps route indices to internal indices
  std::vector<Node> route_pool_; // Active route indices
  std::vector<Node> unused_indices_; // Unused slots
  Node max_index_{}; // Max index for new routes
};

#endif
