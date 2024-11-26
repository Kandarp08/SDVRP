#include "instance.h"

#include <memory>
#include <vector>

class Random;

class SortOperator
{
public:
    virtual ~SortOperator() = default;

    virtual void operator()(const Instance &instance, std::vector<Node> &customers) const = 0;
};

class Sorter
{
public:
    void AddSortFunction(std::unique_ptr<SortOperator> &&sort_function, double weight);

    void Sort(const Instance &instance, std::vector<Node> &customers) const;

private:
    double sum_weights_ = 0;
    std::vector<std::pair<std::unique_ptr<SortOperator>, double>> sort_functions_;
};

class SortByRandom : public SortOperator
{
public:
    void operator()(const Instance &instance, std::vector<Node> &customers) const override;
};

class SortByDemand : public SortOperator
{
public:
    void operator()(const Instance &instance, std::vector<Node> &customers) const override;
};

class SortByFar : public SortOperator
{
public:
    void operator()(const Instance &instance, std::vector<Node> &customers) const override;
};

class SortByClose : public SortOperator
{
public:
    void operator()(const Instance &instance, std::vector<Node> &customers) const override;
};