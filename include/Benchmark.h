#ifndef TRANSIT_BENCHMARK_H
#define TRANSIT_BENCHMARK_H

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

// LatencyBenchmark
//
// Runs a set of labelled operations a fixed number of times each and reports
// the average per-call latency. Batch timing is used on purpose: on Windows
// the steady_clock granularity is often ~1 ms, so timing a single Dijkstra
// query (~0.2 ms) reports 0. Amortising across thousands of iterations per
// query gives sub-microsecond precision without any platform-specific clock.
class LatencyBenchmark {
public:
    // Warm up for a few iterations, then time `iterations` calls of `op`.
    void add(const std::string& label,
             const std::function<void()>& op,
             int iterations = 1000);

    void print(std::ostream& os, const std::string& title) const;

    double avgMs() const;
    double minMs() const;
    double maxMs() const;
    int    rowCount() const { return static_cast<int>(rows_.size()); }

private:
    struct Row {
        std::string label;
        int         iterations = 0;
        double      avgMs      = 0.0;
    };
    std::vector<Row> rows_;
};

#endif
