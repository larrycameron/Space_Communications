#include "Kepler_Physics_Engine.h"
#include "Shapiro_Time_Delay.h"
#include "Spacecraft_Geometry.h"
#include "Spacecraft_Communication_Link.h"
#include "Interstellar_Communications_Network.h"
#include "Interstellar_Communications_Simulator_Display.h"
#include "Interstellar_Network_Socket.h"
#include "Interstellar_Message_Packet.h"
#include "SignalAnalyzer.h"
#include "Interstellar_Relay_Router.h"
#include "Optical_Communication.h"
#include "Relay_Station_Simulation.h"
#include "Relay_Topology_Space.h"
#include "Contact_Window.h"
#include "PAT_Timing.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <limits>
#include <fstream>
#include <cstdlib>
#include <openssl/rand.h>

// ============================================================
// MENU INPUT
// ============================================================
int Read_Menu_Choice(int minimum, int maximum)
{
    while (true)
    {
        std::string input;
        std::getline(std::cin, input);

        try
        {
            std::size_t position = 0;
            int choice = std::stoi(input, &position);

            if (position == input.size() &&
                choice >= minimum &&
                choice <= maximum)
            {
                return choice;
            }
        }
        catch (...)
        {
        }

        std::cout << "Invalid selection. Enter a number from "
                  << minimum << " to " << maximum << ": ";
    }
}

// ============================================================
// KEPLER PHYSICS ENGINE
// ============================================================
void Run_Kepler_Physics_Engine()
{
    Keplers_Physics_Engine KPE;

    KPE.Run_Experiments();
    KPE.Run_All_Statistics();
    KPE.Print_Kepler_Physics_Engine_Calculations();
}

// ============================================================
// SHAPIRO TIME DELAY
// ============================================================
void Run_Shapiro_Time_Delay()
{
    Shapiro_Time_Delay STD;

    STD.Calculate_Extra_Time_Delay(
        1.989e30,
        1.496e11,
        2.279e11,
        2.78e9);

    STD.Print_Calculate_Extra_Time_Delay();
}

// ============================================================
// GEOMETRY RESULT
// ============================================================
struct Geometry_Result
{
    double range_magnitude{};
    double elevation{};
};

// ============================================================
// SPACECRAFT GEOMETRY
// ============================================================
Geometry_Result Run_Spacecraft_Geometry()
{
    Spacecraft_Geometry Geometry;

    double longitude = -1.407;
    double latitude  = 0.489;
    double altitude  = 10.0;

    Eigen::Vector3d ground_station =
        Geometry.Ground_Station_Positions_long_lat_alt(
            longitude,
            latitude,
            altitude);

    Eigen::Vector3d spacecraft_eci =
        Geometry.Spacecraft_Position_x_y_z(
            1200000.0,
            -7200000.0,
            3900000.0);

    double gmst_at_epoch = 0.0;
    double elapsed_seconds = 1000.0;

    double gmst =
        Geometry.Calculate_Greenwich_Sidereal_Time(
            gmst_at_epoch,
            elapsed_seconds);

    Eigen::Vector3d spacecraft_ecef =
        Geometry.Convert_ECI_To_ECEF(
            spacecraft_eci,
            gmst);

    Eigen::Vector3d range_vector =
        Geometry.Calculate_Range_Vector(
            spacecraft_ecef,
            ground_station);

    double range_magnitude =
        Geometry.Calculate_Range_Magnitude(
            range_vector);

    Geometry.Calculate_Line_Of_Sight(range_vector);

    Eigen::Vector3d sez =
        Geometry.Calculate_SEZ_Components(
            range_vector,
            latitude,
            longitude);

    double elevation =
        Geometry.Elevation_Angle(
            sez.z(),
            range_magnitude);

    Geometry.Azimuth_Angle(
        sez.y(),
        sez.x());

    Geometry.Print_Spacecraft_Geometry();

    return {range_magnitude, elevation};
}

