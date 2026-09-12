#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <regex>
#include <string>
#include <vector>
#include <array>
#include <cmath>

#include "Interstellar_Communications_Network.h"
#include "Kepler_Physics_Engine.h"
// ============================================================
// MONTE CARLO EXPERIMENT 4
// RQ4 - MATCHED ROUTING POLICY COMPARISON
//
// Phase 5A diagnostic:
// Replace the Phase 4 scalar Earth-Mars distance with a
// deterministic 3-D heliocentric geometry calculation using
// published JPL approximate Keplerian elements at J2000.0,
// while preserving the matched B1-B4 Monte Carlo world design.
//
// This experiment is being developed for the RQ4
// peer-reviewed journal study.
// ============================================================

enum class Routing_Policy
{
    B1,
    B2,
    B3,
    B4
};

// ============================================================
// PHASE 5A ORBITAL GEOMETRY
//
// JPL approximate planetary elements, Table 1, J2000.0 baseline.
// EM Bary is used as the Earth-system proxy for this diagnostic.
// Phase 5A validates geometry plumbing only; dynamic epoch
// propagation and relay orbits are introduced in later subphases.
// ============================================================

struct RQ4_Orbital_Elements
{
    double Semi_Major_Axis_AU{};
    double Eccentricity{};
    double Inclination_Degrees{};
    double Mean_Longitude_Degrees{};
    double Longitude_Perihelion_Degrees{};
    double Longitude_Ascending_Node_Degrees{};
};

struct RQ4_Planet_State
{
    Eigen::Vector3d Position_Kilometers{};
    double Mean_Anomaly_Radians{};
    double Eccentric_Anomaly_Radians{};
    double True_Anomaly_Radians{};
    double Orbital_Radius_Kilometers{};
};

static constexpr double RQ4_PI =
    3.14159265358979323846;

static constexpr double AU_IN_KILOMETERS =
    149597870.7;

double Degrees_To_Radians(double degrees)
{
    return degrees * RQ4_PI / 180.0;
}

double Normalize_Radians(double angle)
{
    while (angle > RQ4_PI)
    {
        angle -= 2.0 * RQ4_PI;
    }

    while (angle < -RQ4_PI)
    {
        angle += 2.0 * RQ4_PI;
    }

    return angle;
}

RQ4_Planet_State Build_J2000_Planet_State(
    Keplers_Physics_Engine& kepler,
    const RQ4_Orbital_Elements& elements)
{
    RQ4_Planet_State state;

    const double semi_major_axis_km =
        elements.Semi_Major_Axis_AU *
        AU_IN_KILOMETERS;

    const double mean_anomaly_degrees =
        elements.Mean_Longitude_Degrees -
        elements.Longitude_Perihelion_Degrees;

    const double argument_of_periapsis_degrees =
        elements.Longitude_Perihelion_Degrees -
        elements.Longitude_Ascending_Node_Degrees;

    state.Mean_Anomaly_Radians =
        Normalize_Radians(
            Degrees_To_Radians(
                mean_anomaly_degrees));

    state.Eccentric_Anomaly_Radians =
        kepler.Calculate_Eccentric_Anomaly(
            state.Mean_Anomaly_Radians,
            elements.Eccentricity);

    state.True_Anomaly_Radians =
        kepler.Calculate_True_Anomaly(
            state.Eccentric_Anomaly_Radians,
            elements.Eccentricity);

    state.Orbital_Radius_Kilometers =
        kepler.Calculate_Orbital_Radius(
            semi_major_axis_km,
            elements.Eccentricity,
            state.Eccentric_Anomaly_Radians);

    state.Position_Kilometers =
        kepler.Calculate_Cartesian_Position(
            state.Orbital_Radius_Kilometers,
            state.True_Anomaly_Radians,
            Degrees_To_Radians(
                elements.Inclination_Degrees),
            Degrees_To_Radians(
                elements.Longitude_Ascending_Node_Degrees),
            Degrees_To_Radians(
                argument_of_periapsis_degrees));

    return state;
}


// ============================================================
// SHARED MONTE CARLO WORLD
//
// This is generated ONCE for a seed.
//
// B1, B2, B3, and B4 must all receive the same world.
// ============================================================

struct RQ4_Shared_World
{
    std::uint32_t Seed{};

    double Orbital_Phase_Radians{};

