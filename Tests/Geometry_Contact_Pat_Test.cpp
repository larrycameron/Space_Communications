#include "Kepler_Physics_Engine.h"
#include "Spacecraft_Geometry.h"
#include "Contact_Window.h"

#include <Eigen/Dense>
#include <iostream>
#include <cmath>

int main()
{
    Keplers_Physics_Engine KPE;
    Spacecraft_Geometry geometry;
    Contact_Window contact;

    const double PI = 3.141592653589793;

    const double scalar_tolerance = 1.0e-9;
    const double mean_motion_tolerance = 1.0e-11;
    const double mean_anomaly_tolerance = 1.0e-6;
    const double position_tolerance = 0.001;

    int tests_passed = 0;
    const int total_tests = 6;


    // ============================================================
    // CONTROLLED HELIOCENTRIC TEST PARAMETERS
    // ============================================================

    const double sun_mass = 1.989e30;

    // km
    const double semi_major_axis =
        149600000.0;

    const double eccentricity =
        0.0167;

    // Initial mean anomaly
    const double M0 =
        0.50;

    // Propagation time
    const double delta_time =
        86400.0;      // 1 day in seconds


    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "TIME-VARYING KEPLER INTEGRATION BATCH\n";
    std::cout << "KGC-008 THROUGH KGC-013\n";
    std::cout << "========================================\n\n";


    // ============================================================
    // KGC-008
    // MEAN ANOMALY PROPAGATION
    //
    // M(t) = M0 + n * t
    // ============================================================

    double mean_motion =
        KPE.Calculate_Mean_Motion(
            sun_mass,
            semi_major_axis);

    double calculated_M1 =
        M0 +
        mean_motion * delta_time;

    calculated_M1 =
        std::fmod(
            calculated_M1,
            2.0 * PI);

    if (calculated_M1 < 0.0)
    {
        calculated_M1 +=
            2.0 * PI;
    }


    // Independent calculation of expected mean motion
    const double G =
        6.67430e-11;

    double semi_major_axis_m =
        semi_major_axis * 1000.0;

    double expected_mean_motion =
        std::sqrt(
            (G * sun_mass) /
            std::pow(
                semi_major_axis_m,
                3.0));

    double expected_M1 =
        M0 +
        expected_mean_motion *
        delta_time;

    expected_M1 =
        std::fmod(
            expected_M1,
            2.0 * PI);

    if (expected_M1 < 0.0)
    {
        expected_M1 +=
            2.0 * PI;
    }


    double mean_motion_error =
        std::abs(
            mean_motion -
            expected_mean_motion);

    double mean_anomaly_error =
        std::abs(
            calculated_M1 -
            expected_M1);


    bool kgc008_pass =
        mean_motion_error <= mean_motion_tolerance &&
        mean_anomaly_error <= mean_anomaly_tolerance &&
        std::abs(calculated_M1 - M0) > scalar_tolerance;


    std::cout << "----------------------------------------\n";
    std::cout << "KGC-008 MEAN ANOMALY PROPAGATION\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Mean Motion Tolerance:    " << mean_motion_tolerance << " rad/s\n";

    std::cout << "Mean Anomaly Tolerance:   " << mean_anomaly_tolerance << " rad\n";


    std::cout
        << "Semi-Major Axis:          "
        << semi_major_axis
        << " km\n";

    std::cout
        << "Initial Mean Anomaly M0:  "
        << M0
        << " rad\n";

    std::cout
        << "Propagation Time:         "
        << delta_time
        << " s\n";

    std::cout
        << "Calculated Mean Motion:   "
        << mean_motion
        << " rad/s\n";

    std::cout
        << "Expected Mean Motion:     "
        << expected_mean_motion
        << " rad/s\n";

    std::cout
        << "Mean Motion Error:        "
        << mean_motion_error
        << " rad/s\n";

    std::cout
        << "Calculated M(t):          "
        << calculated_M1
        << " rad\n";

    std::cout
        << "Expected M(t):            "
        << expected_M1
        << " rad\n";

    std::cout
        << "Mean Anomaly Error:       "
        << mean_anomaly_error
        << " rad\n";

    if (kgc008_pass)
    {
        ++tests_passed;
        std::cout << "KGC-008 PASS\n\n";
    }
    else
    {
        std::cout << "KGC-008 FAIL\n\n";
    }


    // ============================================================
    // KGC-009
    // ECCENTRIC ANOMALY AT PROPAGATED TIME
    //
    // Verify:
    // M = E - e sin(E)
    // ============================================================

    double E0 =
        KPE.Calculate_Eccentric_Anomaly(
            M0,
            eccentricity);

    double E1 =
        KPE.Calculate_Eccentric_Anomaly(
            calculated_M1,
            eccentricity);

    double reconstructed_M1 =
        E1 -
        eccentricity *
        std::sin(E1);

    double kepler_residual =
        std::abs(
            reconstructed_M1 -
            calculated_M1);


    bool kgc009_pass =
        kepler_residual <= scalar_tolerance &&
        std::abs(E1 - E0) > scalar_tolerance;


    std::cout << "----------------------------------------\n";
    std::cout << "KGC-009 ECCENTRIC ANOMALY UPDATE\n";
    std::cout << "----------------------------------------\n";

    std::cout
        << "Eccentricity:             "
        << eccentricity
        << "\n";

    std::cout
        << "Initial Mean Anomaly:     "
        << M0
        << " rad\n";

    std::cout
        << "Propagated Mean Anomaly:  "
        << calculated_M1
        << " rad\n";

    std::cout
        << "Initial Eccentric Anom.:  "
        << E0
        << " rad\n";

    std::cout
        << "Propagated Eccentric An.: "
        << E1
        << " rad\n";

    std::cout
        << "Reconstructed M:          "
        << reconstructed_M1
        << " rad\n";

    std::cout
        << "Kepler Equation Residual: "
        << kepler_residual
        << " rad\n";

    std::cout
        << "Tolerance:                "
        << scalar_tolerance
        << "\n";

    if (kgc009_pass)
    {
        ++tests_passed;
        std::cout << "KGC-009 PASS\n\n";
    }
    else
    {
        std::cout << "KGC-009 FAIL\n\n";
    }


    // ============================================================
    // KGC-010
    // TRUE ANOMALY CHANGES WITH TIME
    // ============================================================

    double true_anomaly_0 =
        KPE.Calculate_True_Anomaly(
            E0,
            eccentricity);

    double true_anomaly_1 =
        KPE.Calculate_True_Anomaly(
            E1,
            eccentricity);

    double true_anomaly_change =
        true_anomaly_1 -
        true_anomaly_0;


    bool kgc010_pass =
        std::abs(
            true_anomaly_change)
        > scalar_tolerance;


    std::cout << "----------------------------------------\n";
    std::cout << "KGC-010 TRUE ANOMALY CHANGE\n";
    std::cout << "----------------------------------------\n";

    std::cout
        << "True Anomaly t0:          "
        << true_anomaly_0
        << " rad\n";

    std::cout
        << "True Anomaly t1:          "
        << true_anomaly_1
        << " rad\n";

    std::cout
        << "True Anomaly t0:          "
        << true_anomaly_0 *
           180.0 / PI
        << " degrees\n";

    std::cout
        << "True Anomaly t1:          "
        << true_anomaly_1 *
           180.0 / PI
        << " degrees\n";

    std::cout
        << "Change:                   "
        << true_anomaly_change
        << " rad\n";

    if (kgc010_pass)
    {
        ++tests_passed;
        std::cout << "KGC-010 PASS\n\n";
    }
    else
    {
        std::cout << "KGC-010 FAIL\n\n";
    }


    // ============================================================
    // KGC-011
    // ORBITAL RADIUS CHANGES
    //
    // r = a(1 - e cos E)
    // ============================================================

    double calculated_radius_0 =
        KPE.Calculate_Orbital_Radius(
            semi_major_axis,
            eccentricity,
            E0);

    double calculated_radius_1 =
        KPE.Calculate_Orbital_Radius(
            semi_major_axis,
            eccentricity,
            E1);


    double expected_radius_0 =
        semi_major_axis *
        (1.0 -
         eccentricity *
         std::cos(E0));

    double expected_radius_1 =
        semi_major_axis *
        (1.0 -
         eccentricity *
         std::cos(E1));


    double radius_0_error =
        std::abs(
            calculated_radius_0 -
            expected_radius_0);

    double radius_1_error =
        std::abs(
            calculated_radius_1 -
            expected_radius_1);


    bool kgc011_pass =
        radius_0_error <= position_tolerance &&
        radius_1_error <= position_tolerance &&
        std::abs(
            calculated_radius_1 -
            calculated_radius_0)
        > position_tolerance;


    std::cout << "----------------------------------------\n";
    std::cout << "KGC-011 ORBITAL RADIUS CHANGE\n";
    std::cout << "----------------------------------------\n";

    std::cout
        << "Calculated Radius t0:    "
        << calculated_radius_0
        << " km\n";

    std::cout
        << "Expected Radius t0:      "
        << expected_radius_0
        << " km\n";

    std::cout
        << "Radius t0 Error:         "
        << radius_0_error
        << " km\n";

    std::cout
        << "Calculated Radius t1:    "
        << calculated_radius_1
        << " km\n";

    std::cout
        << "Expected Radius t1:      "
        << expected_radius_1
        << " km\n";

    std::cout
        << "Radius t1 Error:         "
        << radius_1_error
        << " km\n";

    std::cout
        << "Radius Change:           "
        << calculated_radius_1 -
           calculated_radius_0
        << " km\n";

    if (kgc011_pass)
    {
        ++tests_passed;
        std::cout << "KGC-011 PASS\n\n";
    }
    else
    {
        std::cout << "KGC-011 FAIL\n\n";
    }


    // ============================================================
    // KGC-012
    // CARTESIAN POSITION CHANGES WITH TIME
    // ============================================================

    double inclination =
        5.0 * PI / 180.0;

    double longitude_ascending_node =
        15.0 * PI / 180.0;

    double argument_periapsis =
        25.0 * PI / 180.0;


    Eigen::Vector3d position_0 =
        KPE.Calculate_Cartesian_Position(
            calculated_radius_0,
            true_anomaly_0,
            inclination,
            longitude_ascending_node,
            argument_periapsis);


    Eigen::Vector3d position_1 =
        KPE.Calculate_Cartesian_Position(
            calculated_radius_1,
            true_anomaly_1,
            inclination,
            longitude_ascending_node,
            argument_periapsis);


    double position_change =
        (position_1 -
         position_0).norm();


    bool kgc012_pass =
        position_change >
        position_tolerance;


    std::cout << "----------------------------------------\n";
    std::cout << "KGC-012 CARTESIAN POSITION CHANGE\n";
    std::cout << "----------------------------------------\n";

    std::cout
        << "Position t0:            "
        << position_0.transpose()
        << " km\n";

    std::cout
        << "Position t1:            "
        << position_1.transpose()
        << " km\n";

    std::cout
        << "Position Change:        "
        << position_change
        << " km\n";

    std::cout
        << "Minimum Required Change:"
        << position_tolerance
        << " km\n";

    if (kgc012_pass)
    {
        ++tests_passed;
        std::cout << "KGC-012 PASS\n\n";
    }
    else
    {
        std::cout << "KGC-012 FAIL\n\n";
    }


    // ============================================================
    // KGC-013
    // DYNAMIC KEPLER -> GEOMETRY HANDOFF
    //
    // Controlled heliocentric relative-motion test.
    //
    // Earth and spacecraft are propagated independently around
    // the Sun. Their difference is passed into the Earth-centered
    // geometry module.
    //
    // THIS IS NOT A REALISTIC LEO PROPAGATION MODEL.
    // ============================================================

    const double earth_a =
        149600000.0;

    const double spacecraft_a =
        149610000.0;

    const double earth_e =
        0.0167;

    const double spacecraft_e =
        0.0170;

    const double earth_M0 =
        0.50;

    const double spacecraft_M0 =
        0.50004;


    double earth_n =
        KPE.Calculate_Mean_Motion(
            sun_mass,
            earth_a);

    double spacecraft_n =
        KPE.Calculate_Mean_Motion(
            sun_mass,
            spacecraft_a);


    double earth_M1 =
        std::fmod(
            earth_M0 +
            earth_n *
            delta_time,
            2.0 * PI);

    double spacecraft_M1 =
        std::fmod(
            spacecraft_M0 +
            spacecraft_n *
            delta_time,
            2.0 * PI);


    if (earth_M1 < 0.0)
    {
        earth_M1 +=
            2.0 * PI;
    }

    if (spacecraft_M1 < 0.0)
    {
        spacecraft_M1 +=
            2.0 * PI;
    }


    double earth_E0 =
        KPE.Calculate_Eccentric_Anomaly(
            earth_M0,
            earth_e);

    double earth_E1 =
        KPE.Calculate_Eccentric_Anomaly(
            earth_M1,
            earth_e);


    double spacecraft_E0 =
        KPE.Calculate_Eccentric_Anomaly(
            spacecraft_M0,
            spacecraft_e);

    double spacecraft_E1 =
        KPE.Calculate_Eccentric_Anomaly(
            spacecraft_M1,
            spacecraft_e);


    double earth_v0 =
        KPE.Calculate_True_Anomaly(
            earth_E0,
            earth_e);

    double earth_v1 =
        KPE.Calculate_True_Anomaly(
            earth_E1,
            earth_e);


    double spacecraft_v0 =
        KPE.Calculate_True_Anomaly(
            spacecraft_E0,
            spacecraft_e);

    double spacecraft_v1 =
        KPE.Calculate_True_Anomaly(
            spacecraft_E1,
            spacecraft_e);


    double earth_r0 =
        KPE.Calculate_Orbital_Radius(
            earth_a,
            earth_e,
            earth_E0);

    double earth_r1 =
        KPE.Calculate_Orbital_Radius(
            earth_a,
            earth_e,
            earth_E1);


    double spacecraft_r0 =
        KPE.Calculate_Orbital_Radius(
            spacecraft_a,
            spacecraft_e,
            spacecraft_E0);

    double spacecraft_r1 =
        KPE.Calculate_Orbital_Radius(
            spacecraft_a,
            spacecraft_e,
            spacecraft_E1);


    Eigen::Vector3d earth_position_0 =
        KPE.Calculate_Cartesian_Position(
            earth_r0,
            earth_v0,
            0.0,
            0.0,
            0.0);

    Eigen::Vector3d earth_position_1 =
        KPE.Calculate_Cartesian_Position(
            earth_r1,
            earth_v1,
            0.0,
            0.0,
            0.0);


    Eigen::Vector3d spacecraft_position_0 =
        KPE.Calculate_Cartesian_Position(
            spacecraft_r0,
            spacecraft_v0,
            0.0,
            0.0,
            0.0);

    Eigen::Vector3d spacecraft_position_1 =
        KPE.Calculate_Cartesian_Position(
            spacecraft_r1,
            spacecraft_v1,
            0.0,
            0.0,
            0.0);


    // Earth-relative positions
    Eigen::Vector3d relative_position_0 =
        (spacecraft_position_0 -
         earth_position_0) *
        1000.0;

    Eigen::Vector3d relative_position_1 =
        (spacecraft_position_1 -
         earth_position_1) *
        1000.0;


    // Ground station at equator / prime meridian
    Eigen::Vector3d ground_station =
        geometry.Ground_Station_Positions_long_lat_alt(
            0.0,
            0.0,
            0.0);


    Eigen::Vector3d range_vector_0 =
        geometry.Calculate_Range_Vector(
            relative_position_0,
            ground_station);

    Eigen::Vector3d range_vector_1 =
        geometry.Calculate_Range_Vector(
            relative_position_1,
            ground_station);


    double range_0 =
        geometry.Calculate_Range_Magnitude(
            range_vector_0);

    double range_1 =
        geometry.Calculate_Range_Magnitude(
            range_vector_1);


    Eigen::Vector3d sez_0 =
        geometry.Calculate_SEZ_Components(
            range_vector_0,
            0.0,
            0.0);

    Eigen::Vector3d sez_1 =
        geometry.Calculate_SEZ_Components(
            range_vector_1,
            0.0,
            0.0);


    double elevation_0 =
        geometry.Elevation_Angle(
            sez_0.z(),
            range_0);

    double elevation_1 =
        geometry.Elevation_Angle(
            sez_1.z(),
            range_1);


    bool contact_0 =
        contact.Is_Contact_Available(
            elevation_0,
            10.0);

    bool contact_1 =
        contact.Is_Contact_Available(
            elevation_1,
            10.0);


    double relative_position_change =
        (relative_position_1 -
         relative_position_0).norm();

    double range_change =
        std::abs(
            range_1 -
            range_0);

    double elevation_change =
        std::abs(
            elevation_1 -
            elevation_0);


    bool kgc013_pass =
        relative_position_change >
        position_tolerance &&
        range_change >
        position_tolerance;


    std::cout << "----------------------------------------\n";
    std::cout << "KGC-013 DYNAMIC GEOMETRY HANDOFF\n";
    std::cout << "----------------------------------------\n";

    std::cout
        << "Earth Position t0:       "
        << earth_position_0.transpose()
        << " km\n";

    std::cout
        << "Earth Position t1:       "
        << earth_position_1.transpose()
        << " km\n\n";

    std::cout
        << "Spacecraft Position t0:  "
        << spacecraft_position_0.transpose()
        << " km\n";

    std::cout
        << "Spacecraft Position t1:  "
        << spacecraft_position_1.transpose()
        << " km\n\n";

    std::cout
        << "Relative Position t0:    "
        << relative_position_0.transpose()
        << " m\n";

    std::cout
        << "Relative Position t1:    "
        << relative_position_1.transpose()
        << " m\n";

    std::cout
        << "Relative Position Change:"
        << relative_position_change
        << " m\n\n";

    std::cout
        << "Range t0:                "
        << range_0
        << " m\n";

    std::cout
        << "Range t1:                "
        << range_1
        << " m\n";

    std::cout
        << "Range Change:            "
        << range_change
        << " m\n\n";

    std::cout
        << "Elevation t0:            "
        << elevation_0
        << " degrees\n";

    std::cout
        << "Elevation t1:            "
        << elevation_1
        << " degrees\n";

    std::cout
        << "Elevation Change:        "
        << elevation_change
        << " degrees\n\n";

    std::cout
        << "Contact t0:              "
        << (contact_0 ? "YES" : "NO")
        << "\n";

    std::cout
        << "Contact t1:              "
        << (contact_1 ? "YES" : "NO")
        << "\n";


    if (kgc013_pass)
    {
        ++tests_passed;
        std::cout << "KGC-013 PASS\n\n";
    }
    else
    {
        std::cout << "KGC-013 FAIL\n\n";
    }


    // ============================================================
    // FINAL RESULTS
    // ============================================================

    std::cout << "========================================\n";

    std::cout
        << "RESULT: "
        << tests_passed
        << " / "
        << total_tests
        << " tests passed\n";

    if (tests_passed == total_tests)
    {
        std::cout << "BATCH RESULT: PASS\n";
    }
    else
    {
        std::cout << "BATCH RESULT: FAIL\n";
    }

    std::cout << "========================================\n";

    return 0;
}