// ============================================================
// SPACE COMMUNICATION LINK
// ============================================================
void Run_Space_Communication_Link(
    const Geometry_Result& geometry_result)
{
    if (geometry_result.elevation > 0.0)
    {
        Communication_Space_Link Link;

        Link.SetSystemParameters(
            geometry_result.range_magnitude,
            2.4e9);

        Link.CalculateFreeSpacePathLoss();
        Link.CalculateEarthAntennaGain(0.60, 10.0);
        Link.CalculateFriisTransmissionEquation(20.0, 4.0);
        Link.ThermalNoisePower(290.0, 5.0e6);
        Link.Calculate_Speed_of_Light_Delay();
        Link.CalculateMaximumDataRate(290.0, 5.0e6);
        Link.CalculateBitErrorRate(35.74e6);
        Link.PrintCalculation();
    }
    else
    {
        std::cout
            << "\nSpacecraft below horizon - communication link unavailable.\n";
    }
}

// ============================================================
// INTERSTELLAR COMMUNICATIONS NETWORK
// ============================================================
double Run_Interstellar_Communications_Network()
{
    Interstellar_Communications_Network space_network;

    double earth_to_mars_distance =
        space_network.Generate_Earth_To_Mars_Distance();

    space_network.Add_Network_ID(NODE_0, 1000000.0);
    space_network.Add_Network_ID(NODE_EARTH, 0.0);
    space_network.Add_Network_ID(NODE_MOON, 384400000.0);
    space_network.Add_Network_ID(NODE_MARS, earth_to_mars_distance);

    space_network.Network_ID_Assignment();

    std::cout << "Earth-to-Mars distance: "
              << earth_to_mars_distance
              << " meters\n";

    return earth_to_mars_distance;
}

// ============================================================
// OPTICAL COMMUNICATIONS
// ============================================================
Optical_Communications Run_Optical_Communications()
{
    Optical_Communications OC;

    double eta_diffraction_efficiency =
        OC.Calcualte_Diffraction_Squence(
            0.05,
            0.20,
            1.93e14,
            384400000.0);

    OC.Free_Space_Optical_Link(
        0.06,
        0.50,
        eta_diffraction_efficiency,
        0.75);

    OC.Calculate_Photon_Information_Efficiency(5.0);
    OC.Calculate_Average_Signal_Photons_Per_Slot(1.0e6);
    OC.Get_Photon_Flux();

    OC.Calculate_Average_Background_Noise_Photons_Per_Slot(
        1.0,
        1.0e-20);

    OC.Calculate_Power_Spectral_Density_in_Photon_Energy_Units();
    OC.Calculate_S1_Information_Rate();
    OC.Calculate_S2_Information_Rate();
    OC.Calculate_PPM_Frame_Photons(16.0);
    OC.Calculate_PPM_Photon_Information_Efficiency();
    OC.Calculate_S1_PIE();
    OC.Calculate_S2_PIE();
    OC.Calculate_Gordon_Holevo_Rate();
    OC.Calculate_Gordon_Holevo_PIE();
    OC.Print_Calculations();

    return OC;
}

// ============================================================
// CONTACT WINDOW
// ============================================================
void Run_Contact_Window()
{
    Spacecraft_Geometry geometry;
    Contact_Window contact;

    Eigen::Vector3d ground_station =
        geometry.Ground_Station_Positions_long_lat_alt(
            0.0,
            0.0,
            0.0);

    Eigen::Vector3d spacecraft =
        geometry.Spacecraft_Position_x_y_z(
            6878137.0,
            0.0,
            0.0);

    Eigen::Vector3d range_vector =
        geometry.Calculate_Range_Vector(
            spacecraft,
            ground_station);

    double range =
        geometry.Calculate_Range_Magnitude(
            range_vector);

    Eigen::Vector3d sez =
        geometry.Calculate_SEZ_Components(
            range_vector,
            0.0,
            0.0);

    double elevation =
        geometry.Elevation_Angle(
            sez.z(),
            range);

    bool contact_available =
        contact.Is_Contact_Available(
            elevation,
            10.0);

    double contact_duration = 0.0;

    if (contact_available)
    {
        contact_duration =
            contact.Calculate_Contact_Window_Duration(
                0.0,
                100.0);
    }

    std::cout << "\n========== CONTACT WINDOW ==========\n";
    std::cout << "Range: " << range << " meters\n";
    std::cout << "Elevation: " << elevation << " degrees\n";
    std::cout << "Contact Available: "
              << (contact_available ? "YES" : "NO")
              << '\n';
    std::cout << "Contact Window Duration: "
              << contact_duration
              << " seconds\n";
}