    // Phase 5A J2000 heliocentric geometry.
    Eigen::Vector3d Earth_Position_Kilometers{};
    Eigen::Vector3d Mars_Position_Kilometers{};

    // Derived from the 3-D Earth/Mars position vectors.
    double Earth_Mars_Distance_Meters{};

    double Optical_Degradation_Factor{};

    std::array<bool, 3> Relay_Online{};

    double Contact_Realization_Probability{};

    int Generated_Packets{};

    std::vector<double> Packet_Release_Times_Seconds;

    double PAT_Delay_Seconds{};
};


// ============================================================
// RQ4 CANDIDATE ROUTE
//
// Phase 2 diagnostic representation only.
// These routes give B1-B4 legitimate alternatives to compare.
// Scientific route costs are added later.
// ============================================================

struct RQ4_Candidate_Route
{
    std::string Route_Name;
    std::vector<std::string> Nodes;
    bool Available{true};
};

struct RQ4_Route_State
{
    RQ4_Candidate_Route Route;

    double Total_Route_Distance_Meters{};
    double Propagation_Delay_Seconds{};
    double Contact_Realization_Probability{};
    double PAT_Cost_Seconds{};
    double Predicted_Optical_Quality{};
    double Link_Stability{};

    bool Relay_Available{true};
    bool Contact_Available{true};
};

// ============================================================
// BUILD CANDIDATE ROUTES
//
// For now availability is driven only by the shared relay state.
// No policy-specific scoring is performed here.
// ============================================================

std::vector<RQ4_Candidate_Route> Build_Candidate_Routes(
    const RQ4_Shared_World& world)
{
    std::vector<RQ4_Candidate_Route> routes;

    routes.push_back(
        {
            "Direct_Earth_to_Mars",
            {"Earth", "Mars"},
            true
        });

    routes.push_back(
        {
            "Earth_Relay0_Mars",
            {"Earth", "Relay_0", "Mars"},
            world.Relay_Online[0]
        });

    routes.push_back(
        {
            "Earth_Relay1_Mars",
            {"Earth", "Relay_1", "Mars"},
            world.Relay_Online[1]
        });

    routes.push_back(
        {
            "Earth_Relay2_Mars",
            {"Earth", "Relay_2", "Mars"},
            world.Relay_Online[2]
        });

    return routes;
}


std::vector<RQ4_Route_State> Build_Route_States(
    const RQ4_Shared_World& world,
    const std::vector<RQ4_Candidate_Route>& routes)
{
    std::vector<RQ4_Route_State> route_states;

    Interstellar_Communications_Network network;

    // --------------------------------------------------------
    // PHASE 5A DIAGNOSTIC ROUTE GEOMETRY
    // --------------------------------------------------------
    // Earth-Mars distance now comes from deterministic 3-D
    // heliocentric positions calculated from JPL J2000 elements.
    //
    // Relay routes remain the Phase 4 collinear diagnostic here.
    // Their purpose is only to verify that the new physical
    // Earth/Mars distance flows correctly through every route state.
    // Physically distinct relay orbits are Phase 5B.
    // --------------------------------------------------------

    const std::array<double, 3> Relay_Fractions{
        0.25,
        0.50,
        0.75
    };

    for (std::size_t i = 0; i < routes.size(); ++i)
    {
        const RQ4_Candidate_Route& route = routes[i];

        RQ4_Route_State state;

        state.Route = route;

        if (i == 0)
        {
            state.Total_Route_Distance_Meters =
                world.Earth_Mars_Distance_Meters;
        }
        else
        {
            const double relay_fraction =
                Relay_Fractions[i - 1];

            const double earth_to_relay_meters =
                world.Earth_Mars_Distance_Meters *
                relay_fraction;

            const double relay_to_mars_meters =
                world.Earth_Mars_Distance_Meters *
                (1.0 - relay_fraction);

            state.Total_Route_Distance_Meters =
                earth_to_relay_meters +
                relay_to_mars_meters;
        }

        state.Propagation_Delay_Seconds =
            network.Calculate_Propagation_Delay(
                state.Total_Route_Distance_Meters);

        state.Contact_Realization_Probability =
            world.Contact_Realization_Probability;

        state.PAT_Cost_Seconds =
            world.PAT_Delay_Seconds;

        state.Relay_Available =
            route.Available;

        route_states.push_back(state);
    }

    return route_states;
}

