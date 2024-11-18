#include <vector>

#include "problem.h"
#include "solution.h"
#include "route_context.h"

class RuinMethod
{
public:

    virtual vector<Node> Ruin(const Problem &problem, SpecificSolution &solution,
                                   RouteContext &context) = 0;
};

class RandomRuin : public RuinMethod
{
public:

    explicit RandomRuin(vector<int> num_perturb_customers);

    vector<Node> Ruin(const Problem &problem, SpecificSolution &solution, 
                        RouteContext &context) override;

private:

    vector<int> num_perturb_customers_;
};

class SisrsRuin : public RuinMethod
{
public:

    SisrsRuin(int average_customers, int max_length, double split_rate,
              double preserved_probability);

    vector<Node> Ruin(const Problem &problem, SpecificSolution &solution, 
                        RouteContext &context) override;

private:

    static Node GetRouteHead(SpecificSolution &solution, Node node_index, int &position);
    static void GetRoute(const SpecificSolution &solution, Node head, vector<Node> &route);
    int average_customers_;
    int max_length_;
    double split_rate_;
    double preserved_probability_;
};