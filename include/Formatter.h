#ifndef TRANSIT_FORMATTER_H
#define TRANSIT_FORMATTER_H

#include <iosfwd>
#include <set>
#include <string>

#include "Graph.h"
#include "RouteResult.h"
#include "TransportMode.h"

// Small collection of presentation helpers. Kept in their own namespace so
// that neither the routing engine nor the graph depends on how output looks.
namespace fmt {

std::string rupees(int amount);                          // "Rs. 55"
std::string minutes(int mins);                           // "42 mins"
std::string modes(const std::set<TransportMode>& modes,
                  const std::string& sep = ", ");
std::string repeatChar(char c, int n);
std::string boxTitle(const std::string& title, int width = 52);
std::string sectionHeader(const std::string& title, int width = 52);
std::string rightPad(const std::string& s, int width);
std::string leftPad(const std::string& s, int width);

} // namespace fmt

// Prints route results. Owns no state beyond a reference to the graph so the
// CLI and benchmark harness can share a single formatter.
class RouteFormatter {
public:
    explicit RouteFormatter(const Graph& g);

    // Full multi-line output: path, per-leg breakdown, totals.
    void printFull(std::ostream& os,
                   const RouteResult& r,
                   const std::string& label) const;

    // One-line compact output with timing suffix. Used in benchmark tables.
    void printCompact(std::ostream& os,
                      const std::string& srcName,
                      const std::string& dstName,
                      const RouteResult& r,
                      double ms) const;

private:
    const Graph& g_;
};

#endif
