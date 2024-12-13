#ifndef DISTANCE_MATRIX_OPTIMIZER_H
#define DISTANCE_MATRIX_OPTIMIZER_H

#include "problem.h"
#include "solution.h"

#include <vector>

// Constructor that optimizes the distance matrix using the Floyd-Warshall algorithm.
// Also keeps track of intermediate nodes for path restoration.
class DistanceMatrixOptimizer{
public:
    explicit DistanceMatrixOptimizer(std::vector<std::vector<int>> &distance_matrix);
    // Restores all paths for the given solution
    void Restore(SpecificSolution &solution) const;

private:
    // Recursively restores the shortest path between nodes i and j
    void Restore(SpecificSolution &solution, Node i, Node j) const;
    
    std::vector<std::vector<int>> original_;
    std::vector<std::vector<Node>> previous_node_indices_;
};

#endif