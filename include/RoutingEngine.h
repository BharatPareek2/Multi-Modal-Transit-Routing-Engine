#ifndef TRANSIT_ROUTING_ENGINE_H
#define TRANSIT_ROUTING_ENGINE_H

#include <set>
#include <vector>

#include "Graph.h"
#include "RouteResult.h"

class RoutingEngine {
public:
    explicit RoutingEngine(const Graph& g, int transferPenalty = 4);

    int  transferPenalty() const { return transferPenalty_; }
    void setTransferPenalty(int p) { transferPenalty_ = p; }

    RouteResult fastestRoute(int src, int dst) const;
    RouteResult leastStopRoute(int src, int dst) const;
    bool        isConnected(int src, int dst) const;
    std::vector<RouteResult> alternateRoutes(int src, int dst, int k = 3) const;

private:
    const Graph& g_;
    int          transferPenalty_;

    RouteResult dijkstra(int src, int dst, const std::set<int>& bannedEdges) const;
    RouteResult buildResult(const std::vector<int>& stationPath,
                            const std::vector<int>& edgePath) const;
};

#endif
