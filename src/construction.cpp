#include "../include/construction.h"
#include <vector>
#include <random>

#include "../include/route_context.h"
#include "../include/utils.h"

using CandidateList = vector<pair<int, int>>;

// Calculate lower bound on the number of fleets required
Node CalcFleetLowerBound(const Problem &problem) 
{
    int sum_demands = 0;

    for (Node i = 1; i < problem.num_customers; ++i)
      sum_demands += problem.demands[i];

    return (sum_demands + problem.capacity - 1) / problem.capacity;
}

// Add a new route. This is a helper function to call the AddRoute method of class RouteContext
int AddRoute(CandidateList &candidateList, SpecificSolution &solution,
               RouteContext &context) 
{
    int range = candidateList.size();
    int position = rand() % range;

    auto [customer, demand] = candidateList[position];
    Node node_index = solution.Insert(customer, demand, 0, 0);
 
    candidateList[position] = candidateList.back();
    candidateList.pop_back();
    
    context.AddRoute(node_index, node_index, demand);
    
    return position;
}

// Implementation of Sequential insertion, as given in [5]
template <class Func> void SequentialInsertion(const Problem& problem, const Func &func, 
                         CandidateList& candidateList, SpecificSolution& solution, RouteContext& context)
{
    InsertionWithCost<float> best_insertion{};
    vector<bool> is_full(context.NumRoutes(), false); // To know whether a route is full or not
    
    // Iterate through all candidates
    while (!candidateList.empty()) 
    {
        bool inserted = false;

        // Iterate through all routes
        for (Node route_index = 0; route_index < context.NumRoutes(); ++route_index) 
        {
            // If the route is already full
            if (is_full[route_index])
                continue;

            int candidate_position = -1; // Best candidate that can be inserted
            best_insertion.cost = Delta(std::numeric_limits<float>::max(), -1);
       
            // Iterate through all the candidates
            for (int i = 0; i < candidateList.size(); ++i) 
            {
                auto [customer, demand] = candidateList[i];
          
                // If the load + demand exceeds vehicle capacity
                if (context.Load(route_index) + demand > problem.capacity)
                    continue;

                auto insertion = CalcBestInsertion(solution, func, context, route_index, customer);
                
                // If the current candidate gives a better insertion
                if (best_insertion.Update(insertion))
                    candidate_position = i;
            
            }

            // No candidate can be added into the route. Set the route to be full in this case
            if (candidate_position == -1)
                is_full[route_index] = true;

            // Candidate can be added in some route
            else 
            {
                auto [customer, demand] = candidateList[candidate_position];

                candidateList[candidate_position] = candidateList.back();
                candidateList.pop_back();

                Node node_index = solution.Insert(customer, demand, best_insertion.predecessor,
                                                    best_insertion.successor); // Insert node_index into the solution

                // Set the node as head of the route
                if (best_insertion.predecessor == 0)
                    context.SetHead(route_index, node_index);
        
                context.AddLoad(route_index, demand); // Add more load along the route
                inserted = true;
            }
        }
        
        if (!inserted) 
        {
            AddRoute(candidateList, solution, context);
            is_full.push_back(false);
        }
    }
}

