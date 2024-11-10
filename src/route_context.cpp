#include "../include/route_context.h"

Node RouteContext::Head(Node route_index)
{
    return routes_[route_index].head;
}

Node RouteContext::Tail(Node route_index)
{
    return routes_[route_index].tail;    
}

int RouteContext::Load(Node route_index)
{
    return routes_[route_index].load;
}

int RouteContext::PreLoad(Node route_index)
{
    return pre_loads_[route_index];
}

void RouteContext::SetHead(Node route_index, Node head)
{
    routes_[route_index].head = head;
}

void RouteContext::AddLoad(Node route_index, int load)
{
    routes_[route_index].load += load;
}

Node RouteContext::NumRoutes()
{
    return routes_.size();
}

void RouteContext::SetNumRoutes(Node num_routes)
{
    routes_.resize(num_routes);
}

void RouteContext::AddRoute(Node head, Node tail, int load)
{
    routes_.emplace_back(RouteData{head, tail, load});
}

void RouteContext::CalcRouteContext(const Solution& solution)
{
    routes_.clear();

    for (Node node_index : solution.NodeIndices())
    {
        if (solution.Predecessor(node_index) == 0)
            AddRoute(node_index, node_index, 0);
    }

    pre_loads_.resize(solution.MaxNodeIndex() + 1);

    for (Node route_index = 0; route_index < NumRoutes(); ++route_index)
        UpdateRouteContext(solution, route_index, 0);
}

void RouteContext::UpdateRouteContext(const Solution& solution, Node route_index, Node predecessor)
{
    pre_loads_.resize(solution.MaxNodeIndex() + 1);
    int load = pre_loads_[predecessor];

    Node node_index = predecessor ? solution.Successor(predecessor) : Head(route_index);
    
    while (node_index) 
    {
        load += solution.Load(node_index);
        pre_loads_[node_index] = load;
      
        predecessor = node_index;
        node_index = solution.Successor(node_index);
    }
    
    routes_[route_index].tail = predecessor;
    routes_[route_index].load = load;
}

void RouteContext::MoveRouteContext(Node dest_route_index, Node src_route_index)
{
    routes_[dest_route_index] = routes_[src_route_index];
}