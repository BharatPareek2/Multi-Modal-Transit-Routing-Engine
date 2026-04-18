#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Benchmark.h"
#include "CLI.h"
#include "Formatter.h"
#include "Graph.h"
#include "NetworkData.h"
#include "RouteResult.h"
#include "RoutingEngine.h"

namespace {

struct Query {
    const char* src;
    const char* dst;
};

const std::vector<Query> kBenchmarkQueries = {
    {"Rajiv Chowk",      "Gurgaon Cyber City"},
    {"Kashmere Gate",    "HUDA City Centre"},
    {"Noida Sector 62",  "IGI Airport"},
    {"Dwarka Sector 21", "Anand Vihar"},
    {"Rohini",           "Saket"},
    {"Vaishali",         "New Delhi"},
    {"Janakpuri West",   "Botanical Garden"},
    {"Chandni Chowk",    "Lajpat Nagar"}
};

template <typename F>
double timeOnce(F&& fn) {
    using clk = std::chrono::steady_clock;
    const auto t0 = clk::now();
    fn();
    const auto t1 = clk::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

std::string queryLabel(const Query& q) {
    return std::string(q.src) + " -> " + q.dst;
}

void runLatencyBenchmarks(const Graph& g, RoutingEngine& engine) {
    std::cout << "\n" << fmt::boxTitle("[1]  Latency Benchmarks (amortised)") << "\n";
    std::cout << "  Each query is timed over many iterations to amortise\n"
                 "  steady_clock granularity on Windows.\n";

    LatencyBenchmark dij;
    for (const auto& q : kBenchmarkQueries) {
        const int a = g.stationId(q.src);
        const int b = g.stationId(q.dst);
        dij.add(queryLabel(q), [&engine, a, b] {
            (void)engine.fastestRoute(a, b);
        }, 2000);
    }
    dij.print(std::cout, "Dijkstra (line-aware, fastest by time)");

    LatencyBenchmark bfs;
    for (const auto& q : kBenchmarkQueries) {
        const int a = g.stationId(q.src);
        const int b = g.stationId(q.dst);
        bfs.add(queryLabel(q), [&engine, a, b] {
            (void)engine.leastStopRoute(a, b);
        }, 5000);
    }
    bfs.print(std::cout, "BFS (least stops)");

    LatencyBenchmark dfs;
    for (const auto& q : kBenchmarkQueries) {
        const int a = g.stationId(q.src);
        const int b = g.stationId(q.dst);
        dfs.add(queryLabel(q), [&engine, a, b] {
            (void)engine.isConnected(a, b);
        }, 5000);
    }
    dfs.print(std::cout, "DFS (connectivity / reachability)");

    LatencyBenchmark alt;
    for (const auto& q : kBenchmarkQueries) {
        const int a = g.stationId(q.src);
        const int b = g.stationId(q.dst);
        alt.add(queryLabel(q), [&engine, a, b] {
            (void)engine.alternateRoutes(a, b, 3);
        }, 300);
    }
    alt.print(std::cout, "alternateRoutes(k=3)");
}

void runFastestRouteDemo(const Graph& g, RoutingEngine& engine,
                          const RouteFormatter& formatter) {
    std::cout << "\n" << fmt::boxTitle("[2]  Eight fastest-route queries") << "\n";
    for (const auto& q : kBenchmarkQueries) {
        const int a = g.stationId(q.src);
        const int b = g.stationId(q.dst);
        RouteResult r;
        const double ms = timeOnce([&] { r = engine.fastestRoute(a, b); });
        formatter.printCompact(std::cout, q.src, q.dst, r, ms);
    }
}

void runOneAndPrint(const Graph& g, RoutingEngine& engine,
                    const RouteFormatter& formatter,
                    const std::string& src, const std::string& dst) {
    const int a = g.stationId(src);
    const int b = g.stationId(dst);
    RouteResult r;
    const double ms = timeOnce([&] { r = engine.fastestRoute(a, b); });
    formatter.printCompact(std::cout, src, dst, r, ms);
}

void runDisruptionScenarios(Graph& g, RoutingEngine& engine,
                             const RouteFormatter& formatter) {
    std::cout << "\n" << fmt::boxTitle("[3]  Scenario: station closure") << "\n";
    g.closeStation(g.stationId("Rajiv Chowk"));
    std::cout << "  >> Closed: Rajiv Chowk\n";
    runOneAndPrint(g, engine, formatter, "Kashmere Gate", "HUDA City Centre");
    runOneAndPrint(g, engine, formatter, "New Delhi",     "Gurgaon Cyber City");
    g.resetAll();

    std::cout << "\n" << fmt::boxTitle("[4]  Scenario: line delay (Yellow Line +5 min/seg)") << "\n";
    g.delayLine("Yellow Line", 5);
    std::cout << "  >> Delayed: Yellow Line by +5 min per segment\n";
    runOneAndPrint(g, engine, formatter, "Kashmere Gate", "HUDA City Centre");
    runOneAndPrint(g, engine, formatter, "Rajiv Chowk",   "Hauz Khas");
    g.resetAll();

    std::cout << "\n" << fmt::boxTitle("[5]  Scenario: multi-hub closure (DFS fallback)") << "\n";
    g.closeStation(g.stationId("Rajiv Chowk"));
    g.closeStation(g.stationId("Mandi House"));
    g.closeStation(g.stationId("Central Secretariat"));
    std::cout << "  >> Closed: Rajiv Chowk, Mandi House, Central Secretariat\n";
    {
        const int a = g.stationId("Kashmere Gate");
        const int b = g.stationId("HUDA City Centre");
        const bool ok = engine.isConnected(a, b);
        std::cout << "  Kashmere Gate  <->  HUDA City Centre  :  "
                  << (ok ? "CONNECTED (fallback route exists)\n"
                         : "NOT CONNECTED\n");
        if (ok) runOneAndPrint(g, engine, formatter,
                               "Kashmere Gate", "HUDA City Centre");
    }
    g.resetAll();

    std::cout << "\n" << fmt::boxTitle("[6]  Scenario: alternate routes (k=3)") << "\n";
    const int a = g.stationId("Rajiv Chowk");
    const int b = g.stationId("Gurgaon Cyber City");
    const auto alts = engine.alternateRoutes(a, b, 3);
    std::cout << "  Rajiv Chowk -> Gurgaon Cyber City\n";
    for (std::size_t i = 0; i < alts.size(); ++i) {
        const std::string tag = (i == 0) ? "Best " : "Alt " + std::to_string(i);
        const auto& r = alts[i];
        std::cout << "    " << fmt::rightPad(tag, 6)
                  << " time="      << fmt::leftPad(std::to_string(r.totalTime) + "m", 4)
                  << "  fare="     << fmt::leftPad(fmt::rupees(r.totalFare), 9)
                  << "  stops="    << fmt::leftPad(std::to_string((int)r.stationPath.size() - 1), 2)
                  << "  transfers=" << r.transfers
                  << "  modes="    << fmt::modes(r.modesUsed, "/") << "\n";
    }
}

void runFullDemo(Graph& g, RoutingEngine& engine) {
    RouteFormatter formatter(g);
    runLatencyBenchmarks(g, engine);
    runFastestRouteDemo(g, engine, formatter);
    runDisruptionScenarios(g, engine, formatter);
    std::cout << "\n" << fmt::repeatChar('=', 52) << "\n"
              << "  Demo complete.\n"
              << fmt::repeatChar('=', 52) << "\n";
}

} // namespace

int main(int argc, char** argv) {
    Graph g;
    loadDelhiNCRNetwork(g);
    RoutingEngine engine(g, /*transferPenalty=*/4);

    bool demoMode = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--demo" || arg == "-d") demoMode = true;
    }

    std::cout << fmt::boxTitle("Delhi NCR Dynamic Public Transport Routing Simulator") << "\n"
              << "  Loaded: "   << g.numStations() << " stations, "
                                 << g.numEdges()   << " connections\n"
              << "  Transfer penalty: " << engine.transferPenalty()
                                        << " min / transfer\n";

    if (demoMode) {
        runFullDemo(g, engine);
        return 0;
    }

    CLI cli(g, engine);
    cli.run();
    return 0;
}
