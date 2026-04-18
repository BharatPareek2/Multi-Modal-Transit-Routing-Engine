# Dynamic Public Transport Routing Simulator — Delhi NCR

A modern C++17 command-line simulator for a multi-modal city transit routing
engine. It models a realistic Delhi NCR network (metro, bus, walking) with
**60 stations** and **124 connections**, computes optimal routes and
alternates, and answers connectivity queries **live** under disruptions
(station closures and line delays).

Built as a showcase of classical graph algorithms (Dijkstra, BFS, DFS,
k-shortest path) on a real-world-shaped graph, with a clean OOP layout,
`chrono`-based micro-benchmarks, and an interactive CLI.

---

## Highlights

- **60 stations, 124 connections** spanning Central Delhi, North Delhi, West
  Delhi / Dwarka, South Delhi, Gurgaon, East Delhi, Noida, and IGI Airport.
- **Multi-modal** edges: Metro (68), Bus (32), Walking (24) — each with its own
  time, fare, and line.
- **Four algorithms** in one engine:
  - line-aware Dijkstra for fastest route,
  - BFS for minimum-stop route,
  - iterative DFS for connectivity,
  - edge-banning k-shortest for 3 distinct alternates.
- **Disruption-aware**: close stations, delay whole lines, disable individual
  edges, and reset in one call. Every algorithm respects live state.
- **Amortised micro-benchmarks** via `std::chrono::steady_clock` with warm-up
  and throughput (queries / sec).
- **Polished CLI** with fuzzy station-name lookup, formatted output, and
  per-query timings.

---

## Folder Structure

```
TransitSystem/
├── include/
│   ├── Station.h          // Station POD
│   ├── TransportMode.h    // enum + helpers
│   ├── Edge.h             // Edge POD (effectiveTime, other())
│   ├── Graph.h            // Adjacency-list graph + disruption API
│   ├── RouteResult.h      // Full routing output
│   ├── RoutingEngine.h    // Dijkstra / BFS / DFS / alternates
│   ├── NetworkData.h      // loadDelhiNCRNetwork()
│   ├── Formatter.h        // fmt:: helpers + RouteFormatter
│   ├── Benchmark.h        // LatencyBenchmark (amortised timing)
│   └── CLI.h              // Interactive menu
├── src/
│   ├── Graph.cpp
│   ├── RoutingEngine.cpp
│   ├── NetworkData.cpp    // 60 stations, 124 edges, by zone
│   ├── Formatter.cpp
│   ├── Benchmark.cpp
│   ├── CLI.cpp
│   └── main.cpp           // entry + --demo orchestrator
├── Makefile
└── README.md
```

---

## Architecture

Responsibilities are split into five layers. Each layer only depends on the
layers below it.

| Layer              | Files                                         | Responsibility                                              |
|--------------------|-----------------------------------------------|-------------------------------------------------------------|
| Data model         | `Station.h`, `Edge.h`, `TransportMode.h`      | POD types for nodes, edges, modes.                          |
| Graph structure    | `Graph.h/.cpp`                                | Adjacency list, name / line indexes, disruption controls.   |
| Routing algorithms | `RoutingEngine.h/.cpp`, `RouteResult.h`       | Dijkstra, BFS, DFS, alternate-route search.                 |
| Presentation       | `Formatter.h/.cpp`, `Benchmark.h/.cpp`        | Route pretty-printing, INR/min formatting, latency tables.  |
| App / UI           | `NetworkData.cpp`, `CLI.h/.cpp`, `main.cpp`   | Sample data loader, interactive menu, `--demo` orchestrator.|

**Key design choices**

- **Undirected multigraph, stored once.** Each `Edge` lives in a single
  `edges_` vector; adjacency lists store **edge indices**, not duplicated
  structs. `edge.other(node)` returns the opposite endpoint — that's the only
  direction-awareness the algorithms need.
- **Line-aware Dijkstra state.** The routing engine keys its distance table on
  `(stationId, lineId+1)` packed into a `long long`. Two ways to reach the same
  station via different lines coexist in the heap so transfer penalties stay
  correct. This is the subtle bug a station-only key silently produces.
