#ifndef INTERSTELLAR_COMMUNICATIONS_NETWORK_H
#define INTERSTELLAR_COMMUNICATIONS_NETWORK_H


#include <iostream>
#include <vector>
#include <cstddef>
#include <string>
#include <random>
#include <iomanip>
#include <algorithm> // Required for std::min

 

enum Network_Node_IDs
{
    NODE_0,
    NODE_1,
    NODE_2,
    NODE_EARTH,
    NODE_MOON,
    NODE_Spacecraft,
    NODE_MARS,
    NODE_FAILURE,
    NODE_SYSTEM_TEST,
    REESTABLISH_CONNECTION
};

struct Network_Node
{
  Network_Node_IDs Node_ID;

  double Distance{};
  double Propagation_Delay{};
  double Transmission_Delay{};
  double Packet_Size{};
  double Bandwidth{};
  double Frame_Size{};
  double Time{};
  bool Online{true};
};

class Interstellar_Communications_Network
{
private:   
        
        static constexpr double Speed_Of_Light{2.998e8};
        std::vector<Network_Node> Network_Nodes{};
        std::random_device rd;
        std::mt19937 gen;
        std::lognormal_distribution<double> log_dist;
        const double minimum_latency = 1250.0;
        const double maximum_latency = 5000.0;
public:   
        Interstellar_Communications_Network();
    
        double Calculate_Propagation_Delay(double distance);
        double Calculate_Transmission_Time(double frame_size, double bandwidth);
        double Calculate_Transmission_Delay(double packet_size, double bandwidth);
        double Calculate_Round_Trip_Time(double time);
        double Calculate_Utilization_Efficiency_Parameter(double propagation_delay, double transmission_time);
        double Generate_Earth_To_Mars_Distance();
        void Add_Network_ID(Network_Node_IDs Node_ID, double distance);
        void Network_ID_Assignment();
        void Check_Assigned_Nodes(Network_Node_IDs Relay_Station);


}; 

#endif