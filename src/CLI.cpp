#include "CLI.h"

#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "Edge.h"

namespace {

std::string normalizeName(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        const unsigned char uc = static_cast<unsigned char>(c);
        if (std::isspace(uc)) continue;
        out.push_back(static_cast<char>(std::tolower(uc)));
    }
    return out;
}

std::string trim(const std::string& s) {
    std::size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

// Time a no-arg callable in milliseconds. Used everywhere the CLI needs to
// report per-query timings without repeating the chrono boilerplate.
template <typename F>
double timeMs(F&& fn) {
    using clk = std::chrono::steady_clock;
    const auto t0 = clk::now();
    fn();
    const auto t1 = clk::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

} // namespace

CLI::CLI(Graph& g, RoutingEngine& engine)
    : g_(g), engine_(engine), formatter_(g) {}

// ---------- I/O helpers ----------

std::string CLI::readLine(const std::string& prompt) const {
    std::cout << prompt;
    std::cout.flush();
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cin.clear();
        return {};
    }
    return trim(line);
}

int CLI::resolveStationName(const std::string& input) const {
    if (input.empty()) return -1;

    // 1) Exact match against the registered name.
    const int exact = g_.stationId(input);
    if (exact >= 0) return exact;

    // 2) Case / whitespace insensitive exact match, + collect substring hits.
    const std::string target = normalizeName(input);
    int normExact = -1;
    std::vector<int> substrMatches;
    for (const auto& st : g_.stations()) {
        const std::string n = normalizeName(st.name);
        if (n == target) { normExact = st.id; break; }
        if (n.find(target) != std::string::npos) substrMatches.push_back(st.id);
    }
    if (normExact >= 0) return normExact;

    // 3) Single unambiguous substring hit -> accept silently.
    if (substrMatches.size() == 1) return substrMatches[0];

    // 4) Ambiguous or missing: show hints and fail this attempt.
    if (!substrMatches.empty()) {
        std::cout << "  ? Multiple matches for '" << input << "':\n";
        for (int id : substrMatches) {
            std::cout << "      - " << g_.station(id).name << "\n";
        }
        std::cout << "    Please retype more precisely.\n";
    } else {
        std::cout << "  ! No station matches '" << input
                  << "'. Enter '?' to list all stations.\n";
    }
    return -1;
}

int CLI::readStation(const std::string& prompt) const {
    constexpr int kMaxAttempts = 3;
    for (int attempt = 0; attempt < kMaxAttempts; ++attempt) {
        const std::string line = readLine(prompt);
        if (line.empty()) return -1;
        if (line == "?" || line == "list") { listStations(); continue; }
        const int id = resolveStationName(line);
        if (id >= 0) return id;
    }
    std::cout << "  ! Cancelled after " << kMaxAttempts << " attempts.\n";
    return -1;
}

bool CLI::readSrcDst(int& src, int& dst) {
    src = readStation("  Source      : ");
    if (src < 0) return false;
    dst = readStation("  Destination : ");
    if (dst < 0) return false;
    return true;
}

void CLI::printQueryTime(double ms) const {
    std::cout << "  [query: " << std::fixed << std::setprecision(3) << ms << " ms]\n";
}

// ---------- screens ----------

void CLI::showMenu() const {
    std::cout << "\n" << fmt::boxTitle("Delhi NCR Dynamic Transit Routing CLI") << "\n";
    std::cout << "  1. View all stations\n";
    std::cout << "  2. Find fastest route          (Dijkstra)\n";
    std::cout << "  3. Find least-stop route       (BFS)\n";
    std::cout << "  4. Check connectivity          (DFS)\n";
    std::cout << "  5. Simulate station closure\n";
    std::cout << "  6. Simulate line delay\n";
    std::cout << "  7. Restore network\n";
    std::cout << "  8. Show alternate routes (up to 3)\n";
    std::cout << "  9. Exit\n";
    std::cout << fmt::repeatChar('-', 52) << "\n";
}