- **Disruption state lives on the graph.** `Station::closed`, `Edge::active`,
  `Edge::delay` are inspected by every read path via `isStationOpen()` /
  `isEdgeUsable()` / `effectiveTime()`. `resetAll()` is one loop and there is
  exactly one source of truth.
- **`const Graph&` in the routing engine.** Routing is a pure read, so parallel
  queries are safe and the API is honest about what it mutates.
- **Fuzzy station lookup.** The CLI normalises input (case / whitespace
  insensitive), then falls back to substring matching with hints on ambiguity —
  `hauz khas`, `HauzKhas`, and `HAUZ KHAS` all resolve.
- **Presentation separated from routing.** `RouteFormatter` owns every line
  of user-facing output; `LatencyBenchmark` owns every timing table. The CLI
  and `main.cpp --demo` are thin orchestrators that just wire them up.

---

## Algorithms

| Algorithm                      | Where used                            | Why                                                          |
|--------------------------------|---------------------------------------|--------------------------------------------------------------|
| **Dijkstra** (min-heap)        | `fastestRoute`, `alternateRoutes`     | Weighted shortest path with non-negative costs and transfer penalties. `O((V+E) log V)`. |
| **BFS**                        | `leastStopRoute`                      | Unweighted shortest path — minimum number of edges (stops).  |
| **Iterative DFS**              | `isConnected`                         | Reachability only; no distance bookkeeping, no heap overhead.|
| **Edge-banning k-shortest**    | `alternateRoutes(k)`                  | Ban each edge of the primary route in turn (and then pairs) and re-run Dijkstra. Produces *distinct* routes without Yen's full bookkeeping and matches what a commuter actually wants. |

### Why line-aware Dijkstra matters

A naïve Dijkstra keyed on station id alone produces **wrong** transfer
counts. Two equally fast paths to the same station can arrive on different
lines, and that changes how expensive the *next* hop is. Keying the state on
`(station, lineId)` fixes this — both "arrived on Yellow" and "arrived on
Violet" are tracked independently, and the transfer penalty fires only when
the line id actually changes.

### Data structures

| Purpose                        | Type                                                 |
|--------------------------------|------------------------------------------------------|
| stations (id → struct)         | `std::vector<Station>`                               |
| canonical edge list            | `std::vector<Edge>`                                  |
| adjacency list (edge indices)  | `std::vector<std::vector<int>>`                      |
| name → id lookup               | `std::unordered_map<std::string,int>`                |
| line → id lookup               | `std::unordered_map<std::string,int>`                |
| Dijkstra distance table        | `std::unordered_map<long long,int>` keyed on `(station,lineId)` |
| Dijkstra frontier              | `std::priority_queue<QNode,vector,greater<>>`        |
| BFS frontier                   | `std::queue<int>`                                    |
| DFS frontier                   | `std::stack<int>`                                    |

---

## Build

Requires a C++17 compiler (g++, clang++, or MSVC `/std:c++17`).

### Makefile

```bash
make
./transit            # interactive CLI
./transit --demo     # scripted demo + latency benchmarks
```

### One-line g++ build

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude \
    src/Graph.cpp src/RoutingEngine.cpp src/NetworkData.cpp \
    src/Formatter.cpp src/Benchmark.cpp src/CLI.cpp src/main.cpp \
    -o transit
```

---

## How to Run

### Interactive CLI

```bash
./transit
```

```
====================================================
         Delhi NCR Dynamic Transit Routing CLI
====================================================
  1. View all stations
  2. Find fastest route          (Dijkstra)
  3. Find least-stop route       (BFS)
  4. Check connectivity          (DFS)
  5. Simulate station closure
  6. Simulate line delay
  7. Restore network
  8. Show alternate routes (up to 3)
  9. Exit
