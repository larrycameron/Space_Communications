#include <iostream>
#include <iomanip>
#include "Spacecraft_Communication_Link.h"

int main()
{
    Communication_Space_Link link;

    std::cout << std::setprecision(12);

    std::cout << "\n=== SCL-023: Final Nominal Regression ===\n";

    link.SetSystemParameters(
        384400000.0,   // meters
        8.4e9          // Hz
    );

    link.CalculateFreeSpacePathLoss();

    link.CalculateEarthAntennaGain(
        0.65,
        34.0
    );

    link.CalculateFriisTransmissionEquation(
        20.0,
        1000.0
    );

    link.ThermalNoisePower(
        100.0,
        1.0e6
    );

    link.Calculate_Speed_of_Light_Delay();

    link.CalculateMaximumDataRate(
        100.0,
        1.0e6
    );

    link.CalculateBitErrorRate(
        1.0e6
    );

    link.PrintCalculation();

    return 0;
}