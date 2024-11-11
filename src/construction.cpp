#include "../include/construction.h"
#include <vector>

#include "../include/route_context.h"
#include "../include/utils.h"

using CandidateList = vector<pair<int, int>>;

void SequentialInsertion(const Problem& problem, const int (*func)(Node, Node, Node), 
                         CandidateList& candidateList, Solution& solution, RouteContext& context)
{

}

void ParallelInsertion(const Problem& problem, const int (*func)(Node, Node, Node),
                       CandidateList& candidateList, Solution& solution, RouteContext& context)
{

}

SpecificSolution Construct(const Problem& problem)
{
    CandidateList candidateList;
}