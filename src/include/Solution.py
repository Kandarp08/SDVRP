class Solution():
    class NodeData:
        def __init__(self, successor=None, predecessor=None, customer=None, load=0, index_in_used_nodes=0):
            self.successor = successor
            self.predecessor = predecessor
            self.customer = customer
            self.load = load
            self.index_in_used_nodes = index_in_used_nodes

    def __init__(self):
        self.node_data = []
        self.used_nodes = []
        self.unused_nodes = []

    def predecessor(self, node_index):
        return self.node_data[node_index].predecessor

    def successor(self, node_index):
        return self.node_data[node_index].successor

    def customer(self, node_index):
        return self.node_data[node_index].customer

    def load(self, node_index):
        return self.node_data[node_index].load

    def set_predecessor(self, node_index, predecessor):
        self.node_data[node_index].predecessor = predecessor

    def set_successor(self, node_index, successor):
        self.node_data[node_index].successor = successor

    def set_customer(self, node_index, customer):
        self.node_data[node_index].customer = customer

    def set_load(self, node_index, load):
        self.node_data[node_index].load = load

    def remove(self, node_index):
        predecessor = self.predecessor(node_index)
        successor = self.successor(node_index)
        self.link(predecessor, successor)
        
        index_in_used_nodes = self.node_data[node_index].index_in_used_nodes
        last_node = self.used_nodes[-1]
        self.node_data[last_node].index_in_used_nodes = index_in_used_nodes
        self.used_nodes[index_in_used_nodes] = last_node
        self.used_nodes.pop()
        self.unused_nodes.append(node_index)

    def insert(self, customer, load, predecessor, successor):
        node_index = self.new_node(customer, load)
        self.link(predecessor, node_index)
        self.link(node_index, successor)
        return node_index

    def new_node(self, customer, load):
        if not self.unused_nodes:
            node_index = len(self.node_data)
            self.node_data.append(self.NodeData())
        else:
            node_index = self.unused_nodes.pop()
        
        self.node_data[node_index].index_in_used_nodes = len(self.used_nodes)
        self.used_nodes.append(node_index)
        self.set_customer(node_index, customer)
        self.set_load(node_index, load)
        return node_index

    def link(self, predecessor, successor):
        self.set_predecessor(successor, predecessor)
        self.set_successor(predecessor, successor)

    def node_indices(self):
        return self.used_nodes

    def max_node_index(self):
        return len(self.node_data) - 1

    def calc_objective(self, problem):
        objective = 0
        for node_index in self.node_indices():
            predecessor = self.predecessor(node_index)
            successor = self.successor(node_index)
            objective += problem.distance_matrix[self.customer(node_index)][self.customer(predecessor)]
            if successor == 0:
                objective += problem.distance_matrix[self.customer(node_index)][0]
        return objective

    def reversed_link(self, left, right, predecessor, successor):
        while True:
            original_predecessor = self.predecessor(right)
            self.link(predecessor, right)
            if right == left:
                break
            predecessor = right
            right = original_predecessor
        self.link(left, successor)

    def __str__(self):
        output = []
        num_routes = 0
        for node_index in self.node_indices():
            if self.predecessor(node_index) is None:
                route = f"Route {num_routes + 1}: 0"
                num_routes += 1
                while node_index:
                    customer = self.customer(node_index)
                    route += f" - {customer} ({self.load(node_index)})"
                    node_index = self.successor(node_index)
                route += " - 0"
                output.append(route)
        return "\n".join(output)

