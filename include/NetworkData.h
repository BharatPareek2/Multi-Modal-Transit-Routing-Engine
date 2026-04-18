#ifndef TRANSIT_NETWORK_DATA_H
#define TRANSIT_NETWORK_DATA_H

#include "Graph.h"

// Populate a Graph with the bundled Delhi NCR sample network
// (60 stations, 124 edges across metro, bus, and walking modes).
void loadDelhiNCRNetwork(Graph& g);

#endif
