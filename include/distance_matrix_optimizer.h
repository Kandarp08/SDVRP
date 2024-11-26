#ifndef DISTANCE_MATRIX_OPTIMIZER_H
#define DISTANCE_MATRIX_OPTIMIZER_H

#include "problem.h"
#include "solution.h"

#include <vector>

class DistanceMatrixOptimizer{
public:
    explicit DistanceMatrixOptimizer(std::vector<std::vector<int>> &distance_matrix);
    void Restore(SpecificSolution &solution) const;

private:
    void Restore(SpecificSolution &solution, Node i, Node j) const;
    std::vector<std::vector<int>> original_;
    std::vector<std::vector<Node>> previous_node_indices_;
};

#endif