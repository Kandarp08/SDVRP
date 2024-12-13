#include <algorithm>
#include <random>
#include <numeric>
#include <set>
#include <vector>
#include <iostream>

#include "../include/ruin_method.h"
#include "../include/route_context.h"

// Random ruin strategy to perturb a random number of customers.
RandomRuin::RandomRuin(vector<int> num_perturb_customers)
      : num_perturb_customers_(move(num_perturb_customers)) {}

vector<Node> RandomRuin::Ruin(const Problem &problem, SpecificSolution &solution,
                                RouteContext &context)                                      
{
    // Select a random number of customers to perturb
    int num_perturb = num_perturb_customers_.size() > 0 
                        ? num_perturb_customers_[rand() % num_perturb_customers_.size()]
                        : num_perturb_customers_[rand() % (num_perturb_customers_.size() + 1)];
    
    vector<Node> customers(problem.num_customers - 1);

    // Fill the customers vector with indices starting from 1
    iota(customers.begin(), customers.end(), 1);

    // Shuffle the customer indices
    std::random_device rd;
    std::mt19937 gen(rd());
    shuffle(customers.begin(), customers.end(), gen);

    // Keep only the number of customers to perturb
    customers.erase(customers.begin() + num_perturb, customers.end());

    return customers;
}

// SISRS ruin strategy to remove structured groups of customers from routes.
SisrsRuin::SisrsRuin(int average_customers, int max_length, double split_rate,
                       double preserved_probability)
      : average_customers_(average_customers),
        max_length_(max_length),
        split_rate_(split_rate),
        preserved_probability_(preserved_probability) {}

vector<Node> SisrsRuin::Ruin(const Problem &problem, SpecificSolution &solution,
                                RouteContext &context)
{
    // Calculate average and max route lengths
    double average_length = static_cast<double>(problem.num_customers - 1) / context.NumRoutes();
    double max_length = min(static_cast<double>(max_length_), average_length);

    // Calculate the number of strings to ruin
    double max_strings = 4.0 * average_customers_ / (1 + max_length_) - 1;
    size_t num_strings = static_cast<size_t>((static_cast<double>(rand()) / RAND_MAX) * max_strings) + 1;

    // Randomly select a seed customer
    int customer_seed = rand() % (problem.num_customers);

    vector<Node> node_indices(solution.NodeIndices());
    auto &&seed_distances = problem.distance_matrix[customer_seed];

    // Sort nodes based on distance to the seed customer
    stable_sort(node_indices.begin(), node_indices.end(), [&](Node lhs, Node rhs) {
        return seed_distances[solution.Customer(lhs)] < seed_distances[solution.Customer(rhs)];
    });

    set<Node> visited_heads; // Keep track of visited route heads
    vector<Node> route;
    vector<Node> customer_indices;

    // Process each node and ruin segments
    for (Node node_index : node_indices)
    {
        if (visited_heads.size() >= num_strings)
            break;

        int position;
        int head = GetRouteHead(solution, node_index, position);

        if (!visited_heads.insert(head).second)
            continue;

        GetRoute(solution, head, route);

        int route_length = static_cast<int>(route.size());
        double max_ruin_length = min(static_cast<double>(route_length), max_length);

        // Calculate ruin length and determine preserved segments
        int ruin_length = static_cast<int>((static_cast<double>(rand()) / RAND_MAX) * max_ruin_length) + 1;
        int num_preserved = 0;
        int preserved_start_position = -1;

        if (ruin_length >= 2 && ruin_length < route_length && 
            (static_cast<double>(rand()) / RAND_MAX) < split_rate_)
        {
            while (ruin_length < route_length)
            {
                if ((static_cast<double>(rand()) / RAND_MAX) < preserved_probability_)
                    break;

                ++num_preserved;
                ++ruin_length;
            }
            preserved_start_position = (rand() % max(1, ruin_length - num_preserved - 1)) + 1;
        }

        int min_start_position = max(0, position - ruin_length + 1);
        int max_start_position = min(route_length - ruin_length, position);
        int start_position = (rand() % (max_start_position - min_start_position + 1)) + min_start_position;

        // Collect ruined customer indices
        for (int j = 0; j < ruin_length; ++j)
        {
            if (j < preserved_start_position || j >= preserved_start_position + num_preserved)
                customer_indices.emplace_back(solution.Customer(route[start_position + j]));
        }
    }

    // Remove duplicate customers and shuffle the result
    sort(customer_indices.begin(), customer_indices.end());
    customer_indices.erase(unique(customer_indices.begin(), customer_indices.end()), customer_indices.end());

    std::random_device rd;
    std::mt19937 gen(rd());
    shuffle(customer_indices.begin(), customer_indices.end(), gen);

    return customer_indices;
}

// Get the head of the route containing the node and its position in the route.
Node SisrsRuin::GetRouteHead(SpecificSolution &solution, Node node_index, int &position)
{
    position = 0;
    while (true)
    {
        Node predecessor = solution.Predecessor(node_index);
        if (!predecessor)
            return node_index;
        node_index = predecessor;
        ++position;
    }
}

// Get the entire route starting from the head node.
void SisrsRuin::GetRoute(const SpecificSolution &solution, Node head, vector<Node> &route)
{
    route.clear();
    while (head)
    {
        route.emplace_back(head);
        head = solution.Successor(head);
    }
}