void CLI::listStations() const {
    std::cout << "\n  " << fmt::sectionHeader(
                    "Stations (" + std::to_string(g_.numStations()) + ")") << "\n";
    int col = 0;
    for (const auto& st : g_.stations()) {
        std::ostringstream entry;
        entry << " [" << std::setw(2) << st.id << "] " << st.name;
        if (st.closed) entry << " (CLOSED)";
        std::cout << fmt::rightPad(entry.str(), 40);
        if (++col % 2 == 0) std::cout << "\n";
    }
    if (col % 2 != 0) std::cout << "\n";
}

// ---------- actions ----------

void CLI::findFastest() {
    int src, dst;
    if (!readSrcDst(src, dst)) return;
    RouteResult r;
    const double ms = timeMs([&] { r = engine_.fastestRoute(src, dst); });
    formatter_.printFull(std::cout, r, "Fastest Route");
    printQueryTime(ms);
}

void CLI::findLeastStops() {
    int src, dst;
    if (!readSrcDst(src, dst)) return;
    RouteResult r;
    const double ms = timeMs([&] { r = engine_.leastStopRoute(src, dst); });
    formatter_.printFull(std::cout, r, "Least-Stop Route");
    printQueryTime(ms);
}

void CLI::checkConnectivity() {
    int src, dst;
    if (!readSrcDst(src, dst)) return;
    bool ok = false;
    const double ms = timeMs([&] { ok = engine_.isConnected(src, dst); });
    std::cout << "\n  " << g_.station(src).name << "  <->  " << g_.station(dst).name
              << (ok ? "   :   CONNECTED\n"
                     : "   :   NOT CONNECTED (check closures/disruptions)\n");
    printQueryTime(ms);
}

void CLI::closeStationMenu() {
    const int id = readStation("  Station to close : ");
    if (id < 0) return;
    g_.closeStation(id);
    std::cout << "  [*] Closed: " << g_.station(id).name << "\n";
}

void CLI::delayLineMenu() {
    const std::string line = readLine("  Line name (e.g. Yellow Line) : ");
    if (line.empty()) return;

    bool matched = false;
    for (const auto& e : g_.edges()) {
        if (e.line == line) { matched = true; break; }
    }
    if (!matched) {
        std::cout << "  ! No edges found on line '" << line << "'.\n";
        return;
    }

    const std::string minsStr = readLine("  Extra minutes per segment  : ");
    int extra = 0;
    try { extra = std::stoi(minsStr); }
    catch (...) {
        std::cout << "  ! Invalid number.\n";
        return;
    }
    if (extra <= 0) {
        std::cout << "  ! Delay must be positive.\n";
        return;
    }
    g_.delayLine(line, extra);
    std::cout << "  [*] Delayed '" << line << "' by +" << extra
              << " min per segment.\n";
}

void CLI::restoreNetwork() {
    g_.resetAll();
    std::cout << "  [*] Network fully restored "
                 "(all stations open, all edges active, no delays).\n";
}

void CLI::showAlternateRoutes() {
    int src, dst;
    if (!readSrcDst(src, dst)) return;
    std::vector<RouteResult> routes;
    const double ms = timeMs([&] {
        routes = engine_.alternateRoutes(src, dst, 3);
    });
    if (routes.empty()) {
        std::cout << "  No routes available.\n";
        return;
    }
    for (std::size_t i = 0; i < routes.size(); ++i) {
        const std::string label = (i == 0)
            ? "Best Route"
            : "Alternate Route " + std::to_string(i);
        formatter_.printFull(std::cout, routes[i], label);
    }
    std::cout << "\n  [" << routes.size() << " route(s), "
              << std::fixed << std::setprecision(3) << ms << " ms]\n";
}

// ---------- run loop ----------

void CLI::run() {
    while (true) {
        showMenu();
        const std::string choice = readLine("  Choose : ");
        if      (choice == "1") listStations();
        else if (choice == "2") findFastest();
        else if (choice == "3") findLeastStops();
        else if (choice == "4") checkConnectivity();
        else if (choice == "5") closeStationMenu();
        else if (choice == "6") delayLineMenu();
        else if (choice == "7") restoreNetwork();
        else if (choice == "8") showAlternateRoutes();
        else if (choice == "9" || choice == "exit" || choice == "quit") {
            std::cout << "\nGoodbye!\n";
            return;
        }
        else if (choice.empty()) continue;
        else std::cout << "  Unknown option: '" << choice << "'. Choose 1-9.\n";
    }
}
