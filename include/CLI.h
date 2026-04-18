#ifndef TRANSIT_CLI_H
#define TRANSIT_CLI_H

#include <string>

#include "Formatter.h"
#include "Graph.h"
#include "RouteResult.h"
#include "RoutingEngine.h"

class CLI {
public:
    CLI(Graph& g, RoutingEngine& engine);
    void run();

private:
    Graph&         g_;
    RoutingEngine& engine_;
    RouteFormatter formatter_;

    // --- menu screens ---
    void showMenu() const;
    void listStations() const;

    // --- actions ---
    void findFastest();
    void findLeastStops();
    void checkConnectivity();
    void closeStationMenu();
    void delayLineMenu();
    void restoreNetwork();
    void showAlternateRoutes();

    // --- input helpers ---
    std::string readLine(const std::string& prompt) const;
    int         readStation(const std::string& prompt) const;
    int         resolveStationName(const std::string& input) const;
    bool        readSrcDst(int& src, int& dst);
    void        printQueryTime(double ms) const;
};

#endif
