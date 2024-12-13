#ifndef UTILS_H
#define UTILS_H


#include "problem.h"
#include "solution.h"

#include "delta.h"
#include "route_context.h"

template <typename T> struct InsertionWithCost
{
    Node predecessor;
    Node successor;
    Node route_index;
    Delta<T> cost;

    bool Update(const InsertionWithCost<T> &insertion) 
    {
        if (cost.Update(insertion.cost)) 
        {
            predecessor = insertion.predecessor;
            successor = insertion.successor;
            route_index = insertion.route_index;
            
            return true;
        }
      
        return false;
    }
};

// Calculate the best insertion
template <class T> auto CalcBestInsertion(const SpecificSolution &solution, const T &func,
                        const RouteContext &context, Node route_index, Node customer) 
{
    Node head = context.Head(route_index);
    auto head_cost = func(0, head, customer);

    InsertionWithCost<decltype(head_cost)> best_insertion{0, head, route_index,
                                                          Delta(head_cost, 1)};
    Node node_index = head;

    while (node_index) 
    {
        auto cost = func(node_index, solution.Successor(node_index), customer);
        
        if (best_insertion.cost.Update(cost)) 
        {
            best_insertion.predecessor = node_index;
            best_insertion.successor = solution.Successor(node_index);
        }

        node_index = solution.Successor(node_index);
    }

    return best_insertion;
}



#endif