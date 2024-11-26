#include "../include/construction.h"
#include <vector>
#include <random>

#include "../include/route_context.h"
#include "../include/utils.h"

using CandidateList = vector<pair<int, int>>;


Node CalcFleetLowerBound(const Problem &problem) 
{
    int sum_demands = 0;

    for (Node i = 1; i < problem.num_customers; ++i)
      sum_demands += problem.demands[i];

    return (sum_demands + problem.capacity - 1) / problem.capacity;
}

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

template <class Func> void SequentialInsertion(const Problem& problem, const Func &func, 
                         CandidateList& candidateList, SpecificSolution& solution, RouteContext& context)
{
    InsertionWithCost<float> best_insertion{};
    vector<bool> is_full(context.NumRoutes(), false);
    
    while (!candidateList.empty()) 
    {
        bool inserted = false;
        for (Node route_index = 0; route_index < context.NumRoutes(); ++route_index) 
        {
            if (is_full[route_index])
                continue;

            int candidate_position = -1;
            best_insertion.cost = Delta(std::numeric_limits<float>::max(), -1);
       
            for (int i = 0; i < candidateList.size(); ++i) 
            {
                auto [customer, demand] = candidateList[i];
          
                if (context.Load(route_index) + demand > problem.capacity)
                    continue;

                auto insertion = CalcBestInsertion(solution, func, context, route_index, customer);
                
                if (best_insertion.Update(insertion))
                    candidate_position = i;
            
            }
        
            if (candidate_position == -1)
                is_full[route_index] = true;
            
            else 
            {
                auto [customer, demand] = candidateList[candidate_position];

                candidateList[candidate_position] = candidateList.back();
                candidateList.pop_back();

                Node node_index = solution.Insert(customer, demand, best_insertion.predecessor,
                                                    best_insertion.successor);
                if (best_insertion.predecessor == 0)
                    context.SetHead(route_index, node_index);
        
                context.AddLoad(route_index, demand);
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

template <class Func> void ParallelInsertion(const Problem& problem, const Func &func,
                       CandidateList& candidateList, SpecificSolution& solution, RouteContext& context)
{
    vector<vector<InsertionWithCost<float>>> best_insertions(candidateList.size());
    
    for (int i = 0; i < candidateList.size(); ++i) 
    {
        for (Node j = 0; j < context.NumRoutes(); ++j) 
        {
            best_insertions[i].push_back(CalcBestInsertion(solution, func, context, j, candidateList[i].first));
        }
    }

    vector<bool> updated(context.NumRoutes(), false);
    InsertionWithCost<float> best_insertion{};
    
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
            
                    if (best_insertion.Update(best_insertions[i][j])) 
                        candidate_position = i;
                }
            }
        }

        if (candidate_position == -1) 
        {
            int position = AddRoute(candidateList, solution, context);
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
            auto [customer, demand] = candidateList[candidate_position];
            candidateList[candidate_position] = candidateList.back();
            candidateList.pop_back();

            best_insertions[candidate_position] = std::move(best_insertions.back());
            best_insertions.pop_back();

            Node node_index = solution.Insert(customer, demand, best_insertion.predecessor,
                                            best_insertion.successor);
            Node route_index = best_insertion.route_index;
            
            if (best_insertion.predecessor == 0)
                context.SetHead(route_index, node_index);
        
            context.AddLoad(route_index, demand);
            updated[route_index] = true;
        }
    }
}

template <class Func> void InsertCandidates(const Problem &problem, const Func &func,
                        CandidateList &candidate_list, SpecificSolution &solution, 
                        RouteContext &context) 
{
    int strategy = rand() % 2;

    if (strategy == 0)
        SequentialInsertion(problem, func, candidate_list, solution, context);
    
    else
        ParallelInsertion(problem, func, candidate_list, solution, context);
}

SpecificSolution Construct(const Problem& problem)
{
    CandidateList candidate_list;
    Node num_fleets = CalcFleetLowerBound(problem);

    for (Node i = 1; i < problem.num_customers; ++i) 
    {
        int demand = problem.demands[i];
      
        while (demand > 0) 
        {
            int split_demand = min(demand, problem.capacity);
            candidate_list.emplace_back(i, split_demand);
            demand -= split_demand;
        }
    }

    SpecificSolution solution;
    RouteContext context;

    for (Node i = 0; i < num_fleets && !candidate_list.empty(); ++i)
      AddRoute(candidate_list, solution, context);
    
    int criterion = rand() % 2;
    
    if (criterion == 0) 
    {
        float gamma = static_cast<float>(rand() % 35) * 0.05f;
      
        auto func = [&](Node predecessor, Node successor, Node customer) {
            Node pre_customer = solution.Customer(predecessor);
            Node suc_customer = solution.Customer(successor);

            return static_cast<float>(problem.distance_matrix[pre_customer][customer]
                                    + problem.distance_matrix[customer][suc_customer]
                                    - problem.distance_matrix[pre_customer][suc_customer])
                - 2 * gamma * problem.distance_matrix[0][customer];
        };
      
        InsertCandidates(problem, func, candidate_list, solution, context);
    } 
    
    else 
    {
        auto func = [&](Node predecessor, [[maybe_unused]] Node successor, Node customer) {
            Node pre_customer = solution.Customer(predecessor);
        
            if (pre_customer == 0)
                return std::numeric_limits<float>::max();
             
            else
                return static_cast<float>(problem.distance_matrix[pre_customer][customer]);  
        };
      
        InsertCandidates(problem, func, candidate_list, solution, context);
    }
    return solution;
}