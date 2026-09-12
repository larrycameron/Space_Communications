#ifndef OPTICAL_COMMUNICATION_H
#define OPTICAL_COMMUNICATION_H


#include <iostream>
#include <cmath>
#include <cstddef>
 

class Optical_Communications
{

private:

    static constexpr double Plancks_Constant{6.626e-34};// joules * seconds
    static constexpr double PI= {3.141592653589793};
    static constexpr double Speed_of_Light{2.998e8};// m/s 

    double Received_Power{}; //watt; w
    double Transmitted_Power{};// watt; w
    double Eta_Atmosphere_Efficiency{};//dimensionless
    double Eta_Diffraction_Efficiency{};//dimensionless
    
    double Eta_Receiver_Efficiency{};//dimensionless
    double Photon_Flux{};
    double Information_Rate{};
    double PIE{};

    double Diameter_RX{};// meters
    double Diameter_TX{};//meters
    double Frequency{};//Hz
    double Distance{};// meters

    //double Distance_Loss{};
    double Bandwidth{};
    double Average_Number_of_Received_Signal_Photons_Per_Time_Slot{};
    double Average_Number_of_Background_Noise_Photons_Per_Time_Slot{};
    double Effective_Number_of_Modes{};
   
    double Noise_Power_Spectral_Density{};
    double Noise_PSD_in_Photon_Energy_Units{};
    double Information_Rate_S1{};
    double Information_Rate_S2{};
    
    double PPM_Order{};
    double Mean_Photons_Per_PPM_Frame{};
    double PIE_PPM{};
    double PIE_S1{};
   
    double PIE_S2{};
    double Gordon_Holevo_Rate{};
    double Gordon_Holevo_PIE{};
    
    std::size_t Packet_Bits{};
    double Packet_Transmission_Time{};
    double Received_Photons_Per_Packet{};

public:

    double Calcualte_Diffraction_Squence(double diameter_rx, double diameter_tx, double frequency, double distance);
    double Free_Space_Optical_Link(double transmitted_power, double eta_atmosphere_efficiency, double eta_diffraction_efficiency,  double eta_receiver_efficiency);
    double Calculate_Photon_Information_Efficiency(double photon_information_efficiency);
    double Calculate_Average_Signal_Photons_Per_Slot(double bandwidth);
    double Calculate_Average_Background_Noise_Photons_Per_Slot(double effective_number_of_modes, double noise_power_spectral_density);
    double Calculate_Power_Spectral_Density_in_Photon_Energy_Units();
    double Calculate_S2_Information_Rate();
    double Calculate_S1_Information_Rate();
    double Calculate_S1_PIE();
    double Calculate_S2_PIE();
    double Gordon_Holevo_g(double x);
    double Calculate_Gordon_Holevo_Rate();
    double Calculate_Gordon_Holevo_PIE();
    double Calculate_PPM_Frame_Photons(double ppm_order);
    double Calculate_PPM_Photon_Information_Efficiency();
    double Get_Photon_Flux() const;
    std::size_t Calculate_Packet_Bits(std::size_t packet_bytes);
    double Calculate_Packet_Transmission_Time();
    double Calculate_Received_Photons_Per_Packet();
    void Print_Calculations() const;

   
};

#endif