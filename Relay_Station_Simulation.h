#ifndef RELAY_STATION_SIMULATION_H
#define RELAY_STATION_SIMULATION_H

#include <iostream>
#include <random>
#include <unordered_map>

enum class Relay_Node_Type
{
    Earth,
    Moon,
    Mars,
    Relay
};

struct Relay_Network_Node
{
    int Node_ID{};
    Relay_Node_Type Type{Relay_Node_Type::Relay};

    bool Online{};
    bool Received_Encrypted_Packet{};
    bool Reconstruct_Packet{};
    bool Packet_Retransmitted{};

    double Distance{};
    double Processing_Delay{};

    // Optical communication results
    double Diffraction_Efficiency{};
    double Received_Power{};
    double Photon_Flux{};
    double Information_Rate{};

    double Signal_Photons_Per_Slot{};
    double Background_Noise_Photons_Per_Slot{};
    double Received_Photons_Per_Packet{};
    double Noise_PSD{};

    double S1_Information_Rate{};
    double S2_Information_Rate{};

    double S1_PIE{};
    double S2_PIE{};

    double Gordon_Holevo_Rate{};
    double Gordon_Holevo_PIE{};

    double Mean_Photons_Per_PPM_Frame{};
    double PPM_PIE{};
};

class Relay_Station_Simulation
{
private:
    std::unordered_map<int, Relay_Network_Node> stations;

    std::random_device rd;
    std::mt19937 gen{rd()};
    std::bernoulli_distribution online_chance{0.8};

public:
    bool Optical_Link_Sufficient(const Relay_Network_Node& station, double minimum_received_power, double minimum_data_rate, double minimum_photons) const;

    bool Process_Optical_Link(int node_id, double received_power, double photon_flux, double information_rate, double signal_photons_per_slot, double received_photons_per_packet, double minimum_received_power, double minimum_data_rate, double minimum_photons);

    void Create_Nodes(int number_of_relay_nodes);
    
    void Print_Relay_Stations() const;
    void Print_Relay_Summary() const;

    void Set_Online_Probability(double probability);


};

#endif
