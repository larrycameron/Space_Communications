#include <iostream>
#include <iomanip>
#include "Optical_Communication.h"

int main()
{
    Optical_Communications optical;

    std::cout << std::setprecision(12);

    std::cout << "\n=== OC-026: Final Nominal Regression ===\n";

    double diffraction =
        optical.Calcualte_Diffraction_Squence(
            1.0,
            0.2,
            193.5e12,
            1.0e9
        );

    double received_power =
        optical.Free_Space_Optical_Link(
            10.0,
            0.90,
            diffraction,
            0.80
        );

    double information_rate =
        optical.Calculate_Photon_Information_Efficiency(5.0);

    double signal_photons =
        optical.Calculate_Average_Signal_Photons_Per_Slot(1.0e9);

    double background_photons =
        optical.Calculate_Average_Background_Noise_Photons_Per_Slot(
            2.0,
            1.0e-20
        );

    double noise_psd =
        optical.Calculate_Power_Spectral_Density_in_Photon_Energy_Units();

    double s1_rate =
        optical.Calculate_S1_Information_Rate();

    double s2_rate =
        optical.Calculate_S2_Information_Rate();

    double s1_pie =
        optical.Calculate_S1_PIE();

    double s2_pie =
        optical.Calculate_S2_PIE();

    double gh_rate =
        optical.Calculate_Gordon_Holevo_Rate();

    double gh_pie =
        optical.Calculate_Gordon_Holevo_PIE();

    double ppm_photons =
        optical.Calculate_PPM_Frame_Photons(16.0);

    double ppm_pie =
        optical.Calculate_PPM_Photon_Information_Efficiency();

    std::size_t packet_bits =
        optical.Calculate_Packet_Bits(1024);

    double packet_time =
        optical.Calculate_Packet_Transmission_Time();

    double packet_photons =
        optical.Calculate_Received_Photons_Per_Packet();

    std::cout << "Diffraction Efficiency: " << diffraction << '\n';
    std::cout << "Received Power: " << received_power << " W\n";
    std::cout << "Photon Flux: " << optical.Get_Photon_Flux()
              << " photons/s\n";
    std::cout << "Information Rate: " << information_rate << " bps\n";
    std::cout << "Signal Photons/Slot: " << signal_photons << '\n';
    std::cout << "Background Photons/Slot: " << background_photons << '\n';
    std::cout << "Noise PSD Photon Units: " << noise_psd << '\n';
    std::cout << "S1 Rate: " << s1_rate << " bps\n";
    std::cout << "S2 Rate: " << s2_rate << " bps\n";
    std::cout << "S1 PIE: " << s1_pie << " bits/photon\n";
    std::cout << "S2 PIE: " << s2_pie << " bits/photon\n";
    std::cout << "Gordon-Holevo Rate: " << gh_rate << " bps\n";
    std::cout << "Gordon-Holevo PIE: " << gh_pie << " bits/photon\n";
    std::cout << "PPM Frame Photons: " << ppm_photons << '\n';
    std::cout << "PPM PIE: " << ppm_pie << " bits/photon\n";
    std::cout << "Packet Bits: " << packet_bits << '\n';
    std::cout << "Packet Transmission Time: " << packet_time << " s\n";
    std::cout << "Received Photons/Packet: " << packet_photons << '\n';

    return 0;
}