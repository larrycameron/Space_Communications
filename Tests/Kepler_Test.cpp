#include "Kepler_Physics_Engine.h"

#include <Eigen/Dense>
#include <iostream>
#include <cmath>

int main()
{
    Keplers_Physics_Engine kepler;

    // ============================================================
    // KGC-001
    // KEPLER -> EARTH-RELATIVE COORDINATE INTEGRATION
    // ============================================================

    // Controlled test:
    // Earth is 149,600,000 km from the Sun.
    // Spacecraft is 500 km farther from the Sun.
    //
    // Expected Earth-relative spacecraft position:
    //
    // (500 km, 0, 0)
    //
    // After conversion:
    //
    // (500000 m, 0, 0)

    double earth_orbital_radius = 149600000.0;       // km
    double spacecraft_orbital_radius = 149600500.0;  // km

    // Keep orbital orientation simple for this controlled test.
    double true_anomaly = 0.0;
    double inclination = 0.0;
    double longitude_ascending_node = 0.0;
    double argument_of_periapsis = 0.0;

    // ============================================================
    // CALCULATE HELIOCENTRIC POSITIONS
    // ============================================================

    Eigen::Vector3d earth_heliocentric =
        kepler.Calculate_Cartesian_Position(
            earth_orbital_radius,
            true_anomaly,
            inclination,
            longitude_ascending_node,
            argument_of_periapsis);

    Eigen::Vector3d spacecraft_heliocentric =
        kepler.Calculate_Cartesian_Position(
            spacecraft_orbital_radius,
            true_anomaly,
            inclination,
            longitude_ascending_node,
            argument_of_periapsis);

    // ============================================================
    // CONVERT TO EARTH-RELATIVE POSITION
    // ============================================================

    Eigen::Vector3d spacecraft_relative_km =
        spacecraft_heliocentric -
        earth_heliocentric;

    // Convert kilometers to meters.
    Eigen::Vector3d spacecraft_relative_m =
        spacecraft_relative_km * 1000.0;

    // ============================================================
    // EXPECTED RESULT
    // ============================================================

    Eigen::Vector3d expected_position(
        500000.0,
        0.0,
        0.0);

    double error =
        (spacecraft_relative_m -
         expected_position).norm();

    double tolerance = 0.001;

    // ============================================================
    // PRINT RESULTS
    // ============================================================

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "KGC-001 KEPLER -> GEOMETRY HANDOFF TEST\n";
    std::cout << "========================================\n";

    std::cout
        << "Earth heliocentric position (km):\n"
        << earth_heliocentric
        << "\n\n";

    std::cout
        << "Spacecraft heliocentric position (km):\n"
        << spacecraft_heliocentric
        << "\n\n";

    std::cout
        << "Earth-relative spacecraft position (km):\n"
        << spacecraft_relative_km
        << "\n\n";

    std::cout
        << "Earth-relative spacecraft position (m):\n"
        << spacecraft_relative_m
        << "\n\n";

    std::cout
        << "Position Error: "
        << error
        << " meters\n";

    // ============================================================
    // PASS / FAIL
    // ============================================================

    if (error <= tolerance)
    {
        std::cout
            << "KGC-001 PASS - Coordinate handoff correct\n";
    }
    else
    {
        std::cout
            << "KGC-001 FAIL - Coordinate handoff incorrect\n";
    }

    return 0;
}