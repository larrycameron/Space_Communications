#include "Spacecraft_Communication_Link.h"

#include <cmath>
#include <iostream>
#include <iomanip>

// 1. Initialize core system parameters

void Communication_Space_Link::SetSystemParameters(double distance_spacecraft_to_earth, double frequency_hz)

{

    Distance_Spacecraft_To_Earth = distance_spacecraft_to_earth;

    Signal_Frequency = frequency_hz;

    // Wavelength is automatically derived: λ = c / f

    Wavelength = c / frequency_hz;
}

// 2. Calculates path loss using the globally set distance and frequency

double Communication_Space_Link::CalculateFreeSpacePathLoss()

{
    if (Distance_Spacecraft_To_Earth <= 0.0 || Signal_Frequency <= 0.0)
    {
        FSPL_Linear = 0.0;
        FSPL_dB = 0.0;
        return FSPL_dB;
    }


    // FSPL = (4 * PI * d * f / c)^2

    FSPL_Linear = std::pow((4.0 * PI * Distance_Spacecraft_To_Earth * Signal_Frequency) / c, 2);

    FSPL_dB = 10.0 * std::log10(FSPL_Linear);

    return FSPL_dB;
}

// 3. Calculates Earth station antenna gain using derived wavelength

double Communication_Space_Link::CalculateEarthAntennaGain(double efficiency, double diameter_meters)
{
    if (efficiency < 0.0 || efficiency > 1.0 || diameter_meters < 0.0)
    {
        Antenna_Gain_Linear = 0.0;
        Gain_Earth_Antenna_Linear = 0.0;
        return 0.0;
    }
    
    
    Antenna_Efficiency = efficiency;

    Dish_Diameter = diameter_meters;

    // Gain = Efficiency * (PI * Diameter / Wavelength)^2

    Antenna_Gain_Linear = Antenna_Efficiency * std::pow((PI * Dish_Diameter / Wavelength), 2);

    Gain_Earth_Antenna_Linear = Antenna_Gain_Linear;

    return Gain_Earth_Antenna_Linear;
}

// 4. Friis Equation uses the internally tracked states to avoid mismatches

double Communication_Space_Link::CalculateFriisTransmissionEquation(double tx_power_watts, double tx_antenna_gain_linear)
{
    if (tx_power_watts < 0.0 || tx_antenna_gain_linear < 0.0 ||  Gain_Earth_Antenna_Linear < 0.0 || Distance_Spacecraft_To_Earth <= 0.0 ||  Wavelength <= 0.0)
    {
        Received_Power_Watts = 0.0;
        return 0.0;
    }

    Transmitter_Power_Watts = tx_power_watts;

    Gain_Spacecraft_Antenna_Linear = tx_antenna_gain_linear;

    // Friis: Pr = Pt * Gt * Gr * (λ / (4 * PI * d))^2

    Received_Power_Watts = Transmitter_Power_Watts * Gain_Spacecraft_Antenna_Linear * Gain_Earth_Antenna_Linear * std::pow((Wavelength / (4.0 * PI * Distance_Spacecraft_To_Earth)), 2);

    return Received_Power_Watts;
}

double Communication_Space_Link::ThermalNoisePower(double system_noise_temperature, double receiver_bandwidth)
{
    if (system_noise_temperature < 0.0 || receiver_bandwidth < 0.0)
    {
        Noise_Power = 0.0;
        return 0.0;
    }
    System_Noise_Temperature = system_noise_temperature;

    Receiver_Bandwidth = receiver_bandwidth;

    // Ensure Boltzmanns_Constant is defined in your class (e.g., 1.380649e-23)

    Noise_Power = Boltzmanns_Constant * System_Noise_Temperature * Receiver_Bandwidth;

    return Noise_Power;
}

double Communication_Space_Link::Calculate_Speed_of_Light_Delay()
{
    if (Distance_Spacecraft_To_Earth < 0.0)
    {
        Signal_Travel_Time = 0.0;
        return 0.0;
    }
    
    Signal_Travel_Time = Distance_Spacecraft_To_Earth / c;

    return Signal_Travel_Time;
}

double Communication_Space_Link::CalculateMaximumDataRate(double system_noise_temperature, double receiver_bandwidth)
{
    ThermalNoisePower(system_noise_temperature, receiver_bandwidth);

        if (Noise_Power <= 0.0)
    {
        Signal_To_Noise_Ratio = 0.0;
        Channel_Capacity = 0.0;

        return Channel_Capacity;
    }

    Signal_To_Noise_Ratio = Received_Power_Watts / Noise_Power;

    Channel_Capacity = Receiver_Bandwidth * std::log2(1.0 + Signal_To_Noise_Ratio);

    return Channel_Capacity;
}

double Communication_Space_Link::CalculateBitErrorRate(double actual_data_rate_bps)
{
    Current_Data_Rate = actual_data_rate_bps;

    if (Signal_To_Noise_Ratio <= 0.0 || Current_Data_Rate <= 0.0)
    {
        Eb_N0_Linear = 0.0;
        Eb_N0_dB = 0.0;
        Bit_Error_Rate = 0.5;

        return Bit_Error_Rate;
    }

    Eb_N0_Linear =  Signal_To_Noise_Ratio *  (Receiver_Bandwidth / Current_Data_Rate);

    Eb_N0_dB = 10.0 * std::log10(Eb_N0_Linear);

    Bit_Error_Rate =   0.5* std::erfc(std::sqrt(Eb_N0_Linear));

    return Bit_Error_Rate;
}
void Communication_Space_Link::PrintCalculation() const
{
    std::cout << "============ LINK BUDGET RESULTS ============" << std::endl;

    std::cout << "Calculated Wavelength:     " << Wavelength << " meters" << std::endl;
    std::cout << "Free Space Path Loss:      " << FSPL_dB << " dB" << std::endl;
    std::cout << "Earth Antenna Linear Gain: " << Antenna_Gain_Linear<< " (" << (10.0 * std::log10(Antenna_Gain_Linear)) << " dBi)" << std::endl;
    std::cout << "Received Signal Power:     " << Received_Power_Watts << " Watts" << std::endl;
    std::cout << "Thermal Noise Power:      " << Noise_Power << " Watts" << std::endl;
    std::cout << "Channel Capacity:         "  << Channel_Capacity << " bps" << std::endl;
    std::cout<< "Current Data Rate:         " << Current_Data_Rate<< " bits/second"<< std::endl;
    std::cout << "Signal Travel Time:       " << Signal_Travel_Time << " seconds" << std::endl;
    std::cout<< "Eb/N0 Linear: " << Eb_N0_Linear<< std::endl;
    std::cout << "Eb/N0: "<< Eb_N0_dB << " dB" << std::endl;  
    std::cout << "Bit Error Rate (BER):     " << Bit_Error_Rate << std::endl;
    std::cout << "Signal-to-Noise Ratio:    " << Signal_To_Noise_Ratio<< std::endl;
    
    std::cout << "=============================================" << std::endl;
}