// ============================================================
// PAT TIMING
// ============================================================
void Run_PAT_Timing()
{
    PAT_Timing pat;

    double total_pat =
        pat.Calculate_Total_PAT_Time(
            10.0,
            5.0,
            5.0);

    double usable_contact_time =
        pat.Calculate_Usable_Contact_Time(
            100.0);

    double efficiency =
        pat.Calculate_Contact_Efficiency();

    double overhead =
        pat.Calculate_PAT_Overhead_Percentage();

    std::cout << "\n========== PAT TIMING ==========\n";
    std::cout << "Total PAT Time: "
              << total_pat << " seconds\n";
    std::cout << "Usable Contact Time: "
              << usable_contact_time << " seconds\n";
    std::cout << "Contact Efficiency: "
              << efficiency * 100.0 << "%\n";
    std::cout << "PAT Overhead: "
              << overhead << "%\n";
}

// ============================================================
// SIGNAL ANALYZER
// ============================================================
void Run_Signal_Analyzer()
{
    Space_Data_Capture DC;

    double dynamic_relative_velocity =
        DC.Calculate_Dynamic_Relative_Velocity(
            3.9,
            36.2,
            12000.0);

    double doppler_factor =
        DC.Calculate_First_Order_Doppler_Factor(
            dynamic_relative_velocity);

    double received_signal =
        DC.Calculate_Doppler_Shifted_Frequency(
            44100.0,
            doppler_factor);

    double relativistic_doppler_factor =
        DC.Relativistic_Doppler_Factor(
            dynamic_relative_velocity,
            0.0);

    double relative_velocity_of_satellite =
        DC.Calculate_Radial_Velocity(
            7700.0,
            1200.0,
            500000.0);

    std::cout << std::setprecision(12);

    std::cout << "\n========== SIGNAL ANALYZER ==========\n";
    std::cout << "Dynamic relative velocity: "
              << dynamic_relative_velocity << " m/s\n";
    std::cout << "First-order Doppler factor: "
              << doppler_factor << '\n';
    std::cout << "Received signal: "
              << received_signal << " Hz\n";
    std::cout << "Relativistic Doppler factor: "
              << relativistic_doppler_factor << '\n';
    std::cout << "Relative velocity of satellite: "
              << relative_velocity_of_satellite << " m/s\n";
}

// ============================================================
// SECURE MESSAGE RESULT
// ============================================================
struct Secure_Message_Result
{
    Secure_Message_Encryption secure_message;
    std::vector<std::uint8_t> key;
    std::vector<std::uint8_t> decrypted_plaintext;
    Message_Packet packet;
    bool route_successful{false};
};

// ============================================================
// SECURE MESSAGE TRANSMISSION
// ============================================================
Secure_Message_Result Run_Secure_Message_Transmission(
    Optical_Communications& OC)
{
    Secure_Message_Result result;

    std::string message;

    std::cout << "Enter message for transmission: ";
    std::getline(std::cin, message);

    if (message.empty())
    {
        std::cout << "Message cannot be empty.\n";
        return result;
    }

    std::vector<std::uint8_t> plaintext(
        message.begin(),
        message.end());

    result.key.resize(32);

    std::vector<std::uint8_t> iv(12);
    std::vector<std::uint8_t> ciphertext;
    std::vector<std::uint8_t> tag;

    if (RAND_bytes(
            result.key.data(),
            static_cast<int>(result.key.size())) != 1 ||
        RAND_bytes(
            iv.data(),
            static_cast<int>(iv.size())) != 1)
    {
        std::cerr << "Failed to generate key or IV.\n";
        return result;
    }

    result.secure_message.Encrypt(
        plaintext,
        result.key,
        iv,
        ciphertext,
        tag);

    result.packet.source = NODE_EARTH;
    result.packet.destination = NODE_MARS;
    result.packet.ciphertext = ciphertext;
    result.packet.iv = iv;
    result.packet.tag = tag;

    std::size_t packet_bits =
        OC.Calculate_Packet_Bits(
            result.packet.ciphertext.size());

    double packet_transmission_time =
        OC.Calculate_Packet_Transmission_Time();

    double received_photons_per_packet =
        OC.Calculate_Received_Photons_Per_Packet();

    std::cout << "Encrypted packet bits: "
              << packet_bits << '\n';
    std::cout << "Packet transmission time: "
              << packet_transmission_time << " seconds\n";
    std::cout << "Received photons per packet: "
              << received_photons_per_packet << '\n';

    Interstellar_Relay_Router relay_router;

    result.route_successful =
        relay_router.Route_Packet(
            result.packet);

    if (!result.route_successful)
    {
        std::cout << "Packet routing failed.\n";
    }

    return result;
}

