#include "../Relay_Topology_Space.h"
#include "../Optical_Communication.h"
#include "../Statistical_Data.h"

#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <iomanip>
#include <limits>
#include <cstddef>
#include <filesystem>
#include <string>

int main()
{
    // ============================================================
    // MONTE CARLO EXPERIMENT 1
    //
    // Relay Count
    // Equal Relay Spacing
    // Optical Link Performance
    // Reliability
    // Throughput
    // Delay
    // Statistical Analysis
    // ============================================================

    std::cout << "\n====================================================\n";

    std::cout<< "MONTE CARLO EXPERIMENT 1\n";

    std::cout<< "RELAY COUNT AND OPTICAL LINK PERFORMANCE\n";

    std::cout << "====================================================\n\n";


    // ============================================================
    // OBJECTS
    // ============================================================

    Relay_Topology_Space Topology;

    Statistical_Data Statistics;


    // ============================================================
    // EXPERIMENT SETTINGS
    // ============================================================

    // Start with 100 for debugging.
    // Later:
    // 1,000
    // 10,000
    // 60,000
    const int Number_Of_Trials = 100;


    // Relay configurations to test.
    const std::vector<std::size_t> Relay_Counts =
    {
        0,
        1,
        2,
        4,
        6,
        8,
        10,
        15
    };


    // ============================================================
    // CONTROLLED TOTAL ROUTE DISTANCE
    //
    // Initial controlled deterministic distance.
    // This is NOT yet dynamic Earth-Mars orbital geometry.
    // ============================================================

    const double Total_Route_Distance = 58695444.9586;     // meters


    // ============================================================
    // OPTICAL PARAMETERS
    // ============================================================

    const double Speed_Of_Light = 2.998e8;           // meters / second


    const double Wavelength = 1.55e-6;           // meters


    const double Optical_Frequency = Speed_Of_Light / Wavelength;


    const double Receiver_Diameter = 0.20;              // meters


    const double Transmitter_Diameter = 0.20;              // meters


    const double Photon_Information_Efficiency = 5.0;               // bits / photon


    const std::size_t Packet_Bytes = 1024;


    // ============================================================
    // INITIAL FAILURE THRESHOLDS
    //
    // These are debugging / experimental starting values.
    // They are NOT final validated physical thresholds.
    // ============================================================

    const double Minimum_Received_Power = 5.0e-7;            // watts


    const double Minimum_Information_Rate = 1.0e13;            // bits / second


    const double Minimum_Photons_Per_Packet = 800.0;


    // ============================================================
    // RELAY AVAILABILITY
    //
    // Each independent relay has a probability of being available
    // during one Monte Carlo trial.
    // ============================================================

    const double Relay_Online_Probability = 0.98;


    // ============================================================
    // PROCESSING DELAY
    //
    // Temporary model:
    // every intermediate relay adds processing delay.
    // ============================================================

    const double Processing_Delay_Per_Relay = 0.005;             // seconds


    // ============================================================
    // RANDOM NUMBER GENERATOR
    // ============================================================

    const unsigned int Random_Seed = 12345;

    std::mt19937 generator(Random_Seed);


    // ============================================================
    // RANDOM TRANSMIT POWER
    //
    // Initial debugging range:
    // 8 W - 12 W
    // ============================================================

    std::uniform_real_distribution<double> Transmit_Power_Distribution( 8.0, 12.0);


    // ============================================================
    // RANDOM ATMOSPHERIC / PROPAGATION EFFICIENCY
    // ============================================================

    std::uniform_real_distribution<double> Atmosphere_Efficiency_Distribution( 0.85, 0.95);


    // ============================================================
    // RANDOM RECEIVER EFFICIENCY
    // ============================================================

    std::uniform_real_distribution<double> Receiver_Efficiency_Distribution(0.75, 0.85);


    // ============================================================
    // RANDOM RELAY AVAILABILITY
    // ============================================================

    std::bernoulli_distribution Relay_Online_Distribution( Relay_Online_Probability);


    // ============================================================
    // RESULTS FOLDER
    // ============================================================

    const std::string Results_Folder = "Results/Monte_Carlo_Experiment_1";

    std::filesystem::create_directories(Results_Folder);


    // ============================================================
    // CSV FILES
    // ============================================================

    std::ofstream Trial_File(Results_Folder + "/Monte_Carlo_Experiment_1_Trials.csv");


    std::ofstream Summary_File( Results_Folder + "/Monte_Carlo_Experiment_1_Summary.csv");


    if (!Trial_File.is_open() || !Summary_File.is_open())
    {
        std::cerr << "ERROR: Could not create Monte Carlo CSV files.\n";

        return 1;
    }


    // ============================================================
    // TRIAL CSV HEADER
    // ============================================================

    Trial_File
        << "Relay_Count,"
        << "Trial,"
        << "Hop_Count,"
        << "Distance_Per_Hop_m,"
        << "Transmit_Power_W,"
        << "Atmosphere_Efficiency,"
        << "Receiver_Efficiency,"
        << "Bottleneck_Rate_bps,"
        << "Weakest_Received_Power_W,"
        << "Minimum_Photons_Per_Packet,"
        << "Propagation_Delay_s,"
        << "Transmission_Delay_s,"
        << "Processing_Delay_s,"
        << "Total_Delay_s,"
        << "Packet_Delivered\n";


    // ============================================================
    // SUMMARY CSV HEADER
    // ============================================================

    Summary_File
        << "Relay_Count,"
        << "Hop_Count,"
        << "Distance_Per_Hop_m,"
        << "Trials,"
        << "Successful_Trials,"
        << "Packet_Delivery_Reliability,"
        << "Mean_Bottleneck_Rate_bps,"
        << "Throughput_StdDev,"
        << "Throughput_Standard_Error,"
        << "Throughput_CI95_Lower,"
        << "Throughput_CI95_Upper,"
        << "Mean_Total_Delay_s,"
        << "Delay_StdDev,"
        << "Delay_CI95_Lower,"
        << "Delay_CI95_Upper,"
        << "Mean_Weakest_Received_Power_W,"
        << "Received_Power_StdDev,"
        << "Mean_Minimum_Photons_Per_Packet,"
        << "Photon_StdDev\n";


    // ============================================================
    // RELAY CONFIGURATION LOOP
    // ============================================================

    for (std::size_t Relay_Count : Relay_Counts)
    {
        // --------------------------------------------------------
        // Calculate topology
        // --------------------------------------------------------

        const std::size_t Hop_Count = Topology.Calculate_Hop_Count(Relay_Count);


        const double Distance_Per_Hop = Topology.Calculate_Equal_Relay_Spacing(Total_Route_Distance, Relay_Count);


        // ========================================================
        // SAMPLE STORAGE
        //
        // Every Monte Carlo result is stored instead of only
        // maintaining running totals.
        // ========================================================

        std::vector<double> Throughput_Samples;


        std::vector<double>  Delay_Samples;


        std::vector<double>  Received_Power_Samples;


        std::vector<double> Photon_Samples;


        std::vector<double> Packet_Delivery_Samples;


        // Reserve memory because we already know sample size.
        Throughput_Samples.reserve(Number_Of_Trials);

        Delay_Samples.reserve(Number_Of_Trials);

        Received_Power_Samples.reserve(Number_Of_Trials);

        Photon_Samples.reserve(Number_Of_Trials);

        Packet_Delivery_Samples.reserve(Number_Of_Trials);


        int Successful_Trials = 0;


        std::cout<< "----------------------------------------------------\n";

        std::cout << "Testing Relay Count: " << Relay_Count << "\n";

        std::cout << "Hop Count: " << Hop_Count << "\n";

        std::cout<< "Distance Per Hop: " << Distance_Per_Hop<< " meters\n";


        // ========================================================
        // MONTE CARLO TRIAL LOOP
        // ========================================================

        for (int Trial = 1; Trial <= Number_Of_Trials; ++Trial)
        {
            // ====================================================
            // RANDOMIZED INPUTS
            // ====================================================

            const double Transmitted_Power = Transmit_Power_Distribution(generator);


            const double Atmosphere_Efficiency = Atmosphere_Efficiency_Distribution(generator);


            const double Receiver_Efficiency = Receiver_Efficiency_Distribution(generator);


            // ====================================================
            // ROUTE-LEVEL VARIABLES
            // ====================================================

            bool Route_Successful = true;


            double Bottleneck_Rate =  std::numeric_limits<double>::max();


            double Weakest_Received_Power = std::numeric_limits<double>::max();


            double Weakest_Photons_Per_Packet = std::numeric_limits<double>::max();


            double Total_Transmission_Delay = 0.0;


            // ====================================================
            // RELAY AVAILABILITY
            //
            // Relay_Count == 0:
            // direct path, no intermediate relay availability.
            // ====================================================

            for (std::size_t Relay_Index = 0; Relay_Index < Relay_Count; ++Relay_Index)
            {
                const bool Relay_Online =  Relay_Online_Distribution(generator);


                if (!Relay_Online)
                {
                    Route_Successful =  false;
                }
            }


            // ====================================================
            // OPTICAL HOP LOOP
            // ====================================================

            for (std::size_t Hop = 0;  Hop < Hop_Count; ++Hop)
            {
                Optical_Communications
                    Optical_Link;


                // =================================================
                // DIFFRACTION EFFICIENCY
                // =================================================

                const double Diffraction_Efficiency =  Optical_Link.Calcualte_Diffraction_Squence(Receiver_Diameter, Transmitter_Diameter, Optical_Frequency, Distance_Per_Hop);


                // =================================================
                // RECEIVED OPTICAL POWER
                // =================================================

                const double Received_Power = Optical_Link.Free_Space_Optical_Link( Transmitted_Power, Atmosphere_Efficiency, Diffraction_Efficiency, Receiver_Efficiency);


                // =================================================
                // PHOTON FLUX + INFORMATION RATE
                //
                // Calculate_Photon_Information_Efficiency()
                // stores photon flux and calculates information rate.
                // =================================================

                const double Information_Rate =  Optical_Link.Calculate_Photon_Information_Efficiency(Photon_Information_Efficiency);


                // =================================================
                // PACKET SIZE
                // =================================================

                Optical_Link.Calculate_Packet_Bits(Packet_Bytes);


                // =================================================
                // PACKET TRANSMISSION TIME
                // =================================================

                const double Packet_Transmission_Time =  Optical_Link.Calculate_Packet_Transmission_Time();


                // =================================================
                // PHOTONS RECEIVED PER PACKET
                // =================================================

                const double Photons_Per_Packet =  Optical_Link.Calculate_Received_Photons_Per_Packet();


                // =================================================
                // BOTTLENECK RATE
                //
                // Serial network throughput is limited by its
                // slowest hop.
                // =================================================

                if (Information_Rate <  Bottleneck_Rate)
                {
                    Bottleneck_Rate =  Information_Rate;
                }


                // =================================================
                // WEAKEST RECEIVED POWER
                // =================================================

                if (Received_Power <  Weakest_Received_Power)
                {
                    Weakest_Received_Power =  Received_Power;
                }


                // =================================================
                // LOWEST PHOTON COUNT
                // =================================================

                if (Photons_Per_Packet < Weakest_Photons_Per_Packet)
                {
                    Weakest_Photons_Per_Packet = Photons_Per_Packet;
                }


                // =================================================
                // TRANSMISSION DELAY
                // =================================================

                Total_Transmission_Delay +=Packet_Transmission_Time;


                // =================================================
                // LINK FAILURE THRESHOLD:
                // RECEIVED POWER
                // =================================================

                if (Received_Power <  Minimum_Received_Power)
                {
                    Route_Successful = false;
                }


                // =================================================
                // LINK FAILURE THRESHOLD:
                // INFORMATION RATE
                // =================================================

                if (Information_Rate < Minimum_Information_Rate)
                {
                    Route_Successful =  false;
                }


                // =================================================
                // LINK FAILURE THRESHOLD:
                // PHOTON COUNT
                // =================================================

                if (Photons_Per_Packet < Minimum_Photons_Per_Packet)
                {
                    Route_Successful =  false;
                }
            }


            // ====================================================
            // PROPAGATION DELAY
            //
            // Adding relays does not reduce pure light-speed delay
            // when total route distance remains the same.
            // ====================================================

            const double Propagation_Delay =  Total_Route_Distance /  Speed_Of_Light;


            // ====================================================
            // RELAY PROCESSING DELAY
            // ====================================================

            const double Processing_Delay = static_cast<double>(Relay_Count) * Processing_Delay_Per_Relay;


            // ====================================================
            // TOTAL NETWORK DELAY
            // ====================================================

            const double Total_Delay =  Propagation_Delay  +  Total_Transmission_Delay   +  Processing_Delay;


            // ====================================================
            // COUNT SUCCESS
            // ====================================================

            if (Route_Successful)
            {
                ++Successful_Trials;
            }


            // ====================================================
            // STORE MONTE CARLO SAMPLES
            // ====================================================

            Throughput_Samples.push_back(Bottleneck_Rate);


            Delay_Samples.push_back(Total_Delay);


            Received_Power_Samples.push_back(Weakest_Received_Power);


            Photon_Samples.push_back(Weakest_Photons_Per_Packet);


            Packet_Delivery_Samples.push_back(Route_Successful ? 1.0 : 0.0);


            // ====================================================
            // WRITE INDIVIDUAL TRIAL
            // ====================================================

            Trial_File
                << Relay_Count
                << ","

                << Trial
                << ","

                << Hop_Count
                << ","

                << Distance_Per_Hop
                << ","

                << Transmitted_Power
                << ","

                << Atmosphere_Efficiency
                << ","

                << Receiver_Efficiency
                << ","

                << Bottleneck_Rate
                << ","

                << Weakest_Received_Power
                << ","

                << Weakest_Photons_Per_Packet
                << ","

                << Propagation_Delay
                << ","

                << Total_Transmission_Delay
                << ","

                << Processing_Delay
                << ","

                << Total_Delay
                << ","

                << (Route_Successful ? 1 : 0)

                << "\n";
        }


        // ========================================================
        // STATISTICAL ANALYSIS
        // ========================================================


        // --------------------------------------------------------
        // PACKET DELIVERY RELIABILITY
        //
        // 1 = successful
        // 0 = failed
        //
        // Mean therefore equals successful fraction.
        // --------------------------------------------------------

        auto Packet_Delivery_Reliability =  Statistics.Calculate_Mean(Packet_Delivery_Samples);


        // --------------------------------------------------------
        // THROUGHPUT STATISTICS
        // --------------------------------------------------------

        auto Mean_Throughput = Statistics.Calculate_Mean(Throughput_Samples);


        auto Throughput_Standard_Deviation = Statistics.Calculate_Standard_Deviation(Throughput_Samples);


        auto Throughput_Standard_Error = Statistics.Calculate_Standard_Error(Throughput_Samples);


        auto Throughput_CI = Statistics.Calculate_Confidence_Interval_95(Throughput_Samples);


        // --------------------------------------------------------
        // DELAY STATISTICS
        // --------------------------------------------------------

        auto Mean_Delay =  Statistics.Calculate_Mean(Delay_Samples);


        auto Delay_Standard_Deviation = Statistics.Calculate_Standard_Deviation( Delay_Samples);


        auto Delay_CI =  Statistics.Calculate_Confidence_Interval_95(Delay_Samples);


        // --------------------------------------------------------
        // RECEIVED POWER STATISTICS
        // --------------------------------------------------------

        auto Mean_Received_Power = Statistics.Calculate_Mean( Received_Power_Samples);


        auto Received_Power_Standard_Deviation = Statistics.Calculate_Standard_Deviation(Received_Power_Samples);


        // --------------------------------------------------------
        // PHOTON STATISTICS
        // --------------------------------------------------------

        auto Mean_Photons =  Statistics.Calculate_Mean(Photon_Samples);


        auto Photon_Standard_Deviation = Statistics.Calculate_Standard_Deviation(Photon_Samples);


        // ========================================================
        // WRITE SUMMARY CSV
        // ========================================================

        Summary_File
            << Relay_Count
            << ","

            << Hop_Count
            << ","

            << Distance_Per_Hop
            << ","

            << Number_Of_Trials
            << ","

            << Successful_Trials
            << ","

            << Packet_Delivery_Reliability.value_or(0.0)
            << ","

            << Mean_Throughput.value_or(0.0)
            << ","

            << Throughput_Standard_Deviation.value_or(0.0)
            << ","

            << Throughput_Standard_Error.value_or(0.0)
            << ","

            << (
                Throughput_CI.has_value()
                ? Throughput_CI->Lower_Bound
                : 0.0
               )
            << ","

            << (
                Throughput_CI.has_value()
                ? Throughput_CI->Upper_Bound
                : 0.0
               )
            << ","

            << Mean_Delay.value_or(0.0)
            << ","

            << Delay_Standard_Deviation.value_or(0.0)
            << ","

            << (
                Delay_CI.has_value()
                ? Delay_CI->Lower_Bound
                : 0.0
               )
            << ","

            << (
                Delay_CI.has_value()
                ? Delay_CI->Upper_Bound
                : 0.0
               )
            << ","

            << Mean_Received_Power.value_or(0.0)
            << ","

            << Received_Power_Standard_Deviation.value_or(0.0)
            << ","

            << Mean_Photons.value_or(0.0)
            << ","

            << Photon_Standard_Deviation.value_or(0.0)

            << "\n";


        // ========================================================
        // PRINT CONFIGURATION RESULTS
        // ========================================================

        std::cout << std::scientific << std::setprecision(6);


        std::cout << "\nSuccessful Trials: " << Successful_Trials << " / " << Number_Of_Trials << "\n";


        std::cout << std::fixed << std::setprecision(4);


        std::cout << "Packet Delivery Reliability: " << Packet_Delivery_Reliability.value_or(0.0) * 100.0 << " %\n";


        std::cout << std::scientific << std::setprecision(6);


        std::cout << "Mean Bottleneck Rate: "  << Mean_Throughput.value_or(0.0)  << " bits/s\n";


        std::cout  << "Throughput Standard Deviation: "  << Throughput_Standard_Deviation.value_or(0.0)  << "\n";


        if (Throughput_CI.has_value())
        {
            std::cout << "Throughput 95% CI: [" << Throughput_CI->Lower_Bound  << ", " << Throughput_CI->Upper_Bound<< "]\n";
        }


        std::cout << "Mean Weakest Received Power: " << Mean_Received_Power.value_or(0.0) << " W\n";


        std::cout << "Mean Minimum Photons Per Packet: " << Mean_Photons.value_or(0.0) << "\n";


        std::cout << std::fixed << std::setprecision(9);


        std::cout<< "Mean Total Delay: " << Mean_Delay.value_or(0.0) << " seconds\n";


        if (Delay_CI.has_value())
        {
            std::cout << "Delay 95% CI: ["<< Delay_CI->Lower_Bound << ", " << Delay_CI->Upper_Bound<< "] seconds\n";
        }


        std::cout << "\n";
    }


    // ============================================================
    // CLOSE CSV FILES
    // ============================================================

    Trial_File.close();

    Summary_File.close();


    // ============================================================
    // EXPERIMENT COMPLETE
    // ============================================================

    std::cout << "====================================================\n";

    std::cout<< "MONTE CARLO EXPERIMENT 1 COMPLETE\n";

    std::cout << "====================================================\n\n";


    std::cout << "Created CSV files in:\n" << Results_Folder << "\n\n";


    std::cout << "Monte_Carlo_Experiment_1_Trials.csv\n";


    std::cout << "Monte_Carlo_Experiment_1_Summary.csv\n";


    return 0;
}
