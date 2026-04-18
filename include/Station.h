#ifndef TRANSIT_STATION_H
#define TRANSIT_STATION_H

#include <string>

struct Station {
    int id = -1;
    std::string name;
    bool closed = false;
};

#endif
