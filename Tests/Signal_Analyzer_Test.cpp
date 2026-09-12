#include <iostream>
#include <iomanip>
#include <Eigen/Dense>
#include "SignalAnalyzer.h"

int main()
{
    Space_Data_Capture signal;

    std::cout << std::setprecision(12);

    std::cout << "=== SA-030: 90% Speed of Light ===\n";
    std::cout << signal.Relativistic_Doppler_Factor(
        0.90 * 2.998e8, 0.0
    ) << '\n';

    std::cout << "\n=== SA-031: Speed of Light ===\n";
    std::cout << signal.Relativistic_Doppler_Factor(
        2.998e8, 0.0
    ) << '\n';

    std::cout << "\n=== SA-032: Above Speed of Light ===\n";
    std::cout << signal.Relativistic_Doppler_Factor(
        1.10 * 2.998e8, 0.0
    ) << '\n';


    std::cout << "\n=== SA-033: Vector Radial Velocity ===\n";

    Eigen::Vector3d relative_velocity(100.0, 0.0, 0.0);
    Eigen::Vector3d line_of_sight(1.0, 0.0, 0.0);

    std::cout << signal.Calculate_Vector_Radial_Velocity(
        relative_velocity,
        line_of_sight
    ) << " m/s\n";


    std::cout << "\n=== SA-034: Perpendicular Vector Radial Velocity ===\n";

    Eigen::Vector3d relative_velocity_034(100.0, 0.0, 0.0);
    Eigen::Vector3d line_of_sight_034(0.0, 1.0, 0.0);

    std::cout << signal.Calculate_Vector_Radial_Velocity(
        relative_velocity_034,
        line_of_sight_034
    ) << " m/s\n";


    std::cout << "\n=== SA-035: Opposite Vector Radial Velocity ===\n";

    Eigen::Vector3d relative_velocity_035(100.0, 0.0, 0.0);
    Eigen::Vector3d line_of_sight_035(-1.0, 0.0, 0.0);

    std::cout << signal.Calculate_Vector_Radial_Velocity(
        relative_velocity_035,
        line_of_sight_035
    ) << " m/s\n";


    std::cout << "\n=== SA-036: Receding First-Order Doppler ===\n";
    std::cout << signal.Calculate_First_Order_Doppler_Factor(
        100.0
    ) << '\n';


    std::cout << "\n=== SA-037: Zero Radial Velocity Doppler ===\n";
    std::cout << signal.Calculate_First_Order_Doppler_Factor(
        0.0
    ) << '\n';


    std::cout << "\n=== SA-038: Approaching First-Order Doppler ===\n";
    std::cout << signal.Calculate_First_Order_Doppler_Factor(
        -100.0
    ) << '\n';


    std::cout << "\n=== SA-039: Receding Shifted Frequency ===\n";

    double receding_factor =
        signal.Calculate_First_Order_Doppler_Factor(100.0);

    std::cout << signal.Calculate_Doppler_Shifted_Frequency(
        1000000.0,
        receding_factor
    ) << " Hz\n";


    std::cout << "\n=== SA-040: Zero Radial Velocity Shifted Frequency ===\n";

    double zero_factor =
        signal.Calculate_First_Order_Doppler_Factor(0.0);

    std::cout << signal.Calculate_Doppler_Shifted_Frequency(
        1000000.0,
        zero_factor
    ) << " Hz\n";


    std::cout << "\n=== SA-041: Approaching Shifted Frequency ===\n";

    double approaching_factor =
        signal.Calculate_First_Order_Doppler_Factor(-100.0);

    std::cout << signal.Calculate_Doppler_Shifted_Frequency(
        1000000.0,
        approaching_factor
    ) << " Hz\n";


    std::cout << "\n=== SA-042: Relativistic Doppler Theta 0 ===\n";

    double velocity = 0.1 * 2.998e8;
    double theta = 0.0;

    std::cout << signal.Relativistic_Doppler_Factor(
        velocity,
        theta
    ) << '\n';


    std::cout << "\n=== SA-043: Relativistic Doppler Theta PI ===\n";

    double velocity2 = 0.1 * 2.998e8;
    double theta2 = 3.141592653589793;

    std::cout << signal.Relativistic_Doppler_Factor(
        velocity2,
        theta2
    ) << '\n';


    std::cout << "\n=== SA-044: Relativistic Doppler Theta PI/2 ===\n";

    double velocity3 = 0.1 * 2.998e8;
    double theta3 = 3.141592653589793 / 2.0;

    std::cout << signal.Relativistic_Doppler_Factor(
        velocity3,
        theta3
    ) << '\n';


    std::cout << "\n=== SA-045: Specialized Radial Velocity at t=0 ===\n";
    std::cout << signal.Calculate_Radial_Velocity(
        1000.0,
        0.0,
        10000.0
    ) << " m/s\n";


    std::cout << "\n=== SA-046: Specialized Radial Velocity Positive Time ===\n";
    std::cout << signal.Calculate_Radial_Velocity(
        1000.0,
        10.0,
        10000.0
    ) << " m/s\n";


    std::cout << "\n=== SA-047: Specialized Radial Velocity Negative Time ===\n";
    std::cout << signal.Calculate_Radial_Velocity(
        1000.0,
        -10.0,
        10000.0
    ) << " m/s\n";


    std::cout << "\n=== SA-048: Zero Closest-Approach Distance ===\n";
    std::cout << signal.Calculate_Radial_Velocity(
        1000.0,
        10.0,
        0.0
    ) << " m/s\n";


    std::cout << "\n=== SA-049: Relativistic Doppler Zero Velocity ===\n";
    std::cout << signal.Relativistic_Doppler_Factor(
        0.0,
        0.0
    ) << '\n';


    std::cout << "\n=== SA-050: Low-Speed Relativistic Doppler ===\n";
    std::cout << signal.Relativistic_Doppler_Factor(
        100.0,
        3.141592653589793
    ) << '\n';


std::cout << "\n=== SA-051: Dynamic Velocity Zero Acceleration ===\n";
std::cout << signal.Calculate_Dynamic_Relative_Velocity(
    100.0,
    0.0,
    10.0
) << " m/s\n";


std::cout << "\n=== SA-052: Dynamic Velocity Positive Acceleration ===\n";
std::cout << signal.Calculate_Dynamic_Relative_Velocity(
    100.0,
    10.0,
    5.0
) << " m/s\n";


std::cout << "\n=== SA-053: Dynamic Velocity Negative Acceleration ===\n";
std::cout << signal.Calculate_Dynamic_Relative_Velocity(
    100.0,
    -10.0,
    5.0
) << " m/s\n";


std::cout << "\n=== SA-054: Dynamic Velocity Zero Time ===\n";
std::cout << signal.Calculate_Dynamic_Relative_Velocity(
    100.0,
    10.0,
    0.0
) << " m/s\n";


std::cout << "\n=== SA-055: Vector Radial Velocity Diagonal LOS ===\n";

Eigen::Vector3d velocity_055(100.0, 100.0, 0.0);
Eigen::Vector3d los_055(
    1.0 / std::sqrt(2.0),
    1.0 / std::sqrt(2.0),
    0.0
);

std::cout << signal.Calculate_Vector_Radial_Velocity(
    velocity_055,
    los_055
) << " m/s\n";


std::cout << "\n=== SA-056: Vector Radial Velocity Zero Velocity ===\n";

Eigen::Vector3d velocity_056(0.0, 0.0, 0.0);
Eigen::Vector3d los_056(1.0, 0.0, 0.0);

std::cout << signal.Calculate_Vector_Radial_Velocity(
    velocity_056,
    los_056
) << " m/s\n";


std::cout << "\n=== SA-057: Stateless Frequency Factor 0.5 ===\n";

std::cout << signal.Calculate_Doppler_Shifted_Frequency(
    1000000.0,
    0.5
) << " Hz\n";


std::cout << "\n=== SA-058: Stateless Frequency Factor 2.0 ===\n";

std::cout << signal.Calculate_Doppler_Shifted_Frequency(
    1000000.0,
    2.0
) << " Hz\n";















    return 0;
}