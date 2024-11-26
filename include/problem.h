#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>
using namespace std;

using Node = short;

struct Problem{
  Node num_customers;                   // The number of customers, including the depot.
  int capacity;                        // The capacity of the vehicles.
  vector<int> demands;                 // The demands of each customer, including the depot
  vector<vector<int>> distance_matrix; // The distance matrix between customers, including the depot.
};

#endif