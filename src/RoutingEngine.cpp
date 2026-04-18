#include "RoutingEngine.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>
#include <stack>
#include <unordered_map>
#include <vector>

namespace {

// Pack (stationId, lineId) into a single 64-bit key.
// lineId is offset by +1 so the "no previous line" sentinel (-1) maps to 0.
constexpr long long LINE_BUCKET = 1LL << 20;

inline long long stateKey(int station, int lineId) {
    return static_cast<long long>(station) * LINE_BUCKET +
           static_cast<long long>(lineId + 1);
}

inline int stationFromKey(long long key) {
    return static_cast<int>(key / LINE_BUCKET);
}

} // namespace

RoutingEngine::RoutingEngine(const Graph& g, int transferPenalty)
    : g_(g), transferPenalty_(transferPenalty) {}

RouteResult RoutingEngine::fastestRoute(int src, int dst) const {
    return dijkstra(src, dst, {});
}

RouteResult RoutingEngine::dijkstra(int src,
                                    int dst,
                                    const std::set<int>& bannedEdges) const {
    RouteResult empty;
    if (src < 0 || dst < 0) return empty;
    if (src >= g_.numStations() || dst >= g_.numStations()) return empty;
    if (!g_.isStationOpen(src) || !g_.isStationOpen(dst)) return empty;

    if (src == dst) {
        RouteResult r;
        r.found = true;
        r.stationPath = {src};
        return r;
    }

    struct QNode {
        int station;
        int lineId;
        int time;
        bool operator>(const QNode& o) const { return time > o.time; }
    };

    std::priority_queue<QNode, std::vector<QNode>, std::greater<QNode>> pq;
    std::unordered_map<long long, int>        dist;
    std::unordered_map<long long, long long>  parentKey;
    std::unordered_map<long long, int>        parentEdge;

    const long long startKey = stateKey(src, -1);
    dist[startKey] = 0;
    pq.push({src, -1, 0});

    long long destKey = -1;

    while (!pq.empty()) {
        const QNode cur = pq.top();
        pq.pop();

        const long long curKey = stateKey(cur.station, cur.lineId);
        auto it = dist.find(curKey);
        if (it == dist.end() || it->second < cur.time) continue;

        if (cur.station == dst) {
            destKey = curKey;
            break;
        }

        for (int eid : g_.incident(cur.station)) {
            if (bannedEdges.count(eid)) continue;
            const Edge& e = g_.edges()[eid];
            if (!g_.isEdgeUsable(e)) continue;

            const int next = e.other(cur.station);
            if (!g_.isStationOpen(next)) continue;

            const int newLineId = g_.lineId(e.line);
            int penalty = 0;
            if (cur.lineId != -1 && cur.lineId != newLineId) {
                penalty = transferPenalty_;
            }
            const int nt = cur.time + e.effectiveTime() + penalty;
            const long long nk = stateKey(next, newLineId);

            auto itN = dist.find(nk);
            if (itN == dist.end() || nt < itN->second) {
                dist[nk] = nt;
                parentKey[nk] = curKey;
                parentEdge[nk] = eid;
                pq.push({next, newLineId, nt});
            }
        }
    }

    if (destKey == -1) return empty;

    std::vector<int> stationPath;
    std::vector<int> edgePath;
    long long cur = destKey;
    while (cur != startKey) {
        stationPath.push_back(stationFromKey(cur));
        auto itE = parentEdge.find(cur);
        if (itE == parentEdge.end()) break;
        edgePath.push_back(itE->second);
        cur = parentKey[cur];
    }
    stationPath.push_back(src);
    std::reverse(stationPath.begin(), stationPath.end());
    std::reverse(edgePath.begin(), edgePath.end());

    return buildResult(stationPath, edgePath);
}

RouteResult RoutingEngine::buildResult(const std::vector<int>& stationPath,
                                       const std::vector<int>& edgePath) const {
    RouteResult r;
    r.found = true;
    r.stationPath = stationPath;
    r.edgePath = edgePath;

    std::string prevLine;
    for (int eid : edgePath) {
        const Edge& e = g_.edges()[eid];
        r.totalTime += e.effectiveTime();
        r.totalFare += e.fare;
        r.modesUsed.insert(e.mode);
        if (!prevLine.empty() && prevLine != e.line) {
            r.transfers += 1;
            r.totalTime += transferPenalty_;
        }
        prevLine = e.line;
    }
    return r;
}

