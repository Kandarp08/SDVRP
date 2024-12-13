#include "../include/route_context.h"

// Return the head node of the route
Node RouteContext::Head(Node route_index) const
{
    return routes_[route_index].head;
}

// Return the tail node of the route
Node RouteContext::Tail(Node route_index) const
{
    return routes_[route_index].tail;    
}

// Return the current load that is being carried on the route
int RouteContext::Load(Node route_index) const
{
    return routes_[route_index].load;
}

// Return the cumulative load upto a given node
int RouteContext::PreLoad(Node node_index) const
{
    return pre_loads_[node_index];
}

// Set the head of a given route
void RouteContext::SetHead(Node route_index, Node head)
{
    routes_[route_index].head = head;
}

// Add load to a given route
void RouteContext::AddLoad(Node route_index, int load)
{
    routes_[route_index].load += load;
}

// Return the number of routes
Node RouteContext::NumRoutes() const
{
    return routes_.size();
}

// Set the number of routes
void RouteContext::SetNumRoutes(Node num_routes) 
{
    routes_.resize(num_routes);
}

// Add a new route
void RouteContext::AddRoute(Node head, Node tail, int load)
{
    routes_.emplace_back(RouteData{head, tail, load});
}

// Calculate the context of the route, given the current solution
void RouteContext::CalcRouteContext(const SpecificSolution& solution)
{
    routes_.clear();

    // Add a new route if the node has no predecessor (node is the head)
    for (Node node_index : solution.NodeIndices())
    {
        if (solution.Predecessor(node_index) == 0)
            AddRoute(node_index, node_index, 0);
    }

    pre_loads_.resize(solution.MaxNodeIndex() + 1);

    // Updat route context for each of the routes that are added
    for (Node route_index = 0; route_index < NumRoutes(); ++route_index)
        UpdateRouteContext(solution, route_index, 0);
}

// Update route context of a given route, with respect to the current solution
void RouteContext::UpdateRouteContext(const SpecificSolution& solution, Node route_index, Node predecessor)
{
    pre_loads_.resize(solution.MaxNodeIndex() + 1);
    int load = pre_loads_[predecessor];

    Node node_index = predecessor ? solution.Successor(predecessor) : Head(route_index);
    
    // Iterate through all nodes of the route
    while (node_index) 
    {
        // Update load and preload
        load += solution.Load(node_index);
        pre_loads_[node_index] = load;
      
        predecessor = node_index;
        node_index = solution.Successor(node_index);
    }
    
    // Set the tail and total load
    routes_[route_index].tail = predecessor;
    routes_[route_index].load = load;
}

// Copy the route at src_route_index to dest_route_index
void RouteContext::MoveRouteContext(Node dest_route_index, Node src_route_index)
{
    routes_[dest_route_index] = routes_[src_route_index];
}