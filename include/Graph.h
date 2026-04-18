#ifndef TRANSIT_GRAPH_H
#define TRANSIT_GRAPH_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Edge.h"
#include "Station.h"

// Undirected multigraph. Each edge is stored once in `edges_` and referenced
// from both endpoints in the adjacency list.
class Graph {
public:
    int addStation(const std::string& name);
    int addEdge(int from,
                int to,
                int travelTime,
                int fare,
                TransportMode mode,
                const std::string& line);

    int                  stationId(const std::string& name) const; // -1 if missing
    const Station&       station(int id) const;
    const std::vector<Station>& stations() const { return stations_; }

    const std::vector<Edge>& edges() const { return edges_; }
    const std::vector<int>&  incident(int stationId) const { return adj_.at(stationId); }

    int numStations() const { return static_cast<int>(stations_.size()); }
    int numEdges()    const { return static_cast<int>(edges_.size()); }

    // Integer id per unique line name. -1 if unknown.
    int lineId(const std::string& line) const;

    // Disruption controls
    void closeStation(int id);
    void openStation(int id);
    void delayLine(const std::string& line, int extraMinutes);
    void setEdgeActive(int edgeId, bool active);
    void resetAll();

    // Query helpers used by the routing engine.
    bool isEdgeUsable(const Edge& e) const;
    bool isStationOpen(int id) const;

private:
    std::vector<Station>              stations_;
    std::vector<Edge>                 edges_;
    std::vector<std::vector<int>>     adj_;
    std::unordered_map<std::string, int> nameIndex_;
    std::unordered_map<std::string, int> lineIndex_;
    int nextLineId_ = 0;
};

#endif
