#include "../include/repair.h"

#include <map>

// Merges consecutive nodes with the same customer in a route by combining their loads.
void MergeAdjacentSameCustomers([[maybe_unused]] const Problem &problem, Node route_index,
                                SpecificSolution &solution, RouteContext &context)
{
    Node node_index = context.Head(route_index);
    while (true)
    {
        Node successor = solution.Successor(node_index);
        if (!successor)
        {
            break; // Exit when there are no more nodes
        }
        if (solution.Customer(node_index) == solution.Customer(successor))
        {
            // Combine the load of adjacent nodes with the same customer
            solution.SetLoad(node_index, solution.Load(node_index) + solution.Load(successor));
            solution.Remove(successor); // Remove the redundant node
        }
        else
        {
            node_index = successor; // Move to the next node
        }
    }
}

// Calculates the change in cost (delta) when a node is removed from a route.
int CalcRemovalDelta(const Problem &problem, const SpecificSolution &solution, Node node_index)
{
    Node predecessor = solution.Predecessor(node_index);
    Node successor = solution.Successor(node_index);
    // Delta = new distance after removal - current distance
    return problem.distance_matrix[solution.Customer(predecessor)][solution.Customer(successor)] -
           problem.distance_matrix[solution.Customer(predecessor)][solution.Customer(node_index)] -
           problem.distance_matrix[solution.Customer(node_index)][solution.Customer(successor)];
}

// Repairs a route by merging duplicate customers and optimizing node placements.
void Repair(const Problem &problem, Node route_index, SpecificSolution &solution, RouteContext &context)
{
    if (!context.Head(route_index))
    {
        return; // Skip if the route is empty
    }

    // Step 1: Merge adjacent nodes with the same customer
    MergeAdjacentSameCustomers(problem, route_index, solution, context);

    std::map<Node, Node> customer_node_map; // Maps customers to their last seen node
    Node node_index = context.Head(route_index);
    solution.SetSuccessor(0, node_index); // Link the dummy start node to the head

    while (node_index)
    {
        Node successor = solution.Successor(node_index);
        Node customer = solution.Customer(node_index);
        auto it = customer_node_map.find(customer);

        if (it == customer_node_map.end())
        {
            // First occurrence of this customer
            customer_node_map[customer] = node_index;
        }
        else
        {
            // Merge with the better option based on removal delta
            Node &last_node_index = it->second;
            if (CalcRemovalDelta(problem, solution, last_node_index) < CalcRemovalDelta(problem, solution, node_index))
            {
                std::swap(last_node_index, node_index);
            }
            solution.SetLoad(last_node_index, solution.Load(last_node_index) + solution.Load(node_index));
            solution.Remove(node_index);
        }
        node_index = successor; // Move to the next node
    }

    // Update the route context with the new head and route details
    context.SetHead(route_index, solution.Successor(0));
    context.UpdateRouteContext(solution, route_index, 0);
}
