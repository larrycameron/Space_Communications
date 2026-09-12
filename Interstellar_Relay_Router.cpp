#include "Interstellar_Relay_Router.h"

#include <iostream>
#include <limits>

bool Interstellar_Relay_Router::Route_Packet(const Message_Packet& packet)
{
    if (packet.source != NODE_EARTH)
    {
        std::cout << "Routing failed: packet did not originate on Earth." << std::endl;

        return false;
    }

    if (packet.destination != NODE_MARS)
    {
        std::cout << "Routing failed: destination is not Mars."  << std::endl;

        return false;
    }

        std::cout << "Routing packet: Earth -> Moon Relay" << std::endl;

        std::cout << "Routing packet: Moon Relay -> Mars" << std::endl;

        std::cout << "Packet successfully reached Mars." << std::endl;

        return true;
}

bool Interstellar_Relay_Router::Is_Route_Available(
    const Route_Option& route) const
{
    if (!route.Available)
    {
        return false;
    }

    if (route.Nodes.size() < 2)
    {
        return false;
    }

    return true;
}


int Interstellar_Relay_Router::Select_Best_Route(
    const std::vector<Route_Option>& routes) const
{
    int best_route_index = -1;

    double best_cost =
        std::numeric_limits<double>::max();

    for (std::size_t i = 0; i < routes.size(); ++i)
    {
        if (!Is_Route_Available(routes[i]))
        {
            continue;
        }

        if (routes[i].Route_Cost < best_cost)
        {
            best_cost = routes[i].Route_Cost;
            best_route_index = static_cast<int>(i);
        }
    }

    return best_route_index;
}


bool Interstellar_Relay_Router::Route_Packet(const Message_Packet& packet, const std::vector<Route_Option>& routes)
{
    if (routes.empty())
    {
        std::cout << "Routing failed: no routes available." << std::endl;

        return false;
    }

    int best_route = Select_Best_Route(routes);

    if (best_route < 0)
    {
        std::cout << "Routing failed: no usable route." << std::endl;

        return false;
    }

    const Route_Option& selected =  routes[best_route];

    if (selected.Nodes.front() != packet.source)
    {
        std::cout << "Routing failed: route source does not match packet source." << std::endl;

        return false;
    }

    if (selected.Nodes.back() != packet.destination)
    {
        std::cout << "Routing failed: route destination does not match packet destination." << std::endl;

        return false;
    }

    std::cout << "Selected route index: " << best_route << std::endl;

    std::cout << "Route cost: " << selected.Route_Cost << std::endl;

    std::cout << "Packet successfully routed." << std::endl;

    return true;
}