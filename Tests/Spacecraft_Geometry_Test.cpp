#include <iostream>
#include <Eigen/Dense>
#include "Spacecraft_Geometry.h"

int main()
{
    Spacecraft_Geometry geometry;

    double result =
        geometry.Calculate_Greenwich_Sidereal_Time(
            -0.1,   // starting GMST
             0.0    // elapsed seconds
        );

    std::cout << "Greenwich Sidereal Time: "
              << result
              << std::endl;

    return 0;
}