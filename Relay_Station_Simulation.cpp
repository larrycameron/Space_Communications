#include "Relay_Station_Simulation.h"

#include <iostream>


void Relay_Station_Simulation::Set_Online_Probability(
    double probability)
{
    if (probability < 0.0)
    {
        probability = 0.0;
    }
    else if (probability > 1.0)
    {
        probability = 1.0;
    }

    online_chance =
        std::bernoulli_distribution(probability);
}


bool Relay_Station_Simulation::Optical_Link_Sufficient(
    const Relay_Network_Node& station,
    double minimum_received_power,
    double minimum_data_rate,
    double minimum_photons) const
{
    return station.Online
        && station.Received_Power >= minimum_received_power
        && station.Information_Rate >= minimum_data_rate
        && station.Received_Photons_Per_Packet >= minimum_photons;
}


void Relay_Station_Simulation::Create_Nodes(int number_of_relay_nodes)
{
    stations.clear();

    Relay_Network_Node earth_station{};
    Relay_Network_Node moon_station{};
    Relay_Network_Node mars_station{};

    earth_station.Node_ID = 0;
    earth_station.Type = Relay_Node_Type::Earth;
    earth_station.Online = true;

    moon_station.Node_ID = 1;
    moon_station.Type = Relay_Node_Type::Moon;
    moon_station.Online = true;

    mars_station.Node_ID = 2;
    mars_station.Type = Relay_Node_Type::Mars;
    mars_station.Online = true;

    stations[0] = earth_station;
    stations[1] = moon_station;
    stations[2] = mars_station;

    for (int i = 3;
         i < number_of_relay_nodes + 3;
         ++i)
    {
        Relay_Network_Node new_station{};

        new_station.Node_ID = i;
        new_station.Type = Relay_Node_Type::Relay;
        new_station.Distance = 1.0;
        new_station.Processing_Delay = 0.0;

        for (int attempt = 1;
             attempt <= 3;
             ++attempt)
        {
            new_station.Online =
                online_chance(gen);

            if (new_station.Online
                && new_station.Processing_Delay >= 0.0
                && new_station.Distance > 0.0)
            {
                std::cout
                    << "Relay "
                    << new_station.Node_ID
                    << " connected on attempt "
                    << attempt
                    << std::endl;

                std::cout
                    << "Relay is online and ready."
                    << std::endl;

                break;
            }

            std::cout
                << "Relay "
                << new_station.Node_ID
                << " failed on attempt "
                << attempt
                << std::endl;
        }

        stations[i] = new_station;
    }
}


bool Relay_Station_Simulation::Process_Optical_Link(
    int node_id,
    double received_power,
    double photon_flux,
    double information_rate,
    double signal_photons_per_slot,
    double received_photons_per_packet,
    double minimum_received_power,
    double minimum_data_rate,
    double minimum_photons)
{
    auto station_iterator =
        stations.find(node_id);

    if (station_iterator == stations.end())
    {
        std::cout
            << "Relay node "
            << node_id
            << " was not found."
            << std::endl;

        return false;
    }

    Relay_Network_Node& station =
        station_iterator->second;

    station.Received_Power =
        received_power;

    station.Photon_Flux =
        photon_flux;

    station.Information_Rate =
        information_rate;

    station.Signal_Photons_Per_Slot =
        signal_photons_per_slot;

    station.Received_Photons_Per_Packet =
        received_photons_per_packet;

    if (!Optical_Link_Sufficient(
            station,
            minimum_received_power,
            minimum_data_rate,
            minimum_photons))
    {
        station.Received_Encrypted_Packet = false;
        station.Reconstruct_Packet = false;
        station.Packet_Retransmitted = false;

        std::cout
            << "Optical link failed at relay "
            << node_id
            << std::endl;

        return false;
    }

    station.Received_Encrypted_Packet = true;
    station.Reconstruct_Packet = true;
    station.Packet_Retransmitted = true;

    std::cout
        << "Relay "
        << node_id
        << " received and retransmitted the packet."
        << std::endl;

    return true;
}


void Relay_Station_Simulation::Print_Relay_Stations() const
{
    for (const auto& entry : stations)
    {
        const Relay_Network_Node& station =
            entry.second;

        std::cout
            << "Relay Node ID: "
            << station.Node_ID
            << std::endl;

        std::cout
            << "Online: "
            << station.Online
            << std::endl;

        std::cout
            << "Received packet: "
            << station.Received_Encrypted_Packet
            << std::endl;

        std::cout
            << "Reconstructed packet: "
            << station.Reconstruct_Packet
            << std::endl;

        std::cout
            << "Retransmitted packet: "
            << station.Packet_Retransmitted
            << std::endl;

        std::cout
            << "Received power: "
            << station.Received_Power
            << " watts"
            << std::endl;

        std::cout
            << "Photon flux: "
            << station.Photon_Flux
            << " photons/second"
            << std::endl;

        std::cout
            << "Received photons per packet: "
            << station.Received_Photons_Per_Packet
            << std::endl;
    }
}


void Relay_Station_Simulation::Print_Relay_Summary() const
{
    int total_relays = 0;
    int successful_relays = 0;
    int offline_relays = 0;
    int first_failed_relay_id = -1;

    for (const auto& entry : stations)
    {
        const Relay_Network_Node& station =
            entry.second;

        if (station.Type != Relay_Node_Type::Relay)
        {
            continue;
        }

        ++total_relays;

        if (!station.Online)
        {
            ++offline_relays;
        }

        if (station.Packet_Retransmitted)
        {
            ++successful_relays;
        }
        else if (
            first_failed_relay_id == -1
            || station.Node_ID < first_failed_relay_id)
        {
            first_failed_relay_id =
                station.Node_ID;
        }
    }

    std::cout
        << "\n===== RELAY NETWORK SUMMARY =====\n";

    std::cout
        << "Total relay nodes: "
        << total_relays
        << '\n';

    std::cout
        << "Successful relay transmissions: "
        << successful_relays
        << '\n';

    std::cout
        << "Offline relay nodes: "
        << offline_relays
        << '\n';

    if (first_failed_relay_id != -1)
    {
        std::cout
            << "First failed relay ID: "
            << first_failed_relay_id
            << '\n';

        std::cout
            << "Packet reached Mars: No\n";
    }
    else
    {
        std::cout
            << "First failed relay ID: None\n";

        std::cout
            << "Packet reached Mars: Yes\n";
    }
}