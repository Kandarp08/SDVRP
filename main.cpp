#include <bits/stdc++.h>
#include "include/distance_matrix_optimizer.h"
#include "include/solver.h"

using namespace std;

// Listener to track the solution process and print updates
class SimpleListener : public Listener
{
public:
    void OnStart() override { start_time_ = std::chrono::system_clock::now(); }
    void OnUpdated([[maybe_unused]] const SpecificSolution &solution, int objective) override
    {
        // Print update time and current objective value
        auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::system_clock::now() - start_time_);
        std::cout << "Update at " << elapsed_time.count() << "s: " << objective << std::endl;
    }
    void OnEnd([[maybe_unused]] const SpecificSolution &solution, int objective) override
    {
        // Print final time and objective value
        auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::system_clock::now() - start_time_);
        std::cout << "End at " << elapsed_time.count() << "s: " << objective << std::endl;
    }

private:
    std::chrono::system_clock::time_point start_time_;
};

// Read problem data from a file
Problem ReadProblemFromFile(const string &problem_path)
{
    ifstream ifs(problem_path);
    if (ifs.fail())
    {
        throw invalid_argument("Cannot open problem.");
    }

    Problem problem{};
    ifs >> problem.num_customers >> problem.capacity;
    ++problem.num_customers; // Adjust for 0-indexing
    problem.demands.resize(problem.num_customers);

    // Read demands for each customer
    for (Node i = 1; i < problem.num_customers; ++i)
    {
        ifs >> problem.demands[i];
    }

    // Read customer locations and compute distances
    vector<pair<int, int>> customers(problem.num_customers);
    for (Node i = 0; i < problem.num_customers; ++i)
    {
        ifs >> customers[i].first >> customers[i].second;
    }

    problem.distance_matrix.resize(problem.num_customers);
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
    // Get range of test cases to process
    int l, r;
    cin >> l >> r;

    l = max(l, 1);  // Ensure valid range
    r = min(r, 21);

    // Process each test case
    for (int i = l; i <= r; i++) {
        cout << endl << "Processing Testcase " << i << endl << endl;

        string problem_path = "./data/SD" + to_string(i) + ".txt";
        string output = "./output/solution" + to_string(i) + ".txt";

        cout << "Problem file: " << problem_path << endl;
        cout << "Output file: " << output << endl;

        // Read problem and initialize solver
        auto problem = ReadProblemFromFile(problem_path);
        auto distance_matrix_optimizer = DistanceMatrixOptimizer(problem.distance_matrix);
        SpecificSolver solver;
        SpecificConfig config;

        config.random_seed = 42;
        config.time_limit = 10;
        config.blink_rate = 0.021;

        // Add operators for optimization
        config.inter_operators.push_back(make_unique<Relocate>());
        config.inter_operators.push_back(make_unique<Swap<2, 0>>());
        config.inter_operators.push_back(make_unique<Swap<2, 1>>());
        config.inter_operators.push_back(make_unique<Swap<2, 2>>());
        config.inter_operators.push_back(make_unique<Cross>());
        config.inter_operators.push_back(make_unique<SwapStar>());
        config.inter_operators.push_back(make_unique<SdSwapStar>());
        config.inter_operators.push_back(make_unique<SdSwapOneOne>());
        config.inter_operators.push_back(make_unique<SdSwapTwoOne>());

        config.intra_operators.push_back(std::make_unique<Exchange>());
        config.intra_operators.push_back(std::make_unique<OrOpt<1>>());
        config.intra_operators.push_back(std::make_unique<OrOpt<2>>());
        config.intra_operators.push_back(std::make_unique<OrOpt<3>>());

        // Configure acceptance rule
        auto length = static_cast<int>(83);
        config.acceptance_rule = [length]()
        {
            return std::make_unique<LateAcceptanceHillClimbing>(length);
        };

        // Configure ruin and recreate method
        auto average_customers = static_cast<int>(36);
        auto max_length = static_cast<int>(8);
        auto split_rate = 0.740;
        auto preserved_probability = 0.096;

        config.ruin_method = make_unique<SisrsRuin>(average_customers, max_length,
                                                    split_rate, preserved_probability);

        // Configure sorting
        Sorter sorter;
        sorter.AddSortFunction(std::make_unique<SortByRandom>(), 0.078);
        sorter.AddSortFunction(std::make_unique<SortByDemand>(), 0.225);
        sorter.AddSortFunction(std::make_unique<SortByFar>(), 0.942);
        sorter.AddSortFunction(std::make_unique<SortByClose>(), 0.120);

        config.sorter = std::move(sorter);

        // Set up listener
        config.listener = std::move(std::make_unique<SimpleListener>());

        // Solve the problem and save the solution
        auto solution = solver.Solve(config, problem);
        distance_matrix_optimizer.Restore(solution);
        ofstream ofs(output);
        ofs << solution;
    }

    return 0;
}
