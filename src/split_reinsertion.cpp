#include "../include/split_reinsertion.h"
#include "../include/utils.h"
#include <algorithm>
#include <vector>

// Represents a split reinsertion move, storing the best insertion and residual capacity.
struct SplitReinsertionMove {
    InsertionWithCost<int> insertion;
    int residual;
    SplitReinsertionMove(const InsertionWithCost<int> &insertion, int residual)
        : insertion(insertion), residual(residual) {}
};

// Perform split reinsertion to reallocate customer demand across routes.
void SplitReinsertion(const Problem& problem, Node customer, int demand, double blink_rate,
                      SpecificSolution& solution, RouteContext& context) {
    // Lambda function to calculate the cost of inserting a customer between two nodes.
    auto func = [&](Node predecessor, Node successor, Node customer) {
        Node preCustomer = solution.Customer(predecessor);
        Node sucCustomer = solution.Customer(successor);
        return problem.distance_matrix[customer][preCustomer]
               + problem.distance_matrix[customer][sucCustomer]
               - problem.distance_matrix[preCustomer][sucCustomer];
    };

    std::vector<SplitReinsertionMove> moves; // Store possible moves.
    moves.reserve(context.NumRoutes());
    int sumResidual = 0;

    // Evaluate all routes for potential insertion.
    for (Node routeIndex = 0; routeIndex < context.NumRoutes(); ++routeIndex) {
        int residual = std::min(demand, problem.capacity - context.Load(routeIndex));
        if (residual > 0) {
            auto insertion = CalcBestInsertion(solution, func, context, routeIndex, customer);
            moves.emplace_back(insertion, residual);
            sumResidual += residual;
        }
    }

    // If the total available capacity is less than demand, exit.
    if (sumResidual < demand) {
        return;
    }

    // Sort moves by cost efficiency (scaled by residual capacity).
    std::stable_sort(moves.begin(), moves.end(),
                     [](const SplitReinsertionMove& lhs, const SplitReinsertionMove& rhs) {
                         return lhs.insertion.cost.value * rhs.residual
                                < rhs.insertion.cost.value * lhs.residual;
                     });

    // Perform the split reinsertion based on the sorted moves.
    for (const auto& move : moves) {
        sumResidual -= move.residual;

        // Randomly skip some moves based on blink_rate.
        if (sumResidual >= demand && rand() < blink_rate) {
            continue;
        }

        // Allocate as much load as possible to the current move.
        int load = std::min(demand, move.residual);
        Node nodeIndex = solution.Insert(customer, load, move.insertion.predecessor, move.insertion.successor);

        // Update context if the insertion is at the route head.
        if (move.insertion.predecessor == 0) {
            context.SetHead(move.insertion.route_index, nodeIndex);
        }

        // Update route context after the insertion.
        context.UpdateRouteContext(solution, move.insertion.route_index, move.insertion.predecessor);

        // Decrease the remaining demand.
        demand -= load;

        // Stop if all demand has been reallocated.
        if (demand == 0) {
            break;
        }
    }
}
