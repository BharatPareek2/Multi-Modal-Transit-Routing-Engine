#include "Formatter.h"

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <sstream>

#include "Edge.h"

namespace fmt {

std::string rupees(int amount) {
    std::ostringstream os;
    os << "Rs. " << amount;
    return os.str();
}

std::string minutes(int mins) {
    std::ostringstream os;
    os << mins << " min" << (mins == 1 ? "" : "s");
    return os.str();
}

std::string modes(const std::set<TransportMode>& modes, const std::string& sep) {
    std::string out;
    for (TransportMode m : modes) {
        if (!out.empty()) out += sep;
        out += modeToString(m);
    }
    return out;
}

std::string repeatChar(char c, int n) {
    return std::string(std::max(0, n), c);
}

std::string rightPad(const std::string& s, int width) {
    if (static_cast<int>(s.size()) >= width) return s;
    return s + std::string(width - s.size(), ' ');
}

std::string leftPad(const std::string& s, int width) {
    if (static_cast<int>(s.size()) >= width) return s;
    return std::string(width - s.size(), ' ') + s;
}

std::string boxTitle(const std::string& title, int width) {
    const int minW = static_cast<int>(title.size()) + 6;
    const int w    = std::max(width, minW);
    const int pad  = (w - static_cast<int>(title.size())) / 2;
    const int rpad = w - static_cast<int>(title.size()) - pad;
    const std::string bar = repeatChar('=', w);

    std::ostringstream os;
    os << bar << "\n"
       << repeatChar(' ', pad) << title << repeatChar(' ', rpad) << "\n"
       << bar;
    return os.str();
}

std::string sectionHeader(const std::string& title, int width) {
    const int textWidth = static_cast<int>(title.size()) + 4;   // "-- " + title + " "
    const int tailLen   = std::max(0, width - textWidth);
    std::ostringstream os;
    os << "-- " << title << " " << repeatChar('-', tailLen);
    return os.str();
}

} // namespace fmt

// ---------------------------------------------------------------------------

RouteFormatter::RouteFormatter(const Graph& g) : g_(g) {}

void RouteFormatter::printFull(std::ostream& os,
                                const RouteResult& r,
                                const std::string& label) const {
    os << "\n  " << fmt::sectionHeader(label) << "\n";
    if (!r.found) {
        os << "  No route available.\n";
        return;
    }

    os << "  Path : ";
    for (std::size_t i = 0; i < r.stationPath.size(); ++i) {
        if (i) os << " -> ";
        os << g_.station(r.stationPath[i]).name;
    }
    os << "\n";

    if (!r.edgePath.empty()) {
        os << "  Legs :\n";
        std::string prevLine;
        for (std::size_t i = 0; i < r.edgePath.size(); ++i) {
            const Edge& e      = g_.edges()[r.edgePath[i]];
            const int   fromId = r.stationPath[i];
            const int   toId   = r.stationPath[i + 1];

            os << "    " << std::setw(2) << (i + 1) << ". "
               << fmt::rightPad(g_.station(fromId).name, 22)
               << " -> "
               << fmt::rightPad(g_.station(toId).name, 22)
               << "  [" << modeToString(e.mode) << " / " << e.line
               << " / " << fmt::minutes(e.effectiveTime())
               << " / " << fmt::rupees(e.fare) << "]";
            if (!prevLine.empty() && prevLine != e.line) os << "  (transfer)";
            os << "\n";
            prevLine = e.line;
        }
    }

    const int stops = static_cast<int>(r.stationPath.size()) - 1;
    os << "  Time      : " << fmt::minutes(r.totalTime) << "\n"
       << "  Fare      : " << fmt::rupees(r.totalFare) << "\n"
       << "  Stops     : " << stops << "\n"
       << "  Transfers : " << r.transfers << "\n"
       << "  Modes     : " << fmt::modes(r.modesUsed) << "\n";
}

void RouteFormatter::printCompact(std::ostream& os,
                                   const std::string& srcName,
                                   const std::string& dstName,
                                   const RouteResult& r,
                                   double ms) const {
    os << "  " << fmt::rightPad(srcName, 22) << " -> "
       << fmt::rightPad(dstName, 22);
    if (!r.found) {
        os << "  NO ROUTE";
    } else {
        const int stops = static_cast<int>(r.stationPath.size()) - 1;
        os << "  time="      << fmt::leftPad(std::to_string(r.totalTime) + "m", 4)
           << "  fare="      << fmt::leftPad(fmt::rupees(r.totalFare), 9)
           << "  stops="     << fmt::leftPad(std::to_string(stops), 2)
           << "  transfers=" << r.transfers;
    }
    std::ostringstream t;
    t << std::fixed << std::setprecision(3) << ms;
    os << "   [" << t.str() << " ms]\n";
}
