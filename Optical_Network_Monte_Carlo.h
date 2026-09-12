#ifndef OPTICAL_NETWORK_MONTE_CARLO_H
#define OPTICAL_NETWORK_MONTE_CARLO_H

#include <vector>
#include <random>
#include <cstddef>

// Validated modules
#include "Kepler_Physics_Engine.h"
#include "Spacecraft_Geometry.h"
#include "Signal_Analyzer.h"
#include "Optical_Communication.h"
#include "Relay_Router.h"
#include "Relay_Station_Simulation.h"
#include "Statistical_Data.h"

enum class Experiment_Type
{
    Relay_Spacing,
    Background_Noise,
    Transmit_Power,
    Data_Rate,
    Relative_Velocity,
    Pointing_Error,

    Spacing_Noise,
    Spacing_Power,
    Spacing_Data_Rate,
    Spacing_Velocity,
    Spacing_Pointing_Error,

    Full_Network
};

struct Optical_Network_Simulation_Result
{
    std::size_t Trial_Number{};

    // Experiment inputs
    int Relay_Count{};
    double Total_Route_Distance{};
    double Relay_Spacing{};
    double Transmit_Power{};
    double Background_Noise{};
    double Data_Rate{};
    double Relative_Velocity{};
    double Pointing_Error{};

    // Physical/link outputs
    double Received_Power{};
    double Photon_Flux{};
    double Signal_To_Noise_Ratio{};
    double Information_Rate{};

    // Network outputs
    int Packets_Transmitted{};
    int Packets_Delivered{};
    int Corrupted_Packets{};
    int Failed_Hops{};

    double Packet_Delivery_Percentage{};
    double Corrupted_Packet_Percentage{};
    double Throughput{};
    double End_To_End_Delay{};

    bool Route_Successful{};
};

class Optical_Network_Monte_Carlo
{
private:

    // Random number generator
    std::mt19937 Generator;

    // Experiment settings
    std::size_t Number_Of_Trials{};
    Experiment_Type Current_Experiment{};

    // Baseline/control values
    double Total_Route_Distance{};
    double Transmit_Power{};
    double Background_Noise{};
    double Data_Rate{};
    double Relative_Velocity{};
    double Pointing_Error{};

    // Reliability requirement
    double Required_Packet_Delivery{99.9};

    // Validated modules
    Kepler_Physics_Engine Kepler;
    Spacecraft_Geometry Geometry;
    Signal_Analyzer Signal;
    Optical_Communications Optical;
    Relay_Router Router;
    Relay_Station_Simulation Relay_Simulation;
    Statistical_Data Statistics;

    // All trial results
    std::vector<Optical_Network_Simulation_Result> Results;

public:

    Optical_Network_Monte_Carlo();

    // -----------------------------
    // Experiment configuration
    // -----------------------------

    void Set_Number_Of_Trials(std::size_t trials);

    void Set_Experiment_Type(
        Experiment_Type experiment);

    void Set_Total_Route_Distance(
        double distance);

    void Set_Transmit_Power(
        double transmit_power);

    void Set_Background_Noise(
        double background_noise);

    void Set_Data_Rate(
        double data_rate);

    void Set_Relative_Velocity(
        double relative_velocity);

    void Set_Pointing_Error(
        double pointing_error);


    // -----------------------------
    // Relay configuration
    // -----------------------------

    double Calculate_Relay_Spacing(
        double total_distance,
        int relay_count);


    // -----------------------------
    // Random-number generation
    // -----------------------------

    double Generate_Random_Value(
        double minimum,
        double maximum);


    // -----------------------------
    // Individual experiments
    // -----------------------------

    void Run_Relay_Spacing_Experiment();

    void Run_Background_Noise_Experiment();

    void Run_Transmit_Power_Experiment();

    void Run_Data_Rate_Experiment();

    void Run_Relative_Velocity_Experiment();

    void Run_Pointing_Error_Experiment();