// ============================================================
// INTERSTELLAR MESSAGE HUB
// ============================================================
bool Run_Interstellar_Message_Hub(
    Secure_Message_Result& secure_result)
{
    Interstellar_Message_Hub message_hub;

    bool message_sent =
        message_hub.Send_Message(
            0,
            secure_result.packet);

    if (!message_sent)
    {
        std::cout << "Encrypted packet was not sent.\n";
        return false;
    }

    bool decrypted =
        secure_result.secure_message.Decrypt(
            secure_result.packet.ciphertext,
            secure_result.key,
            secure_result.packet.iv,
            secure_result.packet.tag,
            secure_result.decrypted_plaintext);

    if (!decrypted)
    {
        std::cout << "Message authentication failed.\n";
        return false;
    }

    std::string recovered_message(
        secure_result.decrypted_plaintext.begin(),
        secure_result.decrypted_plaintext.end());

    std::cout << "Decrypted message: "
              << recovered_message << '\n';

    return true;
}

// ============================================================
// RELAY STATION SIMULATION
// ============================================================
bool Run_Relay_Station_Simulation(
    double earth_to_mars_distance,
    const Message_Packet& packet)
{
    constexpr double maximum_optical_hop_distance = 7.58e8;

    int total_optical_hops =
        static_cast<int>(
            std::ceil(
                earth_to_mars_distance /
                maximum_optical_hop_distance));

    if (total_optical_hops < 1)
    {
        total_optical_hops = 1;
    }

    int number_of_relay_nodes =
        total_optical_hops - 1;

    double distance_per_hop =
        earth_to_mars_distance /
        static_cast<double>(total_optical_hops);

    Relay_Station_Simulation simulation;
    simulation.Create_Nodes(number_of_relay_nodes);

    bool all_relays_successful = true;

    constexpr double minimum_received_power = 1.0e-12;
    constexpr double minimum_information_rate = 1.0e6;
    constexpr double minimum_photons_per_packet = 1.0;

    std::cout << "\n===== AUTOMATIC RELAY CALCULATION =====\n";
    std::cout << "Earth-to-Mars distance: "
              << earth_to_mars_distance << " meters\n";
    std::cout << "Maximum optical hop distance: "
              << maximum_optical_hop_distance << " meters\n";
    std::cout << "Required optical hops: "
              << total_optical_hops << '\n';
    std::cout << "Required relay nodes: "
              << number_of_relay_nodes << '\n';
    std::cout << "Actual distance per hop: "
              << distance_per_hop << " meters\n";

    int first_relay_node_id = 3;
    int last_relay_node_id =
        first_relay_node_id + number_of_relay_nodes - 1;

    for (int node_id = first_relay_node_id;
         node_id <= last_relay_node_id;
         ++node_id)
    {
        std::cout << "\nProcessing relay node "
                  << node_id << '\n';

        Optical_Communications hop_optical;

        double hop_diffraction_efficiency =
            hop_optical.Calcualte_Diffraction_Squence(
                0.05,
                0.20,
                1.93e14,
                distance_per_hop);

        double hop_received_power =
            hop_optical.Free_Space_Optical_Link(
                0.06,
                0.50,
                hop_diffraction_efficiency,
                0.75);

        double hop_information_rate =
            hop_optical.Calculate_Photon_Information_Efficiency(5.0);

        double hop_signal_photons_per_slot =
            hop_optical.Calculate_Average_Signal_Photons_Per_Slot(1.0e6);

        double hop_photon_flux =
            hop_optical.Get_Photon_Flux();

        hop_optical.Calculate_Packet_Bits(
            packet.ciphertext.size());

        double hop_transmission_time =
            hop_optical.Calculate_Packet_Transmission_Time();

        double hop_received_photons_per_packet =
            hop_optical.Calculate_Received_Photons_Per_Packet();

        std::cout << "Hop distance: "
                  << distance_per_hop << " meters\n";
        std::cout << "Hop diffraction efficiency: "
                  << hop_diffraction_efficiency << '\n';
        std::cout << "Hop received power: "
                  << hop_received_power << " watts\n";
        std::cout << "Required received power: "
                  << minimum_received_power << " watts\n";
        std::cout << "Hop photon flux: "
                  << hop_photon_flux << " photons/second\n";
        std::cout << "Hop information rate: "
                  << hop_information_rate << " bps\n";
        std::cout << "Required information rate: "
                  << minimum_information_rate << " bps\n";
        std::cout << "Hop transmission time: "
                  << hop_transmission_time << " seconds\n";
        std::cout << "Hop photons per packet: "
                  << hop_received_photons_per_packet << '\n';
        std::cout << "Required photons per packet: "
                  << minimum_photons_per_packet << '\n';

        bool relay_link_success =
            simulation.Process_Optical_Link(
                node_id,
                hop_received_power,
                hop_photon_flux,
                hop_information_rate,
                hop_signal_photons_per_slot,
                hop_received_photons_per_packet,
                minimum_received_power,
                minimum_information_rate,
                minimum_photons_per_packet);

        if (!relay_link_success)
        {
            std::cout << "Transmission stopped at relay "
                      << node_id << '\n';

            all_relays_successful = false;
            break;
        }
    }

    if (!all_relays_successful)
    {
        std::cout << "\n===== FAILED ROUTE SUMMARY =====\n";
        simulation.Print_Relay_Summary();
        std::cout << "The packet did not reach Mars.\n";
        return false;
    }

    std::cout << "\n===== SUCCESSFUL ROUTE SUMMARY =====\n";
    std::cout << "All relay nodes completed transmission.\n";
    simulation.Print_Relay_Summary();

    return true;
}