RouteResult RoutingEngine::leastStopRoute(int src, int dst) const {
    RouteResult empty;
    if (src < 0 || dst < 0) return empty;
    if (src >= g_.numStations() || dst >= g_.numStations()) return empty;
    if (!g_.isStationOpen(src) || !g_.isStationOpen(dst)) return empty;

    if (src == dst) {
        RouteResult r;
        r.found = true;
        r.stationPath = {src};
        return r;
    }

    const int n = g_.numStations();
    std::vector<int>  parent(n, -1);
    std::vector<int>  parentEdge(n, -1);
    std::vector<char> visited(n, 0);

    std::queue<int> q;
    q.push(src);
    visited[src] = 1;

    bool reached = false;
    while (!q.empty() && !reached) {
        const int u = q.front();
        q.pop();
        for (int eid : g_.incident(u)) {
            const Edge& e = g_.edges()[eid];
            if (!g_.isEdgeUsable(e)) continue;
            const int v = e.other(u);
            if (!g_.isStationOpen(v) || visited[v]) continue;
            visited[v] = 1;
            parent[v] = u;
            parentEdge[v] = eid;
            if (v == dst) { reached = true; break; }
            q.push(v);
        }
    }

    if (!visited[dst]) return empty;

    std::vector<int> sp;
    std::vector<int> ep;
    for (int cur = dst; cur != -1; cur = parent[cur]) {
        sp.push_back(cur);
        if (parentEdge[cur] != -1) ep.push_back(parentEdge[cur]);
    }
    std::reverse(sp.begin(), sp.end());
    std::reverse(ep.begin(), ep.end());
    return buildResult(sp, ep);
}

bool RoutingEngine::isConnected(int src, int dst) const {
    if (src < 0 || dst < 0) return false;
    if (src >= g_.numStations() || dst >= g_.numStations()) return false;
    if (!g_.isStationOpen(src) || !g_.isStationOpen(dst)) return false;
    if (src == dst) return true;

    const int n = g_.numStations();
    std::vector<char> visited(n, 0);
    std::stack<int>   st;
    st.push(src);
    visited[src] = 1;

    while (!st.empty()) {
        const int u = st.top();
        st.pop();
        if (u == dst) return true;
        for (int eid : g_.incident(u)) {
            const Edge& e = g_.edges()[eid];
            if (!g_.isEdgeUsable(e)) continue;
            const int v = e.other(u);
            if (!g_.isStationOpen(v) || visited[v]) continue;
            visited[v] = 1;
            st.push(v);
        }
    }
    return false;
}

std::vector<RouteResult> RoutingEngine::alternateRoutes(int src, int dst, int k) const {
    std::vector<RouteResult> results;
    if (k <= 0) return results;

    RouteResult best = dijkstra(src, dst, {});
    if (!best.found) return results;
    results.push_back(best);
    if (k == 1 || best.edgePath.empty()) return results;

    // Two routes are "the same" only if they traverse the identical sequence of
    // edges. Two routes that visit the same station sequence via parallel edges
    // (e.g. Airport Express metro vs Bus DTC-540 between New Delhi and IGI
    // Airport) are genuinely distinct alternates and must not be deduped.
    auto samePath = [](const RouteResult& a, const RouteResult& b) {
        return a.edgePath == b.edgePath && a.stationPath == b.stationPath;
    };

    // Pass 1: ban each edge of the best path in turn and collect distinct alternates.
    for (int eid : best.edgePath) {
        if (static_cast<int>(results.size()) >= k) break;
        const std::set<int> banned = {eid};
        RouteResult alt = dijkstra(src, dst, banned);
        if (!alt.found) continue;

        bool duplicate = false;
        for (const auto& r : results) {
            if (samePath(r, alt)) { duplicate = true; break; }
        }
        if (!duplicate) results.push_back(std::move(alt));
    }

    // Pass 2: if we still need more, try banning pairs of edges from the best path.
    if (static_cast<int>(results.size()) < k && best.edgePath.size() >= 2) {
        for (std::size_t i = 0; i < best.edgePath.size() && (int)results.size() < k; ++i) {
            for (std::size_t j = i + 1; j < best.edgePath.size() && (int)results.size() < k; ++j) {
                const std::set<int> banned = {best.edgePath[i], best.edgePath[j]};
                RouteResult alt = dijkstra(src, dst, banned);
                if (!alt.found) continue;
                bool duplicate = false;
                for (const auto& r : results) {
                    if (samePath(r, alt)) { duplicate = true; break; }
                }
                if (!duplicate) results.push_back(std::move(alt));
            }
        }
    }

    return results;
}
