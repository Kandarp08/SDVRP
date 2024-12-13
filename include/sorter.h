#ifndef SORTER_H
#define SORTER_H

#include "problem.h"

#include <memory>
#include <vector>

// Base class for operators used during sorting
class SortOperator
{
public:
    virtual ~SortOperator() = default;

    virtual void operator()(const Problem &problem, std::vector<Node> &customers) const = 0;
};

class Sorter
{
public:

    // Add a sort function with its weight to the sorter.
    void AddSortFunction(std::unique_ptr<SortOperator> &&sort_function, double weight);

    // Sort customers using a randomly selected sort function based on weights.
    void Sort(const Problem &problem, std::vector<Node> &customers) const;

private:
    double sum_weights_ = 0;
    std::vector<std::pair<std::unique_ptr<SortOperator>, double>> sort_functions_;
};

// Randomly shuffle the list of customers.
class SortByRandom : public SortOperator
{
public:
    void operator()(const Problem &problem, std::vector<Node> &customers) const override;
};

// Sort customers by demand in descending order.
class SortByDemand : public SortOperator
{
public:
    void operator()(const Problem &problem, std::vector<Node> &customers) const override;
};

// Sort customers by distance from the depot (farthest first).
class SortByFar : public SortOperator
{
public:
    void operator()(const Problem &problem, std::vector<Node> &customers) const override;
};

// Sort customers by distance from the depot (closest first).
class SortByClose : public SortOperator
{
public:
    void operator()(const Problem &problem, std::vector<Node> &customers) const override;
};

#endif
