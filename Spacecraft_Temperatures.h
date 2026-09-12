#ifndef SPACECRAFT_TEMPERATURES_H
#define SPACECRAFT_TEMPERATURES_H

#include <vector>

class Spacecraft_Temperatures
{
private:
    static constexpr double kelvin_constant{273.15};

public:
    std::vector<double> CelsiusTemperatures;

    double Calculate_To_Kelvin(double celsius);

    void RandomTemperatureGenerator(int count);

    void Print_Temperatures() const;
};


#endif