#include "../include/sorter.h"

#include <algorithm>
#include <random>

void Sorter::AddSortFunction(std::unique_ptr<SortOperator> &&sort_function, double weight)
{
    sum_weights_ += weight;
    sort_functions_.emplace_back(std::move(sort_function), weight);
}

void Sorter::Sort(const Instance &instance, std::vector<Node> &customers) const
{
    double r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * sum_weights_;
    for (auto &&[sort_function, weight] : sort_functions_)
    {
        r -= weight;
        if (r < 0)
        {
            (*sort_function)(instance, customers);
            return;
        }
    }
}

void SortByRandom::operator()([[maybe_unused]] const Instance &instance,
                              std::vector<Node> &customers) const
{
    std::random_device rd; // Seed
    std::mt19937 gen(rd());
    shuffle(customers.begin(), customers.end(), gen);
}

void SortByDemand::operator()(const Instance &instance, std::vector<Node> &customers) const
{
    std::stable_sort(customers.begin(), customers.end(), [&](Node lhs, Node rhs)
                     { return instance.demands[lhs] > instance.demands[rhs]; });
}

void SortByFar::operator()(const Instance &instance, std::vector<Node> &customers) const
{
    std::stable_sort(customers.begin(), customers.end(), [&](Node lhs, Node rhs)
                     { return instance.distance_matrix[0][lhs] > instance.distance_matrix[0][rhs]; });
}

void SortByClose::operator()(const Instance &instance, std::vector<Node> &customers) const
{
    std::stable_sort(customers.begin(), customers.end(), [&](Node lhs, Node rhs)
                     { return instance.distance_matrix[0][lhs] < instance.distance_matrix[0][rhs]; });
}