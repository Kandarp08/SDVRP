#ifndef ACCEPTANCE_RULE_H
#define ACCEPTANCE_RULE_H

#include <limits>
#include <vector>

class AcceptanceRule
{
public:
    virtual ~AcceptanceRule() = default;
    virtual bool Accept(int old_value, int new_value) = 0;
};

class HillClimbing : public AcceptanceRule
{
public:
    bool Accept(int old_value, int new_value) override;
};

class HillClimbingWithEqual : public AcceptanceRule
{
public:
    bool Accept(int old_value, int new_value) override;
};

class LateAcceptanceHillClimbing : public AcceptanceRule
{
public:
    explicit LateAcceptanceHillClimbing(int length) : length_(length), position_(0), values_(length, std::numeric_limits<int>::max()) {}
    bool Accept(int old_value, int new_value) override;

private:
    int length_;
    int position_;
    std::vector<int> values_;
};

class SimulatedAnnealing : public AcceptanceRule
{
public:
    SimulatedAnnealing(double initial_temperature, double decay) : temperature_(initial_temperature), decay_(decay) {}
    bool Accept(int old_value, int new_value) override;

private:
    double temperature_;
    double decay_;
};

#endif