#include "Optical_Network_Monte_Carlo.h"
#include "Optical_Communication.h"

#include <cmath>
#include <iostream>

void Monte_Carlo_Optical_Network_Simulator::Run_Single_Simulation()
{
    First_Failed_Relay_ID = -1;

    // One complete optical-network experiment goes here.
}

void Monte_Carlo_Optical_Network_Simulator::Run_Monte_Carlo(
    std::size_t number_of_simulations)
{
    for (std::size_t simulation = 0;
         simulation < number_of_simulations;
         ++simulation)
    {
        Run_Single_Simulation();
    }
}

void Monte_Carlo_Optical_Network_Simulator::Print_Monte_Carlo_Summary() const
{
    // Print statistics after all simulations finish.
}

Route Successful
Earth-Mars Distance
Relay Spacing
Relay Count
Transmit Power
Received Power
Background Noise
Photon Flux
Photons Per Packet
SNR
Information Rate
BER
Packet Error Probability
Packets Transmitted
Packets Delivered
Corrupted Packets
Packet Delivery %
Corrupted Packet %
Throughput
Relative Velocity
Pointing Error
Earth True Anomaly
Mars True Anomaly
Orbital Time
First Failed Relay ID

Calculate_Orbital_State();

Calculate_Spacecraft_Geometry();

double distance = Calculate_Range(...);

double relative_velocity = ...;

Run_Optical_Link(
    distance,
    relative_velocity,
    transmit_power,
    background_noise,
    pointing_error);