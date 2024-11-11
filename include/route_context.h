#include "problem.h"
#include "solution.h"

#include <vector>

class RouteContext
{
public:

    Node Head(Node route_index) const;
    Node Tail(Node route_index) const;
    int Load(Node route_index) const;
    int PreLoad(Node node_index) const;
    void SetHead(Node route_index, Node head);
    void AddLoad(Node route_index, int load);
    Node NumRoutes() const;
    void SetNumRoutes(Node num_routes);
    void AddRoute(Node head, Node tail, int load);
    void CalcRouteContext(const SpecificSolution &solution);
    void UpdateRouteContext(const SpecificSolution &solution, Node route_index, Node predecessor);
    void MoveRouteContext(Node dest_route_index, Node src_route_index);

private:

    struct RouteData 
    {
        Node head;
        Node tail;
        int load;
    };

    std::vector<RouteData> routes_;
    std::vector<int> pre_loads_;
};