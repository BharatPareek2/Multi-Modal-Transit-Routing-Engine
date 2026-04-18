#include "Benchmark.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <limits>
#include <ostream>
#include <sstream>

#include "Formatter.h"

void LatencyBenchmark::add(const std::string& label,
                            const std::function<void()>& op,
                            int iterations) {
    if (iterations <= 0 || !op) return;

    using clk = std::chrono::steady_clock;
    const int warmUp = std::min(iterations, 64);
    for (int i = 0; i < warmUp; ++i) op();

    const auto t0 = clk::now();
    for (int i = 0; i < iterations; ++i) op();
    const auto t1 = clk::now();

    const double totalMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    Row row;
    row.label      = label;
    row.iterations = iterations;
    row.avgMs      = totalMs / iterations;
    rows_.push_back(std::move(row));
}

double LatencyBenchmark::avgMs() const {
    if (rows_.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& r : rows_) sum += r.avgMs;
    return sum / static_cast<double>(rows_.size());
}

double LatencyBenchmark::minMs() const {
    if (rows_.empty()) return 0.0;
    double m = std::numeric_limits<double>::infinity();
    for (const auto& r : rows_) m = std::min(m, r.avgMs);
    return m;
}

double LatencyBenchmark::maxMs() const {
    double m = 0.0;
    for (const auto& r : rows_) m = std::max(m, r.avgMs);
    return m;
}

void LatencyBenchmark::print(std::ostream& os, const std::string& title) const {
    const int W_LABEL = 44;
    const int W_ITER  = 10;
    const int W_AVG   = 12;
    const int W_RATE  = 14;

    const std::string bar = "+" + fmt::repeatChar('-', W_LABEL + 2)
                          + "+" + fmt::repeatChar('-', W_ITER  + 2)
                          + "+" + fmt::repeatChar('-', W_AVG   + 2)
                          + "+" + fmt::repeatChar('-', W_RATE  + 2) + "+";

    os << "\n  " << title << "\n  " << bar << "\n"
       << "  | " << fmt::rightPad("Query", W_LABEL)
       << " | " << fmt::leftPad("Iter", W_ITER)
       << " | " << fmt::leftPad("Avg (ms)", W_AVG)
       << " | " << fmt::leftPad("Throughput", W_RATE)
       << " |\n  " << bar << "\n";

    for (const auto& r : rows_) {
        std::ostringstream avg, rate;
        avg << std::fixed << std::setprecision(4) << r.avgMs;

        if (r.avgMs > 0.0) {
            const double ops = 1000.0 / r.avgMs;
            rate << std::fixed << std::setprecision(1);
            if      (ops >= 1e6) rate << (ops / 1e6) << " M/s";
            else if (ops >= 1e3) rate << (ops / 1e3) << " k/s";
            else                 rate << ops << "/s";
        } else {
            rate << "n/a";
        }

        os << "  | " << fmt::rightPad(r.label, W_LABEL)
           << " | " << fmt::leftPad(std::to_string(r.iterations), W_ITER)
           << " | " << fmt::leftPad(avg.str(), W_AVG)
           << " | " << fmt::leftPad(rate.str(), W_RATE)
           << " |\n";
    }
    os << "  " << bar << "\n";

    std::ostringstream summary;
    summary << std::fixed << std::setprecision(4)
            << "    min="  << minMs()
            << " ms   avg=" << avgMs()
            << " ms   max=" << maxMs()
            << " ms   across " << rows_.size() << " queries";
    os << summary.str() << "\n";
}