```

Enter numeric choice. For route queries, type station names (fuzzy matching
enabled). Empty input cancels the current action. Type `?` at any station
prompt to list all stations.

### `--demo` mode

```bash
./transit --demo
```

runs, in order:

1. **Four amortised latency benchmark tables** (Dijkstra, BFS, DFS, alternates).
2. **Eight fastest-route queries** printed compactly.
3. **Station closure** scenario — closes Rajiv Chowk and re-routes.
4. **Line delay** scenario — Yellow Line `+5 min / segment`.
5. **Multi-hub closure** — closes Rajiv Chowk, Mandi House, Central
   Secretariat, and falls back via DFS connectivity.
6. **Alternate routes** — 3 distinct paths between Rajiv Chowk and Gurgaon
   Cyber City.

---

## Benchmarks (real numbers)

Measured with `std::chrono::steady_clock` under `-O2` on a Windows 11 laptop.
`steady_clock` on Windows has ~1 ms granularity, so every query is run
2 000–5 000 times after a 64-iteration warm-up and the total is divided — this
gives sub-microsecond resolution without needing a platform-specific clock.

**Graph size:** 60 stations, 124 edges, transfer penalty 4 min.

### Dijkstra (line-aware, fastest route by time)

| Query                                  | Iter  | Avg (ms) | Throughput |
|----------------------------------------|------:|---------:|-----------:|
| Rajiv Chowk → Gurgaon Cyber City       | 2 000 |   0.1358 |   7.4 k/s  |
| Kashmere Gate → HUDA City Centre       | 2 000 |   0.1710 |   5.8 k/s  |
| Noida Sector 62 → IGI Airport          | 2 000 |   0.1220 |   8.2 k/s  |
| Dwarka Sector 21 → Anand Vihar         | 2 000 |   0.0893 |  11.2 k/s  |
| Rohini → Saket                         | 2 000 |   0.1059 |   9.4 k/s  |
| Vaishali → New Delhi                   | 2 000 |   0.0715 |  14.0 k/s  |
| Janakpuri West → Botanical Garden      | 2 000 |   0.0508 |  19.7 k/s  |
| Chandni Chowk → Lajpat Nagar           | 2 000 |   0.0448 |  22.3 k/s  |

**min=0.0448 ms   avg=0.0989 ms   max=0.1710 ms across 8 queries**

### BFS (least stops)

**min=0.0014 ms   avg=0.0024 ms   max=0.0032 ms** — ~300–700 k queries/sec.

### DFS (connectivity)

**min=0.0006 ms   avg=0.0014 ms   max=0.0026 ms** — up to ~1.7 M queries/sec.

### `alternateRoutes(k=3)`

**min=0.2568 ms   avg=0.4410 ms   max=0.5876 ms** — ~2–4 k queries/sec, as
expected: the call runs up to `1 + k` Dijkstras under edge bans.

### What the numbers mean

| Query class                     | Order of magnitude |
|---------------------------------|---------------------|
| Dijkstra fastest route          | ~100 µs             |
| BFS least-stop route            | ~2 µs               |
| DFS connectivity check          | ~1 µs               |
| Alternates (k=3, up to 4 runs)  | ~400 µs             |

On a 60-station network that comfortably supports well over **10 000 route
queries / second** from a single thread on a laptop.

---

## Sample Output

### Eight fastest-route queries (compact, from `--demo`)

```
Rajiv Chowk       -> Gurgaon Cyber City   time= 50m  fare= Rs. 95   stops= 3  transfers=2   [0.000 ms]
Kashmere Gate     -> HUDA City Centre     time= 64m  fare= Rs. 610  stops=19  transfers=1   [0.000 ms]
Noida Sector 62   -> IGI Airport          time= 49m  fare= Rs. 350  stops= 9  transfers=2   [0.000 ms]
Dwarka Sector 21  -> Anand Vihar          time= 40m  fare= Rs. 190  stops=10  transfers=1   [0.000 ms]
Rohini            -> Saket                time= 47m  fare= Rs. 240  stops=12  transfers=2   [0.000 ms]
Vaishali          -> New Delhi            time= 32m  fare= Rs. 190  stops= 8  transfers=2   [0.000 ms]
Janakpuri West    -> Botanical Garden     time= 27m  fare= Rs.  90  stops= 2  transfers=0   [0.000 ms]
Chandni Chowk     -> Lajpat Nagar         time= 18m  fare= Rs.  70  stops= 5  transfers=1   [0.000 ms]
```

### Alternate routes

```
Rajiv Chowk -> Gurgaon Cyber City
  Best   time= 50m  fare= Rs.  95  stops= 3  transfers=2  modes=Metro/Bus
  Alt 1  time= 53m  fare= Rs.  85  stops= 3  transfers=2  modes=Metro/Bus/Walking
  Alt 2  time= 60m  fare= Rs. 440  stops=17  transfers=1  modes=Metro/Bus
