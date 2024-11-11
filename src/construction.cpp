#include "../include/construction.h"
#include <vector>
#include <random>

#include "../include/route_context.h"
#include "../include/utils.h"

using CandidateList = vector<pair<int, int>>;

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

void SequentialInsertion(const Problem& problem, const int (*func)(Node, Node, Node), 
                         CandidateList& candidateList, SpecificSolution& solution, RouteContext& context)
{
    
}

void ParallelInsertion(const Problem& problem, const int (*func)(Node, Node, Node),
                       CandidateList& candidateList, SpecificSolution& solution, RouteContext& context)
{

}

SpecificSolution Construct(const Problem& problem)
{
    CandidateList candidateList;
}