// ============================================================
// POLICY NAME
// ============================================================

std::string Policy_Name(Routing_Policy policy)
{
    switch (policy)
    {
        case Routing_Policy::B1:
            return "B1";

        case Routing_Policy::B2:
            return "B2";

        case Routing_Policy::B3:
            return "B3";

        case Routing_Policy::B4:
            return "B4";
    }

    return "UNKNOWN";
}


// ============================================================
// GENERATE SHARED MONTE CARLO WORLD
// ============================================================

RQ4_Shared_World Generate_Shared_World(
    std::uint32_t seed)
{
    std::mt19937 generator(seed);


    std::uniform_real_distribution<double>
        Orbital_Phase_Distribution(
            0.0,
            6.28318530717958647692);


    std::uniform_real_distribution<double>
        Optical_Degradation_Distribution(
            0.85,
            1.00);


    std::bernoulli_distribution
        Relay_Online_Distribution(
            0.90);


    std::uniform_real_distribution<double>
        Contact_Probability_Distribution(
            0.80,
            1.00);


    // Packet releases occur during the first
    // 12 hours of the 24-hour trial window.
    std::uniform_real_distribution<double>
        Packet_Release_Distribution(
            0.0,
            12.0 * 60.0 * 60.0);


    RQ4_Shared_World world;


    world.Seed = seed;


    world.Orbital_Phase_Radians =
        Orbital_Phase_Distribution(
            generator);


    // --------------------------------------------------------
    // Phase 5A: deterministic J2000 Earth/Mars geometry.
    //
    // JPL Table 1 baseline elements (1800-2050 fit):
    // EM Bary: a=1.00000261 AU, e=0.01671123,
    //          I=-0.00001531 deg, L=100.46457166 deg,
    //          long.peri=102.93768193 deg, long.node=0 deg
    // Mars:    a=1.52371034 AU, e=0.09339410,
    //          I=1.84969142 deg, L=-4.55343205 deg,
    //          long.peri=-23.94362959 deg,
    //          long.node=49.55953891 deg
    //
    // These calculations contain no new random draws, so all
    // previously validated RNG draws remain unchanged.
    // --------------------------------------------------------

    Keplers_Physics_Engine kepler;

    const RQ4_Orbital_Elements Earth_Elements{
        1.00000261,
        0.01671123,
        -0.00001531,
        100.46457166,
        102.93768193,
        0.0
    };

    const RQ4_Orbital_Elements Mars_Elements{
        1.52371034,
        0.09339410,
        1.84969142,
        -4.55343205,
        -23.94362959,
        49.55953891
    };

    const RQ4_Planet_State Earth_State =
        Build_J2000_Planet_State(
            kepler,
            Earth_Elements);

    const RQ4_Planet_State Mars_State =
        Build_J2000_Planet_State(
            kepler,
            Mars_Elements);

    world.Earth_Position_Kilometers =
        Earth_State.Position_Kilometers;

    world.Mars_Position_Kilometers =
        Mars_State.Position_Kilometers;

    const Eigen::Vector3d Earth_Mars_Vector_Kilometers =
        world.Mars_Position_Kilometers -
        world.Earth_Position_Kilometers;

    world.Earth_Mars_Distance_Meters =
        Earth_Mars_Vector_Kilometers.norm() *
        1000.0;


    world.Optical_Degradation_Factor =
        Optical_Degradation_Distribution(
            generator);


    for (bool& relay_online :
         world.Relay_Online)
    {
        relay_online =
            Relay_Online_Distribution(
                generator);
    }


    world.Contact_Realization_Probability =
        Contact_Probability_Distribution(
            generator);


    world.Generated_Packets = 200;


    world.Packet_Release_Times_Seconds.reserve(
        static_cast<std::size_t>(
            world.Generated_Packets));


    for (int packet = 0;
         packet < world.Generated_Packets;
         ++packet)
    {
        world.Packet_Release_Times_Seconds.push_back(
            Packet_Release_Distribution(
                generator));
    }


    world.PAT_Delay_Seconds = 60.0;


    return world;
}


// ============================================================
// WRITE ONE TRIAL / POLICY ROW
// ============================================================

