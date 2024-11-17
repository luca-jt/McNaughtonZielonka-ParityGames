#include <unordered_set>
#include <algorithm>
#include <utility>
using std::pair;
#include <string>
using std::string;
#include <iostream>
using std::cout;

//
// types + util --------------------------------------------------------------------------------------
//

enum NodeType {
    ADAM,
    EVE
};

struct ParityNode {
    char name;
    unsigned int prio;
    NodeType type;

    bool operator == (ParityNode const& other) const {
        return name == other.name;
    }
};

struct node_hash {
    size_t operator () (ParityNode const& node) const {
        return std::hash<char>()(node.name);
    }
};

using NodeSet = std::unordered_set<ParityNode, node_hash>;

string node_set_string(NodeSet const & s) {
    string result = "{";
    for (auto& node : s) {
        result += node.name;
        result += ", ";
    }
    if (result.size() > 1) {
        result.pop_back();
        result.pop_back();
    }
    result += "}";
    return result;
}

struct Edge {
    ParityNode node1;
    ParityNode node2;

    Edge(NodeSet const& nodes, char char1, char char2)
    :   node1(*std::find_if(begin(nodes), end(nodes), [=](ParityNode const& node){ return node.name == char1; })),
        node2(*std::find_if(begin(nodes), end(nodes), [=](ParityNode const& node){ return node.name == char2; }))
    {}

    bool operator == (Edge const& other) const {
        return node1.name == other.node1.name && node2.name == other.node2.name;
    }
};

struct edge_hash {
    size_t operator () (Edge const& edge) const {
        return std::hash<string>()(string{edge.node1.name, edge.node2.name});
    }
};

using EdgeSet = std::unordered_set<Edge, edge_hash>;


NodeSet set_difference(NodeSet const& nodes1, NodeSet const& nodes2) {
    NodeSet difference;
    for (auto& node : nodes1) {
        if (!nodes2.contains(node)) {
            difference.insert(node);
        }
    }
    return difference;
}

NodeSet set_union(NodeSet const& nodes1, NodeSet const& nodes2) {
    NodeSet union_set = nodes1;
    union_set.insert(begin(nodes2), end(nodes2));
    return union_set;
}

//
// example game data --------------------------------------------------------------------------------
//

static NodeSet const NODES = {
    ParityNode{'a', 3, EVE},
    ParityNode{'b', 3, ADAM},
    ParityNode{'c', 2, EVE},
    ParityNode{'d', 1, EVE},
    ParityNode{'e', 2, ADAM}
    /*ParityNode{'f', 4, EVE},
    ParityNode{'g', 1, ADAM},
    ParityNode{'h', 2, EVE},
    ParityNode{'i', 3, ADAM}*/
};

static EdgeSet const EDGES = {
    Edge(NODES, 'a', 'e'),
    Edge(NODES, 'e', 'd'),
    Edge(NODES, 'd', 'e'),
    Edge(NODES, 'd', 'c'),
    Edge(NODES, 'c', 'b'),
    Edge(NODES, 'b', 'c'),
    Edge(NODES, 'a', 'b'),
    Edge(NODES, 'b', 'a')
    /*Edge(NODES, 'a', 'f'),
    Edge(NODES, 'f', 'a'),
    Edge(NODES, 'a', 'b'),
    Edge(NODES, 'g', 'f'),
    Edge(NODES, 'b', 'f'),
    Edge(NODES, 'g', 'a'),
    Edge(NODES, 'g', 'b'),
    Edge(NODES, 'b', 'g'),
    Edge(NODES, 'e', 'g'),
    Edge(NODES, 'e', 'c'),
    Edge(NODES, 'b', 'e'),
    Edge(NODES, 'h', 'e'),
    Edge(NODES, 'g', 'h'),
    Edge(NODES, 'h', 'g'),
    Edge(NODES, 'b', 'c'),
    Edge(NODES, 'c', 'b'),
    Edge(NODES, 'c', 'h'),
    Edge(NODES, 'h', 'c'),
    Edge(NODES, 'c', 'd'),
    Edge(NODES, 'd', 'c'),
    Edge(NODES, 'h', 'i'),
    Edge(NODES, 'i', 'h'),
    Edge(NODES, 'h', 'd'),
    Edge(NODES, 'i', 'c'),
    Edge(NODES, 'd', 'i'),
    Edge(NODES, 'i', 'd')*/
};

//
// mcnaughton zielonka algo code -----------------------------------------------------------------------------------
//

NodeSet reach_attr(NodeType type, NodeSet const& k, NodeSet const& nodes, EdgeSet const& edges) {
    NodeSet attr = k;
    bool nodes_added = true;
    while (nodes_added) {
        nodes_added = false;
        for (auto& node : nodes) {
            if (!attr.contains(node)) {
                if (node.type == type) {
                    if (std::find_if(begin(edges), end(edges), [&](Edge const& edge){
                        // player can reach k from node
                        return edge.node1.name == node.name && attr.contains(edge.node2);
                    }) != end(edges)) {
                        nodes_added = true;
                        attr.insert(node);
                    }
                } else {
                    if (std::find_if(begin(edges), end(edges), [&](Edge const& edge){
                        // opponent can not avoid going to k
                        return edge.node1.name == node.name && !attr.contains(edge.node2);
                    }) == end(edges)) {
                        nodes_added = true;
                        attr.insert(node);
                    }
                }
            }
        }
    }
    return attr;
}

