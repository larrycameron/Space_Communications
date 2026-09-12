#include "Interstellar_Communications_Network.h"

#include <iostream>
#include <vector>
#include <cstddef>
#include <string>
#include <random>
#include <iomanip>
#include <algorithm> // Required for std::min


    double Interstellar_Communications_Network::Calculate_Propagation_Delay(double distance)
    {
        if (distance < 0.0)
        {
            std::cout << "Invalid distance." << std::endl;
            return 0.0;
        }

        double Propagation_Delay =  distance / Speed_Of_Light;

        return Propagation_Delay;
    }

    double Interstellar_Communications_Network::Calculate_Transmission_Delay( double packet_size, double bandwidth)
    {
        if (packet_size < 0.0)
        {
            std::cout << "Invalid packet size." << std::endl;
            return 0.0;
        }

        if (bandwidth <= 0.0)
        {
            std::cout << "Invalid bandwidth." << std::endl;
            return 0.0;
        }

        double Transmission_Delay = packet_size / bandwidth;
        return Transmission_Delay;
    }

    double Interstellar_Communications_Network::Calculate_Transmission_Time( double frame_size, double bandwidth)
    {
        if (frame_size < 0.0)
        {
            std::cout << "Invalid frame size." << std::endl;
            return 0.0;
        }

        if (bandwidth <= 0.0)
        {
            std::cout << "Invalid bandwidth." << std::endl;
            return 0.0;
        }

        double Transmission_Time = frame_size / bandwidth;
        return Transmission_Time;
    }

    double Interstellar_Communications_Network::Calculate_Round_Trip_Time(double time)
    {
        if (time < 0.0)
        {
            std::cout << "Invalid time." << std::endl;
            return 0.0;
        }

        double Round_Trip_Time = 2.0 * time;
        return Round_Trip_Time;
    }

    double Interstellar_Communications_Network::Calculate_Utilization_Efficiency_Parameter(double propagation_delay, double transmission_time)
    {
        if (propagation_delay < 0.0)
        {
            std::cout << "Invalid propagation delay." << std::endl;
            return 0.0;
        }

        if (transmission_time <= 0.0)
        {
            std::cout << "Invalid transmission time." << std::endl;
            return 0.0;
        }

        double Utilization_Efficiency_Parameter = propagation_delay / transmission_time;

        return Utilization_Efficiency_Parameter;
    }

    double Interstellar_Communications_Network::Generate_Earth_To_Mars_Distance()
    {
        std::uniform_real_distribution<double> earth_to_mars_dist(5.46e10, 4.01e11);

        double distance = earth_to_mars_dist(gen);

        return distance;
    }
   
    Interstellar_Communications_Network::Interstellar_Communications_Network()
        : gen(rd()), log_dist(6.5, 0.5)
    {
    }

    void Interstellar_Communications_Network::Add_Network_ID(Network_Node_IDs Node_ID, double distance)
    {
        if (distance < 0.0)
        {
            std::cout << "Invalid node distance." << std::endl;
            return;
        }

        Network_Node node;
        node.Node_ID = Node_ID;
        node.Distance = distance;

        Network_Nodes.push_back(node);
    }

    void Interstellar_Communications_Network::Network_ID_Assignment()
    {
        for (std::size_t i = 0; i < Network_Nodes.size(); ++i)
        {
            Network_Nodes[i].Propagation_Delay = Calculate_Propagation_Delay(Network_Nodes[i].Distance);

            std::cout << "\nStored Node ID: " << Network_Nodes[i].Node_ID << std::endl;

            std::cout << "Distance: "  << Network_Nodes[i].Distance << " meters" << std::endl;

            std::cout << "Propagation Delay: " << Network_Nodes[i].Propagation_Delay<< " seconds" << std::endl;
            
            double raw_value = log_dist(gen);
            double bounded_value = std::min(minimum_latency + raw_value, maximum_latency);
            std::size_t loop_limit = static_cast<std::size_t>(bounded_value);
            std::cout << "Latency: " << loop_limit << " Simulated Processing Delay..." << std::endl;
            volatile std::size_t work_sink = 0;

            for (std::size_t k = 0; k < loop_limit; ++k)
            {

                work_sink += k;
            }

            Check_Assigned_Nodes(Network_Nodes[i].Node_ID);
        }

    }

    void Interstellar_Communications_Network::Check_Assigned_Nodes(Network_Node_IDs Relay_Station)
    {
        switch(Relay_Station)
        {
            case NODE_0:
                std::cout << "Node 0 selected" << std::endl;
                break;

            case NODE_1:
                std::cout << "Node 1 selected" << std::endl;
                break;

            case NODE_2:
                std::cout << "Node 2 selected" << std::endl;
                break;

            case NODE_EARTH:
                std::cout << "Connected to Earth base station" << std::endl;
                break;

            case NODE_MOON:
                std::cout << "Connected to Lunar relay station" << std::endl;
                break;

            case NODE_Spacecraft:
                std::cout << "Connected to Spacecraft data link" << std::endl;
                break;

            case NODE_MARS:
                std::cout << "Connected to Mars base station" << std::endl;
                break;

            case NODE_FAILURE:
                std::cout << "CRITICAL: Node connection failure!" << std::endl;
                break;

            case NODE_SYSTEM_TEST:
                std::cout << "Running diagnostic system test..." << std::endl;
                break;

            case REESTABLISH_CONNECTION:
                std::cout << "Attempting to reconnect..." << std::endl;
                break;

            default:
                std::cout << "Unknown Node ID" << std::endl;
                break;
        }

    }

