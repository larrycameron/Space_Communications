
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
// ============================================================
// MONTE CARLO EXPERIMENT 4
// RQ4 - MATCHED ROUTING POLICY COMPARISON
//
// Phase 1:
// Verify that B1, B2, B3, and B4 receive the exact same
// realized Monte Carlo world for each seed.
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

    for (const RQ4_Candidate_Route& route : routes)
    {
        RQ4_Route_State state;

        state.Route = route;

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
    const std::vector<RQ4_Candidate_Route>& routes)
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
        << "MATCHED ROUTING-POLICY WORLD VERIFICATION\n";

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
        << "Available_Route_Count\n";


    // ========================================================
    // MATCHING CHECK CSV HEADER
    // ========================================================

    Matching_File
        << "Trial,"
        << "Seed,"
        << "Policy,"
        << "Orbital_Phase_Radians,"
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
        << "Available_Route_Count\n";


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

        for (const RQ4_Route_State& state : Route_States)
        {
            std::cout
                << "    ROUTE STATE: "
                << state.Route.Route_Name
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
                Candidate_Routes);

            Write_Trial_Row(
                Matching_File,
                trial,
                policy,
                world,
                Candidate_Routes);
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