void Write_Trial_Row(
    std::ofstream& file,
    int trial,
    Routing_Policy policy,
    const RQ4_Shared_World& world,
    const std::vector<RQ4_Candidate_Route>& routes,
    const std::vector<RQ4_Route_State>& route_states)
{
    file
        << trial
        << ","

        << world.Seed
        << ","

        << Policy_Name(policy)
        << ","

        << std::setprecision(17)

        << world.Orbital_Phase_Radians
        << ","

        << world.Earth_Position_Kilometers.x()
        << ","

        << world.Earth_Position_Kilometers.y()
        << ","

        << world.Earth_Position_Kilometers.z()
        << ","

        << world.Mars_Position_Kilometers.x()
        << ","

        << world.Mars_Position_Kilometers.y()
        << ","

        << world.Mars_Position_Kilometers.z()
        << ","

        << world.Earth_Mars_Distance_Meters
        << ","

        << world.Optical_Degradation_Factor
        << ","

        << (world.Relay_Online[0] ? 1 : 0)
        << ","

        << (world.Relay_Online[1] ? 1 : 0)
        << ","

        << (world.Relay_Online[2] ? 1 : 0)
        << ","

        << world.Contact_Realization_Probability
        << ","

        << world.Generated_Packets
        << ","

        << world.PAT_Delay_Seconds
        << ","

        << world.Packet_Release_Times_Seconds.front()
        << ","

        << world.Packet_Release_Times_Seconds.back()
        << ","

        << (routes[0].Available ? 1 : 0)
        << ","

        << (routes[1].Available ? 1 : 0)
        << ","

        << (routes[2].Available ? 1 : 0)
        << ","

        << (routes[3].Available ? 1 : 0)
        << ",";

        int available_route_count = 0;

        for (const RQ4_Candidate_Route& route : routes)
        {
            if (route.Available)
            {
                ++available_route_count;
            }
        }

file << available_route_count
     << ","
     << route_states[0].Propagation_Delay_Seconds
     << ","
     << route_states[1].Propagation_Delay_Seconds
     << ","
     << route_states[2].Propagation_Delay_Seconds
     << ","
     << route_states[3].Propagation_Delay_Seconds
     << "\n";
}


// ============================================================
// GET NEXT RUN NUMBER
//
// Example:
//
// Trial_1_Monte_Carlo_Experiment_4.csv
// Trial_2_Monte_Carlo_Experiment_4.csv
//
// If Trial_2 exists, next run becomes 3.
// ============================================================