// ============================================================
// RELAY TOPOLOGY
// ============================================================
void Run_Relay_Topology()
{
    Relay_Topology_Space topology;

    const std::size_t relay_count = 3;
    const double total_distance = 1000.0;

    std::size_t hop_count =
        topology.Calculate_Hop_Count(relay_count);

    double equal_spacing =
        topology.Calculate_Equal_Relay_Spacing(
            total_distance,
            relay_count);

    std::vector<double> hop_reliabilities =
    {
        0.99,
        0.98,
        0.995,
        0.99
    };

    double route_reliability =
        topology.Calculate_Route_Reliability(
            hop_reliabilities);

    double average_reliability =
        topology.Calculate_Average_Hop_Reliability(
            hop_reliabilities);

    double weakest_reliability =
        topology.Calculate_Weakest_Hop_Reliability(
            hop_reliabilities);

    std::cout << "\n========== RELAY TOPOLOGY ==========\n";
    std::cout << "Relay Count: " << relay_count << '\n';
    std::cout << "Hop Count: " << hop_count << '\n';
    std::cout << "Total Route Distance: "
              << total_distance << " meters\n";
    std::cout << "Equal Relay Spacing: "
              << equal_spacing << " meters\n";
    std::cout << "Route Reliability: "
              << route_reliability << '\n';
    std::cout << "Average Hop Reliability: "
              << average_reliability << '\n';
    std::cout << "Weakest Hop Reliability: "
              << weakest_reliability << '\n';
}

// ============================================================
// RELAY ROUTING
// ============================================================
void Run_Relay_Routing()
{
    Interstellar_Relay_Router router;

    Message_Packet packet;
    packet.source = NODE_EARTH;
    packet.destination = NODE_MARS;

    Route_Option route_1;
    route_1.Nodes = {NODE_EARTH, NODE_MOON, NODE_MARS};
    route_1.Route_Cost = 10.0;
    route_1.Available = true;

    Route_Option route_2;
    route_2.Nodes = {NODE_EARTH, NODE_1, NODE_MARS};
    route_2.Route_Cost = 7.0;
    route_2.Available = true;

    Route_Option route_3;
    route_3.Nodes = {NODE_EARTH, NODE_2, NODE_MARS};
    route_3.Route_Cost = 5.0;
    route_3.Available = false;

    std::vector<Route_Option> routes =
    {
        route_1,
        route_2,
        route_3
    };

    std::cout << "\n========== RELAY ROUTING ==========\n";

    bool routed =
        router.Route_Packet(
            packet,
            routes);

    std::cout << "Routing Result: "
              << (routed ? "SUCCESS" : "FAILURE")
              << '\n';
}

