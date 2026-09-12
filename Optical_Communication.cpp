#include "Optical_Communication.h"

#include <iostream>
#include <cmath>

double Optical_Communications::Calcualte_Diffraction_Squence(double diameter_rx, double diameter_tx, double frequency, double distance)    
{

        if (diameter_rx < 0.0 ||  diameter_tx < 0.0 ||  frequency <= 0.0 ||  distance <= 0.0)
        {
            Eta_Diffraction_Efficiency = 0.0;
            return 0.0;
        }
    
    
    
    
        Diameter_RX = diameter_rx;
        Diameter_TX = diameter_tx;
        Frequency = frequency;
        Distance = distance;
       
        double top = PI * diameter_rx * diameter_tx * frequency;
        double bottom = 4.0 * Speed_of_Light * distance;

        Eta_Diffraction_Efficiency = std::pow((top / bottom), 2);

        if(Eta_Diffraction_Efficiency > 1.0)
        {
            std::cout << "Warning: diffraction efficiency exceeds physical limits." << std::endl;
        }
        return Eta_Diffraction_Efficiency;

}


    double Optical_Communications::Free_Space_Optical_Link(double transmitted_power, double eta_atmosphere_efficiency, double eta_diffraction_efficiency,  double eta_receiver_efficiency)
    { 
        if (transmitted_power < 0.0 ||  eta_atmosphere_efficiency < 0.0 ||  eta_atmosphere_efficiency > 1.0 ||  eta_diffraction_efficiency < 0.0 || eta_diffraction_efficiency > 1.0 ||  eta_receiver_efficiency < 0.0 ||  eta_receiver_efficiency > 1.0)
        {
            Received_Power = 0.0;
            return 0.0;
        }
          
        Transmitted_Power = transmitted_power;
        Eta_Atmosphere_Efficiency = eta_atmosphere_efficiency;
        Eta_Diffraction_Efficiency =  eta_diffraction_efficiency;
        Eta_Receiver_Efficiency = eta_receiver_efficiency;
     
        Received_Power = transmitted_power * eta_atmosphere_efficiency * eta_diffraction_efficiency * eta_receiver_efficiency;
       
        return Received_Power;

    }

    double Optical_Communications::Calculate_Photon_Information_Efficiency(double photon_information_efficiency)
    {

        PIE = photon_information_efficiency;     
        Photon_Flux = Received_Power/(Plancks_Constant * Frequency);
        Information_Rate = PIE * Photon_Flux;

        return Information_Rate;
    }

    double Optical_Communications::Calculate_Average_Signal_Photons_Per_Slot(double bandwidth)
    {
        if (bandwidth <= 0.0)
        {
            Bandwidth = 0.0;
            Average_Number_of_Received_Signal_Photons_Per_Time_Slot = 0.0;
            return 0.0;
        }

        Bandwidth = bandwidth;
        Average_Number_of_Received_Signal_Photons_Per_Time_Slot = (1.0/bandwidth) * (Received_Power/(Plancks_Constant * Frequency));

        return Average_Number_of_Received_Signal_Photons_Per_Time_Slot;
    }

    double Optical_Communications::Calculate_Average_Background_Noise_Photons_Per_Slot(double effective_number_of_modes, double noise_power_spectral_density)
    {
        if (effective_number_of_modes < 0.0 || noise_power_spectral_density < 0.0)
        {
            Average_Number_of_Background_Noise_Photons_Per_Time_Slot = 0.0;
            Noise_Power_Spectral_Density = 0.0;
            return 0.0;
        }
     
        Effective_Number_of_Modes = effective_number_of_modes;

        Noise_Power_Spectral_Density = noise_power_spectral_density;

        Average_Number_of_Background_Noise_Photons_Per_Time_Slot = effective_number_of_modes * (noise_power_spectral_density / (Plancks_Constant * Frequency));

        return Average_Number_of_Background_Noise_Photons_Per_Time_Slot;
    }

    double Optical_Communications::Calculate_Power_Spectral_Density_in_Photon_Energy_Units()
    {

        Noise_PSD_in_Photon_Energy_Units =   Noise_Power_Spectral_Density/(Plancks_Constant * Frequency);
       
        return Noise_PSD_in_Photon_Energy_Units;
    }

    double Optical_Communications::Calculate_S1_Information_Rate()
    {

        double SNR = (4.0 * Average_Number_of_Received_Signal_Photons_Per_Time_Slot) / (1.0 + 2.0 * Noise_PSD_in_Photon_Energy_Units);
        Information_Rate_S1 =  (Bandwidth / 2.0) * std::log2(1.0 + SNR);
        
        return Information_Rate_S1;

    }

    double Optical_Communications::Calculate_S2_Information_Rate()
    {
        double SNR = Average_Number_of_Received_Signal_Photons_Per_Time_Slot / (1.0 + Noise_PSD_in_Photon_Energy_Units); 
        Information_Rate_S2 = Bandwidth *  std::log2(1.0 + SNR);

        return Information_Rate_S2;
    }

    double Optical_Communications::Calculate_S1_PIE()
    {
        PIE_S1 = (2.0 / (1.0 + 2.0 * Noise_PSD_in_Photon_Energy_Units)) * std::log2(std::exp(1.0));

        return PIE_S1;
    }

    double Optical_Communications::Calculate_S2_PIE()
    {
        PIE_S2 = (1.0 / (1.0 + Noise_PSD_in_Photon_Energy_Units)) * std::log2(std::exp(1.0));

        return PIE_S2;
    }

    double Optical_Communications::Gordon_Holevo_g(double x)
    {
        if (x == 0.0)
        {
            return 0.0;
        }
        return (x + 1.0) * std::log2(x + 1.0)- x * std::log2(x);
    }

    double Optical_Communications::Calculate_Gordon_Holevo_Rate()
    {

        double ns = Average_Number_of_Received_Signal_Photons_Per_Time_Slot;
        double nn = Noise_PSD_in_Photon_Energy_Units;

        Gordon_Holevo_Rate = Bandwidth * (Gordon_Holevo_g(ns + nn)- Gordon_Holevo_g(nn));

        return Gordon_Holevo_Rate;
    }

    double Optical_Communications::Calculate_Gordon_Holevo_PIE()
    {
        double ns = Average_Number_of_Received_Signal_Photons_Per_Time_Slot;

        double nn = Noise_PSD_in_Photon_Energy_Units;

        if (ns <= 0.0)
        {
            Gordon_Holevo_PIE = 0.0;
            return 0.0;
        }

        Gordon_Holevo_PIE = (Gordon_Holevo_g(ns + nn) - Gordon_Holevo_g(nn)) / ns;

        return Gordon_Holevo_PIE;
    }
    
    double Optical_Communications::Calculate_PPM_Frame_Photons(double ppm_order)
    {
        if (ppm_order <= 0.0)
        {
            PPM_Order = 0.0;
            Mean_Photons_Per_PPM_Frame = 0.0;
            return 0.0;
        }
        
        PPM_Order = ppm_order;
        Mean_Photons_Per_PPM_Frame = PPM_Order *  Average_Number_of_Received_Signal_Photons_Per_Time_Slot;

        return Mean_Photons_Per_PPM_Frame;
    }

    double Optical_Communications::Calculate_PPM_Photon_Information_Efficiency()
    {
        if (Mean_Photons_Per_PPM_Frame <= 0.0 || PPM_Order <= 0.0)
        {
            PIE_PPM = 0.0;
            return 0.0;
        }
        
        PIE_PPM = (1.0 / Mean_Photons_Per_PPM_Frame) * (1.0 - std::exp(-Mean_Photons_Per_PPM_Frame)) * std::log2(PPM_Order);

        return PIE_PPM;
    }

    void Optical_Communications::Print_Calculations() const
    {

        std::cout<<Eta_Diffraction_Efficiency * 100<< ": "<< " percent"<<std::endl;
        std::cout<< "Received Power: "<< Received_Power << " watts"<< std::endl;
        std::cout<< "Received Photon Flux: "<< Photon_Flux << " photons/second" << std::endl;
        std::cout << "Photon Information Efficiency: " << PIE << " bits/photon" << std::endl;
        std::cout<< "Information Rate: "<< Information_Rate<< " bps"<< std::endl;
        std::cout << "Assumed PIE Test Rate: "<< Information_Rate / 1e6 << " Mbps" << std::endl;
        std::cout << "Average Received Signal Photons Per Time Slot: " << Average_Number_of_Received_Signal_Photons_Per_Time_Slot << " photons/slot"<< std::endl;
        std::cout << "Average Background Noise Photons Per Slot: " << Average_Number_of_Background_Noise_Photons_Per_Time_Slot<< " photons/slot"<< std::endl;   
        std::cout << "Noise PSD in Photon Energy Units: " << Noise_PSD_in_Photon_Energy_Units << std::endl;
        std::cout << "S1 Information Rate: " << Information_Rate_S1 << " bps" << std::endl;
        std::cout << "S2 Information Rate: " << Information_Rate_S2 << " bps" << std::endl;
        std::cout << "PPM Order: " << PPM_Order << std::endl;
        std::cout << "Mean Photons Per PPM Frame: "  << Mean_Photons_Per_PPM_Frame << " photons/frame" << std::endl;
        std::cout << "PPM Photon Information Efficiency: " << PIE_PPM << " bits/photon" << std::endl;
        std::cout << "S1 PIE Limit: "<< PIE_S1 << " bits/photon" << std::endl;
        std::cout << "S2 PIE Limit: "<< PIE_S2<< " bits/photon" << std::endl;
        std::cout << "Gordon-Holevo Information Rate: "<< Gordon_Holevo_Rate<< " bps" << std::endl;
        std::cout << "Gordon-Holevo PIE: "<< Gordon_Holevo_PIE<< " bits/photon" << std::endl;
    }
    double Optical_Communications::Get_Photon_Flux() const
    {
        return Photon_Flux;
    }
    
    std::size_t Optical_Communications::Calculate_Packet_Bits(std::size_t packet_bytes)
    {
        Packet_Bits = packet_bytes * 8;

        return Packet_Bits;
    }

    double Optical_Communications::Calculate_Packet_Transmission_Time()
    {
        if (Information_Rate <= 0.0)
        {
            std::cout << "Information rate must be greater than zero." << std::endl;

            return 0.0;
        }

        Packet_Transmission_Time = static_cast<double>(Packet_Bits) / Information_Rate;

        return Packet_Transmission_Time;
    }

    double Optical_Communications::Calculate_Received_Photons_Per_Packet()
    {
        Received_Photons_Per_Packet = Photon_Flux * Packet_Transmission_Time;

        return Received_Photons_Per_Packet;
    }