int Get_Next_Run_Number(
    const std::string& results_folder)
{
    int highest_run_number = 0;


    std::regex trial_pattern(
        R"(Trial_(\d+)_Monte_Carlo_Experiment_4\.csv)");


    for (const auto& entry :
         std::filesystem::directory_iterator(
             results_folder))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }


        std::string filename =
            entry.path()
                .filename()
                .string();


        std::smatch match;


        if (std::regex_match(
                filename,
                match,
                trial_pattern))
        {
            int run_number =
                std::stoi(
                    match[1].str());


            if (run_number >
                highest_run_number)
            {
                highest_run_number =
                    run_number;
            }
        }
    }


    return highest_run_number + 1;
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    std::cout
        << "\n====================================================\n";

    std::cout
        << "MONTE CARLO EXPERIMENT 4 - RQ4\n";

    std::cout
        << "PHASE 5A - J2000 EARTH/MARS 3-D GEOMETRY\n";

    std::cout
        << "====================================================\n\n";


    // ========================================================
    // EXPERIMENT SETTINGS
    // ========================================================

    // Start small for debugging.
    const int Number_Of_Debug_Trials = 5;


    const std::uint32_t Base_Seed =
        40000;


    const std::array<Routing_Policy, 4>
        Policies =
    {
        Routing_Policy::B1,
        Routing_Policy::B2,
        Routing_Policy::B3,
        Routing_Policy::B4
    };


    // ========================================================
    // RESULTS FOLDER
    // ========================================================

    const std::string Results_Folder =
        "./"
        "Monte_Carlo_Experiment 4";


    std::filesystem::create_directories(
        Results_Folder);


    // ========================================================
    // DETERMINE RUN NUMBER
    // ========================================================

    const int Run_Number =
        Get_Next_Run_Number(
            Results_Folder);


    // ========================================================
    // BUILD FILE NAMES
    // ========================================================

    const std::string Trial_File_Name =
        Results_Folder +
        "/Trial_" +
        std::to_string(
            Run_Number) +
        "_Monte_Carlo_Experiment_4.csv";


    const std::string Summary_File_Name =
        Results_Folder +
        "/Summary_" +
        std::to_string(
            Run_Number) +
        "_Monte_Carlo_Experiment_4.csv";


    const std::string Matching_File_Name =
        Results_Folder +
        "/RQ4_Matching_Check.csv";


    // ========================================================
    // OPEN OUTPUT FILES
    // ========================================================

    std::ofstream Trial_File(
        Trial_File_Name);


    std::ofstream Summary_File(
        Summary_File_Name);


    std::ofstream Matching_File(
        Matching_File_Name);


    // ========================================================
    // VERIFY FILES OPENED
    // ========================================================

    if (!Trial_File.is_open())
    {
        std::cerr
            << "ERROR: Could not create "
            << "RQ4 trial CSV.\n";

        return 1;
    }


    if (!Summary_File.is_open())
    {
        std::cerr
            << "ERROR: Could not create "
            << "RQ4 summary CSV.\n";

        return 1;
    }


    if (!Matching_File.is_open())
    {
        std::cerr
            << "ERROR: Could not create "
            << "RQ4 matching CSV.\n";

        return 1;
    }


    // ========================================================
    // TRIAL CSV HEADER
    // ========================================================

    Trial_File
        << "Trial,"
        << "Seed,"
        << "Policy,"
        << "Orbital_Phase_Radians,"
        << "Earth_X_km,"
        << "Earth_Y_km,"
        << "Earth_Z_km,"
        << "Mars_X_km,"
        << "Mars_Y_km,"
        << "Mars_Z_km,"
        << "Earth_Mars_Distance_Meters,"
        << "Optical_Degradation_Factor,"
        << "Relay_0_Online,"
        << "Relay_1_Online,"
        << "Relay_2_Online,"
        << "Contact_Realization_Probability,"
        << "Generated_Packets,"
        << "PAT_Delay_Seconds,"
        << "First_Packet_Release_s,"
        << "Last_Packet_Release_s,"
        << "Direct_Available,"
        << "Relay0_Route_Available,"
        << "Relay1_Route_Available,"
        << "Relay2_Route_Available,"
        << "Available_Route_Count,"
        << "Direct_Propagation_Delay_s,"
        << "Relay0_Propagation_Delay_s,"
        << "Relay1_Propagation_Delay_s,"
        << "Relay2_Propagation_Delay_s\n";


    // ========================================================
    // MATCHING CHECK CSV HEADER
    // ========================================================

    Matching_File
        << "Trial,"
        << "Seed,"
        << "Policy,"
        << "Orbital_Phase_Radians,"
        << "Earth_X_km,"
        << "Earth_Y_km,"
        << "Earth_Z_km,"
        << "Mars_X_km,"
        << "Mars_Y_km,"
        << "Mars_Z_km,"
        << "Earth_Mars_Distance_Meters,"
        << "Optical_Degradation_Factor,"
        << "Relay_0_Online,"
        << "Relay_1_Online,"
        << "Relay_2_Online,"
        << "Contact_Realization_Probability,"
        << "Generated_Packets,"
        << "PAT_Delay_Seconds,"
        << "First_Packet_Release_s,"
        << "Last_Packet_Release_s,"
        << "Direct_Available,"
        << "Relay0_Route_Available,"
        << "Relay1_Route_Available,"
        << "Relay2_Route_Available,"
        << "Available_Route_Count,"
        << "Direct_Propagation_Delay_s,"
        << "Relay0_Propagation_Delay_s,"
        << "Relay1_Propagation_Delay_s,"
        << "Relay2_Propagation_Delay_s\n";


    // ========================================================
    // MONTE CARLO TRIAL LOOP
    // ========================================================

    for (int trial = 1;
         trial <= Number_Of_Debug_Trials;
         ++trial)
    {
        const std::uint32_t seed =
            Base_Seed +
            static_cast<std::uint32_t>(
                trial - 1);


        // ====================================================
        // Generate the realized Monte Carlo world ONCE.
        // ====================================================

        const RQ4_Shared_World world =
            Generate_Shared_World(
                seed);


        const std::vector<RQ4_Candidate_Route> Candidate_Routes =
            Build_Candidate_Routes(
                world);

        const std::vector<RQ4_Route_State> Route_States =
            Build_Route_States(
                world,
                Candidate_Routes);

        std::cout
            << "    EARTH J2000 (km): ["
            << world.Earth_Position_Kilometers.x()
            << ", "
            << world.Earth_Position_Kilometers.y()
            << ", "
            << world.Earth_Position_Kilometers.z()
            << "]\n";

        std::cout
            << "    MARS J2000 (km):  ["
            << world.Mars_Position_Kilometers.x()
            << ", "
            << world.Mars_Position_Kilometers.y()
            << ", "
            << world.Mars_Position_Kilometers.z()
            << "]\n";

        std::cout
            << "    3-D Earth-Mars Distance (m): "
            << world.Earth_Mars_Distance_Meters
            << "\n";

        for (const RQ4_Route_State& state : Route_States)
        {
            std::cout
                << "    ROUTE STATE: "
                << state.Route.Route_Name
                << " | Distance (m): "
                << state.Total_Route_Distance_Meters
                << " | Propagation Delay (s): "
                << state.Propagation_Delay_Seconds
                << " | Contact Probability: "
                << state.Contact_Realization_Probability
                << " | PAT Cost: "
                << state.PAT_Cost_Seconds
                << " | Relay Available: "
                << (state.Relay_Available ? "YES" : "NO")
                << "\n";
        }

        int Available_Route_Count = 0;

        for (const RQ4_Candidate_Route& route :
             Candidate_Routes)
        {
            if (route.Available)
            {
                ++Available_Route_Count;
            }
        }


        std::cout
            << "Trial "
            << trial

            << " | Seed "
            << seed

            << " | Orbital phase "
            << world.Orbital_Phase_Radians

            << " | Optical factor "
            << world.Optical_Degradation_Factor

            << " | Available routes "
            << Available_Route_Count

            << "\n";


        for (const RQ4_Candidate_Route& route :
             Candidate_Routes)
        {
            std::cout
                << "    "
                << route.Route_Name
                << " : "
                << (route.Available ? "AVAILABLE" : "UNAVAILABLE")
                << "\n";
        }


        // ====================================================
        // Every routing policy receives the SAME world.
        //
        // Later only routing-policy decision logic changes.
        // ====================================================

        for (Routing_Policy policy :
             Policies)
        {
            Write_Trial_Row(
                Trial_File,
                trial,
                policy,
                world,
                Candidate_Routes,
                Route_States);

            Write_Trial_Row(
                Matching_File,
                trial,
                policy,
                world,
                Candidate_Routes,
                Route_States);
        }
    }


    // ========================================================
    // SUMMARY CSV
    //
    // This is currently a run-level verification summary.
    //
    // Later this file will contain:
    //
    // reliability
    // throughput
    // latency
    // paired differences
    // confidence intervals
    // effect sizes
    // policy comparisons
    // ========================================================

    Summary_File
        << "Run_Number,"
        << "Number_Of_Trials,"
        << "Policies_Per_Trial,"
        << "Total_Policy_Rows,"
        << "First_Seed,"
        << "Last_Seed\n";


    Summary_File
        << Run_Number
        << ","

        << Number_Of_Debug_Trials
        << ","

        << Policies.size()
        << ","

        << Number_Of_Debug_Trials *
           static_cast<int>(
               Policies.size())
        << ","

        << Base_Seed
        << ","

        << Base_Seed +
           static_cast<std::uint32_t>(
               Number_Of_Debug_Trials - 1)

        << "\n";


    // ========================================================
    // CLOSE FILES
    // ========================================================

    Trial_File.close();

    Summary_File.close();

    Matching_File.close();


    // ========================================================
    // COMPLETION OUTPUT
    // ========================================================

    std::cout
        << "\n====================================================\n";

    std::cout
        << "RQ4 RUN "
        << Run_Number
        << " COMPLETE\n";

    std::cout
        << "====================================================\n\n";


    std::cout
        << "Trial file:\n"
        << Trial_File_Name
        << "\n\n";


    std::cout
        << "Summary file:\n"
        << Summary_File_Name
        << "\n\n";


    std::cout
        << "Matching diagnostic:\n"
        << Matching_File_Name
        << "\n\n";


    std::cout
        << "For each Trial/Seed, "
        << "B1-B4 must have identical "
        << "shared-world values.\n";


    return 0;
}
