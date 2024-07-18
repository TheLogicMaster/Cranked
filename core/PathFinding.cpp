#include "Cranked.hpp"

using namespace cranked;

PDGraphNode_32::PDGraphNode_32(NodeGraph graph) : NativeResource(graph->cranked), graph(graph) {
    graph->allocatedNodes.emplace(this);
}

PDGraphNode_32::~PDGraphNode_32() {
    graph->allocatedNodes.erase(this);
    for (auto node : references)
        node->connections.erase(this);
}

PDNodeGraph_32::PDNodeGraph_32(Cranked &cranked) : NativeResource(cranked) {}

PDNodeGraph_32::~PDNodeGraph_32() {
    for (auto node : allocatedNodes)
        node->graph = nullptr;
}

GraphNode PDNodeGraph_32::addNewNode() {
    auto node = cranked.heap.construct<PDGraphNode_32>(this);
    nodes.emplace(node);
    return node;
}

[[nodiscard]] vector<GraphNode> PDNodeGraph_32::pathfind(GraphNode start, GraphNode goal, const optional<function<int(GraphNode, GraphNode)>> &heuristicFunc) const {
    using FrontierElem = pair<int, GraphNode>;
    priority_queue<FrontierElem, vector<FrontierElem>, greater<>> frontier;
    unordered_map<GraphNode, GraphNode> cameFrom;
    unordered_map<GraphNode, int> costSoFar;

    auto heuristic = heuristicFunc.value_or([](GraphNode a, GraphNode b) {
        return abs(a->x - b->x) + abs(a->y - b->y); // Manhatten distance
    });

    auto enqueue = [&](GraphNode node, int priority) {
        frontier.emplace(priority, node);
    };

    auto dequeue = [&] {
        auto best = frontier.top().second;
        frontier.pop();
        return best;
    };

    enqueue(start, 0);
    cameFrom[start] = start;
    costSoFar[start] = 0;

    while (not frontier.empty()) {
        auto current = dequeue();
        if (current == goal)
            break;
        for (auto connection : current->getConnections()) {
            auto next = connection.first;
            int newCost = costSoFar[current] + connection.second;
            if (auto it = costSoFar.find(next); it != costSoFar.end() and newCost >= it->second)
                continue;
            costSoFar[next] = newCost;
            int priority = newCost + heuristic(next, goal);
            enqueue(next, priority);
            cameFrom[next] = current;
        }
    }

    vector<GraphNode> path;
    for (auto node = goal; node != start; node = cameFrom[node]) {
        path.push_back(node);
    }

    return path;
}
