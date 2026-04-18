#ifndef TRANSIT_EDGE_H
#define TRANSIT_EDGE_H

#include <string>
#include "TransportMode.h"

struct Edge {
    int id = -1;
    int from = -1;
    int to = -1;
    int travelTime = 0;      // base minutes
    int fare = 0;            // INR
    TransportMode mode = TransportMode::Metro;
    std::string line;
    bool active = true;
    int delay = 0;           // extra minutes added by disruption

    int effectiveTime() const { return travelTime + delay; }

    int other(int node) const { return (from == node) ? to : from; }
};

#endif
