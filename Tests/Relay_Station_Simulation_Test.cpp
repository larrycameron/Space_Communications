#include "Relay_Station_Simulation.h"

#include <iostream>

int main()
{
    Relay_Station_Simulation simulation;

    simulation.Create_Nodes(0);

    std::cout << std::boolalpha;


    // RS-001
    std::cout << "\n=== RS-001: Valid Optical Link ===\n";

    std::cout << simulation.Process_Optical_Link(
        0,
        10.0,       // received power
        1000.0,     // photon flux
        500.0,      // information rate
        20.0,       // signal photons per slot
        100.0,      // photons per packet
        5.0,        // minimum power
        400.0,      // minimum data rate
        50.0        // minimum photons
    ) << '\n';


    // RS-002
    std::cout << "\n=== RS-002: Received Power Below Minimum ===\n";

    std::cout << simulation.Process_Optical_Link(
        0,
        4.0,
        1000.0,
        500.0,
        20.0,
        100.0,
        5.0,
        400.0,
        50.0
    ) << '\n';


    // RS-003
    std::cout << "\n=== RS-003: Information Rate Below Minimum ===\n";

    std::cout << simulation.Process_Optical_Link(
        0,
        10.0,
        1000.0,
        399.0,
        20.0,
        100.0,
        5.0,
        400.0,
        50.0
    ) << '\n';


    // RS-004
    std::cout << "\n=== RS-004: Photons Below Minimum ===\n";

    std::cout << simulation.Process_Optical_Link(
        0,
        10.0,
        1000.0,
        500.0,
        20.0,
        49.0,
        5.0,
        400.0,
        50.0
    ) << '\n';


    // RS-005
    std::cout << "\n=== RS-005: Exactly At All Thresholds ===\n";

    std::cout << simulation.Process_Optical_Link(
        0,
        5.0,
        1000.0,
        400.0,
        20.0,
        50.0,
        5.0,
        400.0,
        50.0
    ) << '\n';


    // RS-006
    std::cout << "\n=== RS-006: Node Does Not Exist ===\n";

    std::cout << simulation.Process_Optical_Link(
        999,
        10.0,
        1000.0,
        500.0,
        20.0,
        100.0,
        5.0,
        400.0,
        50.0
    ) << '\n';


// RS-007
std::cout << "\n=== RS-007: Success Followed By Failure ===\n";

Relay_Station_Simulation state_test;

state_test.Create_Nodes(0);

// First transmission succeeds.
bool first_result = state_test.Process_Optical_Link(
    0,
    10.0,
    1000.0,
    500.0,
    20.0,
    100.0,
    5.0,
    400.0,
    50.0
);

// Second transmission deliberately fails on received power.
bool second_result = state_test.Process_Optical_Link(
    0,
    1.0,
    1000.0,
    500.0,
    20.0,
    100.0,
    5.0,
    400.0,
    50.0
);

std::cout << "First result: "
          << first_result << '\n';

std::cout << "Second result: "
          << second_result << '\n';

state_test.Print_Relay_Stations();


// RS-008
std::cout << "\n=== RS-008: Zero Relay Nodes ===\n";

Relay_Station_Simulation zero_relay_test;
zero_relay_test.Create_Nodes(0);
zero_relay_test.Print_Relay_Summary();


// RS-009
std::cout << "\n=== RS-009: One Relay Node ===\n";

Relay_Station_Simulation one_relay_test;
one_relay_test.Create_Nodes(1);
one_relay_test.Print_Relay_Summary();


// RS-010
std::cout << "\n=== RS-010: Five Relay Nodes ===\n";

Relay_Station_Simulation five_relay_test;
five_relay_test.Create_Nodes(5);
five_relay_test.Print_Relay_Summary();


// RS-011
std::cout << "\n=== RS-011: Process Relay Node 3 ===\n";

bool relay_result =
    five_relay_test.Process_Optical_Link(
        3,
        10.0,
        1000.0,
        500.0,
        20.0,
        100.0,
        5.0,
        400.0,
        50.0);

std::cout << "Relay 3 result: "
          << relay_result << '\n';

five_relay_test.Print_Relay_Stations();


// RS-012
std::cout << "\n=== RS-012: Summary After Processing ===\n";

five_relay_test.Print_Relay_Summary();

std::cout << "\n=== RS-013: Forced Offline Relay ===\n";

Relay_Station_Simulation forced_offline_test;

forced_offline_test.Set_Online_Probability(0.0);
forced_offline_test.Create_Nodes(1);
forced_offline_test.Print_Relay_Summary();


std::cout << "\n=== RS-014: Forced Online Relay ===\n";

Relay_Station_Simulation forced_online_test;

forced_online_test.Set_Online_Probability(1.0);
forced_online_test.Create_Nodes(1);
forced_online_test.Print_Relay_Summary();


// RS-015
std::cout << "\n=== RS-015: Re-create Network ===\n";

Relay_Station_Simulation test15;
test15.Set_Online_Probability(1.0);

test15.Create_Nodes(3);

std::cout << "--- Re-creating with only 1 relay ---\n";

test15.Create_Nodes(1);
test15.Print_Relay_Summary();


// RS-016
std::cout << "\n=== RS-016: Negative Relay Count ===\n";

Relay_Station_Simulation test16;
test16.Create_Nodes(-1);
test16.Print_Relay_Summary();


// RS-017
std::cout << "\n=== RS-017: Offline Relay Rejects Optical Link ===\n";

Relay_Station_Simulation test17;
test17.Set_Online_Probability(0.0);
test17.Create_Nodes(1);

bool offline_result =
    test17.Process_Optical_Link(
        3,
        1000.0,
        1000000.0,
        1000000.0,
        1000.0,
        1000000.0,
        1.0,
        1.0,
        1.0);

std::cout << "Offline relay result: "
          << offline_result << '\n';

test17.Print_Relay_Summary();


// RS-018
std::cout << "\n=== RS-018: All Relays Successfully Retransmit ===\n";

Relay_Station_Simulation test18;
test18.Set_Online_Probability(1.0);
test18.Create_Nodes(3);

for (int node_id = 3; node_id <= 5; ++node_id)
{
    bool result =
        test18.Process_Optical_Link(
            node_id,
            10.0,
            1000.0,
            500.0,
            20.0,
            100.0,
            5.0,
            400.0,
            50.0);

    std::cout << "Relay "
              << node_id
              << " result: "
              << result
              << '\n';
}

test18.Print_Relay_Summary();


// RS-019
std::cout << "\n=== RS-019: Final Regression ===\n";

Relay_Station_Simulation test19;
test19.Set_Online_Probability(1.0);
test19.Create_Nodes(1);

bool regression_success =
    test19.Process_Optical_Link(
        3,
        10.0,
        1000.0,
        500.0,
        20.0,
        100.0,
        5.0,
        400.0,
        50.0);

bool regression_failure =
    test19.Process_Optical_Link(
        3,
        1.0,
        1000.0,
        500.0,
        20.0,
        100.0,
        5.0,
        400.0,
        50.0);

std::cout << "Success result: "
          << regression_success << '\n';

std::cout << "Failure result: "
          << regression_failure << '\n';

test19.Print_Relay_Summary();


return 0;
}