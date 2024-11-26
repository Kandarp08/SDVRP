#ifndef SOLUTION_H
#define SOLUTION_H

#include "problem.h"
#include <ostream>
#include <vector>

using namespace std;

class Solution {
public:
    virtual int CalcObjective(const Problem &problem) const = 0;
};

class SpecificSolution : public Solution {
public:
    SpecificSolution() { 
        node_data_.push_back({}); 
    }

    Node Predecessor(Node node_index) const { 
        return node_data_[node_index].predecessor; 
    }

    Node Successor(Node node_index) const { 
        return node_data_[node_index].successor; 
    }

    Node Customer(Node node_index) const { 
        return node_data_[node_index].customer; 
    }

    int Load(Node node_index) const { 
        return node_data_[node_index].load; 
    }

    void SetPredecessor(Node node_index, Node predecessor) {
        node_data_[node_index].predecessor = predecessor;
    }

    void SetSuccessor(Node node_index, Node successor) {
        node_data_[node_index].successor = successor;
    }

    void SetCustomer(Node node_index, Node customer) { 
        node_data_[node_index].customer = customer; 
    }

    void SetLoad(Node node_index, int load) { 
        node_data_[node_index].load = load; 
    }

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

    Node Insert(Node customer, int load, Node predecessor, Node successor) {
        Node node_index = NewNode(customer, load);
        Link(predecessor, node_index);
        Link(node_index, successor);
        return node_index;
    }

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

    void Link(Node predecessor, Node successor) {
        SetPredecessor(successor, predecessor);
        SetSuccessor(predecessor, successor);
    }

    const vector<Node>& NodeIndices() const { 
        return used_nodes_; 
    }

    Node MaxNodeIndex() const { 
        return node_data_.size() - 1; 
    }

    virtual int CalcObjective(const Problem &problem) const override {
        int objective = 0;

        for (Node node_index : NodeIndices()) {
            Node predecessor = Predecessor(node_index);
            Node successor = Successor(node_index);
            
            objective += problem.distance_matrix[Customer(node_index)][Customer(predecessor)];
            
            if (successor == 0) {
                objective += problem.distance_matrix[Customer(node_index)][0];
            }
        }

        return objective;
    }

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

    ostream& PrintJson(ostream &os) {
        os << "[";
        for (Node node_index : NodeIndices()) {
            if (!Predecessor(node_index)) {
                os << "[{ \"customer\": 0, \"quantity\": 0 }";
                
                while (node_index) {
                    Node customer = Customer(node_index);
                    os << ", { \"customer\": " << customer << ", \"quantity\": " << Load(node_index) << " }";
                    node_index = Successor(node_index);
                }

                os << ",{ \"customer\": 0, \"quantity\": 0 }],\n";
            }
        }
        os.seekp(-2, ios::cur);
        return os << "]";
    }

private:
    struct NodeData {
        Node successor;
        Node predecessor;
        Node customer;
        int load;
        Node index_in_used_nodes;
    };

    vector<NodeData> node_data_;
    vector<Node> used_nodes_;
    vector<Node> unused_nodes_;
};


#endif