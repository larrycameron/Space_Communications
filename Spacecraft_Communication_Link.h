#ifndef SPACE_COMMUNICATION_LINK_H
#define SPACE_COMMUNICATION_LINK_H


class Communication_Space_Link

{

private:

    static constexpr double c{2.99792458e8}; // meter per sec
    static constexpr double PI{3.141592653589793};
    static constexpr double Boltzmanns_Constant{1.380649e-23}; // Jules per Kelvin

    // Internal state variables shared across calculations

    double Distance_Spacecraft_To_Earth{}; // meters
    double Signal_Frequency{};             // Hertz
    double Wavelength{};                   // meters
    double FSPL_Linear{};                  // Linear Path Loss
    double FSPL_dB{};                      // Path Loss in dB
    double Antenna_Gain_Linear{};          // Linear Antenna Gain
    double Antenna_Efficiency{};           // 0.0 to 1.0
    double Dish_Diameter{};                // meters
    double Transmitter_Power_Watts{};      // Watts
    double Gain_Spacecraft_Antenna_Linear{};
    double Gain_Earth_Antenna_Linear{};
    double Received_Power_Watts{};         // Watts
    double Noise_Power{}; //Watts
    double System_Noise_Temperature{};// Kelvins
    double Receiver_Bandwidth{};// Hertz
    double Channel_Capacity{};// bits per second
    double Signal_To_Noise_Ratio{};
    double Signal_Travel_Time{};
    double Bit_Error_Rate{};
    double Eb_N0_Linear{};
    double Eb_N0_dB{};
    double Current_Data_Rate{};

public:

    void SetSystemParameters(double distance_spacecraft_to_earth, double frequency_hz);
    double CalculateFreeSpacePathLoss();
    double CalculateEarthAntennaGain(double efficiency, double diameter_meters);
    double CalculateFriisTransmissionEquation(double tx_power_watts, double tx_antenna_gain_linear);
    double ThermalNoisePower(double system_noise_temperature, double receiver_bandwidth);
    double Calculate_Speed_of_Light_Delay();
    double CalculateMaximumDataRate(double system_noise_temperature, double receiver_bandwidth);
    double CalculateBitErrorRate(double actual_data_rate_bps);
    void PrintCalculation() const;
};

#endif