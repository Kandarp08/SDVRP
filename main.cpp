#include <bits/stdc++.h>
#include <include/distance_matrix_optimizer.h>
#include <include/solver.h>

using namespace std;

Problem ReadProblemFromFile(const string &problem_path)
{
    ifstream ifs(problem_path);
    if (ifs.fail())
    {
        throw invalid_argument("Cannot open problem.");
    }
    Problem problem{};
    ifs >> problem.num_customers >> problem.capacity;
    ++problem.num_customers;
    problem.demands.resize(problem.num_customers);
    for (Node i = 1; i < problem.num_customers; ++i)
    {
        ifs >> problem.demands[i];
    }

    problem.distance_matrix.resize(problem.num_customers);

    vector<pair<int, int>> customers(problem.num_customers);
    for (Node i = 0; i < problem.num_customers; ++i)
    {
        ifs >> customers[i].first >> customers[i].second;
    }
    for (Node i = 0; i < problem.num_customers; ++i)
    {
        problem.distance_matrix[i].resize(problem.num_customers);
        for (Node j = 0; j < problem.num_customers; ++j)
        {
            auto [x1, y1] = customers[i];
            auto [x2, y2] = customers[j];
            problem.distance_matrix[i][j] = lround(hypot(x1 - x2, y1 - y2));
        }
    }
    return problem;
}

int main()
{
    string problem_path = "./data/SET-1/SD1.txt";
    string output = "./solution.txt";

    auto problem = ReadProblemFromFile(problem_path);
    auto distance_matrix_optimizer = DistanceMatrixOptimizer(problem.distance_matrix);
    SpecificSolver solver;
    
    // TODO: pass configs to solver (make cofig.h)
    auto solution = solver.Solve(problem);
    distance_matrix_optimizer.Restore(solution);
    ofstream ofs(output);
    string json_ext(".json");
    if (output.substr(output.size() - json_ext.size()) == json_ext)
    {
        solution.PrintJson(ofs);
    }
    else
    {
        ofs << solution;
    }
    return 0;
}