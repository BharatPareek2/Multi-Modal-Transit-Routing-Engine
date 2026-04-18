#ifndef TRANSIT_TRANSPORT_MODE_H
#define TRANSIT_TRANSPORT_MODE_H

#include <string>

enum class TransportMode {
    Metro,
    Bus,
    Walking
};

inline std::string modeToString(TransportMode m) {
    switch (m) {
        case TransportMode::Metro:   return "Metro";
        case TransportMode::Bus:     return "Bus";
        case TransportMode::Walking: return "Walking";
    }
    return "Unknown";
}

#endif
