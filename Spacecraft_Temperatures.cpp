#include "Spacecraft_Temperatures.h"

#include <iostream>
#include <random>


double Spacecraft_Temperatures::Calculate_To_Kelvin(double celsius)
{
    return celsius + kelvin_constant;
}


void Spacecraft_Temperatures::RandomTemperatureGenerator(int count)
{
    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dis(-272.0, 120.0);

    for (int i = 0; i < count; ++i)
    {
        CelsiusTemperatures.push_back(dis(gen));
    }
}


void Spacecraft_Temperatures::Print_Temperatures() const
{
    std::cout
        << "\n================ Temperature Calculations ================"<< std::endl;

    for (double celsius : CelsiusTemperatures)
    {
        double kelvin = celsius + kelvin_constant;

        std::cout<< "Celsius: "<< celsius << " C" << "     Kelvin: " << kelvin << " K" << std::endl;
    }

    std::cout<< "=========================================================="<< std::endl;
}