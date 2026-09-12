#ifndef INTERSTELLAR_RELAY_ROUTER_H
#define INTERSTELLAR_RELAY_ROUTER_H

#include "Interstellar_Message_Packet.h"

#include <vector>

struct Route_Option
{
    std::vector<Network_Node_IDs> Nodes;

    double Route_Cost{};

    bool Available{true};
};

class Interstellar_Relay_Router
{
public:

    bool Route_Packet(
        const Message_Packet& packet);

    bool Is_Route_Available(
        const Route_Option& route) const;

    int Select_Best_Route(
        const std::vector<Route_Option>& routes) const;

    bool Route_Packet(
        const Message_Packet& packet,
        const std::vector<Route_Option>& routes);
};

#endif