// Implementation of Parallel insertion, as given in [5]
template <class Func> void ParallelInsertion(const Problem& problem, const Func &func,
                       CandidateList& candidateList, SpecificSolution& solution, RouteContext& context)
{
    vector<vector<InsertionWithCost<float>>> best_insertions(candidateList.size()); // Keep track of the best insertions
    
    // Initialization
    for (int i = 0; i < candidateList.size(); ++i) 
    {
        for (Node j = 0; j < context.NumRoutes(); ++j) 
        {
            best_insertions[i].push_back(CalcBestInsertion(solution, func, context, j, candidateList[i].first));
        }
    }

    vector<bool> updated(context.NumRoutes(), false);
    InsertionWithCost<float> best_insertion{};
    
    // Iterate through all candidates
    while (!candidateList.empty()) 
    {
        int candidate_position = -1;
        best_insertion.cost = Delta(std::numeric_limits<float>::max(), -1);
        
        for (int i = 0; i < best_insertions.size(); ++i) 
        {
            for (int j = 0; j < static_cast<int>(best_insertions[i].size()); ++j) 
            {
                Node route_index = best_insertions[i][j].route_index;
                auto [customer, demand] = candidateList[i];

                // If the (load of the route) + demand exceeds the capacity of vehicle
                if (context.Load(route_index) + demand > problem.capacity) 
                {
                    best_insertions[i][j] = best_insertions[i].back();
                    best_insertions[i].pop_back();
                    --j;
                } 
                
                else 
                {
                    if (updated[route_index]) 
                        best_insertions[i][j] = CalcBestInsertion(solution, func, context, route_index, customer);
            
                    // If the current candidate is a better insertion, then update candidate_position
                    if (best_insertion.Update(best_insertions[i][j])) 
                        candidate_position = i;
                }
            }
        }

        // No candidate added
        if (candidate_position == -1) 
        {
            int position = AddRoute(candidateList, solution, context); // Add a new route for the candidate
            best_insertions[position] = std::move(best_insertions.back());
            best_insertions.pop_back();

            for (int i = 0; i < candidateList.size(); ++i) 
            {
                auto [customer, demand] = candidateList[i];
                best_insertions[i].push_back(CalcBestInsertion(solution, func, context, context.NumRoutes() - 1, customer));
            }
            
            updated.push_back(false);
        } 
        
        else 
        {
            // Remove candidate at candidate_position from the list
            auto [customer, demand] = candidateList[candidate_position];
            candidateList[candidate_position] = candidateList.back();
            candidateList.pop_back();

            best_insertions[candidate_position] = std::move(best_insertions.back());
            best_insertions.pop_back();

            Node node_index = solution.Insert(customer, demand, best_insertion.predecessor,
                                            best_insertion.successor); // Insert the node_index into the solution
            Node route_index = best_insertion.route_index;
            
            // Set the node as head of the route
            if (best_insertion.predecessor == 0)
                context.SetHead(route_index, node_index);
        
            context.AddLoad(route_index, demand); // Add the demand to the route
            updated[route_index] = true;
        }
    }
}

// Insert candidates into different routes. The insertion strategy is randomly selected
template <class Func> void InsertCandidates(const Problem &problem, const Func &func,
                        CandidateList &candidate_list, SpecificSolution &solution, 
                        RouteContext &context) 
{
    int strategy = rand() % 2;

    if (strategy == 0)
        SequentialInsertion(problem, func, candidate_list, solution, context); // Perform sequential insertion
    
    else
        ParallelInsertion(problem, func, candidate_list, solution, context); // Perform parallel insertion
}

// Construct an inital solution, as per reference [5]
SpecificSolution Construct(const Problem& problem)
{
    CandidateList candidate_list; // To store the split demands of different customers
    Node num_fleets = CalcFleetLowerBound(problem); // Calculate a lower bound for the number of vehicles

    // Iterate through all customers
    for (Node i = 1; i < problem.num_customers; ++i) 
    {
        int demand = problem.demands[i]; // Demand of the ith customer
      
        // Split the demand between different vehicles
        while (demand > 0) 
        {
            int split_demand = min(demand, problem.capacity);
            candidate_list.emplace_back(i, split_demand);
            demand -= split_demand;
        }
    }

    SpecificSolution solution; // Solution to be returned
    RouteContext context; // Details about the routes decided

    // Add some initial routes
    for (Node i = 0; i < num_fleets && !candidate_list.empty(); ++i)
      AddRoute(candidate_list, solution, context);
    
    int criterion = rand() % 2; // Randomly decide the insertion criterion
    
    // MCFIC criteria
    if (criterion == 0) 
    {
        float gamma = static_cast<float>(rand() % 35) * 0.05f; // Randomly select gamma
      
        // Cost function of MCFIC
        auto func = [&](Node predecessor, Node successor, Node customer) {
            Node pre_customer = solution.Customer(predecessor);
            Node suc_customer = solution.Customer(successor);

            return static_cast<float>(problem.distance_matrix[pre_customer][customer]
                                    + problem.distance_matrix[customer][suc_customer]
                                    - problem.distance_matrix[pre_customer][suc_customer])
                - 2 * gamma * problem.distance_matrix[0][customer];
        };
      
        InsertCandidates(problem, func, candidate_list, solution, context); // Insert candidates
    } 
    
    // NFIC criteria
    else 
    {
        // Cost function of NFIC
        auto func = [&](Node predecessor, [[maybe_unused]] Node successor, Node customer) {
            Node pre_customer = solution.Customer(predecessor);
        
            if (pre_customer == 0)
                return std::numeric_limits<float>::max();
             
            else
                return static_cast<float>(problem.distance_matrix[pre_customer][customer]);  
        };
      
        InsertCandidates(problem, func, candidate_list, solution, context); // Insert candidates
    }

    return solution; // Return the obtained solution
}