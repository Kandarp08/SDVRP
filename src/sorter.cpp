#include "../include/sorter.h"

#include <algorithm>
#include <random>

// Add a sort function with its weight to the sorter.
void Sorter::AddSortFunction(std::unique_ptr<SortOperator> &&sort_function, double weight)
{
    sum_weights_ += weight; // Update the total weight.
    sort_functions_.emplace_back(std::move(sort_function), weight);
}

// Sort customers using a randomly selected sort function based on weights.
void Sorter::Sort(const Problem &problem, std::vector<Node> &customers) const
{
    // Generate a random number within the sum of weights.
    double r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * sum_weights_;
    
    // Iterate through the sort functions to find which one to apply.
    for (auto &&[sort_function, weight] : sort_functions_)
    {
        r -= weight;
        if (r < 0)
        {
            // Apply the selected sort function.
            (*sort_function)(problem, customers);
            return;
        }
    }
}

// Randomly shuffle the list of customers.
void SortByRandom::operator()([[maybe_unused]] const Problem &problem,
                              std::vector<Node> &customers) const
{
    std::random_device rd; // Seed for randomness.
    std::mt19937 gen(rd()); // Random number generator.
    shuffle(customers.begin(), customers.end(), gen); // Shuffle the customers.
}

// Sort customers by demand in descending order.
void SortByDemand::operator()(const Problem &problem, std::vector<Node> &customers) const
{
    std::stable_sort(customers.begin(), customers.end(), [&](Node lhs, Node rhs)
                     { return problem.demands[lhs] > problem.demands[rhs]; });
}

// Sort customers by distance from the depot (farthest first).
void SortByFar::operator()(const Problem &problem, std::vector<Node> &customers) const
{
    std::stable_sort(customers.begin(), customers.end(), [&](Node lhs, Node rhs)
                     { return problem.distance_matrix[0][lhs] > problem.distance_matrix[0][rhs]; });
}

// Sort customers by distance from the depot (closest first).
void SortByClose::operator()(const Problem &problem, std::vector<Node> &customers) const
{
    std::stable_sort(customers.begin(), customers.end(), [&](Node lhs, Node rhs)
                     { return problem.distance_matrix[0][lhs] < problem.distance_matrix[0][rhs]; });
}
