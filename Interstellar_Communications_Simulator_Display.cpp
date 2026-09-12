#include "Interstellar_Communications_Simulator_Display.h"

#include <iostream>


void Interstellar_Communications_Simulator_Display::Display_Main_Menu()
{
    std::cout << "\n";
    std::cout << "====================================================\n";
    std::cout << "   INTERPLANETARY COMMUNICATIONS SIMULATOR\n";
    std::cout << "====================================================\n";
    std::cout << "1. Physics and Geometry\n";
    std::cout << "2. Optical Communications\n";
    std::cout << "3. Network and Encryption\n";
    std::cout << "4. Monte Carlo Experiments\n";
    std::cout << "5. Research Results\n";
    std::cout << "6. Exit\n";
    std::cout << "====================================================\n";
}


void Interstellar_Communications_Simulator_Display::Display_Menu(
    Simulator_Display display_menu)
{
    switch (display_menu)
    {
        case Simulator_Display::MainMenu:
            Display_Main_Menu();
            break;

        case Simulator_Display::KeplerPhysicsEngine:
            std::cout << "Kepler Physics Engine\n";
            break;

        case Simulator_Display::SpacecraftGeometry:
            std::cout << "Spacecraft Geometry\n";
            break;

        case Simulator_Display::ShapiroTimeDelay:
            std::cout << "Shapiro Time Delay\n";
            break;

        case Simulator_Display::SignalAnalysis:
            std::cout << "Signal Analysis\n";
            break;

        case Simulator_Display::OpticalCommunications:
            std::cout << "Optical Communications\n";
            break;

        case Simulator_Display::SpaceCommunicationsLink:
            std::cout << "Space Communications Link\n";
            break;

        case Simulator_Display::InterstellarNetwork:
            std::cout << "Interstellar Communications Network\n";
            break;

        case Simulator_Display::EncryptedMessageTransmission:
            std::cout << "Encrypted Message Transmission\n";
            break;

        case Simulator_Display::RelayStation:
            std::cout << "Relay Station Simulation\n";
            break;

        case Simulator_Display::RelayTopology:
            std::cout << "Relay Topology\n";
            break;

        case Simulator_Display::RelayRouting:
            std::cout << "Interstellar Relay Routing\n";
            break;

        case Simulator_Display::ContactWindow:
            std::cout << "Contact Window\n";
            break;

        case Simulator_Display::PATTiming:
            std::cout << "Pointing, Acquisition, and Tracking Timing\n";
            break;

        case Simulator_Display::MonteCarloExperiment1:
            std::cout << "Monte Carlo Experiment 1\n";
            break;

        case Simulator_Display::MonteCarloExperiment2:
            std::cout << "Monte Carlo Experiment 2\n";
            break;

        case Simulator_Display::MonteCarloExperiment3:
            std::cout << "Monte Carlo Experiment 3\n";
            break;

        case Simulator_Display::MonteCarloExperiment4:
            std::cout << "Monte Carlo Experiment 4\n";
            break;

        case Simulator_Display::ResearchResults:
            std::cout << "Research Results\n";
            break;

        case Simulator_Display::Exit:
            std::cout << "Exiting Simulation\n";
            break;

        default:
            std::cout << "Unknown Display Mode\n";
            break;
    }
}


void Interstellar_Communications_Simulator_Display::Display_Physics_Menu()
{
    std::cout << "\n";
    std::cout << "========== PHYSICS AND GEOMETRY ==========\n";
    std::cout << "1. Kepler Physics Engine\n";
    std::cout << "2. Spacecraft Geometry\n";
    std::cout << "3. Shapiro Time Delay\n";
    std::cout << "4. Return to Main Menu\n";
}


void Interstellar_Communications_Simulator_Display::Display_Communications_Menu()
{
    std::cout << "\n";
    std::cout << "========== OPTICAL COMMUNICATIONS ==========\n";
    std::cout << "1. Signal Analysis\n";
    std::cout << "2. Optical Communications\n";
    std::cout << "3. Space Communications Link\n";
    std::cout << "4. Contact Window\n";
    std::cout << "5. PAT Timing\n";
    std::cout << "6. Return to Main Menu\n";
}


void Interstellar_Communications_Simulator_Display::Display_Network_Menu()
{
    std::cout << "\n";
    std::cout << "========== NETWORK AND ENCRYPTION ==========\n";
    std::cout << "1. Interstellar Communications Network\n";
    std::cout << "2. Encrypted Message Transmission\n";
    std::cout << "3. Relay Station Simulation\n";
    std::cout << "4. Relay Topology\n";
    std::cout << "5. Relay Routing\n";
    std::cout << "6. Return to Main Menu\n";
}


void Interstellar_Communications_Simulator_Display::Display_Monte_Carlo_Menu()
{
    std::cout << "\n";
    std::cout << "========== MONTE CARLO EXPERIMENTS ==========\n";
    std::cout << "1. Experiment 1 - Optical Link and Relay Spacing\n";
    std::cout << "2. Experiment 2 - Dynamic Orbital Geometry\n";
    std::cout << "3. Experiment 3 - Relay Placement Optimization\n";
    std::cout << "4. Experiment 4 - Autonomous Network Resilience\n";
    std::cout << "5. Return to Main Menu\n";
}


void Interstellar_Communications_Simulator_Display::Display_Research_Menu()
{
    std::cout << "\n";
    std::cout << "========== RESEARCH RESULTS ==========\n";
    std::cout << "1. Experiment Results\n";
    std::cout << "2. Statistical Analysis\n";
    std::cout << "3. Hypothesis Testing\n";
    std::cout << "4. Return to Main Menu\n";
}