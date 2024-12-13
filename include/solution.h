#ifndef SOLUTION_H
#define SOLUTION_H

#include "problem.h"
#include <ostream>
#include <vector>

using namespace std;

// To represent the obtained solution
class Solution {
public:
    virtual int CalcObjective(const Problem &problem) const = 0;
};

// Derived class
class SpecificSolution : public Solution {
public:
    SpecificSolution() { 
        node_data_.push_back({}); 
    }

    // Get predecessor of a node
    Node Predecessor(Node node_index) const { 
        return node_data_[node_index].predecessor; 
    }

    // Get successor of a node
    Node Successor(Node node_index) const { 
        return node_data_[node_index].successor; 
    }

    // Get the customer number
    Node Customer(Node node_index) const { 
        return node_data_[node_index].customer; 
    }

    // Get the load of a node
    int Load(Node node_index) const { 
        return node_data_[node_index].load; 
    }

    // Set the predecessor of a node
    void SetPredecessor(Node node_index, Node predecessor) {
        node_data_[node_index].predecessor = predecessor;
    }

    // Set the successor of a node
    void SetSuccessor(Node node_index, Node successor) {
        node_data_[node_index].successor = successor;
    }

    // Set the customer for a node
    void SetCustomer(Node node_index, Node customer) { 
        node_data_[node_index].customer = customer; 
    }

    // Set the load for a node
    void SetLoad(Node node_index, int load) { 
        node_data_[node_index].load = load; 
    }

    // Remove a node
    void Remove(Node node_index) {
        Node predecessor = Predecessor(node_index);
        Node successor = Successor(node_index);
        Link(predecessor, successor);

        Node index_in_used_nodes = node_data_[node_index].index_in_used_nodes;
        Node last_node = used_nodes_.back();
        
        node_data_[last_node].index_in_used_nodes = index_in_used_nodes;
        used_nodes_[index_in_used_nodes] = last_node;
        
        used_nodes_.pop_back();
        unused_nodes_.push_back(node_index);
    }

    // Add a node
    Node Insert(Node customer, int load, Node predecessor, Node successor) {
        Node node_index = NewNode(customer, load);
        Link(predecessor, node_index);
        Link(node_index, successor);
        return node_index;
    }

    // Create a new node
    Node NewNode(Node customer, int load) {
        Node node_index;

        if (unused_nodes_.empty()) {
            node_index = node_data_.size();
            node_data_.push_back({});
        } else {
            node_index = unused_nodes_.back();
            unused_nodes_.pop_back();
        }

        node_data_[node_index].index_in_used_nodes = used_nodes_.size();
        used_nodes_.push_back(node_index);
        
        SetCustomer(node_index, customer);
        SetLoad(node_index, load);

        return node_index;
    }

    // Link a node to its successor and predecessor
    void Link(Node predecessor, Node successor) {
        SetPredecessor(successor, predecessor);
        SetSuccessor(predecessor, successor);
    }

    // Return the list of used nodes
    const vector<Node>& NodeIndices() const { 
        return used_nodes_; 
    }

    // Return the index of the maximum node
    Node MaxNodeIndex() const { 
        return node_data_.size() - 1;
    }

    // Calculate value of the objective function
    virtual int CalcObjective(const Problem &problem) const override {
        int objective = 0;

        // Iterate through all nodes
        for (Node node_index : NodeIndices()) {
            Node predecessor = Predecessor(node_index);
            Node successor = Successor(node_index);
            
            // Distance between predecessor and current node
            objective += problem.distance_matrix[Customer(node_index)][Customer(predecessor)];
            
            // If the current node is end of the route
            if (successor == 0) {
                objective += problem.distance_matrix[Customer(node_index)][0];
            }
        }

        return objective; // Return the objective function value
    }

    // Reverse the route
    void ReversedLink(Node left, Node right, Node predecessor, Node successor) {
        while (true) {
            Node original_predecessor = Predecessor(right);
            Link(predecessor, right);
            
            if (right == left) {
                break;
            }

            predecessor = right;
            right = original_predecessor;
        }

        Link(left, successor);
    }

    // Output the solution
    friend ostream& operator<<(ostream &os, const SpecificSolution &solution) {
        Node num_routes = 0;
        for (Node node_index : solution.NodeIndices()) {
            if (!solution.Predecessor(node_index)) {
                os << "Route " << ++num_routes << ": 0";
                while (node_index) {
                    Node customer = solution.Customer(node_index);
                    os << " - " << customer << " (" << solution.Load(node_index) << ")";
                    node_index = solution.Successor(node_index);
                }

                os << " - 0\n";
            }
        }
        return os;
    }

private:

    // Represents a node
    struct NodeData {
        Node successor; // Successor of the node
        Node predecessor; // Predecessor of the node
        Node customer; // Customer corresponding to the node
        int load; // Load for the node
        Node index_in_used_nodes; // Index in the vector used_nodes_
    };

    vector<NodeData> node_data_; // Data of all nodes
    vector<Node> used_nodes_; // List of nodes used
    vector<Node> unused_nodes_; // List of nodes that are not used
};


#endif