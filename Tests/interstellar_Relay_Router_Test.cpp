#include "Interstellar_Relay_Router.h"

#include <iostream>
#include <vector>

int main()
{
    Interstellar_Relay_Router router;

    Message_Packet packet;

    // RR-001
    std::cout << "\n=== RR-001: Earth to Mars ===\n";
    packet.source = NODE_EARTH;
    packet.destination = NODE_MARS;

    std::cout << std::boolalpha
              << router.Route_Packet(packet)
              << '\n';


    // RR-002
    std::cout << "\n=== RR-002: Moon to Mars ===\n";
    packet.source = NODE_MOON;
    packet.destination = NODE_MARS;

    std::cout << router.Route_Packet(packet) << '\n';


    // RR-003
    std::cout << "\n=== RR-003: Earth to Moon ===\n";
    packet.source = NODE_EARTH;
    packet.destination = NODE_MOON;

    std::cout << router.Route_Packet(packet) << '\n';


    // RR-004
    std::cout << "\n=== RR-004: Mars to Earth ===\n";
    packet.source = NODE_MARS;
    packet.destination = NODE_EARTH;

    std::cout << router.Route_Packet(packet) << '\n';


    // RR-005
    std::cout << "\n=== RR-005: Spacecraft to Mars ===\n";
    packet.source = NODE_Spacecraft;
    packet.destination = NODE_MARS;

    std::cout << router.Route_Packet(packet) << '\n';


    Message_Packet packet_006;
    packet_006.source = NODE_EARTH;
    packet_006.destination = NODE_MARS;


    // RR-006
    std::cout << "\n=== RR-006: Lowest Cost Route Selected ===\n";

    std::vector<Route_Option> routes_006 =
    {
        {{NODE_EARTH, NODE_MOON, NODE_MARS}, 10.0, true},
        {{NODE_EARTH, NODE_Spacecraft, NODE_MARS}, 5.0, true},
        {{NODE_EARTH, NODE_1, NODE_MARS}, 8.0, true}
    };

    std::cout << router.Route_Packet(
        packet_006,
        routes_006
    ) << '\n';


    // RR-007
    std::cout << "\n=== RR-007: Cheapest Route Offline ===\n";

    std::vector<Route_Option> routes_007 =
    {
        {{NODE_EARTH, NODE_MOON, NODE_MARS}, 10.0, true},
        {{NODE_EARTH, NODE_Spacecraft, NODE_MARS}, 5.0, false},
        {{NODE_EARTH, NODE_1, NODE_MARS}, 8.0, true}
    };

    std::cout << router.Route_Packet(
        packet_006,
        routes_007
    ) << '\n';


    // RR-008
    std::cout << "\n=== RR-008: All Routes Offline ===\n";

    std::vector<Route_Option> routes_008 =
    {
        {{NODE_EARTH, NODE_MOON, NODE_MARS}, 10.0, false},
        {{NODE_EARTH, NODE_Spacecraft, NODE_MARS}, 5.0, false}
    };

    std::cout << router.Route_Packet(
        packet_006,
        routes_008
    ) << '\n';


    // RR-009
    std::cout << "\n=== RR-009: Empty Route List ===\n";

    std::vector<Route_Option> routes_009;

    std::cout << router.Route_Packet(
        packet_006,
        routes_009
    ) << '\n';


    // RR-010
    std::cout << "\n=== RR-010: Invalid Route Size ===\n";

    std::vector<Route_Option> routes_010 =
    {
        {{NODE_EARTH}, 1.0, true}
    };

    std::cout << router.Route_Packet(
        packet_006,
        routes_010
    ) << '\n';


    // RR-011
    std::cout << "\n=== RR-011: Route Source Mismatch ===\n";

    std::vector<Route_Option> routes_011 =
    {
        {{NODE_MOON, NODE_MARS}, 1.0, true}
    };

    std::cout << router.Route_Packet(
        packet_006,
        routes_011
    ) << '\n';


    // RR-012
    std::cout << "\n=== RR-012: Route Destination Mismatch ===\n";

    std::vector<Route_Option> routes_012 =
    {
        {{NODE_EARTH, NODE_MOON}, 1.0, true}
    };

    std::cout << router.Route_Packet(
        packet_006,
        routes_012
    ) << '\n';

    return 0;
}