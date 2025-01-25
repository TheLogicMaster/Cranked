// Based on https://www.redblobgames.com/pathfinding/a-star/implementation.html

#pragma once

#include "Utils.hpp"

namespace cranked {

    class Cranked;

    struct PDGraphNode_32 : NativeResource {
        explicit PDGraphNode_32(NodeGraph graph);
        ~PDGraphNode_32() override;

        void assertValidGraph() const {
            if (!graph)
                throw CrankedError("Parent Node Graph is invalid");
        }

        void addConnection(GraphNode other, int weight) {
            other->references.emplace(this);
            connections[other] = weight;
        }

        void removeConnection(GraphNode other) {
            other->references.erase(this);
            connections.erase(other);
        }

        void removeConnections() {
            for (auto pair : unordered_map(connections))
                removeConnection(pair.first);
        }

        void removeIncomingConnections() {
            for (auto ref : unordered_set(references))
                ref->removeConnection(this);
        }

        void removeAllConnections() {
            removeConnections();
            removeIncomingConnections();
        }

        const unordered_map<GraphNode, int> &getConnections() {
            return connections;
        }

        int id{};
        int x{}, y{};
        NodeGraph graph; // Weak reference to NodeGraph, must be cleared when graph is disposed to invalidate this node

    private:
        unordered_map<GraphNode, int> connections; // Mapping of other node ID to weight
        unordered_set<GraphNode> references; // Weak references to this node from other nodes, will disconnect others from *this* when disposed
    };

    struct PDNodeGraph_32 : NativeResource {
        friend PDGraphNode_32;

        explicit PDNodeGraph_32(Cranked &cranked);
        ~PDNodeGraph_32() override;

        GraphNode addNewNode();

        [[nodiscard]] GraphNode findById(int id) const {
            for (const auto& node : nodes) {
                if (node->id == id)
                    return node;
            }
            return nullptr;
        }

        [[nodiscard]] GraphNode findByXY(int x, int y) const {
            for (const auto& node : nodes) {
                if (node->x == x and node->y == y)
                    return node;
            }
            return nullptr;
        }

        [[nodiscard]] vector<GraphNode> pathfind(GraphNode start, GraphNode end, const optional<function<int(GraphNode, GraphNode)>> &heuristic = {}) const;

        unordered_resource_set<GraphNode> nodes;

    private:
        unordered_set<GraphNode> allocatedNodes;
    };

}
