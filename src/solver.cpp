#include "../include/solver.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <numeric>
#include <vector>
#include <random>

#include "../include/cache.h"
#include "../include/construction.h"
#include "../include/repair.h"
#include "../include/split_reinsertion.h"
#include "../include/utils.h"

// Searches for improvements within a single route.
void IntraRouteSearch(const Problem &problem, const SpecificConfig &config, Node route_index,
                        SpecificSolution &solution, RouteContext &context) 
{
    Repair(problem, route_index, solution, context); // Repair the route first.
    vector<Node> intra_neighborhoods(config.intra_operators.size());
    iota(intra_neighborhoods.begin(), intra_neighborhoods.end(), 0);
    
    while (true) 
    {
        // Randomize the order of neighborhoods to explore.
        std::random_device rd;
        std::mt19937 gen(rd());
        shuffle(intra_neighborhoods.begin(), intra_neighborhoods.end(), gen);
        
        bool improved = false;
        
        // Try each neighborhood operator.
        for (Node neighborhood : intra_neighborhoods) 
        {
            improved = (*config.intra_operators[neighborhood])(problem, route_index, solution, context);
            if (improved) break; // Stop if any improvement is found.
        }

        if (!improved) break; // Exit if no improvements are possible.
    }
}

// Randomized exploration of neighborhoods to find better solutions.
void RandomizedVariableNeighborhoodDescent(const Problem &problem, const SpecificConfig &config,
                                            SpecificSolution &solution, RouteContext &context,
                                            CacheMap &cache_map) 
{
    cache_map.Reset(solution, context); // Reset cache for the current solution.

    while (true) 
    {
        vector<int> inter_neighborhoods(config.inter_operators.size());
        iota(inter_neighborhoods.begin(), inter_neighborhoods.end(), 0);

        // Shuffle the neighborhoods to ensure randomness.
        std::random_device rd;
        std::mt19937 gen(rd());
        shuffle(inter_neighborhoods.begin(), inter_neighborhoods.end(), gen);
        
        bool improved = false;
        
        for (int neighborhood : inter_neighborhoods) 
        {
            Node original_num_routes = context.NumRoutes();
            auto routes = (*config.inter_operators[neighborhood])(problem, solution, context, cache_map);
            
            if (!routes.empty()) 
            {
                sort(routes.begin(), routes.end()); // Sort routes for consistency.
                improved = true;
                vector<Node> heads;
                
                for (Node route_index : routes) 
                {
                    Node head = context.Head(route_index);
                    if (head) heads.emplace_back(head);
                    if (route_index < original_num_routes) cache_map.RemoveRoute(route_index);
                }

                Node num_routes = 0;
                
                // Compact routes by removing unused ones.
                for (Node route_index = 0; route_index < context.NumRoutes(); ++route_index) 
                {
                    if (find(routes.begin(), routes.end(), route_index) == routes.end()) 
                    {
                        context.MoveRouteContext(num_routes, route_index);
                        cache_map.MoveRoute(num_routes, route_index);
                        ++num_routes;
                    }
                }

                // Add back updated routes and perform intra-route search.
                for (Node head : heads) 
                {
                    context.SetHead(num_routes, head);
                    context.UpdateRouteContext(solution, num_routes, 0);
                    cache_map.AddRoute(num_routes);
                    IntraRouteSearch(problem, config, num_routes, solution, context);
                    ++num_routes;
                }

                context.SetNumRoutes(num_routes); // Update route count.
                break;
            }
        }

        if (!improved) break; // Exit if no improvements are made.
    }

    cache_map.Save(solution, context); // Save the final state of the cache.
}

// Introduce changes to the solution to escape local optima.
void Perturb(const Problem &problem, const SpecificConfig &config, SpecificSolution &solution,
               RouteContext &context) 
{
    context.CalcRouteContext(solution); // Update the context.
    vector<Node> customers = config.ruin_method->Ruin(problem, solution, context); // Ruin part of the solution.
    config.sorter.Sort(problem, customers); // Sort customers for reinsertion.

    // Remove customers from routes.
    for (Node customer : customers) 
    {
        for (Node route_index = 0; route_index < context.NumRoutes(); ++route_index) 
        {
            Node node_index = context.Head(route_index);
            while (node_index) 
            {
                Node successor = solution.Successor(node_index);
                
                if (solution.Customer(node_index) == customer) 
                {
                    Node predecessor = solution.Predecessor(node_index);
                    solution.Remove(node_index);
                    if (predecessor == 0) context.SetHead(route_index, successor);
                    context.UpdateRouteContext(solution, route_index, predecessor);
                }
                node_index = successor;
            }
        }
    }

    // Reinsert customers using SplitReinsertion.
    for (Node customer : customers) 
    {
        SplitReinsertion(problem, customer, problem.demands[customer], config.blink_rate, solution,
                       context);
    }
}

// Measure the elapsed time since the given start time.
double ElapsedTime(std::chrono::time_point<std::chrono::high_resolution_clock> start_time) 
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(
               std::chrono::high_resolution_clock::now() - start_time)
        .count();
}

// Solve the given problem using the specified metaheuristic.
SpecificSolution SpecificSolver::Solve(const SpecificConfig &config, const Problem &problem) 
{
    if (config.listener != nullptr) config.listener->OnStart(); // Notify start.

    RouteContext context;
    CacheMap cache_map;
    SpecificSolution best_solution;
    int best_objective = std::numeric_limits<int>::max();
    auto start_time = std::chrono::high_resolution_clock::now();
    const int kMaxStagnation = std::min(5000, static_cast<int>(problem.num_customers)
                                                  * static_cast<int>(CalcFleetLowerBound(problem)));

    while (ElapsedTime(start_time) < config.time_limit) 
    {
        auto solution = Construct(problem); // Create an initial solution.
        int objective = solution.CalcObjective(problem);
        int iter_best_objective = objective;
        auto new_solution = solution;
        auto acceptance_rule = config.acceptance_rule();
        int num_stagnation = 0;

        while (num_stagnation < kMaxStagnation && ElapsedTime(start_time) < config.time_limit) 
        {
            ++num_stagnation;
            context.CalcRouteContext(new_solution);

            for (Node i = 0; i < context.NumRoutes(); ++i)
                IntraRouteSearch(problem, config, i, new_solution, context); // Improve routes.

            RandomizedVariableNeighborhoodDescent(problem, config, new_solution, context, cache_map);

            int new_objective = new_solution.CalcObjective(problem);

            // Update best solutions if improvements are found.
            if (new_objective < iter_best_objective) 
            {
                num_stagnation = 0;
                iter_best_objective = new_objective;
            }

            if (new_objective < best_objective) 
            {
                best_objective = new_objective;
                best_solution = new_solution;
                if (config.listener != nullptr)
                    config.listener->OnUpdated(best_solution, best_objective);
            }

            // Decide whether to accept the new solution.
            if (acceptance_rule->Accept(objective, new_objective)) 
            {
                objective = new_objective;
                solution = new_solution;
            } 
            else
                new_solution = solution;

            Perturb(problem, config, new_solution, context); // Perturb the solution.
        }
    }
    
    if (config.listener != nullptr) config.listener->OnEnd(best_solution, best_objective); // Notify end.

    return best_solution; // Return the best solution found.
}