// ============================================================
// ENCRYPTED MESSAGE WORKFLOW
// ============================================================
void Run_Encrypted_Message_Workflow()
{
    Optical_Communications OC =
        Run_Optical_Communications();

    Secure_Message_Result secure_result =
        Run_Secure_Message_Transmission(OC);

    if (!secure_result.route_successful)
    {
        std::cout << "Encrypted message workflow stopped: routing failed.\n";
        return;
    }

    if (!Run_Interstellar_Message_Hub(secure_result))
    {
        std::cout << "Encrypted message workflow failed.\n";
        return;
    }

    std::cout << "Encrypted message workflow completed successfully.\n";
}

// ============================================================
// RELAY STATION MENU DEMO
// ============================================================
void Run_Relay_Station_Menu_Demo()
{
    double earth_to_mars_distance =
        Run_Interstellar_Communications_Network();

    Message_Packet packet;
    packet.source = NODE_EARTH;
    packet.destination = NODE_MARS;
    packet.ciphertext.assign(128, 0x01);

    bool success =
        Run_Relay_Station_Simulation(
            earth_to_mars_distance,
            packet);

    std::cout << "\nRelay Station Result: "
              << (success ? "SUCCESS" : "FAILURE")
              << '\n';
}

// ============================================================
// MONTE CARLO EXPERIMENT LAUNCHERS
// ============================================================
void Run_Monte_Carlo_Experiment_1()
{
    std::cout << "\nLaunching Monte Carlo Experiment 1...\n";

    int result =
        std::system("./Builds/monte_carlo_experiment_1");

    if (result != 0)
    {
        std::cout
            << "Monte Carlo Experiment 1 executable was not found or failed.\n"
            << "Compile it first with:\n\n"
            << "g++ -std=c++17 "
            << "Monte_Carlo_Experiments/Monte_Carlo_Experiment_1.cpp "
            << "Relay_Topology_Space.cpp Optical_Communication.cpp "
            << "Statistical_Data.cpp -o Builds/monte_carlo_experiment_1\n";
    }
}

void Display_Not_Yet_Implemented(const std::string& item)
{
    std::cout << "\n" << item
              << " is not implemented yet.\n";
}

// ============================================================
// RESEARCH RESULTS
// ============================================================
void Display_Experiment_Results()
{
    std::ifstream results_file(
        "Results/Monte_Carlo_Experiment_1_Summary.csv");

    if (!results_file)
    {
        std::cout
            << "\nExperiment 1 summary file not found.\n"
            << "Run Monte Carlo Experiment 1 first.\n";
        return;
    }

    std::cout << "\n===== MONTE CARLO EXPERIMENT 1 SUMMARY =====\n";

    std::string line;

    while (std::getline(results_file, line))
    {
        std::cout << line << '\n';
    }
}

void Display_Statistical_Analysis_Status()
{
    std::cout << "\n===== STATISTICAL ANALYSIS STATUS =====\n";
    std::cout << "Statistical_Data module: validated and available.\n";
    std::cout << "Experiment 1 produces summary/trial CSV output.\n";
    std::cout << "Final cross-experiment statistical campaign: not yet complete.\n";
}

void Display_Hypothesis_Testing_Status()
{
    std::cout << "\n===== HYPOTHESIS TESTING STATUS =====\n";
    std::cout << "Final research hypotheses and full experiment comparisons\n";
    std::cout << "will be performed after Experiments 2-4 are implemented.\n";
}

// ============================================================
// PHYSICS MENU CONTROLLER
// ============================================================
void Run_Physics_Menu(
    Interstellar_Communications_Simulator_Display& Sims)
{
    bool submenu_running = true;

    while (submenu_running)
    {
        Sims.Display_Physics_Menu();
        std::cout << "Please Enter Your Selection 1-4: ";

        int choice = Read_Menu_Choice(1, 4);

        switch (choice)
        {
            case 1:
                Run_Kepler_Physics_Engine();
                break;

            case 2:
                Run_Spacecraft_Geometry();
                break;

            case 3:
                Run_Shapiro_Time_Delay();
                break;

            case 4:
                submenu_running = false;
                break;
        }
    }
}

