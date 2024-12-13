#include "../include/distance_matrix_optimizer.h"
#include <cmath>

// Constructor that optimizes the distance matrix using the Floyd-Warshall algorithm.
// Also keeps track of intermediate nodes for path restoration.
DistanceMatrixOptimizer::DistanceMatrixOptimizer(std::vector<std::vector<int>> &distance_matrix) 
    : original_(distance_matrix), previous_node_indices_(distance_matrix.size(), std::vector<Node>(distance_matrix.size()))
{
    Node num_customers = static_cast<Node>(distance_matrix.size());
    for (Node k = 1; k < num_customers; ++k)
    {
        for (Node i = 0; i < num_customers; ++i)
        {
            for (Node j = 0; j < num_customers; ++j)
            {
                int distance = distance_matrix[i][k] + distance_matrix[k][j];
                // Update to shorter path if a better route is found
                if (distance_matrix[i][j] > distance)
                {
                    distance_matrix[i][j] = distance;
                    previous_node_indices_[i][j] = k; // Record the intermediate node
                }
            }
        }
    }
}

// Recursively restores the shortest path between nodes i and j
void DistanceMatrixOptimizer::Restore(SpecificSolution &solution, Node i, Node j) const
{
    Node customer = previous_node_indices_[solution.Customer(i)][solution.Customer(j)];
    if (customer != 0) // If an intermediate node exists
    {
        Node k = solution.Insert(customer, 0, i, j); // Insert the intermediate node
        Restore(solution, i, k); // Restore path from i to the intermediate node
        Restore(solution, k, j); // Restore path from the intermediate node to j
    }
}

// Restores all paths for the given solution
void DistanceMatrixOptimizer::Restore(SpecificSolution &solution) const
{
    std::vector<Node> heads;
    for (Node node_index : solution.NodeIndices())
    {
        if (!solution.Predecessor(node_index)) // Find starting nodes
        {
            heads.push_back(node_index);
        }
    }
    for (Node node_index : heads)
    {
        Node predecessor = 0;
        while (node_index) // Traverse the chain of nodes
        {
            Restore(solution, predecessor, node_index); // Restore path between predecessor and current node
            predecessor = node_index; // Move to the next node
            node_index = solution.Successor(node_index); // Get the next node in the chain
        }
        Restore(solution, predecessor, 0); // Close the loop back to the starting node
    }
}