// find the winning regions for eve and adam
pair<NodeSet, NodeSet> mcnaughtonzielonka(NodeSet const& nodes, EdgeSet const& edges) {
    // find highest prio
    unsigned int max_prio = 0;
    for (auto& node : nodes) {
        max_prio = std::max(max_prio, node.prio);
    }
    cout << "max prio is " << max_prio << "\n";
    // base case
    if (max_prio == 0) {
        // eve wins
        cout << "return base case prio 0: W_E = " << node_set_string(nodes) << ", W_A = {}\n";
        return {nodes, NodeSet{}};
    }
    // find nodes with that prio
    NodeSet k;
    for (auto& node : nodes) {
        if (node.prio == max_prio) {
            k.insert(node);
        }
    }
    cout << "K = " << node_set_string(k) << "\n";
    if (max_prio % 2 == 0) {
        // calculate eve's reach attractor of K
        NodeSet attr = reach_attr(EVE, k, nodes, edges);
        cout << "Attr_E = " << node_set_string(attr) << "\n";
        // create a new subgame without the attractor
        NodeSet subgame_nodes = set_difference(nodes, attr);
        EdgeSet subgame_edges;
        for (auto& edge : edges) {
            if (!attr.contains(edge.node1) && !attr.contains(edge.node2)) {
                subgame_edges.insert(edge);
            }
        }
        cout << "created subgame with nodes: " << node_set_string(subgame_nodes) << "\n";
        // calculate winning regions recursively
        auto [eve_region, adam_region] = mcnaughtonzielonka(subgame_nodes, subgame_edges);
        if (eve_region == set_difference(nodes, attr)) {
            cout << "W_A' == Q\\Attr_E -> return W_E = " << node_set_string(nodes) << ", W_A = {}\n";
            return {nodes, NodeSet{}};
        }
        auto opponent_attr = reach_attr(ADAM, adam_region, nodes, edges);
        cout << "B = Attr_A(W_A') = " << node_set_string(opponent_attr) << "\n";
        // create a new subgame without the opponent attractor
        NodeSet opp_subgame_nodes = set_difference(nodes, opponent_attr);
        EdgeSet opp_subgame_edges;
        for (auto& edge : edges) {
            if (!opponent_attr.contains(edge.node1) && !opponent_attr.contains(edge.node2)) {
                opp_subgame_edges.insert(edge);
            }
        }
        cout << "subgame with Q\\B: " << node_set_string(opp_subgame_nodes) << "\n";
        // winning regions of game without the opponent attractor
        auto [opp_eve_region, opp_adam_region] = mcnaughtonzielonka(opp_subgame_nodes, opp_subgame_edges);
        cout << "Eve: return W_E = " << node_set_string(opp_eve_region) << ", W_A = W_A' u B = " << node_set_string(set_union(opp_adam_region, opponent_attr)) << "\n";
        return {opp_eve_region, set_union(opp_adam_region, opponent_attr)};

    } else {
        // calculate adam's reach attractor of K
        NodeSet attr = reach_attr(ADAM, k, nodes, edges);
        cout << "Attr_A = " << node_set_string(attr) << "\n";
        // create a new subgame without the attractor
        NodeSet subgame_nodes = set_difference(nodes, attr);
        EdgeSet subgame_edges;
        for (auto& edge : edges) {
            if (!attr.contains(edge.node1) && !attr.contains(edge.node2)) {
                subgame_edges.insert(edge);
            }
        }
        cout << "created subgame with nodes: " << node_set_string(subgame_nodes) << "\n";
        // calculate winning regions recursively
        auto [eve_region, adam_region] = mcnaughtonzielonka(subgame_nodes, subgame_edges);
        if (adam_region == set_difference(nodes, attr)) {
            cout << "W_A' == Q\\Attr_E -> return W_E = {}, W_A = " << node_set_string(nodes) << "\n";
            return {NodeSet{}, nodes};
        }
        auto opponent_attr = reach_attr(EVE, eve_region, nodes, edges);
        cout << "B = Attr_E(W_E') = " << node_set_string(opponent_attr) << "\n";
        // create a new subgame without the opponent attractor
        NodeSet opp_subgame_nodes = set_difference(nodes, opponent_attr);
        EdgeSet opp_subgame_edges;
        for (auto& edge : edges) {
            if (!opponent_attr.contains(edge.node1) && !opponent_attr.contains(edge.node2)) {
                opp_subgame_edges.insert(edge);
            }
        }
        cout << "subgame without Q\\B: " << node_set_string(opp_subgame_nodes) << "\n";
        // winning regions of game without the opponent attractor
        auto [opp_eve_region, opp_adam_region] = mcnaughtonzielonka(opp_subgame_nodes, opp_subgame_edges);
        cout << "Adam: return W_E = W_E' u B = " << node_set_string(set_union(opp_eve_region, opponent_attr)) << ", W_A = " << node_set_string(opp_adam_region) << "\n";
        return {set_union(opp_eve_region, opponent_attr), opp_adam_region};
    }
}

//
// test ----------------------------------------------------------------------------------------------
//

int main() {
    auto [eve_region, adam_region] = mcnaughtonzielonka(NODES, EDGES);
    cout << "-----------------------------------------------------\n";
    cout << "EVE: " << node_set_string(eve_region) << "\n";
    cout << "ADAM: " << node_set_string(adam_region) << std::endl;
    return 0;
}
