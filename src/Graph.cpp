#include "Graph.h"

#include <stdexcept>

int Graph::addStation(const std::string& name) {
    auto it = nameIndex_.find(name);
    if (it != nameIndex_.end()) return it->second;

    const int id = static_cast<int>(stations_.size());
    Station s;
    s.id = id;
    s.name = name;
    s.closed = false;
    stations_.push_back(std::move(s));
    adj_.emplace_back();
    nameIndex_.emplace(stations_.back().name, id);
    return id;
}

int Graph::addEdge(int from,
                   int to,
                   int travelTime,
                   int fare,
                   TransportMode mode,
                   const std::string& line) {
    if (from < 0 || from >= numStations() || to < 0 || to >= numStations()) {
        throw std::out_of_range("Graph::addEdge: invalid station id");
    }
    const int id = static_cast<int>(edges_.size());
    Edge e;
    e.id = id;
    e.from = from;
    e.to = to;
    e.travelTime = travelTime;
    e.fare = fare;
    e.mode = mode;
    e.line = line;
    e.active = true;
    e.delay = 0;
    edges_.push_back(std::move(e));

    adj_[from].push_back(id);
    adj_[to].push_back(id);

    if (lineIndex_.find(line) == lineIndex_.end()) {
        lineIndex_.emplace(line, nextLineId_++);
    }
    return id;
}

int Graph::stationId(const std::string& name) const {
    auto it = nameIndex_.find(name);
    return (it == nameIndex_.end()) ? -1 : it->second;
}

const Station& Graph::station(int id) const {
    return stations_.at(id);
}

int Graph::lineId(const std::string& line) const {
    auto it = lineIndex_.find(line);
    return (it == lineIndex_.end()) ? -1 : it->second;
}

void Graph::closeStation(int id) {
    if (id >= 0 && id < numStations()) stations_[id].closed = true;
}

void Graph::openStation(int id) {
    if (id >= 0 && id < numStations()) stations_[id].closed = false;
}

void Graph::delayLine(const std::string& line, int extraMinutes) {
    if (extraMinutes <= 0) return;
    for (auto& e : edges_) {
        if (e.line == line) e.delay += extraMinutes;
    }
}

void Graph::setEdgeActive(int edgeId, bool active) {
    if (edgeId >= 0 && edgeId < numEdges()) edges_[edgeId].active = active;
}

void Graph::resetAll() {
    for (auto& e : edges_) {
        e.active = true;
        e.delay = 0;
    }
    for (auto& s : stations_) {
        s.closed = false;
    }
}

bool Graph::isEdgeUsable(const Edge& e) const {
    if (!e.active) return false;
    if (stations_[e.from].closed) return false;
    if (stations_[e.to].closed) return false;
    return true;
}

bool Graph::isStationOpen(int id) const {
    if (id < 0 || id >= numStations()) return false;
    return !stations_[id].closed;
}