    // -----------------------------
    // Paired experiments
    // -----------------------------

    void Run_Spacing_Noise_Experiment();

    void Run_Spacing_Power_Experiment();

    void Run_Spacing_Data_Rate_Experiment();

    void Run_Spacing_Velocity_Experiment();

    void Run_Spacing_Pointing_Error_Experiment();


    // -----------------------------
    // Full network experiment
    // -----------------------------

    void Run_Full_Network_Monte_Carlo();


    // -----------------------------
    // Single trial
    // -----------------------------

    Optical_Network_Simulation_Result
    Run_Single_Trial(
        int relay_count,
        double route_distance,
        double transmit_power,
        double background_noise,
        double data_rate,
        double relative_velocity,
        double pointing_error);


    // -----------------------------
    // Reliability evaluation
    // -----------------------------

    bool Determine_Route_Success(
        double packet_delivery_percentage) const;


    // -----------------------------
    // Results
    // -----------------------------

    const std::vector<Optical_Network_Simulation_Result>&
    Get_Results() const;

    void Analyze_Results();

    void Print_Results() const;
};

#endif

/*
std::size_t Number_Of_Trials{};

double Minimum_Relay_Spacing{};
double Maximum_Relay_Spacing{};

double Minimum_Background_Noise{};
double Maximum_Background_Noise{};

double Minimum_Transmit_Power{};
double Maximum_Transmit_Power{};

double Minimum_Data_Rate{};
double Maximum_Data_Rate{};

double Minimum_Relative_Velocity{};
double Maximum_Relative_Velocity{};

double Minimum_Pointing_Error{};
double Maximum_Pointing_Error{};

double Required_Packet_Delivery{99.9};


*/


/*
void Optical_Network_Monte_Carlo::
Run_Background_Noise_Experiment()
{
    for (std::size_t Trial = 0;
         Trial < Number_Of_Trials;
         ++Trial)
    {
        double Noise =
            Generate_Random_Value(
                Minimum_Background_Noise,
                Maximum_Background_Noise);

        Run_Single_Trial(
            Relay_Count,
            Total_Route_Distance,
            Transmit_Power,
            Noise,
            Data_Rate,
            Relative_Velocity,
            Pointing_Error);
    }
}


for (int Relay_Count : Relay_Counts)
{
    double Relay_Spacing =
        Calculate_Relay_Spacing(
            Total_Route_Distance,
            Relay_Count);

    for (std::size_t Trial = 0;
         Trial < Number_Of_Trials;
         ++Trial)
    {
        Run_Single_Trial(
            Relay_Count,
            Total_Route_Distance,
            Transmit_Power,
            Background_Noise,
            Data_Rate,
            Relative_Velocity,
            Pointing_Error);
    }
}

struct Experiment_Config
{
    std::size_t Number_Of_Trials{};

    double Variable_A_Minimum{};
    double Variable_A_Maximum{};

    double Variable_B_Minimum{};
    double Variable_B_Maximum{};

    double Required_Packet_Delivery{};
};

Experiment_Config Config =
    Experiment_Configurations.at(
        Experiment_Type::Background_Noise);

Config.Minimum_Value
Config.Maximum_Value
Config.Number_Of_Trials

#include <unordered_map>

enum class Experiment_Type
{
    Relay_Spacing,
    Background_Noise,
    Transmit_Power,
    Data_Rate,
    Relative_Velocity,
    Pointing_Error
};

struct Experiment_Config
{
    std::size_t Number_Of_Trials{};

    double Minimum_Value{};
    double Maximum_Value{};

    double Required_Packet_Delivery{};
};

struct Experiment_Config
{
    std::size_t Number_Of_Trials{};

    double Minimum_Value{};
    double Maximum_Value{};

    double Required_Packet_Delivery{};
};

std::unordered_map<Experiment_Type, Experiment_Config>
    Experiment_Configurations;


*/