// ============================================================
// COMMUNICATIONS MENU CONTROLLER
// ============================================================
void Run_Communications_Menu(
    Interstellar_Communications_Simulator_Display& Sims)
{
    bool submenu_running = true;

    while (submenu_running)
    {
        Sims.Display_Communications_Menu();
        std::cout << "Please Enter Your Selection 1-6: ";

        int choice = Read_Menu_Choice(1, 6);

        switch (choice)
        {
            case 1:
                Run_Signal_Analyzer();
                break;

            case 2:
                Run_Optical_Communications();
                break;

            case 3:
            {
                Geometry_Result geometry_result =
                    Run_Spacecraft_Geometry();

                Run_Space_Communication_Link(
                    geometry_result);
                break;
            }

            case 4:
                Run_Contact_Window();
                break;

            case 5:
                Run_PAT_Timing();
                break;

            case 6:
                submenu_running = false;
                break;
        }
    }
}

// ============================================================
// NETWORK MENU CONTROLLER
// ============================================================
void Run_Network_Menu(
    Interstellar_Communications_Simulator_Display& Sims)
{
    bool submenu_running = true;

    while (submenu_running)
    {
        Sims.Display_Network_Menu();
        std::cout << "Please Enter Your Selection 1-6: ";

        int choice = Read_Menu_Choice(1, 6);

        switch (choice)
        {
            case 1:
                Run_Interstellar_Communications_Network();
                break;

            case 2:
                Run_Encrypted_Message_Workflow();
                break;

            case 3:
                Run_Relay_Station_Menu_Demo();
                break;

            case 4:
                Run_Relay_Topology();
                break;

            case 5:
                Run_Relay_Routing();
                break;

            case 6:
                submenu_running = false;
                break;
        }
    }
}

// ============================================================
// MONTE CARLO MENU CONTROLLER
// ============================================================
void Run_Monte_Carlo_Menu(
    Interstellar_Communications_Simulator_Display& Sims)
{
    bool submenu_running = true;

    while (submenu_running)
    {
        Sims.Display_Monte_Carlo_Menu();
        std::cout << "Please Enter Your Selection 1-5: ";

        int choice = Read_Menu_Choice(1, 5);

        switch (choice)
        {
            case 1:
                Run_Monte_Carlo_Experiment_1();
                break;

            case 2:
                Display_Not_Yet_Implemented(
                    "Monte Carlo Experiment 2 - Dynamic Orbital Geometry");
                break;

            case 3:
                Display_Not_Yet_Implemented(
                    "Monte Carlo Experiment 3 - Relay Placement Optimization");
                break;

            case 4:
                Display_Not_Yet_Implemented(
                    "Monte Carlo Experiment 4 - Autonomous Network Resilience");
                break;

            case 5:
                submenu_running = false;
                break;
        }
    }
}

// ============================================================
// RESEARCH MENU CONTROLLER
// ============================================================
void Run_Research_Menu(
    Interstellar_Communications_Simulator_Display& Sims)
{
    bool submenu_running = true;

    while (submenu_running)
    {
        Sims.Display_Research_Menu();
        std::cout << "Please Enter Your Selection 1-4: ";

        int choice = Read_Menu_Choice(1, 4);

        switch (choice)
        {
            case 1:
                Display_Experiment_Results();
                break;

            case 2:
                Display_Statistical_Analysis_Status();
                break;

            case 3:
                Display_Hypothesis_Testing_Status();
                break;

            case 4:
                submenu_running = false;
                break;
        }
    }
}

// ============================================================
// MAIN
// ============================================================
int main()
{
    Interstellar_Communications_Simulator_Display Sims;

    bool running = true;

    while (running)
    {
        Sims.Display_Main_Menu();
        std::cout << "Please Enter Your Selection 1-6: ";

        int choice = Read_Menu_Choice(1, 6);

        switch (choice)
        {
            case 1:
                Run_Physics_Menu(Sims);
                break;

            case 2:
                Run_Communications_Menu(Sims);
                break;

            case 3:
                Run_Network_Menu(Sims);
                break;

            case 4:
                Run_Monte_Carlo_Menu(Sims);
                break;

            case 5:
                Run_Research_Menu(Sims);
                break;

            case 6:
                Sims.Display_Menu(Simulator_Display::Exit);
                running = false;
                break;
        }
    }

    return 0;
}
