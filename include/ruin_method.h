#ifndef RUIN_METHOD_H
#define RUIN_METHOD_H

#include <vector>

#include "problem.h"
#include "solution.h"
#include "route_context.h"

// Base class for Ruin methods
class RuinMethod
{
public:

    // Ruin strategy to perturb a random number of customers.
    virtual vector<Node> Ruin(const Problem &problem, SpecificSolution &solution,
                                   RouteContext &context) = 0;
};

// Random ruin class
class RandomRuin : public RuinMethod
{
public:

    explicit RandomRuin(vector<int> num_perturb_customers);

    vector<Node> Ruin(const Problem &problem, SpecificSolution &solution, 
                        RouteContext &context) override;

private:

    vector<int> num_perturb_customers_;
};

// SISRs ruin class
class SisrsRuin : public RuinMethod
{
public:

    SisrsRuin(int average_customers, int max_length, double split_rate,
              double preserved_probability);

    vector<Node> Ruin(const Problem &problem, SpecificSolution &solution, 
                        RouteContext &context) override;

private:

    static Node GetRouteHead(SpecificSolution &solution, Node node_index, int &position); // Get head of a route
    static void GetRoute(const SpecificSolution &solution, Node head, vector<Node> &route); // Get the complete route, starting from the head
    int average_customers_; // Average number of removed customers
    int max_length_; // Maximum cardinality of removed strings
    double split_rate_; // Probability of executing the split string
    double preserved_probability_; // Probability of preserving a node
};

#endif