```

Best route is a fast Yellow + bus combo; Alt 1 trades 3 minutes for Rs. 10
savings via a walking leg; Alt 2 is the all-metro route — slower but only one
transfer. All three are genuinely distinct — the dedup compares *both*
`edgePath` and `stationPath`, so parallel edges (Airport Express vs. the
DTC-540 bus between New Delhi and IGI Airport, for example) aren't collapsed.

### Disruption: Yellow Line +5 min/segment

```
>> Delayed: Yellow Line by +5 min per segment
Kashmere Gate -> HUDA City Centre  time= 92m  fare= Rs. 120  stops= 6  transfers=4   [0.000 ms]
Rajiv Chowk   -> Hauz Khas         time= 39m  fare= Rs. 280  stops= 7  transfers=1   [0.000 ms]
```

Kashmere Gate → HUDA City Centre was previously one Yellow-Line ride; under
the delay the engine shifts onto Blue Line + bus connectors, trading transfers
for speed.

### Disruption: multi-hub closure

```
>> Closed: Rajiv Chowk, Mandi House, Central Secretariat
Kashmere Gate <-> HUDA City Centre  :  CONNECTED (fallback route exists)
Kashmere Gate -> HUDA City Centre   time= 77m  fare= Rs. 180  stops= 6  transfers=4   [0.000 ms]
```

With all three central interchanges down, the DFS connectivity check still
returns CONNECTED, and Dijkstra finds a longer fallback via Pink Line / Blue
Line Branch / buses.

---

## Design Targets (all met)

| Target                                                   | Achieved                                    |
|----------------------------------------------------------|---------------------------------------------|
| ≥ 50 stations, ≥ 120 connections                         | 60 stations, 124 connections                |
| Multi-modal network (metro + bus + walk)                 | 68 metro + 32 bus + 24 walk                 |
| Adjacency list (sparse-friendly)                         | `O(V+E)` storage, ~124 edge structs total   |
| Dijkstra with priority queue                             | `O((V+E) log V)` vs. naïve `O(V²)`          |
| Correct transfer-penalty routing                         | Line-aware `(station, lineId)` state        |
| Up to 3 distinct alternate routes                        | `edgePath`-based dedup handles parallel edges |
| Live disruption (close, delay, disable, reset)           | All paths respect `Station::closed`, `Edge::active`, `Edge::delay` |
| Dijkstra avg query well under 10 ms                      | **0.099 ms** average (~100× headroom)        |
| `chrono`-based benchmarking                              | `LatencyBenchmark` with amortised timings   |

---

## Future Improvements

- **Time-dependent edge weights** — different travel times by time of day
  and frequency-aware waiting cost at each stop.
- **True Yen's k-shortest paths** for provably optimal ranked alternates.
- **GTFS ingestion** — drop the bundled `NetworkData.cpp` and load directly
  from `stops.txt` / `routes.txt` / `stop_times.txt`.
- **A\*** with geographic coordinates for faster queries at city scale.
- **Multi-criteria Pareto** — trade time vs. fare vs. transfers as a front
  rather than a single scalar.
- **JSON route export** so the CLI output can power a web UI.
- **Unit tests** for routing invariants (symmetry, closure handling, transfer
  counting) with a lightweight framework.
- **Persistent disruption log** with timestamped restore for simulation
  replays.
