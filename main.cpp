#include <bits/stdc++.h>
#include "include/distance_matrix_optimizer.h"
#include "include/solver.h"

using namespace std;

class SimpleListener : public Listener
{
public:
    void OnStart() override { start_time_ = std::chrono::system_clock::now(); }
    void OnUpdated([[maybe_unused]] const SpecificSolution &solution, int objective) override
    {
        auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::system_clock::now() - start_time_);
        std::cout << "Update at " << elapsed_time.count() << "s: " << objective << std::endl;
    }
    void OnEnd([[maybe_unused]] const SpecificSolution &solution, int objective) override
    {
        auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::system_clock::now() - start_time_);
        std::cout << "End at " << elapsed_time.count() << "s: " << objective << std::endl;
    }

private:
    std::chrono::system_clock::time_point start_time_;
};

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
    SpecificConfig config;

    config.random_seed = 42;
    config.time_limit = 10;
    config.blink_rate = 0.021;
    config.inter_operators = {make_unique<Relocate>(), make_unique<Swap<2, 0>>(), make_unique<Swap<2, 1>>(), make_unique<Swap<2, 1>>(), make_unique<Cross>(), make_unique<SwapStar>(), make_unique<SdSwapStar>(), make_unique<SdSwapOneOne>(), make_unique<SdSwapTwoOne>()};

    config.intra_operators = {make_unique<Exchange>(), make_unique<OrOpt<1>>()};

    auto length = static_cast<int>(83);
    config.acceptance_rule = [length]()
    {
        return std::make_unique<LateAcceptanceHillClimbing>(length);
    };

    auto average_customers = static_cast<int>(36);
    auto max_length = static_cast<int>(8);
    auto split_rate = 0.740;
    auto preserved_probability = 0.096;

    config.ruin_method = make_unique<SisrsRuin>(average_customers, max_length,
                                                split_rate, preserved_probability);

    Sorter sorter;
    sorter.AddSortFunction(std::make_unique<SortByRandom>(), 0.078);
    sorter.AddSortFunction(std::make_unique<SortByDemand>(), 0.225);
    sorter.AddSortFunction(std::make_unique<SortByFar>(), 0.942);
    sorter.AddSortFunction(std::make_unique<SortByClose>(), 0.120);

    config.sorter = sorter;

    config.listener = make_unique<SimpleListener>();

    auto solution = solver.Solve(config, problem);
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