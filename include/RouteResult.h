#ifndef TRANSIT_ROUTE_RESULT_H
#define TRANSIT_ROUTE_RESULT_H

#include <set>
#include <vector>

#include "TransportMode.h"

struct RouteResult {
    bool                       found = false;
    std::vector<int>           stationPath;   // ordered station ids
    std::vector<int>           edgePath;      // edge ids between consecutive stations
    int                        totalTime = 0; // minutes (includes transfer penalties)
    int                        totalFare = 0; // INR
    int                        transfers = 0;
    std::set<TransportMode>    modesUsed;
};

#endif
