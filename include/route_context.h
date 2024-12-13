#ifndef ROUTE_CONTEXT_H
#define ROUTE_CONTEXT_H

#include "problem.h"
#include "solution.h"

#include <vector>

// Contains information regarding the routes that are currently decided.
class RouteContext
{
public:

    Node Head(Node route_index) const; // Get head of a route
    Node Tail(Node route_index) const; // Get tail of a route
    int Load(Node route_index) const; // Get load of a route
    int PreLoad(Node node_index) const; // Get prefix load upto a node
    void SetHead(Node route_index, Node head); // Set a node as the head of a route
    void AddLoad(Node route_index, int load);  // Add load to a route
    Node NumRoutes() const; // Return number of routes
    void SetNumRoutes(Node num_routes); // Set the number of routes
    void AddRoute(Node head, Node tail, int load); // Add a new route
    void CalcRouteContext(const SpecificSolution &solution); // Calculate the context of a route
    void UpdateRouteContext(const SpecificSolution &solution, Node route_index, Node predecessor); // Update the context of a route
    void MoveRouteContext(Node dest_route_index, Node src_route_index); // Move information of one route from one index to other

private:

    struct RouteData 
    {
        Node head; // Head of the route
        Node tail; // Tail of the route
        int load;  // Total load delivered along a route
    };

    std::vector<RouteData> routes_; // Routes decided by the algorithm
    std::vector<int> pre_loads_; // Cumulative load for each node
};

#endif