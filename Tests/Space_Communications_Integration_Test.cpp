#include "Relay_Topology_Space.h"
#include "Relay_Station_Simulation.h"
#include "SignalAnalyzer.h"
#include "Spacecraft_Geometry.h"
#include "Kepler_Physics_Engine.h"
#include "Optical_Communication.h"


#include <Eigen/Dense>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>


int main()
{
    // ============================================================================
    // SIGNAL ANALYZER INTEGRATION BATCH
    // GSA-001 THROUGH GSA-005
    // ============================================================================

    std::cout << std::setprecision(12);

    std::cout << "\n========================================\n";
    std::cout << "SIGNAL ANALYZER INTEGRATION BATCH\n";
    std::cout << "GSA-001 THROUGH GSA-005\n";
    std::cout << "========================================\n";

    Space_Data_Capture signal_analyzer;

    const double C = 2.998e8;
    const double signal_tolerance = 1.0e-9;
    const double frequency_tolerance = 1.0e-6;

    int gsa_tests_passed = 0;
    const int gsa_total_tests = 5;


    // ============================================================================
    // GSA-001
    // Dynamic relative velocity
    //
    // Equation:
    // v = v0 + at
    // ============================================================================

    std::cout << "\n----------------------------------------\n";
    std::cout << "GSA-001 DYNAMIC RELATIVE VELOCITY\n";
    std::cout << "----------------------------------------\n";

    double initial_velocity = 100.0;
    double acceleration = 10.0;
    double time = 5.0;

    double calculated_velocity =
        signal_analyzer.Calculate_Dynamic_Relative_Velocity(
            initial_velocity,
            acceleration,
            time);

    double expected_velocity =
        initial_velocity + acceleration * time;

    double velocity_error =
        std::abs(
            calculated_velocity -
            expected_velocity);

    bool gsa001_pass =
        velocity_error <= signal_tolerance;

    std::cout << "Initial Velocity:          "
              << initial_velocity
              << " m/s\n";

    std::cout << "Acceleration:              "
              << acceleration
              << " m/s^2\n";

    std::cout << "Time:                      "
              << time
              << " s\n";

    std::cout << "Calculated Velocity:       "
              << calculated_velocity
              << " m/s\n";

    std::cout << "Expected Velocity:         "
              << expected_velocity
              << " m/s\n";

    std::cout << "Error:                     "
              << velocity_error
              << " m/s\n";

    std::cout << "Tolerance:                 "
              << signal_tolerance
              << " m/s\n";

    std::cout << "GSA-001: "
              << (gsa001_pass ? "PASS" : "FAIL")
              << '\n';

    if (gsa001_pass)
    {
        gsa_tests_passed++;
    }


    // ============================================================================
    // GSA-002
    // Receding first-order Doppler factor
    //
    // Current Signal Analyzer convention:
    //
    // Doppler Factor = 1 - vr / c
    //
    // Positive radial velocity = receding.
    // ============================================================================

    std::cout << "\n----------------------------------------\n";
    std::cout << "GSA-002 RECEDING DOPPLER FACTOR\n";
    std::cout << "----------------------------------------\n";

    double receding_radial_velocity = 100.0;

    double calculated_receding_factor =
        1.0 -
        (receding_radial_velocity / C);

    double expected_receding_factor =
        0.999999666444296;

    double receding_factor_error =
        std::abs(
            calculated_receding_factor -
            expected_receding_factor);

    bool gsa002_pass =
        receding_factor_error <= signal_tolerance &&
        calculated_receding_factor < 1.0;

    std::cout << "Radial Velocity:           "
              << receding_radial_velocity
              << " m/s\n";

    std::cout << "Calculated Doppler Factor: "
              << calculated_receding_factor
              << '\n';

    std::cout << "Expected Doppler Factor:   "
              << expected_receding_factor
              << '\n';

    std::cout << "Error:                     "
              << receding_factor_error
              << '\n';

    std::cout << "Tolerance:                 "
              << signal_tolerance
              << '\n';

    std::cout << "Physical Condition:        "
              << "factor < 1 for receding motion\n";

    std::cout << "GSA-002: "
              << (gsa002_pass ? "PASS" : "FAIL")
              << '\n';

    if (gsa002_pass)
    {
        gsa_tests_passed++;
    }


    // ============================================================================
    // GSA-003
    // Approaching first-order Doppler factor
    //
    // Negative radial velocity = approaching.
    // ============================================================================

    std::cout << "\n----------------------------------------\n";
    std::cout << "GSA-003 APPROACHING DOPPLER FACTOR\n";
    std::cout << "----------------------------------------\n";

    double approaching_radial_velocity = -100.0;

    double calculated_approaching_factor =
        1.0 -
        (approaching_radial_velocity / C);

    double expected_approaching_factor =
        1.000000333555704;

    double approaching_factor_error =
        std::abs(
            calculated_approaching_factor -
            expected_approaching_factor);

    bool gsa003_pass =
        approaching_factor_error <= signal_tolerance &&
        calculated_approaching_factor > 1.0;

    std::cout << "Radial Velocity:           "
              << approaching_radial_velocity
              << " m/s\n";

    std::cout << "Calculated Doppler Factor: "
              << calculated_approaching_factor
              << '\n';

    std::cout << "Expected Doppler Factor:   "
              << expected_approaching_factor
              << '\n';

    std::cout << "Error:                     "
              << approaching_factor_error
              << '\n';

    std::cout << "Tolerance:                 "
              << signal_tolerance
              << '\n';

    std::cout << "Physical Condition:        "
              << "factor > 1 for approaching motion\n";

    std::cout << "GSA-003: "
              << (gsa003_pass ? "PASS" : "FAIL")
              << '\n';

    if (gsa003_pass)
    {
        gsa_tests_passed++;
    }


    // ============================================================================
    // GSA-004
    // Zero radial velocity
    //
    // No radial motion should give a Doppler factor of 1.
    // ============================================================================

    std::cout << "\n----------------------------------------\n";
    std::cout << "GSA-004 ZERO RADIAL VELOCITY\n";
    std::cout << "----------------------------------------\n";

    double zero_radial_velocity = 0.0;

    double calculated_zero_factor =
        1.0 -
        (zero_radial_velocity / C);

    double expected_zero_factor = 1.0;

    double zero_factor_error =
        std::abs(
            calculated_zero_factor -
            expected_zero_factor);

    bool gsa004_pass =
        zero_factor_error <= signal_tolerance;

    std::cout << "Radial Velocity:           "
              << zero_radial_velocity
              << " m/s\n";

    std::cout << "Calculated Doppler Factor: "
              << calculated_zero_factor
              << '\n';

    std::cout << "Expected Doppler Factor:   "
              << expected_zero_factor
              << '\n';

    std::cout << "Error:                     "
              << zero_factor_error
              << '\n';

    std::cout << "Tolerance:                 "
              << signal_tolerance
              << '\n';

    std::cout << "GSA-004: "
              << (gsa004_pass ? "PASS" : "FAIL")
              << '\n';

    if (gsa004_pass)
    {
        gsa_tests_passed++;
    }


    // ============================================================================
    // GSA-005
    // Receding Doppler shifted frequency
    //
    // Current stateless interface:
    //
    // Calculate_Doppler_Shifted_Frequency(
    //     original_frequency,
    //     doppler_factor)
    // ============================================================================

    std::cout << "\n----------------------------------------\n";
    std::cout << "GSA-005 RECEDING RECEIVED FREQUENCY\n";
    std::cout << "----------------------------------------\n";

    double transmitted_frequency =
        1000000.0;

    double calculated_received_frequency =
        signal_analyzer.Calculate_Doppler_Shifted_Frequency(
            transmitted_frequency,
            calculated_receding_factor);

    double expected_received_frequency =
        transmitted_frequency *
        expected_receding_factor;

    double frequency_error =
        std::abs(
            calculated_received_frequency -
            expected_received_frequency);

    bool gsa005_pass =
        frequency_error <= frequency_tolerance &&
        calculated_received_frequency <
        transmitted_frequency;

    std::cout << "Transmitted Frequency:     "
              << transmitted_frequency
              << " Hz\n";

    std::cout << "Radial Velocity:           "
              << receding_radial_velocity
              << " m/s\n";

    std::cout << "Doppler Factor:            "
              << calculated_receding_factor
              << '\n';

    std::cout << "Calculated Frequency:      "
              << calculated_received_frequency
              << " Hz\n";

    std::cout << "Expected Frequency:        "
              << expected_received_frequency
              << " Hz\n";

    std::cout << "Frequency Error:           "
              << frequency_error
              << " Hz\n";

    std::cout << "Tolerance:                 "
              << frequency_tolerance
              << " Hz\n";

    std::cout << "Physical Condition:        "
              << "received frequency < transmitted frequency\n";

    std::cout << "GSA-005: "
              << (gsa005_pass ? "PASS" : "FAIL")
              << '\n';

    if (gsa005_pass)
    {
        gsa_tests_passed++;
    }


    // ============================================================================
    // BATCH SUMMARY
    // ============================================================================

    std::cout << "\n========================================\n";
    std::cout << "GSA BATCH SUMMARY\n";
    std::cout << "========================================\n";

    std::cout << "RESULT: "
              << gsa_tests_passed
              << " / "
              << gsa_total_tests
              << " tests passed\n";

    std::cout << "BATCH RESULT: "
              << (gsa_tests_passed == gsa_total_tests
                      ? "PASS"
                      : "FAIL")
              << '\n';

    std::cout << "========================================\n";



// ============================================================================
// GEOMETRY -> SIGNAL ANALYZER INTEGRATION
// GSA-006 THROUGH GSA-010
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "GEOMETRY -> SIGNAL ANALYZER INTEGRATION\n";
std::cout << "GSA-006 THROUGH GSA-010\n";
std::cout << "========================================\n";

Spacecraft_Geometry geometry;

int geometry_signal_tests_passed = 0;
const int geometry_signal_total_tests = 5;

const double vector_tolerance = 1.0e-9;
const double radial_velocity_tolerance = 1.0e-9;
const double doppler_tolerance = 1.0e-9;
const double received_frequency_tolerance = 1.0e-6;


// ============================================================================
// GSA-006
// Geometry produces LOS unit vector along +X
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-006 GEOMETRY LOS UNIT VECTOR\n";
std::cout << "----------------------------------------\n";

Eigen::Vector3d range_vector_006(
    1000.0,
    0.0,
    0.0);

Eigen::Vector3d calculated_los_006 =
    geometry.Calculate_Line_Of_Sight(
        range_vector_006);

Eigen::Vector3d expected_los_006(
    1.0,
    0.0,
    0.0);

double los_error_006 =
    (calculated_los_006 -
     expected_los_006).norm();

bool gsa006_pass =
    los_error_006 <= vector_tolerance;

std::cout << "Input Range Vector:        "
          << range_vector_006.transpose()
          << " m\n";

std::cout << "Calculated LOS Vector:     "
          << calculated_los_006.transpose()
          << '\n';

std::cout << "Expected LOS Vector:       "
          << expected_los_006.transpose()
          << '\n';

std::cout << "Vector Error:              "
          << los_error_006
          << '\n';

std::cout << "Tolerance:                 "
          << vector_tolerance
          << '\n';

std::cout << "GSA-006: "
          << (gsa006_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa006_pass)
{
    geometry_signal_tests_passed++;
}


// ============================================================================
// GSA-007
// Relative velocity directly along LOS
//
// v_rel = (100, 0, 0)
// LOS   = (1, 0, 0)
//
// Expected:
// vr = v_rel dot LOS = 100 m/s
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-007 RECEDING VECTOR RADIAL VELOCITY\n";
std::cout << "----------------------------------------\n";

Eigen::Vector3d relative_velocity_007(
    100.0,
    0.0,
    0.0);

double calculated_radial_velocity_007 =
    signal_analyzer.Calculate_Vector_Radial_Velocity(
        relative_velocity_007,
        calculated_los_006);

double expected_radial_velocity_007 =
    relative_velocity_007.dot(
        expected_los_006);

double radial_velocity_error_007 =
    std::abs(
        calculated_radial_velocity_007 -
        expected_radial_velocity_007);

bool gsa007_pass =
    radial_velocity_error_007 <=
    radial_velocity_tolerance;

std::cout << "Relative Velocity Vector:  "
          << relative_velocity_007.transpose()
          << " m/s\n";

std::cout << "Geometry LOS Vector:       "
          << calculated_los_006.transpose()
          << '\n';

std::cout << "Calculated Radial Velocity:"
          << calculated_radial_velocity_007
          << " m/s\n";

std::cout << "Expected Radial Velocity:  "
          << expected_radial_velocity_007
          << " m/s\n";

std::cout << "Error:                     "
          << radial_velocity_error_007
          << " m/s\n";

std::cout << "Tolerance:                 "
          << radial_velocity_tolerance
          << " m/s\n";

std::cout << "Physical Meaning:          "
          << "positive = receding\n";

std::cout << "GSA-007: "
          << (gsa007_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa007_pass)
{
    geometry_signal_tests_passed++;
}


// ============================================================================
// GSA-008
// Velocity perpendicular to LOS
//
// v_rel = (0, 100, 0)
// LOS   = (1, 0, 0)
//
// Expected:
// vr = 0 m/s
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-008 TRANSVERSE VELOCITY\n";
std::cout << "----------------------------------------\n";

Eigen::Vector3d relative_velocity_008(
    0.0,
    100.0,
    0.0);

double calculated_radial_velocity_008 =
    signal_analyzer.Calculate_Vector_Radial_Velocity(
        relative_velocity_008,
        calculated_los_006);

double expected_radial_velocity_008 =
    relative_velocity_008.dot(
        expected_los_006);

double radial_velocity_error_008 =
    std::abs(
        calculated_radial_velocity_008 -
        expected_radial_velocity_008);

bool gsa008_pass =
    radial_velocity_error_008 <=
    radial_velocity_tolerance;

std::cout << "Relative Velocity Vector:  "
          << relative_velocity_008.transpose()
          << " m/s\n";

std::cout << "Geometry LOS Vector:       "
          << calculated_los_006.transpose()
          << '\n';

std::cout << "Calculated Radial Velocity:"
          << calculated_radial_velocity_008
          << " m/s\n";

std::cout << "Expected Radial Velocity:  "
          << expected_radial_velocity_008
          << " m/s\n";

std::cout << "Error:                     "
          << radial_velocity_error_008
          << " m/s\n";

std::cout << "Tolerance:                 "
          << radial_velocity_tolerance
          << " m/s\n";

std::cout << "Physical Meaning:          "
          << "motion is perpendicular to LOS\n";

std::cout << "GSA-008: "
          << (gsa008_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa008_pass)
{
    geometry_signal_tests_passed++;
}


// ============================================================================
// GSA-009
// Approaching velocity
//
// v_rel = (-100, 0, 0)
// LOS   = (1, 0, 0)
//
// Expected:
// vr = -100 m/s
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-009 APPROACHING VECTOR RADIAL VELOCITY\n";
std::cout << "----------------------------------------\n";

Eigen::Vector3d relative_velocity_009(
    -100.0,
    0.0,
    0.0);

double calculated_radial_velocity_009 =
    signal_analyzer.Calculate_Vector_Radial_Velocity(
        relative_velocity_009,
        calculated_los_006);

double expected_radial_velocity_009 =
    relative_velocity_009.dot(
        expected_los_006);

double radial_velocity_error_009 =
    std::abs(
        calculated_radial_velocity_009 -
        expected_radial_velocity_009);

bool gsa009_pass =
    radial_velocity_error_009 <=
    radial_velocity_tolerance;

std::cout << "Relative Velocity Vector:  "
          << relative_velocity_009.transpose()
          << " m/s\n";

std::cout << "Geometry LOS Vector:       "
          << calculated_los_006.transpose()
          << '\n';

std::cout << "Calculated Radial Velocity:"
          << calculated_radial_velocity_009
          << " m/s\n";

std::cout << "Expected Radial Velocity:  "
          << expected_radial_velocity_009
          << " m/s\n";

std::cout << "Error:                     "
          << radial_velocity_error_009
          << " m/s\n";

std::cout << "Tolerance:                 "
          << radial_velocity_tolerance
          << " m/s\n";

std::cout << "Physical Meaning:          "
          << "negative = approaching\n";

std::cout << "GSA-009: "
          << (gsa009_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa009_pass)
{
    geometry_signal_tests_passed++;
}


// ============================================================================
// GSA-010
// COMPLETE GEOMETRY -> RADIAL VELOCITY -> DOPPLER -> FREQUENCY HANDOFF
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-010 FULL LOS TO DOPPLER HANDOFF\n";
std::cout << "----------------------------------------\n";

// Geometry produced this LOS.
Eigen::Vector3d range_vector_010(
    3000.0,
    4000.0,
    0.0);

Eigen::Vector3d calculated_los_010 =
    geometry.Calculate_Line_Of_Sight(
        range_vector_010);

// Independent expected unit vector:
//
// magnitude = sqrt(3000^2 + 4000^2)
//           = 5000
//
// LOS = (0.6, 0.8, 0)

Eigen::Vector3d expected_los_010(
    0.6,
    0.8,
    0.0);

double los_error_010 =
    (calculated_los_010 -
     expected_los_010).norm();


// Relative velocity vector.
Eigen::Vector3d relative_velocity_010(
    100.0,
    200.0,
    0.0);


// Signal Analyzer calculates radial velocity.
double calculated_radial_velocity_010 =
    signal_analyzer.Calculate_Vector_Radial_Velocity(
        relative_velocity_010,
        calculated_los_010);


// Independent expected radial velocity:
//
// vr = (100)(0.6) + (200)(0.8)
// vr = 60 + 160
// vr = 220 m/s

double expected_radial_velocity_010 =
    relative_velocity_010.dot(
        expected_los_010);

double radial_velocity_error_010 =
    std::abs(
        calculated_radial_velocity_010 -
        expected_radial_velocity_010);


// Doppler factor.
//
// Positive radial velocity means receding.
//
// factor = 1 - vr/c

double calculated_doppler_factor_010 =
    1.0 -
    (calculated_radial_velocity_010 / C);

double expected_doppler_factor_010 =
    1.0 -
    (expected_radial_velocity_010 / C);

double doppler_error_010 =
    std::abs(
        calculated_doppler_factor_010 -
        expected_doppler_factor_010);


// Received frequency.
double transmitted_frequency_010 =
    1000000.0;

double calculated_received_frequency_010 =
    signal_analyzer.Calculate_Doppler_Shifted_Frequency(
        transmitted_frequency_010,
        calculated_doppler_factor_010);

double expected_received_frequency_010 =
    transmitted_frequency_010 *
    expected_doppler_factor_010;

double frequency_error_010 =
    std::abs(
        calculated_received_frequency_010 -
        expected_received_frequency_010);


bool gsa010_pass =
    los_error_010 <= vector_tolerance &&
    radial_velocity_error_010 <= radial_velocity_tolerance &&
    doppler_error_010 <= doppler_tolerance &&
    frequency_error_010 <= received_frequency_tolerance &&
    calculated_radial_velocity_010 > 0.0 &&
    calculated_received_frequency_010 <
        transmitted_frequency_010;


std::cout << "Input Range Vector:        "
          << range_vector_010.transpose()
          << " m\n";

std::cout << "Calculated LOS Vector:     "
          << calculated_los_010.transpose()
          << '\n';

std::cout << "Expected LOS Vector:       "
          << expected_los_010.transpose()
          << '\n';

std::cout << "LOS Vector Error:          "
          << los_error_010
          << '\n';

std::cout << "\n";

std::cout << "Relative Velocity Vector:  "
          << relative_velocity_010.transpose()
          << " m/s\n";

std::cout << "Calculated Radial Velocity:"
          << calculated_radial_velocity_010
          << " m/s\n";

std::cout << "Expected Radial Velocity:  "
          << expected_radial_velocity_010
          << " m/s\n";

std::cout << "Radial Velocity Error:     "
          << radial_velocity_error_010
          << " m/s\n";

std::cout << "\n";

std::cout << "Calculated Doppler Factor: "
          << calculated_doppler_factor_010
          << '\n';

std::cout << "Expected Doppler Factor:   "
          << expected_doppler_factor_010
          << '\n';

std::cout << "Doppler Factor Error:      "
          << doppler_error_010
          << '\n';

std::cout << "\n";

std::cout << "Transmitted Frequency:     "
          << transmitted_frequency_010
          << " Hz\n";

std::cout << "Calculated Frequency:      "
          << calculated_received_frequency_010
          << " Hz\n";

std::cout << "Expected Frequency:        "
          << expected_received_frequency_010
          << " Hz\n";

std::cout << "Frequency Error:           "
          << frequency_error_010
          << " Hz\n";

std::cout << "\n";

std::cout << "LOS Tolerance:             "
          << vector_tolerance
          << '\n';

std::cout << "Radial Velocity Tolerance: "
          << radial_velocity_tolerance
          << " m/s\n";

std::cout << "Doppler Tolerance:         "
          << doppler_tolerance
          << '\n';

std::cout << "Frequency Tolerance:       "
          << received_frequency_tolerance
          << " Hz\n";

std::cout << "GSA-010: "
          << (gsa010_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa010_pass)
{
    geometry_signal_tests_passed++;
}


// ============================================================================
// GSA-006 THROUGH GSA-010 SUMMARY
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "GEOMETRY -> SIGNAL ANALYZER SUMMARY\n";
std::cout << "========================================\n";

std::cout << "RESULT: "
          << geometry_signal_tests_passed
          << " / "
          << geometry_signal_total_tests
          << " tests passed\n";

std::cout << "BATCH RESULT: "
          << (geometry_signal_tests_passed ==
                      geometry_signal_total_tests
                  ? "PASS"
                  : "FAIL")
          << '\n';

std::cout << "========================================\n";


// ============================================================================
// DYNAMIC GEOMETRY -> SIGNAL ANALYZER INTEGRATION
// GSA-011 THROUGH GSA-015
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "DYNAMIC GEOMETRY -> SIGNAL ANALYZER\n";
std::cout << "GSA-011 THROUGH GSA-015\n";
std::cout << "========================================\n";

int dynamic_signal_tests_passed = 0;
const int dynamic_signal_total_tests = 5;

const double dynamic_vector_tolerance = 1.0e-9;
const double dynamic_velocity_tolerance = 1.0e-9;
const double dynamic_doppler_tolerance = 1.0e-9;
const double dynamic_frequency_tolerance = 1.0e-6;


// ============================================================================
// GSA-011
// TWO POSITIONS -> RELATIVE VELOCITY
//
// Position t0 = (1000, 2000, 0) m
// Position t1 = (2000, 4000, 0) m
// delta t = 10 s
//
// delta r = (1000, 2000, 0) m
//
// v = delta r / delta t
//   = (100, 200, 0) m/s
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-011 POSITION CHANGE -> VELOCITY\n";
std::cout << "----------------------------------------\n";

Eigen::Vector3d position_t0_011(
    1000.0,
    2000.0,
    0.0);

Eigen::Vector3d position_t1_011(
    2000.0,
    4000.0,
    0.0);

double delta_time_011 = 10.0;

Eigen::Vector3d calculated_velocity_011 =
    (position_t1_011 - position_t0_011) /
    delta_time_011;

Eigen::Vector3d expected_velocity_011(
    100.0,
    200.0,
    0.0);

double velocity_error_011 =
    (calculated_velocity_011 -
     expected_velocity_011).norm();

bool gsa011_pass =
    velocity_error_011 <= dynamic_velocity_tolerance;

std::cout << "Position t0:               "
          << position_t0_011.transpose()
          << " m\n";

std::cout << "Position t1:               "
          << position_t1_011.transpose()
          << " m\n";

std::cout << "Time Difference:           "
          << delta_time_011
          << " s\n";

std::cout << "Position Change:           "
          << (position_t1_011 -
              position_t0_011).transpose()
          << " m\n";

std::cout << "Calculated Velocity:       "
          << calculated_velocity_011.transpose()
          << " m/s\n";

std::cout << "Expected Velocity:         "
          << expected_velocity_011.transpose()
          << " m/s\n";

std::cout << "Velocity Vector Error:     "
          << velocity_error_011
          << " m/s\n";

std::cout << "Tolerance:                 "
          << dynamic_velocity_tolerance
          << " m/s\n";

std::cout << "GSA-011: "
          << (gsa011_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa011_pass)
{
    dynamic_signal_tests_passed++;
}


// ============================================================================
// GSA-012
// CURRENT POSITION -> GEOMETRY LOS VECTOR
//
// Position = (3000, 4000, 0)
//
// Magnitude = 5000 m
//
// Expected LOS = (0.6, 0.8, 0)
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-012 CURRENT POSITION -> LOS\n";
std::cout << "----------------------------------------\n";

Eigen::Vector3d current_range_012(
    3000.0,
    4000.0,
    0.0);

Eigen::Vector3d calculated_los_012 =
    geometry.Calculate_Line_Of_Sight(
        current_range_012);

Eigen::Vector3d expected_los_012(
    0.6,
    0.8,
    0.0);

double los_error_012 =
    (calculated_los_012 -
     expected_los_012).norm();

bool gsa012_pass =
    los_error_012 <= dynamic_vector_tolerance;

std::cout << "Current Range Vector:      "
          << current_range_012.transpose()
          << " m\n";

std::cout << "Range Magnitude:           "
          << current_range_012.norm()
          << " m\n";

std::cout << "Calculated LOS Vector:     "
          << calculated_los_012.transpose()
          << '\n';

std::cout << "Expected LOS Vector:       "
          << expected_los_012.transpose()
          << '\n';

std::cout << "LOS Vector Error:          "
          << los_error_012
          << '\n';

std::cout << "Tolerance:                 "
          << dynamic_vector_tolerance
          << '\n';

std::cout << "GSA-012: "
          << (gsa012_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa012_pass)
{
    dynamic_signal_tests_passed++;
}


// ============================================================================
// GSA-013
// DYNAMIC VELOCITY -> RADIAL VELOCITY
//
// Velocity from GSA-011:
//     (100, 200, 0) m/s
//
// LOS from GSA-012:
//     (0.6, 0.8, 0)
//
// vr = v dot LOS
//
// vr = (100)(0.6) + (200)(0.8)
// vr = 60 + 160
// vr = 220 m/s
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-013 DYNAMIC RADIAL VELOCITY\n";
std::cout << "----------------------------------------\n";

double calculated_radial_velocity_013 =
    signal_analyzer.Calculate_Vector_Radial_Velocity(
        calculated_velocity_011,
        calculated_los_012);

double expected_radial_velocity_013 =
    expected_velocity_011.dot(
        expected_los_012);

double radial_velocity_error_013 =
    std::abs(
        calculated_radial_velocity_013 -
        expected_radial_velocity_013);

bool gsa013_pass =
    radial_velocity_error_013 <=
        dynamic_velocity_tolerance &&
    calculated_radial_velocity_013 > 0.0;

std::cout << "Dynamic Velocity Vector:   "
          << calculated_velocity_011.transpose()
          << " m/s\n";

std::cout << "Geometry LOS Vector:       "
          << calculated_los_012.transpose()
          << '\n';

std::cout << "Calculated Radial Velocity:"
          << calculated_radial_velocity_013
          << " m/s\n";

std::cout << "Expected Radial Velocity:  "
          << expected_radial_velocity_013
          << " m/s\n";

std::cout << "Error:                     "
          << radial_velocity_error_013
          << " m/s\n";

std::cout << "Tolerance:                 "
          << dynamic_velocity_tolerance
          << " m/s\n";

std::cout << "Physical Meaning:          "
          << "positive radial velocity = receding\n";

std::cout << "GSA-013: "
          << (gsa013_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa013_pass)
{
    dynamic_signal_tests_passed++;
}


// ============================================================================
// GSA-014
// DYNAMIC RADIAL VELOCITY -> DOPPLER FACTOR
//
// factor = 1 - vr / c
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-014 DYNAMIC DOPPLER FACTOR\n";
std::cout << "----------------------------------------\n";

double calculated_doppler_factor_014 =
    1.0 -
    (calculated_radial_velocity_013 / C);

double expected_doppler_factor_014 =
    1.0 -
    (220.0 / C);

double doppler_error_014 =
    std::abs(
        calculated_doppler_factor_014 -
        expected_doppler_factor_014);

bool gsa014_pass =
    doppler_error_014 <= dynamic_doppler_tolerance &&
    calculated_doppler_factor_014 < 1.0;

std::cout << "Radial Velocity:           "
          << calculated_radial_velocity_013
          << " m/s\n";

std::cout << "Speed of Light:            "
          << C
          << " m/s\n";

std::cout << "Calculated Doppler Factor: "
          << calculated_doppler_factor_014
          << '\n';

std::cout << "Expected Doppler Factor:   "
          << expected_doppler_factor_014
          << '\n';

std::cout << "Error:                     "
          << doppler_error_014
          << '\n';

std::cout << "Tolerance:                 "
          << dynamic_doppler_tolerance
          << '\n';

std::cout << "Physical Condition:        "
          << "factor < 1 for receding motion\n";

std::cout << "GSA-014: "
          << (gsa014_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa014_pass)
{
    dynamic_signal_tests_passed++;
}


// ============================================================================
// GSA-015
// COMPLETE DYNAMIC SIGNAL HANDOFF
//
// position change
//      ->
// velocity
//      ->
// LOS
//      ->
// radial velocity
//      ->
// Doppler factor
//      ->
// received frequency
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-015 FULL DYNAMIC DOPPLER HANDOFF\n";
std::cout << "----------------------------------------\n";

double transmitted_frequency_015 =
    1000000.0;

double calculated_received_frequency_015 =
    signal_analyzer.Calculate_Doppler_Shifted_Frequency(
        transmitted_frequency_015,
        calculated_doppler_factor_014);

double expected_received_frequency_015 =
    transmitted_frequency_015 *
    expected_doppler_factor_014;

double frequency_error_015 =
    std::abs(
        calculated_received_frequency_015 -
        expected_received_frequency_015);

bool gsa015_pass =
    velocity_error_011 <= dynamic_velocity_tolerance &&
    los_error_012 <= dynamic_vector_tolerance &&
    radial_velocity_error_013 <= dynamic_velocity_tolerance &&
    doppler_error_014 <= dynamic_doppler_tolerance &&
    frequency_error_015 <= dynamic_frequency_tolerance &&
    calculated_received_frequency_015 <
        transmitted_frequency_015;

std::cout << "Position t0:               "
          << position_t0_011.transpose()
          << " m\n";

std::cout << "Position t1:               "
          << position_t1_011.transpose()
          << " m\n";

std::cout << "Delta Time:                "
          << delta_time_011
          << " s\n";

std::cout << "\n";

std::cout << "Calculated Velocity:       "
          << calculated_velocity_011.transpose()
          << " m/s\n";

std::cout << "Expected Velocity:         "
          << expected_velocity_011.transpose()
          << " m/s\n";

std::cout << "Velocity Error:            "
          << velocity_error_011
          << " m/s\n";

std::cout << "\n";

std::cout << "Range Vector:              "
          << current_range_012.transpose()
          << " m\n";

std::cout << "Calculated LOS:            "
          << calculated_los_012.transpose()
          << '\n';

std::cout << "Expected LOS:              "
          << expected_los_012.transpose()
          << '\n';

std::cout << "LOS Error:                 "
          << los_error_012
          << '\n';

std::cout << "\n";

std::cout << "Calculated Radial Velocity:"
          << calculated_radial_velocity_013
          << " m/s\n";

std::cout << "Expected Radial Velocity:  "
          << expected_radial_velocity_013
          << " m/s\n";

std::cout << "Radial Velocity Error:     "
          << radial_velocity_error_013
          << " m/s\n";

std::cout << "\n";

std::cout << "Calculated Doppler Factor: "
          << calculated_doppler_factor_014
          << '\n';

std::cout << "Expected Doppler Factor:   "
          << expected_doppler_factor_014
          << '\n';

std::cout << "Doppler Error:             "
          << doppler_error_014
          << '\n';

std::cout << "\n";

std::cout << "Transmitted Frequency:     "
          << transmitted_frequency_015
          << " Hz\n";

std::cout << "Calculated Frequency:      "
          << calculated_received_frequency_015
          << " Hz\n";

std::cout << "Expected Frequency:        "
          << expected_received_frequency_015
          << " Hz\n";

std::cout << "Frequency Error:           "
          << frequency_error_015
          << " Hz\n";

std::cout << "\n";

std::cout << "Velocity Tolerance:        "
          << dynamic_velocity_tolerance
          << " m/s\n";

std::cout << "LOS Tolerance:             "
          << dynamic_vector_tolerance
          << '\n';

std::cout << "Doppler Tolerance:         "
          << dynamic_doppler_tolerance
          << '\n';

std::cout << "Frequency Tolerance:       "
          << dynamic_frequency_tolerance
          << " Hz\n";

std::cout << "GSA-015: "
          << (gsa015_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa015_pass)
{
    dynamic_signal_tests_passed++;
}


// ============================================================================
// GSA-011 THROUGH GSA-015 SUMMARY
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "DYNAMIC SIGNAL INTEGRATION SUMMARY\n";
std::cout << "========================================\n";

std::cout << "RESULT: "
          << dynamic_signal_tests_passed
          << " / "
          << dynamic_signal_total_tests
          << " tests passed\n";

std::cout << "BATCH RESULT: "
          << (dynamic_signal_tests_passed ==
                      dynamic_signal_total_tests
                  ? "PASS"
                  : "FAIL")
          << '\n';

std::cout << "========================================\n";


// ============================================================================
// KEPLER -> GEOMETRY -> SIGNAL ANALYZER INTEGRATION
// GSA-016 THROUGH GSA-020
//
// Controlled heliocentric relative-motion test.
//
// Earth and spacecraft are propagated independently around the Sun.
// Spacecraft heliocentric position - Earth heliocentric position
// produces the Earth-relative position.
//
// THIS IS NOT A REALISTIC LEO PROPAGATION MODEL.
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "KEPLER -> GEOMETRY -> SIGNAL ANALYZER\n";
std::cout << "GSA-016 THROUGH GSA-020\n";
std::cout << "========================================\n";

Keplers_Physics_Engine KPE_GSA;

int kepler_signal_tests_passed = 0;
const int kepler_signal_total_tests = 5;

const double PI_GSA =
    3.141592653589793;

const double sun_mass_GSA =
    1.989e30;

// Same controlled orbital inputs used in KGC-013.
const double earth_a_GSA =
    149600000.0;       // km

const double spacecraft_a_GSA =
    149610000.0;       // km

const double earth_e_GSA =
    0.0167;

const double spacecraft_e_GSA =
    0.0170;

const double earth_M0_GSA =
    0.50;

const double spacecraft_M0_GSA =
    0.50004;

const double delta_time_GSA =
    86400.0;           // seconds


// ============================================================================
// PROPAGATE EARTH
// ============================================================================

double earth_n_GSA =
    KPE_GSA.Calculate_Mean_Motion(
        sun_mass_GSA,
        earth_a_GSA);

double earth_M1_GSA =
    std::fmod(
        earth_M0_GSA +
        earth_n_GSA *
        delta_time_GSA,
        2.0 * PI_GSA);

if (earth_M1_GSA < 0.0)
{
    earth_M1_GSA +=
        2.0 * PI_GSA;
}

double earth_E0_GSA =
    KPE_GSA.Calculate_Eccentric_Anomaly(
        earth_M0_GSA,
        earth_e_GSA);

double earth_E1_GSA =
    KPE_GSA.Calculate_Eccentric_Anomaly(
        earth_M1_GSA,
        earth_e_GSA);

double earth_true_anomaly_0_GSA =
    KPE_GSA.Calculate_True_Anomaly(
        earth_E0_GSA,
        earth_e_GSA);

double earth_true_anomaly_1_GSA =
    KPE_GSA.Calculate_True_Anomaly(
        earth_E1_GSA,
        earth_e_GSA);

double earth_radius_0_GSA =
    KPE_GSA.Calculate_Orbital_Radius(
        earth_a_GSA,
        earth_e_GSA,
        earth_E0_GSA);

double earth_radius_1_GSA =
    KPE_GSA.Calculate_Orbital_Radius(
        earth_a_GSA,
        earth_e_GSA,
        earth_E1_GSA);

Eigen::Vector3d earth_position_0_GSA =
    KPE_GSA.Calculate_Cartesian_Position(
        earth_radius_0_GSA,
        earth_true_anomaly_0_GSA,
        0.0,
        0.0,
        0.0);

Eigen::Vector3d earth_position_1_GSA =
    KPE_GSA.Calculate_Cartesian_Position(
        earth_radius_1_GSA,
        earth_true_anomaly_1_GSA,
        0.0,
        0.0,
        0.0);


// ============================================================================
// PROPAGATE SPACECRAFT
// ============================================================================

double spacecraft_n_GSA =
    KPE_GSA.Calculate_Mean_Motion(
        sun_mass_GSA,
        spacecraft_a_GSA);

double spacecraft_M1_GSA =
    std::fmod(
        spacecraft_M0_GSA +
        spacecraft_n_GSA *
        delta_time_GSA,
        2.0 * PI_GSA);

if (spacecraft_M1_GSA < 0.0)
{
    spacecraft_M1_GSA +=
        2.0 * PI_GSA;
}

double spacecraft_E0_GSA =
    KPE_GSA.Calculate_Eccentric_Anomaly(
        spacecraft_M0_GSA,
        spacecraft_e_GSA);

double spacecraft_E1_GSA =
    KPE_GSA.Calculate_Eccentric_Anomaly(
        spacecraft_M1_GSA,
        spacecraft_e_GSA);

double spacecraft_true_anomaly_0_GSA =
    KPE_GSA.Calculate_True_Anomaly(
        spacecraft_E0_GSA,
        spacecraft_e_GSA);

double spacecraft_true_anomaly_1_GSA =
    KPE_GSA.Calculate_True_Anomaly(
        spacecraft_E1_GSA,
        spacecraft_e_GSA);

double spacecraft_radius_0_GSA =
    KPE_GSA.Calculate_Orbital_Radius(
        spacecraft_a_GSA,
        spacecraft_e_GSA,
        spacecraft_E0_GSA);

double spacecraft_radius_1_GSA =
    KPE_GSA.Calculate_Orbital_Radius(
        spacecraft_a_GSA,
        spacecraft_e_GSA,
        spacecraft_E1_GSA);

Eigen::Vector3d spacecraft_position_0_GSA =
    KPE_GSA.Calculate_Cartesian_Position(
        spacecraft_radius_0_GSA,
        spacecraft_true_anomaly_0_GSA,
        0.0,
        0.0,
        0.0);

Eigen::Vector3d spacecraft_position_1_GSA =
    KPE_GSA.Calculate_Cartesian_Position(
        spacecraft_radius_1_GSA,
        spacecraft_true_anomaly_1_GSA,
        0.0,
        0.0,
        0.0);


// ============================================================================
// GSA-016
// KEPLER POSITION PROPAGATION
//
// We are not re-validating Kepler mathematics here.
// That was already done in the KGC batch.
//
// Integration requirement:
//
// Earth position must change.
// Spacecraft position must change.
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-016 KEPLER POSITION PROPAGATION\n";
std::cout << "----------------------------------------\n";

double earth_position_change_GSA =
    (earth_position_1_GSA -
     earth_position_0_GSA).norm();

double spacecraft_position_change_GSA =
    (spacecraft_position_1_GSA -
     spacecraft_position_0_GSA).norm();

const double minimum_position_change_GSA =
    0.001;      // km

bool gsa016_pass =
    earth_position_change_GSA >
        minimum_position_change_GSA &&
    spacecraft_position_change_GSA >
        minimum_position_change_GSA;

std::cout << "Propagation Time:          "
          << delta_time_GSA
          << " s\n";

std::cout << "Earth Position t0:         "
          << earth_position_0_GSA.transpose()
          << " km\n";

std::cout << "Earth Position t1:         "
          << earth_position_1_GSA.transpose()
          << " km\n";

std::cout << "Earth Position Change:     "
          << earth_position_change_GSA
          << " km\n";

std::cout << "\n";

std::cout << "Spacecraft Position t0:    "
          << spacecraft_position_0_GSA.transpose()
          << " km\n";

std::cout << "Spacecraft Position t1:    "
          << spacecraft_position_1_GSA.transpose()
          << " km\n";

std::cout << "Spacecraft Position Change:"
          << spacecraft_position_change_GSA
          << " km\n";

std::cout << "Required Minimum Change:   "
          << minimum_position_change_GSA
          << " km\n";

std::cout << "GSA-016: "
          << (gsa016_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa016_pass)
{
    kepler_signal_tests_passed++;
}


// ============================================================================
// GSA-017
// HELIOCENTRIC -> EARTH-RELATIVE POSITION
//
// r_relative = r_spacecraft - r_earth
//
// Kepler positions are km.
// Signal/Geometry calculations use meters.
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-017 EARTH-RELATIVE POSITION HANDOFF\n";
std::cout << "----------------------------------------\n";

Eigen::Vector3d relative_position_0_GSA =
    (spacecraft_position_0_GSA -
     earth_position_0_GSA) *
    1000.0;

Eigen::Vector3d relative_position_1_GSA =
    (spacecraft_position_1_GSA -
     earth_position_1_GSA) *
    1000.0;


// Independent handoff check:
//
// Convert the relative meter vectors back to km.
// They should equal the original heliocentric subtraction.

Eigen::Vector3d recovered_relative_km_0_GSA =
    relative_position_0_GSA /
    1000.0;

Eigen::Vector3d recovered_relative_km_1_GSA =
    relative_position_1_GSA /
    1000.0;

Eigen::Vector3d expected_relative_km_0_GSA =
    spacecraft_position_0_GSA -
    earth_position_0_GSA;

Eigen::Vector3d expected_relative_km_1_GSA =
    spacecraft_position_1_GSA -
    earth_position_1_GSA;

double relative_conversion_error_0_GSA =
    (recovered_relative_km_0_GSA -
     expected_relative_km_0_GSA).norm();

double relative_conversion_error_1_GSA =
    (recovered_relative_km_1_GSA -
     expected_relative_km_1_GSA).norm();

const double conversion_tolerance_GSA =
    1.0e-9;        // km

bool gsa017_pass =
    relative_conversion_error_0_GSA <=
        conversion_tolerance_GSA &&
    relative_conversion_error_1_GSA <=
        conversion_tolerance_GSA;

std::cout << "Relative Position t0:      "
          << relative_position_0_GSA.transpose()
          << " m\n";

std::cout << "Relative Position t1:      "
          << relative_position_1_GSA.transpose()
          << " m\n";

std::cout << "Recovered Relative t0:     "
          << recovered_relative_km_0_GSA.transpose()
          << " km\n";

std::cout << "Expected Relative t0:      "
          << expected_relative_km_0_GSA.transpose()
          << " km\n";

std::cout << "Conversion Error t0:       "
          << relative_conversion_error_0_GSA
          << " km\n";

std::cout << "Recovered Relative t1:     "
          << recovered_relative_km_1_GSA.transpose()
          << " km\n";

std::cout << "Expected Relative t1:      "
          << expected_relative_km_1_GSA.transpose()
          << " km\n";

std::cout << "Conversion Error t1:       "
          << relative_conversion_error_1_GSA
          << " km\n";

std::cout << "Tolerance:                 "
          << conversion_tolerance_GSA
          << " km\n";

std::cout << "GSA-017: "
          << (gsa017_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa017_pass)
{
    kepler_signal_tests_passed++;
}


// ============================================================================
// GSA-018
// EARTH-RELATIVE POSITION CHANGE -> RELATIVE VELOCITY
//
// v_relative = (r1 - r0) / delta_t
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-018 KEPLER RELATIVE VELOCITY\n";
std::cout << "----------------------------------------\n";

Eigen::Vector3d relative_position_change_GSA =
    relative_position_1_GSA -
    relative_position_0_GSA;

Eigen::Vector3d calculated_relative_velocity_GSA =
    relative_position_change_GSA /
    delta_time_GSA;


// Independent component-by-component calculation.

Eigen::Vector3d expected_relative_velocity_GSA(
    (relative_position_1_GSA.x() -
     relative_position_0_GSA.x()) /
        delta_time_GSA,

    (relative_position_1_GSA.y() -
     relative_position_0_GSA.y()) /
        delta_time_GSA,

    (relative_position_1_GSA.z() -
     relative_position_0_GSA.z()) /
        delta_time_GSA);

double relative_velocity_error_GSA =
    (calculated_relative_velocity_GSA -
     expected_relative_velocity_GSA).norm();

const double kepler_velocity_tolerance_GSA =
    1.0e-9;        // m/s

bool gsa018_pass =
    relative_velocity_error_GSA <=
        kepler_velocity_tolerance_GSA &&
    calculated_relative_velocity_GSA.norm() >
        0.0;

std::cout << "Relative Position t0:      "
          << relative_position_0_GSA.transpose()
          << " m\n";

std::cout << "Relative Position t1:      "
          << relative_position_1_GSA.transpose()
          << " m\n";

std::cout << "Position Change:           "
          << relative_position_change_GSA.transpose()
          << " m\n";

std::cout << "Delta Time:                "
          << delta_time_GSA
          << " s\n";

std::cout << "Calculated Velocity:       "
          << calculated_relative_velocity_GSA.transpose()
          << " m/s\n";

std::cout << "Expected Velocity:         "
          << expected_relative_velocity_GSA.transpose()
          << " m/s\n";

std::cout << "Velocity Error:            "
          << relative_velocity_error_GSA
          << " m/s\n";

std::cout << "Tolerance:                 "
          << kepler_velocity_tolerance_GSA
          << " m/s\n";

std::cout << "GSA-018: "
          << (gsa018_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa018_pass)
{
    kepler_signal_tests_passed++;
}


// ============================================================================
// GSA-019
// CURRENT EARTH-RELATIVE POSITION -> LOS -> RADIAL VELOCITY
//
// LOS = r / |r|
//
// vr = v_relative dot LOS
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-019 KEPLER LOS -> RADIAL VELOCITY\n";
std::cout << "----------------------------------------\n";

Eigen::Vector3d calculated_los_GSA =
    geometry.Calculate_Line_Of_Sight(
        relative_position_1_GSA);

double expected_range_GSA =
    std::sqrt(
        relative_position_1_GSA.x() *
            relative_position_1_GSA.x() +
        relative_position_1_GSA.y() *
            relative_position_1_GSA.y() +
        relative_position_1_GSA.z() *
            relative_position_1_GSA.z());

Eigen::Vector3d expected_los_GSA(
    relative_position_1_GSA.x() /
        expected_range_GSA,

    relative_position_1_GSA.y() /
        expected_range_GSA,

    relative_position_1_GSA.z() /
        expected_range_GSA);

double los_error_GSA =
    (calculated_los_GSA -
     expected_los_GSA).norm();

double calculated_radial_velocity_GSA =
    signal_analyzer.Calculate_Vector_Radial_Velocity(
        calculated_relative_velocity_GSA,
        calculated_los_GSA);

double expected_radial_velocity_GSA =
    expected_relative_velocity_GSA.x() *
        expected_los_GSA.x() +
    expected_relative_velocity_GSA.y() *
        expected_los_GSA.y() +
    expected_relative_velocity_GSA.z() *
        expected_los_GSA.z();

double radial_velocity_error_GSA =
    std::abs(
        calculated_radial_velocity_GSA -
        expected_radial_velocity_GSA);

const double kepler_los_tolerance_GSA =
    1.0e-9;

const double kepler_radial_tolerance_GSA =
    1.0e-9;

bool gsa019_pass =
    los_error_GSA <=
        kepler_los_tolerance_GSA &&
    radial_velocity_error_GSA <=
        kepler_radial_tolerance_GSA;

std::cout << "Current Relative Position: "
          << relative_position_1_GSA.transpose()
          << " m\n";

std::cout << "Range Magnitude:           "
          << expected_range_GSA
          << " m\n";

std::cout << "Calculated LOS:            "
          << calculated_los_GSA.transpose()
          << '\n';

std::cout << "Expected LOS:              "
          << expected_los_GSA.transpose()
          << '\n';

std::cout << "LOS Error:                 "
          << los_error_GSA
          << '\n';

std::cout << "\n";

std::cout << "Relative Velocity:         "
          << calculated_relative_velocity_GSA.transpose()
          << " m/s\n";

std::cout << "Calculated Radial Velocity:"
          << calculated_radial_velocity_GSA
          << " m/s\n";

std::cout << "Expected Radial Velocity:  "
          << expected_radial_velocity_GSA
          << " m/s\n";

std::cout << "Radial Velocity Error:     "
          << radial_velocity_error_GSA
          << " m/s\n";

std::cout << "LOS Tolerance:             "
          << kepler_los_tolerance_GSA
          << '\n';

std::cout << "Radial Velocity Tolerance: "
          << kepler_radial_tolerance_GSA
          << " m/s\n";

std::cout << "Physical Meaning:          "
          << (calculated_radial_velocity_GSA > 0.0
                  ? "receding"
                  : calculated_radial_velocity_GSA < 0.0
                        ? "approaching"
                        : "no radial motion")
          << '\n';

std::cout << "GSA-019: "
          << (gsa019_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa019_pass)
{
    kepler_signal_tests_passed++;
}


// ============================================================================
// GSA-020
// FULL KEPLER -> GEOMETRY -> SIGNAL ANALYZER -> DOPPLER HANDOFF
//
// Doppler factor = 1 - vr/c
//
// received frequency = transmitted frequency * Doppler factor
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-020 FULL KEPLER TO DOPPLER HANDOFF\n";
std::cout << "----------------------------------------\n";

double calculated_doppler_factor_GSA =
    1.0 -
    (calculated_radial_velocity_GSA / C);

double expected_doppler_factor_GSA =
    1.0 -
    (expected_radial_velocity_GSA / C);

double kepler_doppler_error_GSA =
    std::abs(
        calculated_doppler_factor_GSA -
        expected_doppler_factor_GSA);

double transmitted_frequency_GSA =
    1000000.0;       // Hz

double calculated_received_frequency_GSA =
    signal_analyzer.Calculate_Doppler_Shifted_Frequency(
        transmitted_frequency_GSA,
        calculated_doppler_factor_GSA);

double expected_received_frequency_GSA =
    transmitted_frequency_GSA *
    expected_doppler_factor_GSA;

double kepler_frequency_error_GSA =
    std::abs(
        calculated_received_frequency_GSA -
        expected_received_frequency_GSA);

const double kepler_doppler_tolerance_GSA =
    1.0e-9;

const double kepler_frequency_tolerance_GSA =
    1.0e-6;      // Hz

bool doppler_direction_valid_GSA = true;

if (calculated_radial_velocity_GSA > 0.0)
{
    doppler_direction_valid_GSA =
        calculated_received_frequency_GSA <
        transmitted_frequency_GSA;
}
else if (calculated_radial_velocity_GSA < 0.0)
{
    doppler_direction_valid_GSA =
        calculated_received_frequency_GSA >
        transmitted_frequency_GSA;
}

bool gsa020_pass =
    relative_velocity_error_GSA <=
        kepler_velocity_tolerance_GSA &&
    los_error_GSA <=
        kepler_los_tolerance_GSA &&
    radial_velocity_error_GSA <=
        kepler_radial_tolerance_GSA &&
    kepler_doppler_error_GSA <=
        kepler_doppler_tolerance_GSA &&
    kepler_frequency_error_GSA <=
        kepler_frequency_tolerance_GSA &&
    doppler_direction_valid_GSA;

std::cout << "Earth Position t0:         "
          << earth_position_0_GSA.transpose()
          << " km\n";

std::cout << "Earth Position t1:         "
          << earth_position_1_GSA.transpose()
          << " km\n";

std::cout << "Spacecraft Position t0:    "
          << spacecraft_position_0_GSA.transpose()
          << " km\n";

std::cout << "Spacecraft Position t1:    "
          << spacecraft_position_1_GSA.transpose()
          << " km\n";

std::cout << "\n";

std::cout << "Earth-Relative t0:         "
          << relative_position_0_GSA.transpose()
          << " m\n";

std::cout << "Earth-Relative t1:         "
          << relative_position_1_GSA.transpose()
          << " m\n";

std::cout << "Relative Velocity:         "
          << calculated_relative_velocity_GSA.transpose()
          << " m/s\n";

std::cout << "Velocity Error:            "
          << relative_velocity_error_GSA
          << " m/s\n";

std::cout << "\n";

std::cout << "Calculated LOS:            "
          << calculated_los_GSA.transpose()
          << '\n';

std::cout << "Expected LOS:              "
          << expected_los_GSA.transpose()
          << '\n';

std::cout << "LOS Error:                 "
          << los_error_GSA
          << '\n';

std::cout << "\n";

std::cout << "Calculated Radial Velocity:"
          << calculated_radial_velocity_GSA
          << " m/s\n";

std::cout << "Expected Radial Velocity:  "
          << expected_radial_velocity_GSA
          << " m/s\n";

std::cout << "Radial Velocity Error:     "
          << radial_velocity_error_GSA
          << " m/s\n";

std::cout << "\n";

std::cout << "Calculated Doppler Factor: "
          << calculated_doppler_factor_GSA
          << '\n';

std::cout << "Expected Doppler Factor:   "
          << expected_doppler_factor_GSA
          << '\n';

std::cout << "Doppler Error:             "
          << kepler_doppler_error_GSA
          << '\n';

std::cout << "\n";

std::cout << "Transmitted Frequency:     "
          << transmitted_frequency_GSA
          << " Hz\n";

std::cout << "Calculated Frequency:      "
          << calculated_received_frequency_GSA
          << " Hz\n";

std::cout << "Expected Frequency:        "
          << expected_received_frequency_GSA
          << " Hz\n";

std::cout << "Frequency Error:           "
          << kepler_frequency_error_GSA
          << " Hz\n";

std::cout << "\n";

std::cout << "Velocity Tolerance:        "
          << kepler_velocity_tolerance_GSA
          << " m/s\n";

std::cout << "LOS Tolerance:             "
          << kepler_los_tolerance_GSA
          << '\n';

std::cout << "Radial Velocity Tolerance: "
          << kepler_radial_tolerance_GSA
          << " m/s\n";

std::cout << "Doppler Tolerance:         "
          << kepler_doppler_tolerance_GSA
          << '\n';

std::cout << "Frequency Tolerance:       "
          << kepler_frequency_tolerance_GSA
          << " Hz\n";

std::cout << "Motion:                    "
          << (calculated_radial_velocity_GSA > 0.0
                  ? "RECEDING"
                  : calculated_radial_velocity_GSA < 0.0
                        ? "APPROACHING"
                        : "TRANSVERSE")
          << '\n';

std::cout << "GSA-020: "
          << (gsa020_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa020_pass)
{
    kepler_signal_tests_passed++;
}


// ============================================================================
// GSA-016 THROUGH GSA-020 SUMMARY
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "KEPLER -> SIGNAL ANALYZER SUMMARY\n";
std::cout << "========================================\n";

std::cout << "RESULT: "
          << kepler_signal_tests_passed
          << " / "
          << kepler_signal_total_tests
          << " tests passed\n";

std::cout << "BATCH RESULT: "
          << (kepler_signal_tests_passed ==
                      kepler_signal_total_tests
                  ? "PASS"
                  : "FAIL")
          << '\n';

std::cout << "========================================\n";


// ============================================================================
// SIGNAL ANALYZER -> OPTICAL COMMUNICATIONS INTEGRATION
// GSA-021 THROUGH GSA-025
//
// Kepler orbital propagation
// -> Earth-relative motion
// -> radial velocity
// -> optical Doppler shift
// -> diffraction efficiency
// -> received optical power
// -> photon flux
// -> information rate
// -> photons per time slot
//
// Controlled integration model.
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "SIGNAL ANALYZER -> OPTICAL COMMUNICATIONS\n";
std::cout << "GSA-021 THROUGH GSA-025\n";
std::cout << "========================================\n";

Optical_Communications optical_GSA;

int optical_signal_tests_passed = 0;
const int optical_signal_total_tests = 5;


// --------------------------------------------------------------------------
// Controlled optical parameters
// --------------------------------------------------------------------------

const double optical_wavelength_GSA =
    1550.0e-9;                // meters

const double transmitted_optical_frequency_GSA =
    C / optical_wavelength_GSA;

const double optical_distance_GSA =
    relative_position_1_GSA.norm();

const double diameter_rx_GSA =
    0.20;                     // meters

const double diameter_tx_GSA =
    0.20;                     // meters

const double transmitted_power_GSA =
    10.0;                     // watts

const double atmosphere_efficiency_GSA =
    0.90;

const double receiver_efficiency_GSA =
    0.80;

const double optical_PIE_GSA =
    5.0;                      // bits / photon

const double optical_bandwidth_GSA =
    1.0e9;                    // Hz

const double plancks_constant_GSA =
    6.626e-34;                // J*s


// ============================================================================
// GSA-021
// RADIAL VELOCITY -> OPTICAL DOPPLER-SHIFTED FREQUENCY
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-021 OPTICAL DOPPLER FREQUENCY\n";
std::cout << "----------------------------------------\n";

double calculated_optical_frequency_GSA =
    signal_analyzer.Calculate_Doppler_Shifted_Frequency(
        transmitted_optical_frequency_GSA,
        calculated_doppler_factor_GSA);

double expected_optical_frequency_GSA =
    transmitted_optical_frequency_GSA *
    expected_doppler_factor_GSA;

double optical_frequency_error_GSA =
    std::abs(
        calculated_optical_frequency_GSA -
        expected_optical_frequency_GSA);

double optical_frequency_relative_error_GSA =
    optical_frequency_error_GSA /
    std::abs(expected_optical_frequency_GSA);

const double optical_frequency_tolerance_GSA =
    1.0e-12;

bool optical_frequency_direction_GSA = true;

if (calculated_radial_velocity_GSA > 0.0)
{
    optical_frequency_direction_GSA =
        calculated_optical_frequency_GSA <
        transmitted_optical_frequency_GSA;
}
else if (calculated_radial_velocity_GSA < 0.0)
{
    optical_frequency_direction_GSA =
        calculated_optical_frequency_GSA >
        transmitted_optical_frequency_GSA;
}

bool gsa021_pass =
    optical_frequency_relative_error_GSA <=
        optical_frequency_tolerance_GSA &&
    optical_frequency_direction_GSA;

std::cout << "Optical Wavelength:        "
          << optical_wavelength_GSA
          << " m\n";

std::cout << "Transmitted Frequency:     "
          << transmitted_optical_frequency_GSA
          << " Hz\n";

std::cout << "Radial Velocity:           "
          << calculated_radial_velocity_GSA
          << " m/s\n";

std::cout << "Doppler Factor:            "
          << calculated_doppler_factor_GSA
          << '\n';

std::cout << "Calculated Frequency:      "
          << calculated_optical_frequency_GSA
          << " Hz\n";

std::cout << "Expected Frequency:        "
          << expected_optical_frequency_GSA
          << " Hz\n";

std::cout << "Absolute Error:            "
          << optical_frequency_error_GSA
          << " Hz\n";

std::cout << "Relative Error:            "
          << optical_frequency_relative_error_GSA
          << '\n';

std::cout << "Relative Tolerance:        "
          << optical_frequency_tolerance_GSA
          << '\n';

std::cout << "Motion:                    "
          << (calculated_radial_velocity_GSA > 0.0
                  ? "RECEDING"
                  : calculated_radial_velocity_GSA < 0.0
                        ? "APPROACHING"
                        : "TRANSVERSE")
          << '\n';

std::cout << "GSA-021: "
          << (gsa021_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa021_pass)
{
    optical_signal_tests_passed++;
}


// ============================================================================
// GSA-022
// DOPPLER-SHIFTED FREQUENCY -> DIFFRACTION EFFICIENCY
//
// eta_diff =
// [ pi * Dr * Dt * f / (4 * c * distance) ]^2
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-022 OPTICAL DIFFRACTION HANDOFF\n";
std::cout << "----------------------------------------\n";

double calculated_diffraction_GSA =
    optical_GSA.Calcualte_Diffraction_Squence(
        diameter_rx_GSA,
        diameter_tx_GSA,
        calculated_optical_frequency_GSA,
        optical_distance_GSA);

double expected_diffraction_top_GSA =
    3.141592653589793 *
    diameter_rx_GSA *
    diameter_tx_GSA *
    expected_optical_frequency_GSA;

double expected_diffraction_bottom_GSA =
    4.0 *
    C *
    optical_distance_GSA;

double expected_diffraction_GSA =
    std::pow(
        expected_diffraction_top_GSA /
        expected_diffraction_bottom_GSA,
        2.0);

double diffraction_error_GSA =
    std::abs(
        calculated_diffraction_GSA -
        expected_diffraction_GSA);

double diffraction_relative_error_GSA =
    diffraction_error_GSA /
    std::abs(expected_diffraction_GSA);

const double diffraction_tolerance_GSA =
    1.0e-12;

bool gsa022_pass =
    diffraction_relative_error_GSA <=
        diffraction_tolerance_GSA &&
    calculated_diffraction_GSA >= 0.0 &&
    calculated_diffraction_GSA <= 1.0;

std::cout << "Link Distance:             "
          << optical_distance_GSA
          << " m\n";

std::cout << "Receiver Diameter:         "
          << diameter_rx_GSA
          << " m\n";

std::cout << "Transmitter Diameter:      "
          << diameter_tx_GSA
          << " m\n";

std::cout << "Optical Frequency:         "
          << calculated_optical_frequency_GSA
          << " Hz\n";

std::cout << "Calculated Diffraction:    "
          << calculated_diffraction_GSA
          << '\n';

std::cout << "Expected Diffraction:      "
          << expected_diffraction_GSA
          << '\n';

std::cout << "Absolute Error:            "
          << diffraction_error_GSA
          << '\n';

std::cout << "Relative Error:            "
          << diffraction_relative_error_GSA
          << '\n';

std::cout << "Relative Tolerance:        "
          << diffraction_tolerance_GSA
          << '\n';

std::cout << "GSA-022: "
          << (gsa022_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa022_pass)
{
    optical_signal_tests_passed++;
}


// ============================================================================
// GSA-023
// DIFFRACTION EFFICIENCY -> RECEIVED OPTICAL POWER
//
// Pr = Pt * eta_atmosphere * eta_diffraction * eta_receiver
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-023 RECEIVED OPTICAL POWER\n";
std::cout << "----------------------------------------\n";

double calculated_received_power_GSA =
    optical_GSA.Free_Space_Optical_Link(
        transmitted_power_GSA,
        atmosphere_efficiency_GSA,
        calculated_diffraction_GSA,
        receiver_efficiency_GSA);

double expected_received_power_GSA =
    transmitted_power_GSA *
    atmosphere_efficiency_GSA *
    expected_diffraction_GSA *
    receiver_efficiency_GSA;

double received_power_error_GSA =
    std::abs(
        calculated_received_power_GSA -
        expected_received_power_GSA);

double received_power_relative_error_GSA =
    received_power_error_GSA /
    std::abs(expected_received_power_GSA);

const double received_power_tolerance_GSA =
    1.0e-12;

bool gsa023_pass =
    received_power_relative_error_GSA <=
        received_power_tolerance_GSA &&
    calculated_received_power_GSA > 0.0 &&
    calculated_received_power_GSA <
        transmitted_power_GSA;

std::cout << "Transmitted Power:         "
          << transmitted_power_GSA
          << " W\n";

std::cout << "Atmosphere Efficiency:     "
          << atmosphere_efficiency_GSA
          << '\n';

std::cout << "Diffraction Efficiency:    "
          << calculated_diffraction_GSA
          << '\n';

std::cout << "Receiver Efficiency:       "
          << receiver_efficiency_GSA
          << '\n';

std::cout << "Calculated Received Power: "
          << calculated_received_power_GSA
          << " W\n";

std::cout << "Expected Received Power:   "
          << expected_received_power_GSA
          << " W\n";

std::cout << "Absolute Error:            "
          << received_power_error_GSA
          << " W\n";

std::cout << "Relative Error:            "
          << received_power_relative_error_GSA
          << '\n';

std::cout << "Relative Tolerance:        "
          << received_power_tolerance_GSA
          << '\n';

std::cout << "GSA-023: "
          << (gsa023_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa023_pass)
{
    optical_signal_tests_passed++;
}


// ============================================================================
// GSA-024
// RECEIVED POWER -> PHOTON FLUX -> INFORMATION RATE
//
// photon flux = Pr / (h*f)
//
// information rate = PIE * photon flux
//
// Calculate_Photon_Information_Efficiency()
// stores both Photon_Flux and Information_Rate internally.
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-024 PHOTON FLUX -> INFORMATION RATE\n";
std::cout << "----------------------------------------\n";

double calculated_information_rate_GSA =
    optical_GSA.Calculate_Photon_Information_Efficiency(
        optical_PIE_GSA);

double calculated_photon_flux_GSA =
    optical_GSA.Get_Photon_Flux();

double expected_photon_flux_GSA =
    expected_received_power_GSA /
    (plancks_constant_GSA *
     expected_optical_frequency_GSA);

double expected_information_rate_GSA =
    optical_PIE_GSA *
    expected_photon_flux_GSA;

double photon_flux_error_GSA =
    std::abs(
        calculated_photon_flux_GSA -
        expected_photon_flux_GSA);

double photon_flux_relative_error_GSA =
    photon_flux_error_GSA /
    std::abs(expected_photon_flux_GSA);

double information_rate_error_GSA =
    std::abs(
        calculated_information_rate_GSA -
        expected_information_rate_GSA);

double information_rate_relative_error_GSA =
    information_rate_error_GSA /
    std::abs(expected_information_rate_GSA);

const double photon_flux_tolerance_GSA =
    1.0e-12;

const double information_rate_tolerance_GSA =
    1.0e-12;

bool gsa024_pass =
    photon_flux_relative_error_GSA <=
        photon_flux_tolerance_GSA &&
    information_rate_relative_error_GSA <=
        information_rate_tolerance_GSA &&
    calculated_photon_flux_GSA > 0.0 &&
    calculated_information_rate_GSA > 0.0;

std::cout << "Received Power:            "
          << calculated_received_power_GSA
          << " W\n";

std::cout << "Optical Frequency:         "
          << calculated_optical_frequency_GSA
          << " Hz\n";

std::cout << "Photon Energy:             "
          << plancks_constant_GSA *
             calculated_optical_frequency_GSA
          << " J\n";

std::cout << "Calculated Photon Flux:    "
          << calculated_photon_flux_GSA
          << " photons/s\n";

std::cout << "Expected Photon Flux:      "
          << expected_photon_flux_GSA
          << " photons/s\n";

std::cout << "Photon Flux Error:         "
          << photon_flux_error_GSA
          << " photons/s\n";

std::cout << "Photon Flux Relative Error:"
          << photon_flux_relative_error_GSA
          << '\n';

std::cout << "PIE:                       "
          << optical_PIE_GSA
          << " bits/photon\n";

std::cout << "Calculated Information:    "
          << calculated_information_rate_GSA
          << " bits/s\n";

std::cout << "Expected Information:      "
          << expected_information_rate_GSA
          << " bits/s\n";

std::cout << "Information Rate Error:    "
          << information_rate_error_GSA
          << " bits/s\n";

std::cout << "Information Relative Error:"
          << information_rate_relative_error_GSA
          << '\n';

std::cout << "GSA-024: "
          << (gsa024_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa024_pass)
{
    optical_signal_tests_passed++;
}


// ============================================================================
// GSA-025
// FULL KEPLER -> SIGNAL ANALYZER -> OPTICAL COMMUNICATIONS CHAIN
//
// Test final photon population per bandwidth time slot.
//
// Ns = (1/B) * Pr/(h*f)
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-025 FULL ORBITAL TO OPTICAL HANDOFF\n";
std::cout << "----------------------------------------\n";

double calculated_signal_photons_slot_GSA =
    optical_GSA.Calculate_Average_Signal_Photons_Per_Slot(
        optical_bandwidth_GSA);

double expected_signal_photons_slot_GSA =
    (1.0 / optical_bandwidth_GSA) *
    (expected_received_power_GSA /
        (plancks_constant_GSA *
         expected_optical_frequency_GSA));

double photons_slot_error_GSA =
    std::abs(
        calculated_signal_photons_slot_GSA -
        expected_signal_photons_slot_GSA);

double photons_slot_relative_error_GSA =
    photons_slot_error_GSA /
    std::abs(expected_signal_photons_slot_GSA);

const double photons_slot_tolerance_GSA =
    1.0e-12;

bool gsa025_pass =
    optical_frequency_relative_error_GSA <=
        optical_frequency_tolerance_GSA &&
    diffraction_relative_error_GSA <=
        diffraction_tolerance_GSA &&
    received_power_relative_error_GSA <=
        received_power_tolerance_GSA &&
    photon_flux_relative_error_GSA <=
        photon_flux_tolerance_GSA &&
    information_rate_relative_error_GSA <=
        information_rate_tolerance_GSA &&
    photons_slot_relative_error_GSA <=
        photons_slot_tolerance_GSA &&
    calculated_signal_photons_slot_GSA > 0.0;

std::cout << "Radial Velocity:           "
          << calculated_radial_velocity_GSA
          << " m/s\n";

std::cout << "Original Optical Frequency:"
          << transmitted_optical_frequency_GSA
          << " Hz\n";

std::cout << "Received Optical Frequency:"
          << calculated_optical_frequency_GSA
          << " Hz\n";

std::cout << "Link Distance:             "
          << optical_distance_GSA
          << " m\n";

std::cout << "Diffraction Efficiency:    "
          << calculated_diffraction_GSA
          << '\n';

std::cout << "Received Optical Power:    "
          << calculated_received_power_GSA
          << " W\n";

std::cout << "Photon Flux:               "
          << calculated_photon_flux_GSA
          << " photons/s\n";

std::cout << "PIE:                       "
          << optical_PIE_GSA
          << " bits/photon\n";

std::cout << "Information Rate:          "
          << calculated_information_rate_GSA
          << " bits/s\n";

std::cout << "Bandwidth:                 "
          << optical_bandwidth_GSA
          << " Hz\n";

std::cout << "Calculated Photons/Slot:   "
          << calculated_signal_photons_slot_GSA
          << '\n';

std::cout << "Expected Photons/Slot:     "
          << expected_signal_photons_slot_GSA
          << '\n';

std::cout << "Photons/Slot Error:        "
          << photons_slot_error_GSA
          << '\n';

std::cout << "Photons/Slot Rel Error:    "
          << photons_slot_relative_error_GSA
          << '\n';

std::cout << "Relative Tolerance:        "
          << photons_slot_tolerance_GSA
          << '\n';

std::cout << "GSA-025: "
          << (gsa025_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa025_pass)
{
    optical_signal_tests_passed++;
}


// ============================================================================
// GSA-021 THROUGH GSA-025 SUMMARY
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "SIGNAL -> OPTICAL COMMUNICATION SUMMARY\n";
std::cout << "========================================\n";

std::cout << "RESULT: "
          << optical_signal_tests_passed
          << " / "
          << optical_signal_total_tests
          << " tests passed\n";

std::cout << "BATCH RESULT: "
          << (optical_signal_tests_passed ==
                      optical_signal_total_tests
                  ? "PASS"
                  : "FAIL")
          << '\n';

std::cout << "========================================\n";


// ============================================================================
// OPTICAL COMMUNICATIONS -> RELAY STATION INTEGRATION
// GSA-026 THROUGH GSA-030
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "OPTICAL COMMUNICATIONS -> RELAY STATION\n";
std::cout << "GSA-026 THROUGH GSA-030\n";
std::cout << "========================================\n";

int relay_integration_tests_passed = 0;
const int relay_integration_total_tests = 5;


// --------------------------------------------------------------------------
// Complete the optical packet calculations.
//
// The optical object already contains:
//   Received_Power
//   Frequency
//   Photon_Flux
//   Information_Rate
//
// from GSA-021 through GSA-025.
// --------------------------------------------------------------------------

const std::size_t relay_packet_bytes_GSA = 1024;

std::size_t relay_packet_bits_GSA =
    optical_GSA.Calculate_Packet_Bits(
        relay_packet_bytes_GSA);

double relay_packet_time_GSA =
    optical_GSA.Calculate_Packet_Transmission_Time();

double relay_photons_per_packet_GSA =
    optical_GSA.Calculate_Received_Photons_Per_Packet();


// ============================================================================
// GSA-026
// OPTICAL PACKET METRICS
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-026 OPTICAL PACKET METRICS\n";
std::cout << "----------------------------------------\n";

std::size_t expected_packet_bits_GSA =
    relay_packet_bytes_GSA * 8;

double expected_packet_time_GSA =
    static_cast<double>(expected_packet_bits_GSA) /
    calculated_information_rate_GSA;

double expected_photons_packet_GSA =
    calculated_photon_flux_GSA *
    expected_packet_time_GSA;

double packet_time_error_GSA =
    std::abs(
        relay_packet_time_GSA -
        expected_packet_time_GSA);

double photons_packet_error_GSA =
    std::abs(
        relay_photons_per_packet_GSA -
        expected_photons_packet_GSA);

double packet_time_relative_error_GSA =
    packet_time_error_GSA /
    std::abs(expected_packet_time_GSA);

double photons_packet_relative_error_GSA =
    photons_packet_error_GSA /
    std::abs(expected_photons_packet_GSA);

const double packet_metric_tolerance_GSA =
    1.0e-12;

bool gsa026_pass =
    relay_packet_bits_GSA ==
        expected_packet_bits_GSA &&
    packet_time_relative_error_GSA <=
        packet_metric_tolerance_GSA &&
    photons_packet_relative_error_GSA <=
        packet_metric_tolerance_GSA &&
    relay_photons_per_packet_GSA > 0.0;

std::cout << "Packet Bytes:              "
          << relay_packet_bytes_GSA
          << '\n';

std::cout << "Calculated Packet Bits:    "
          << relay_packet_bits_GSA
          << '\n';

std::cout << "Expected Packet Bits:      "
          << expected_packet_bits_GSA
          << '\n';

std::cout << "Information Rate:          "
          << calculated_information_rate_GSA
          << " bits/s\n";

std::cout << "Calculated Packet Time:    "
          << relay_packet_time_GSA
          << " s\n";

std::cout << "Expected Packet Time:      "
          << expected_packet_time_GSA
          << " s\n";

std::cout << "Photon Flux:               "
          << calculated_photon_flux_GSA
          << " photons/s\n";

std::cout << "Calculated Photons/Packet: "
          << relay_photons_per_packet_GSA
          << '\n';

std::cout << "Expected Photons/Packet:   "
          << expected_photons_packet_GSA
          << '\n';

std::cout << "GSA-026: "
          << (gsa026_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa026_pass)
{
    relay_integration_tests_passed++;
}


// ============================================================================
// Controlled relay thresholds
//
// Set below the actual optical values for the valid-link test.
// ============================================================================

double minimum_power_GSA =
    calculated_received_power_GSA * 0.50;

double minimum_rate_GSA =
    calculated_information_rate_GSA * 0.50;

double minimum_photons_GSA =
    relay_photons_per_packet_GSA * 0.50;


// ============================================================================
// GSA-027
// FULL PHYSICAL CHAIN -> RELAY ACCEPTANCE
//
// Kepler
// -> Geometry
// -> Signal Analyzer
// -> Optical Communications
// -> Relay Station
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-027 FULL CHAIN RELAY ACCEPTANCE\n";
std::cout << "----------------------------------------\n";

Relay_Station_Simulation relay_GSA027;

// Remove random availability from this integration test.
relay_GSA027.Set_Online_Probability(1.0);
relay_GSA027.Create_Nodes(1);

bool relay_accept_GSA027 =
    relay_GSA027.Process_Optical_Link(
        3,
        calculated_received_power_GSA,
        calculated_photon_flux_GSA,
        calculated_information_rate_GSA,
        calculated_signal_photons_slot_GSA,
        relay_photons_per_packet_GSA,
        minimum_power_GSA,
        minimum_rate_GSA,
        minimum_photons_GSA);

bool expected_relay_accept_GSA027 =
    calculated_received_power_GSA >=
        minimum_power_GSA &&
    calculated_information_rate_GSA >=
        minimum_rate_GSA &&
    relay_photons_per_packet_GSA >=
        minimum_photons_GSA;

bool gsa027_pass =
    relay_accept_GSA027 ==
        expected_relay_accept_GSA027 &&
    relay_accept_GSA027;

std::cout << "Received Power:            "
          << calculated_received_power_GSA
          << " W\n";

std::cout << "Minimum Power:             "
          << minimum_power_GSA
          << " W\n";

std::cout << "Information Rate:          "
          << calculated_information_rate_GSA
          << " bits/s\n";

std::cout << "Minimum Information Rate:  "
          << minimum_rate_GSA
          << " bits/s\n";

std::cout << "Photons Per Packet:        "
          << relay_photons_per_packet_GSA
          << '\n';

std::cout << "Minimum Photons:           "
          << minimum_photons_GSA
          << '\n';

std::cout << "Calculated Relay Result:   "
          << std::boolalpha
          << relay_accept_GSA027
          << '\n';

std::cout << "Expected Relay Result:     "
          << expected_relay_accept_GSA027
          << '\n';

std::cout << "GSA-027: "
          << (gsa027_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa027_pass)
{
    relay_integration_tests_passed++;
}


// ============================================================================
// GSA-028
// RECEIVED POWER BELOW RELAY REQUIREMENT
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-028 RECEIVED POWER REJECTION\n";
std::cout << "----------------------------------------\n";

Relay_Station_Simulation relay_GSA028;

relay_GSA028.Set_Online_Probability(1.0);
relay_GSA028.Create_Nodes(1);

// Requirement deliberately placed ABOVE the available power.
double failing_power_threshold_GSA =
    calculated_received_power_GSA * 1.10;

bool relay_accept_GSA028 =
    relay_GSA028.Process_Optical_Link(
        3,
        calculated_received_power_GSA,
        calculated_photon_flux_GSA,
        calculated_information_rate_GSA,
        calculated_signal_photons_slot_GSA,
        relay_photons_per_packet_GSA,
        failing_power_threshold_GSA,
        minimum_rate_GSA,
        minimum_photons_GSA);

bool gsa028_pass =
    relay_accept_GSA028 == false;

std::cout << "Received Power:            "
          << calculated_received_power_GSA
          << " W\n";

std::cout << "Required Power:            "
          << failing_power_threshold_GSA
          << " W\n";

std::cout << "Expected Result:           false\n";

std::cout << "Calculated Result:         "
          << relay_accept_GSA028
          << '\n';

std::cout << "GSA-028: "
          << (gsa028_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa028_pass)
{
    relay_integration_tests_passed++;
}


// ============================================================================
// GSA-029
// INFORMATION RATE BELOW RELAY REQUIREMENT
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-029 INFORMATION RATE REJECTION\n";
std::cout << "----------------------------------------\n";

Relay_Station_Simulation relay_GSA029;

relay_GSA029.Set_Online_Probability(1.0);
relay_GSA029.Create_Nodes(1);

double failing_rate_threshold_GSA =
    calculated_information_rate_GSA * 1.10;

bool relay_accept_GSA029 =
    relay_GSA029.Process_Optical_Link(
        3,
        calculated_received_power_GSA,
        calculated_photon_flux_GSA,
        calculated_information_rate_GSA,
        calculated_signal_photons_slot_GSA,
        relay_photons_per_packet_GSA,
        minimum_power_GSA,
        failing_rate_threshold_GSA,
        minimum_photons_GSA);

bool gsa029_pass =
    relay_accept_GSA029 == false;

std::cout << "Information Rate:          "
          << calculated_information_rate_GSA
          << " bits/s\n";

std::cout << "Required Information Rate: "
          << failing_rate_threshold_GSA
          << " bits/s\n";

std::cout << "Expected Result:           false\n";

std::cout << "Calculated Result:         "
          << relay_accept_GSA029
          << '\n';

std::cout << "GSA-029: "
          << (gsa029_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa029_pass)
{
    relay_integration_tests_passed++;
}


// ============================================================================
// GSA-030
// PHOTONS PER PACKET BELOW RELAY REQUIREMENT
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-030 PHOTONS PER PACKET REJECTION\n";
std::cout << "----------------------------------------\n";

Relay_Station_Simulation relay_GSA030;

relay_GSA030.Set_Online_Probability(1.0);
relay_GSA030.Create_Nodes(1);

double failing_photon_threshold_GSA =
    relay_photons_per_packet_GSA * 1.10;

bool relay_accept_GSA030 =
    relay_GSA030.Process_Optical_Link(
        3,
        calculated_received_power_GSA,
        calculated_photon_flux_GSA,
        calculated_information_rate_GSA,
        calculated_signal_photons_slot_GSA,
        relay_photons_per_packet_GSA,
        minimum_power_GSA,
        minimum_rate_GSA,
        failing_photon_threshold_GSA);

bool gsa030_pass =
    relay_accept_GSA030 == false;

std::cout << "Received Photons/Packet:   "
          << relay_photons_per_packet_GSA
          << '\n';

std::cout << "Required Photons/Packet:   "
          << failing_photon_threshold_GSA
          << '\n';

std::cout << "Expected Result:           false\n";

std::cout << "Calculated Result:         "
          << relay_accept_GSA030
          << '\n';

std::cout << "GSA-030: "
          << (gsa030_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa030_pass)
{
    relay_integration_tests_passed++;
}


// ============================================================================
// GSA-026 THROUGH GSA-030 SUMMARY
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "OPTICAL -> RELAY INTEGRATION SUMMARY\n";
std::cout << "========================================\n";

std::cout << "RESULT: "
          << relay_integration_tests_passed
          << " / "
          << relay_integration_total_tests
          << " tests passed\n";

std::cout << "BATCH RESULT: "
          << (relay_integration_tests_passed ==
                      relay_integration_total_tests
                  ? "PASS"
                  : "FAIL")
          << '\n';

std::cout << "========================================\n";


// ============================================================================
// MULTI-HOP OPTICAL RELAY CHAIN
// GSA-031 THROUGH GSA-035
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "MULTI-HOP OPTICAL RELAY CHAIN\n";
std::cout << "GSA-031 THROUGH GSA-035\n";
std::cout << "========================================\n";

int multi_hop_tests_passed_GSA = 0;
const int multi_hop_total_tests_GSA = 5;


// ============================================================================
// CONTROLLED NETWORK CONFIGURATION
// ============================================================================

const int relay_count_GSA = 3;

// Three relays produce four physical optical hops:
//
// Source -> Relay 3
// Relay 3 -> Relay 4
// Relay 4 -> Relay 5
// Relay 5 -> Mars
//
const int total_hops_GSA =
    relay_count_GSA + 1;

// Use the distance already produced by the orbital/geometry chain.
const double total_network_distance_GSA =
    optical_distance_GSA;

const double distance_per_hop_GSA =
    total_network_distance_GSA /
    static_cast<double>(total_hops_GSA);

// Use the Doppler-shifted optical frequency already stored
// by the Optical Communications object.
const double multi_hop_frequency_GSA =
    calculated_optical_frequency_GSA;


// Controlled optical parameters
const double multi_hop_rx_diameter_GSA = 0.20;
const double multi_hop_tx_diameter_GSA = 0.20;

const double multi_hop_transmitted_power_GSA = 10.0;

const double multi_hop_atmosphere_efficiency_GSA = 0.90;
const double multi_hop_receiver_efficiency_GSA = 0.80;

const double multi_hop_PIE_GSA = 5.0;

const double multi_hop_bandwidth_GSA = 1.0e9;

const std::size_t multi_hop_packet_bytes_GSA = 1024;


// ============================================================================
// GSA-031
// EQUAL RELAY SPACING
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-031 EQUAL RELAY HOP SPACING\n";
std::cout << "----------------------------------------\n";

double reconstructed_distance_GSA =
    distance_per_hop_GSA *
    static_cast<double>(total_hops_GSA);

double spacing_error_GSA =
    std::abs(
        reconstructed_distance_GSA -
        total_network_distance_GSA);

const double spacing_tolerance_GSA =
    1.0e-6;

bool gsa031_pass =
    total_hops_GSA == 4 &&
    distance_per_hop_GSA > 0.0 &&
    spacing_error_GSA <= spacing_tolerance_GSA;

std::cout << "Relay Count:               "
          << relay_count_GSA << '\n';

std::cout << "Total Optical Hops:        "
          << total_hops_GSA << '\n';

std::cout << "Total Network Distance:    "
          << total_network_distance_GSA
          << " m\n";

std::cout << "Distance Per Hop:          "
          << distance_per_hop_GSA
          << " m\n";

std::cout << "Reconstructed Distance:    "
          << reconstructed_distance_GSA
          << " m\n";

std::cout << "Spacing Error:             "
          << spacing_error_GSA
          << " m\n";

std::cout << "GSA-031: "
          << (gsa031_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa031_pass)
{
    multi_hop_tests_passed_GSA++;
}


// ============================================================================
// CALCULATE A BASELINE SINGLE-HOP LINK
//
// We use this only to establish controlled relay requirements.
// ============================================================================

Optical_Communications baseline_hop_GSA;

double baseline_diffraction_GSA =
    baseline_hop_GSA.Calcualte_Diffraction_Squence(
        multi_hop_rx_diameter_GSA,
        multi_hop_tx_diameter_GSA,
        multi_hop_frequency_GSA,
        distance_per_hop_GSA);

double baseline_received_power_GSA =
    baseline_hop_GSA.Free_Space_Optical_Link(
        multi_hop_transmitted_power_GSA,
        multi_hop_atmosphere_efficiency_GSA,
        baseline_diffraction_GSA,
        multi_hop_receiver_efficiency_GSA);

double baseline_information_rate_GSA =
    baseline_hop_GSA.Calculate_Photon_Information_Efficiency(
        multi_hop_PIE_GSA);

double baseline_signal_photons_GSA =
    baseline_hop_GSA.Calculate_Average_Signal_Photons_Per_Slot(
        multi_hop_bandwidth_GSA);

double baseline_photon_flux_GSA =
    baseline_hop_GSA.Get_Photon_Flux();

baseline_hop_GSA.Calculate_Packet_Bits(
    multi_hop_packet_bytes_GSA);

baseline_hop_GSA.Calculate_Packet_Transmission_Time();

double baseline_photons_packet_GSA =
    baseline_hop_GSA.Calculate_Received_Photons_Per_Packet();


// Set requirements at 50% of the controlled link capability.
const double multi_hop_minimum_power_GSA =
    baseline_received_power_GSA * 0.50;

const double multi_hop_minimum_rate_GSA =
    baseline_information_rate_GSA * 0.50;

const double multi_hop_minimum_photons_GSA =
    baseline_photons_packet_GSA * 0.50;


// ============================================================================
// CREATE THE THREE-RELAY NETWORK
// ============================================================================

Relay_Station_Simulation multi_hop_network_GSA;

// Deterministic integration testing.
// No random offline relays in this batch.
multi_hop_network_GSA.Set_Online_Probability(1.0);

multi_hop_network_GSA.Create_Nodes(
    relay_count_GSA);


// Store relay results.
//
// index 0 = Relay 3
// index 1 = Relay 4
// index 2 = Relay 5
//
bool relay_results_GSA[3] =
{
    false,
    false,
    false
};


// ============================================================================
// PROCESS THE THREE RELAY HOPS
// ============================================================================

for (int relay_index_GSA = 0;
     relay_index_GSA < relay_count_GSA;
     ++relay_index_GSA)
{
    int node_id_GSA =
        3 + relay_index_GSA;

    std::cout << "\nProcessing optical hop to Relay "
              << node_id_GSA
              << '\n';

    Optical_Communications hop_optical_GSA;

    double hop_diffraction_GSA =
        hop_optical_GSA.Calcualte_Diffraction_Squence(
            multi_hop_rx_diameter_GSA,
            multi_hop_tx_diameter_GSA,
            multi_hop_frequency_GSA,
            distance_per_hop_GSA);

    double hop_received_power_GSA =
        hop_optical_GSA.Free_Space_Optical_Link(
            multi_hop_transmitted_power_GSA,
            multi_hop_atmosphere_efficiency_GSA,
            hop_diffraction_GSA,
            multi_hop_receiver_efficiency_GSA);

    double hop_information_rate_GSA =
        hop_optical_GSA.Calculate_Photon_Information_Efficiency(
            multi_hop_PIE_GSA);

    double hop_signal_photons_GSA =
        hop_optical_GSA.Calculate_Average_Signal_Photons_Per_Slot(
            multi_hop_bandwidth_GSA);

    double hop_photon_flux_GSA =
        hop_optical_GSA.Get_Photon_Flux();

    hop_optical_GSA.Calculate_Packet_Bits(
        multi_hop_packet_bytes_GSA);

    double hop_transmission_time_GSA =
        hop_optical_GSA.Calculate_Packet_Transmission_Time();

    double hop_photons_packet_GSA =
        hop_optical_GSA.Calculate_Received_Photons_Per_Packet();

    std::cout << "Hop Distance:               "
              << distance_per_hop_GSA
              << " m\n";

    std::cout << "Diffraction Efficiency:     "
              << hop_diffraction_GSA
              << '\n';

    std::cout << "Received Power:             "
              << hop_received_power_GSA
              << " W\n";

    std::cout << "Photon Flux:                "
              << hop_photon_flux_GSA
              << " photons/s\n";

    std::cout << "Information Rate:           "
              << hop_information_rate_GSA
              << " bits/s\n";

    std::cout << "Packet Transmission Time:   "
              << hop_transmission_time_GSA
              << " s\n";

    std::cout << "Photons Per Packet:         "
              << hop_photons_packet_GSA
              << '\n';

    relay_results_GSA[relay_index_GSA] =
        multi_hop_network_GSA.Process_Optical_Link(
            node_id_GSA,
            hop_received_power_GSA,
            hop_photon_flux_GSA,
            hop_information_rate_GSA,
            hop_signal_photons_GSA,
            hop_photons_packet_GSA,
            multi_hop_minimum_power_GSA,
            multi_hop_minimum_rate_GSA,
            multi_hop_minimum_photons_GSA);
}


// ============================================================================
// GSA-032
// SOURCE -> RELAY 3
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-032 SOURCE -> RELAY 3\n";
std::cout << "----------------------------------------\n";

bool gsa032_pass =
    relay_results_GSA[0];

std::cout << "Relay 3 Result: "
          << relay_results_GSA[0]
          << '\n';

std::cout << "GSA-032: "
          << (gsa032_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa032_pass)
{
    multi_hop_tests_passed_GSA++;
}


// ============================================================================
// GSA-033
// RELAY 3 -> RELAY 4
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-033 RELAY 3 -> RELAY 4\n";
std::cout << "----------------------------------------\n";

bool gsa033_pass =
    relay_results_GSA[0] &&
    relay_results_GSA[1];

std::cout << "Previous Relay Successful: "
          << relay_results_GSA[0]
          << '\n';

std::cout << "Relay 4 Result:             "
          << relay_results_GSA[1]
          << '\n';

std::cout << "GSA-033: "
          << (gsa033_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa033_pass)
{
    multi_hop_tests_passed_GSA++;
}


// ============================================================================
// GSA-034
// RELAY 4 -> RELAY 5
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-034 RELAY 4 -> RELAY 5\n";
std::cout << "----------------------------------------\n";

bool gsa034_pass =
    relay_results_GSA[0] &&
    relay_results_GSA[1] &&
    relay_results_GSA[2];

std::cout << "Relay 3 Result: "
          << relay_results_GSA[0]
          << '\n';

std::cout << "Relay 4 Result: "
          << relay_results_GSA[1]
          << '\n';

std::cout << "Relay 5 Result: "
          << relay_results_GSA[2]
          << '\n';

std::cout << "GSA-034: "
          << (gsa034_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa034_pass)
{
    multi_hop_tests_passed_GSA++;
}


// ============================================================================
// GSA-035
// RELAY 5 -> MARS PHYSICAL OPTICAL RECEPTION
//
// Mars is NOT treated as another relay.
//
// We calculate the final physical optical hop and determine whether the
// received signal satisfies the same physical requirements.
// ============================================================================

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-035 FINAL OPTICAL HOP -> MARS\n";
std::cout << "----------------------------------------\n";

Optical_Communications mars_hop_GSA;

double mars_diffraction_GSA =
    mars_hop_GSA.Calcualte_Diffraction_Squence(
        multi_hop_rx_diameter_GSA,
        multi_hop_tx_diameter_GSA,
        multi_hop_frequency_GSA,
        distance_per_hop_GSA);

double mars_received_power_GSA =
    mars_hop_GSA.Free_Space_Optical_Link(
        multi_hop_transmitted_power_GSA,
        multi_hop_atmosphere_efficiency_GSA,
        mars_diffraction_GSA,
        multi_hop_receiver_efficiency_GSA);

double mars_information_rate_GSA =
    mars_hop_GSA.Calculate_Photon_Information_Efficiency(
        multi_hop_PIE_GSA);

double mars_signal_photons_GSA =
    mars_hop_GSA.Calculate_Average_Signal_Photons_Per_Slot(
        multi_hop_bandwidth_GSA);

double mars_photon_flux_GSA =
    mars_hop_GSA.Get_Photon_Flux();

mars_hop_GSA.Calculate_Packet_Bits(
    multi_hop_packet_bytes_GSA);

double mars_packet_time_GSA =
    mars_hop_GSA.Calculate_Packet_Transmission_Time();

double mars_photons_packet_GSA =
    mars_hop_GSA.Calculate_Received_Photons_Per_Packet();

bool mars_physical_link_GSA =
    mars_received_power_GSA >=
        multi_hop_minimum_power_GSA &&
    mars_information_rate_GSA >=
        multi_hop_minimum_rate_GSA &&
    mars_photons_packet_GSA >=
        multi_hop_minimum_photons_GSA;

bool all_relays_GSA =
    relay_results_GSA[0] &&
    relay_results_GSA[1] &&
    relay_results_GSA[2];

bool gsa035_pass =
    all_relays_GSA &&
    mars_physical_link_GSA;

std::cout << "Final Hop Distance:         "
          << distance_per_hop_GSA
          << " m\n";

std::cout << "Mars Received Power:        "
          << mars_received_power_GSA
          << " W\n";

std::cout << "Required Power:             "
          << multi_hop_minimum_power_GSA
          << " W\n";

std::cout << "Mars Photon Flux:           "
          << mars_photon_flux_GSA
          << " photons/s\n";

std::cout << "Mars Information Rate:      "
          << mars_information_rate_GSA
          << " bits/s\n";

std::cout << "Required Information Rate:  "
          << multi_hop_minimum_rate_GSA
          << " bits/s\n";

std::cout << "Mars Packet Time:           "
          << mars_packet_time_GSA
          << " s\n";

std::cout << "Mars Photons Per Packet:    "
          << mars_photons_packet_GSA
          << '\n';

std::cout << "Required Photons/Packet:    "
          << multi_hop_minimum_photons_GSA
          << '\n';

std::cout << "All Relays Successful:      "
          << all_relays_GSA
          << '\n';

std::cout << "Mars Physical Link Valid:   "
          << mars_physical_link_GSA
          << '\n';

std::cout << "Packet Physically Reached "
             "Destination:             "
          << gsa035_pass
          << '\n';

std::cout << "GSA-035: "
          << (gsa035_pass ? "PASS" : "FAIL")
          << '\n';

if (gsa035_pass)
{
    multi_hop_tests_passed_GSA++;
}


// ============================================================================
// MULTI-HOP SUMMARY
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "MULTI-HOP RELAY INTEGRATION SUMMARY\n";
std::cout << "========================================\n";

std::cout << "RESULT: "
          << multi_hop_tests_passed_GSA
          << " / "
          << multi_hop_total_tests_GSA
          << " tests passed\n";

std::cout << "BATCH RESULT: "
          << (multi_hop_tests_passed_GSA ==
                      multi_hop_total_tests_GSA
                  ? "PASS"
                  : "FAIL")
          << '\n';

std::cout << "========================================\n";


// ============================================================================
// GSA-036 THROUGH GSA-052
// FINAL DETERMINISTIC NETWORK INTEGRATION BATCH
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "FINAL NETWORK INTEGRATION BATCH\n";
std::cout << "GSA-036 THROUGH GSA-052\n";
std::cout << "========================================\n";

int final_batch_pass_count_GSA = 0;
const int final_batch_total_GSA = 17;

// ---------------------------------------------------------------------------
// Controlled constants
// ---------------------------------------------------------------------------

const double final_total_distance_GSA =
    optical_distance_GSA;

const double final_optical_frequency_GSA =
    calculated_optical_frequency_GSA;

const double final_receiver_diameter_GSA = 0.20;
const double final_transmitter_diameter_GSA = 0.20;

const double final_transmitted_power_GSA = 10.0;
const double final_atmosphere_efficiency_GSA = 0.90;
const double final_receiver_efficiency_GSA = 0.80;

const double final_pie_GSA = 5.0;
const double final_bandwidth_GSA = 1.0e9;

const std::size_t final_packet_bytes_GSA = 1024;

const double final_speed_of_light_GSA = 2.998e8;


// ============================================================================
// Small local structure used only by integration testing
// ============================================================================

struct Final_Link_Metrics_GSA
{
    double Distance{};
    double Diffraction{};
    double Received_Power{};
    double Photon_Flux{};
    double Information_Rate{};
    double Signal_Photons_Per_Slot{};
    double Packet_Time{};
    double Photons_Per_Packet{};
};


// ============================================================================
// Helper lambda
//
// Give it a link distance.
// It runs the real Optical_Communications calculations and returns metrics.
// ============================================================================

auto Calculate_Final_Link_GSA =
    [&](double link_distance) -> Final_Link_Metrics_GSA
{
    Optical_Communications optical_link;

    Final_Link_Metrics_GSA metrics;

    metrics.Distance = link_distance;

    metrics.Diffraction =
        optical_link.Calcualte_Diffraction_Squence(
            final_receiver_diameter_GSA,
            final_transmitter_diameter_GSA,
            final_optical_frequency_GSA,
            link_distance);

    metrics.Received_Power =
        optical_link.Free_Space_Optical_Link(
            final_transmitted_power_GSA,
            final_atmosphere_efficiency_GSA,
            metrics.Diffraction,
            final_receiver_efficiency_GSA);

    metrics.Information_Rate =
        optical_link.Calculate_Photon_Information_Efficiency(
            final_pie_GSA);

    metrics.Photon_Flux =
        optical_link.Get_Photon_Flux();

    metrics.Signal_Photons_Per_Slot =
        optical_link.Calculate_Average_Signal_Photons_Per_Slot(
            final_bandwidth_GSA);

    optical_link.Calculate_Packet_Bits(
        final_packet_bytes_GSA);

    metrics.Packet_Time =
        optical_link.Calculate_Packet_Transmission_Time();

    metrics.Photons_Per_Packet =
        optical_link.Calculate_Received_Photons_Per_Packet();

    return metrics;
};


// ============================================================================
// Establish fixed baseline from original direct link
// ============================================================================

Final_Link_Metrics_GSA direct_baseline_GSA =
    Calculate_Final_Link_GSA(
        final_total_distance_GSA);

const double final_minimum_power_GSA =
    direct_baseline_GSA.Received_Power * 0.50;

const double final_minimum_rate_GSA =
    direct_baseline_GSA.Information_Rate * 0.50;

const double final_minimum_photons_GSA =
    direct_baseline_GSA.Photons_Per_Packet * 0.50;


// ============================================================================
// Helper lambda for equal-spaced relay chains
// ============================================================================

auto Test_Equal_Relay_Chain_GSA =
    [&](int relay_count) -> bool
{
    const int total_hops =
        relay_count + 1;

    const double hop_distance =
        final_total_distance_GSA /
        static_cast<double>(total_hops);

    Final_Link_Metrics_GSA hop_metrics =
        Calculate_Final_Link_GSA(
            hop_distance);

    // -----------------------------------------------------
    // Physical optical link must satisfy fixed thresholds.
    // -----------------------------------------------------

    bool optical_link_valid =
        hop_metrics.Received_Power >= final_minimum_power_GSA &&
        hop_metrics.Information_Rate >= final_minimum_rate_GSA &&
        hop_metrics.Photons_Per_Packet >= final_minimum_photons_GSA;

    if (!optical_link_valid)
    {
        return false;
    }

    // -----------------------------------------------------
    // Zero relays = direct source -> Mars physical link.
    // -----------------------------------------------------

    if (relay_count == 0)
    {
        return true;
    }

    // -----------------------------------------------------
    // Create actual relay nodes.
    // -----------------------------------------------------

    Relay_Station_Simulation network;

    network.Set_Online_Probability(1.0);
    network.Create_Nodes(relay_count);

    bool all_relays_successful = true;

    for (int relay_index = 0;
         relay_index < relay_count;
         ++relay_index)
    {
        const int node_id =
            3 + relay_index;

        bool relay_result =
            network.Process_Optical_Link(
                node_id,
                hop_metrics.Received_Power,
                hop_metrics.Photon_Flux,
                hop_metrics.Information_Rate,
                hop_metrics.Signal_Photons_Per_Slot,
                hop_metrics.Photons_Per_Packet,
                final_minimum_power_GSA,
                final_minimum_rate_GSA,
                final_minimum_photons_GSA);

        if (!relay_result)
        {
            all_relays_successful = false;
        }
    }

    // Final hop to Mars uses same equal-hop physics.
    bool mars_physical_link_valid =
        hop_metrics.Received_Power >= final_minimum_power_GSA &&
        hop_metrics.Information_Rate >= final_minimum_rate_GSA &&
        hop_metrics.Photons_Per_Packet >= final_minimum_photons_GSA;

    return
        all_relays_successful &&
        mars_physical_link_valid;
};


// ============================================================================
// GSA-036 THROUGH GSA-043
// RELAY COUNT VARIATION
// ============================================================================

std::vector<int> relay_counts_GSA =
{
    0,
    1,
    2,
    4,
    6,
    8,
    10,
    15
};

int relay_test_number_GSA = 36;

for (int relay_count : relay_counts_GSA)
{
    const int hop_count =
        relay_count + 1;

    const double hop_distance =
        final_total_distance_GSA /
        static_cast<double>(hop_count);

    bool result =
        Test_Equal_Relay_Chain_GSA(
            relay_count);

    std::cout << "\n----------------------------------------\n";
    std::cout << "GSA-" << relay_test_number_GSA
              << " RELAY COUNT = "
              << relay_count << "\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Relay Count:              "
              << relay_count << "\n";

    std::cout << "Total Hops:               "
              << hop_count << "\n";

    std::cout << "Distance Per Hop:         "
              << hop_distance
              << " m\n";

    std::cout << "End-to-End Result:        "
              << std::boolalpha
              << result
              << "\n";

    std::cout << "GSA-"
              << relay_test_number_GSA
              << ": "
              << (result ? "PASS" : "FAIL")
              << "\n";

    if (result)
    {
        final_batch_pass_count_GSA++;
    }

    relay_test_number_GSA++;
}


// ============================================================================
// GSA-044
// UNEQUAL SPACING RECONSTRUCTION
// ============================================================================

std::vector<double> unequal_fraction_GSA =
{
    0.10,
    0.20,
    0.30,
    0.40
};

double unequal_reconstructed_distance_GSA = 0.0;

for (double fraction : unequal_fraction_GSA)
{
    unequal_reconstructed_distance_GSA +=
        final_total_distance_GSA *
        fraction;
}

double unequal_spacing_error_GSA =
    std::abs(
        unequal_reconstructed_distance_GSA -
        final_total_distance_GSA);

bool gsa044_pass =
    unequal_spacing_error_GSA <= 1.0e-6;

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-044 UNEQUAL SPACING RECONSTRUCTION\n";
std::cout << "----------------------------------------\n";

std::cout << "Hop Fractions:             "
          << "0.10, 0.20, 0.30, 0.40\n";

std::cout << "Total Network Distance:    "
          << final_total_distance_GSA
          << " m\n";

std::cout << "Reconstructed Distance:    "
          << unequal_reconstructed_distance_GSA
          << " m\n";

std::cout << "Spacing Error:             "
          << unequal_spacing_error_GSA
          << " m\n";

std::cout << "GSA-044: "
          << (gsa044_pass ? "PASS" : "FAIL")
          << "\n";

if (gsa044_pass)
{
    final_batch_pass_count_GSA++;
}


// ============================================================================
// GSA-045
// ALL UNEQUAL HOPS PHYSICALLY VALID
// ============================================================================

bool all_unequal_hops_valid_GSA = true;

double unequal_bottleneck_rate_GSA = 0.0;

for (std::size_t i = 0;
     i < unequal_fraction_GSA.size();
     ++i)
{
    const double hop_distance =
        final_total_distance_GSA *
        unequal_fraction_GSA[i];

    Final_Link_Metrics_GSA metrics =
        Calculate_Final_Link_GSA(
            hop_distance);

    if (i == 0 ||
        metrics.Information_Rate <
        unequal_bottleneck_rate_GSA)
    {
        unequal_bottleneck_rate_GSA =
            metrics.Information_Rate;
    }

    bool hop_valid =
        metrics.Received_Power >= final_minimum_power_GSA &&
        metrics.Information_Rate >= final_minimum_rate_GSA &&
        metrics.Photons_Per_Packet >= final_minimum_photons_GSA;

    if (!hop_valid)
    {
        all_unequal_hops_valid_GSA = false;
    }

    std::cout << "\nUnequal Hop "
              << (i + 1)
              << "\n";

    std::cout << "Distance:                  "
              << hop_distance
              << " m\n";

    std::cout << "Received Power:            "
              << metrics.Received_Power
              << " W\n";

    std::cout << "Information Rate:          "
              << metrics.Information_Rate
              << " bits/s\n";

    std::cout << "Physical Link Valid:       "
              << std::boolalpha
              << hop_valid
              << "\n";
}

bool gsa045_pass =
    all_unequal_hops_valid_GSA;

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-045 UNEQUAL HOP PHYSICAL LINKS\n";
std::cout << "----------------------------------------\n";

std::cout << "All Unequal Hops Valid:    "
          << std::boolalpha
          << all_unequal_hops_valid_GSA
          << "\n";

std::cout << "GSA-045: "
          << (gsa045_pass ? "PASS" : "FAIL")
          << "\n";

if (gsa045_pass)
{
    final_batch_pass_count_GSA++;
}


// ============================================================================
// GSA-046
// EQUAL VS UNEQUAL BOTTLENECK
//
// Same total distance.
// 3 relays -> 4 hops.
// Equal: 25% each.
// Unequal longest hop: 40%.
// Longer bottleneck hop should have lower information rate.
// ============================================================================

const double equal_four_hop_distance_GSA =
    final_total_distance_GSA / 4.0;

Final_Link_Metrics_GSA equal_four_hop_metrics_GSA =
    Calculate_Final_Link_GSA(
        equal_four_hop_distance_GSA);

const double equal_bottleneck_rate_GSA =
    equal_four_hop_metrics_GSA.Information_Rate;

bool gsa046_pass =
    unequal_bottleneck_rate_GSA <
    equal_bottleneck_rate_GSA;

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-046 EQUAL VS UNEQUAL BOTTLENECK\n";
std::cout << "----------------------------------------\n";

std::cout << "Equal Hop Distance:        "
          << equal_four_hop_distance_GSA
          << " m\n";

std::cout << "Equal Bottleneck Rate:     "
          << equal_bottleneck_rate_GSA
          << " bits/s\n";

std::cout << "Unequal Bottleneck Rate:   "
          << unequal_bottleneck_rate_GSA
          << " bits/s\n";

std::cout << "Expected Condition:        "
          << "unequal bottleneck < equal bottleneck\n";

std::cout << "GSA-046: "
          << (gsa046_pass ? "PASS" : "FAIL")
          << "\n";

if (gsa046_pass)
{
    final_batch_pass_count_GSA++;
}


// ============================================================================
// GSA-047
// FORCED OFFLINE RELAY
// ============================================================================

Relay_Station_Simulation offline_network_GSA;

offline_network_GSA.Set_Online_Probability(0.0);
offline_network_GSA.Create_Nodes(1);

Final_Link_Metrics_GSA one_relay_hop_GSA =
    Calculate_Final_Link_GSA(
        final_total_distance_GSA / 2.0);

bool offline_relay_result_GSA =
    offline_network_GSA.Process_Optical_Link(
        3,
        one_relay_hop_GSA.Received_Power,
        one_relay_hop_GSA.Photon_Flux,
        one_relay_hop_GSA.Information_Rate,
        one_relay_hop_GSA.Signal_Photons_Per_Slot,
        one_relay_hop_GSA.Photons_Per_Packet,
        final_minimum_power_GSA,
        final_minimum_rate_GSA,
        final_minimum_photons_GSA);

bool gsa047_pass =
    !offline_relay_result_GSA;

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-047 FORCED OFFLINE RELAY\n";
std::cout << "----------------------------------------\n";

std::cout << "Online Probability:        0.0\n";

std::cout << "Relay Result:              "
          << std::boolalpha
          << offline_relay_result_GSA
          << "\n";

std::cout << "Expected Result:           false\n";

std::cout << "GSA-047: "
          << (gsa047_pass ? "PASS" : "FAIL")
          << "\n";

if (gsa047_pass)
{
    final_batch_pass_count_GSA++;
}


// ============================================================================
// GSA-048
// ONE RELAY FAILS PHYSICAL THRESHOLD
// ============================================================================

Relay_Station_Simulation threshold_failure_network_GSA;

threshold_failure_network_GSA.Set_Online_Probability(1.0);
threshold_failure_network_GSA.Create_Nodes(3);

Final_Link_Metrics_GSA failure_hop_metrics_GSA =
    Calculate_Final_Link_GSA(
        final_total_distance_GSA / 4.0);

bool relay_3_success_GSA =
    threshold_failure_network_GSA.Process_Optical_Link(
        3,
        failure_hop_metrics_GSA.Received_Power,
        failure_hop_metrics_GSA.Photon_Flux,
        failure_hop_metrics_GSA.Information_Rate,
        failure_hop_metrics_GSA.Signal_Photons_Per_Slot,
        failure_hop_metrics_GSA.Photons_Per_Packet,
        final_minimum_power_GSA,
        final_minimum_rate_GSA,
        final_minimum_photons_GSA);

const double impossible_power_threshold_GSA =
    failure_hop_metrics_GSA.Received_Power *
    1.10;

bool relay_4_success_GSA =
    threshold_failure_network_GSA.Process_Optical_Link(
        4,
        failure_hop_metrics_GSA.Received_Power,
        failure_hop_metrics_GSA.Photon_Flux,
        failure_hop_metrics_GSA.Information_Rate,
        failure_hop_metrics_GSA.Signal_Photons_Per_Slot,
        failure_hop_metrics_GSA.Photons_Per_Packet,
        impossible_power_threshold_GSA,
        final_minimum_rate_GSA,
        final_minimum_photons_GSA);

bool relay_5_success_GSA =
    threshold_failure_network_GSA.Process_Optical_Link(
        5,
        failure_hop_metrics_GSA.Received_Power,
        failure_hop_metrics_GSA.Photon_Flux,
        failure_hop_metrics_GSA.Information_Rate,
        failure_hop_metrics_GSA.Signal_Photons_Per_Slot,
        failure_hop_metrics_GSA.Photons_Per_Packet,
        final_minimum_power_GSA,
        final_minimum_rate_GSA,
        final_minimum_photons_GSA);

bool gsa048_pass =
    relay_3_success_GSA &&
    !relay_4_success_GSA &&
    relay_5_success_GSA;

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-048 SINGLE PHYSICAL HOP FAILURE\n";
std::cout << "----------------------------------------\n";

std::cout << "Relay 3:                  "
          << std::boolalpha
          << relay_3_success_GSA
          << "\n";

std::cout << "Relay 4:                  "
          << relay_4_success_GSA
          << "\n";

std::cout << "Relay 5:                  "
          << relay_5_success_GSA
          << "\n";

std::cout << "Expected Relay 4:         false\n";

std::cout << "GSA-048: "
          << (gsa048_pass ? "PASS" : "FAIL")
          << "\n";

if (gsa048_pass)
{
    final_batch_pass_count_GSA++;
}


// ============================================================================
// GSA-049
// BROKEN SERIAL CHAIN MUST FAIL END-TO-END DELIVERY
// ============================================================================

bool broken_chain_delivery_GSA =
    relay_3_success_GSA &&
    relay_4_success_GSA &&
    relay_5_success_GSA;

bool gsa049_pass =
    !broken_chain_delivery_GSA;

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-049 BROKEN CHAIN DELIVERY\n";
std::cout << "----------------------------------------\n";

std::cout << "Relay 3 Successful:        "
          << std::boolalpha
          << relay_3_success_GSA
          << "\n";

std::cout << "Relay 4 Successful:        "
          << relay_4_success_GSA
          << "\n";

std::cout << "Relay 5 Successful:        "
          << relay_5_success_GSA
          << "\n";

std::cout << "End-to-End Delivery:       "
          << broken_chain_delivery_GSA
          << "\n";

std::cout << "Expected Delivery:         false\n";

std::cout << "GSA-049: "
          << (gsa049_pass ? "PASS" : "FAIL")
          << "\n";

if (gsa049_pass)
{
    final_batch_pass_count_GSA++;
}


// ============================================================================
// GSA-050
// TOTAL PROPAGATION DELAY
//
// Splitting a fixed path into equal segments must not change
// pure propagation delay.
// ============================================================================

const double direct_propagation_delay_GSA =
    final_total_distance_GSA /
    final_speed_of_light_GSA;

const int propagation_test_hops_GSA = 4;

const double propagation_test_hop_distance_GSA =
    final_total_distance_GSA /
    static_cast<double>(
        propagation_test_hops_GSA);

double summed_hop_propagation_delay_GSA = 0.0;

for (int i = 0;
     i < propagation_test_hops_GSA;
     ++i)
{
    summed_hop_propagation_delay_GSA +=
        propagation_test_hop_distance_GSA /
        final_speed_of_light_GSA;
}

double propagation_delay_error_GSA =
    std::abs(
        summed_hop_propagation_delay_GSA -
        direct_propagation_delay_GSA);

bool gsa050_pass =
    propagation_delay_error_GSA <= 1.0e-12;

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-050 END-TO-END PROPAGATION DELAY\n";
std::cout << "----------------------------------------\n";

std::cout << "Direct Propagation Delay:  "
          << direct_propagation_delay_GSA
          << " s\n";

std::cout << "Four-Hop Propagation Delay:"
          << summed_hop_propagation_delay_GSA
          << " s\n";

std::cout << "Delay Error:               "
          << propagation_delay_error_GSA
          << " s\n";

std::cout << "GSA-050: "
          << (gsa050_pass ? "PASS" : "FAIL")
          << "\n";

if (gsa050_pass)
{
    final_batch_pass_count_GSA++;
}


// ============================================================================
// GSA-051
// EQUAL-RELAY BOTTLENECK INFORMATION RATE
//
// For 3 relays -> 4 equal hops.
// Controlled optical model predicts shorter hops have
// stronger received optical power and higher information rate.
// ============================================================================

const double four_hop_bottleneck_rate_GSA =
    equal_four_hop_metrics_GSA.Information_Rate;

const double direct_information_rate_GSA =
    direct_baseline_GSA.Information_Rate;

bool gsa051_pass =
    four_hop_bottleneck_rate_GSA >
    direct_information_rate_GSA;

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-051 BOTTLENECK INFORMATION RATE\n";
std::cout << "----------------------------------------\n";

std::cout << "Direct Link Rate:          "
          << direct_information_rate_GSA
          << " bits/s\n";

std::cout << "Four-Hop Bottleneck Rate:  "
          << four_hop_bottleneck_rate_GSA
          << " bits/s\n";

std::cout << "Expected Condition:        "
          << "four-hop rate > direct rate\n";

std::cout << "GSA-051: "
          << (gsa051_pass ? "PASS" : "FAIL")
          << "\n";

if (gsa051_pass)
{
    final_batch_pass_count_GSA++;
}


// ============================================================================
// GSA-052
// PACKET TRANSMISSION TIME ACROSS FOUR EQUAL HOPS
//
// This is TRANSMISSION time only.
// It does NOT yet include processing, queueing, PAT, or contact delays.
// ============================================================================

const double direct_packet_time_GSA =
    direct_baseline_GSA.Packet_Time;

const double four_hop_packet_time_GSA =
    equal_four_hop_metrics_GSA.Packet_Time *
    4.0;

bool gsa052_pass =
    four_hop_packet_time_GSA <
    direct_packet_time_GSA;

std::cout << "\n----------------------------------------\n";
std::cout << "GSA-052 END-TO-END PACKET TX TIME\n";
std::cout << "----------------------------------------\n";

std::cout << "Direct Packet TX Time:     "
          << direct_packet_time_GSA
          << " s\n";

std::cout << "Four-Hop Packet TX Time:   "
          << four_hop_packet_time_GSA
          << " s\n";

std::cout << "Expected Condition:        "
          << "four-hop controlled TX time < direct TX time\n";

std::cout << "Important:                 "
          << "processing/queue/PAT/contact delays not included\n";

std::cout << "GSA-052: "
          << (gsa052_pass ? "PASS" : "FAIL")
          << "\n";

if (gsa052_pass)
{
    final_batch_pass_count_GSA++;
}


// ============================================================================
// FINAL BATCH SUMMARY
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "FINAL NETWORK INTEGRATION SUMMARY\n";
std::cout << "========================================\n";

std::cout << "RESULT: "
          << final_batch_pass_count_GSA
          << " / "
          << final_batch_total_GSA
          << " tests passed\n";

std::cout << "BATCH RESULT: "
          << (final_batch_pass_count_GSA ==
                      final_batch_total_GSA
                  ? "PASS"
                  : "FAIL")
          << "\n";

std::cout << "========================================\n";




// ============================================================================
// RELAY TOPOLOGY SPACE VALIDATION
// RTS-001 THROUGH RTS-011
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "RELAY TOPOLOGY SPACE VALIDATION\n";
std::cout << "RTS-001 THROUGH RTS-011\n";
std::cout << "========================================\n";

Relay_Topology_Space RTS;

int RTS_Passed = 0;
int RTS_Total = 11;

const double RTS_Tolerance = 1e-9;


// ============================================================================
// RTS-001 HOP COUNT
// ============================================================================

{
    std::size_t relay_count = 3;

    std::size_t calculated_hops =
        RTS.Calculate_Hop_Count(relay_count);

    std::size_t expected_hops = 4;

    bool Test_Passed =
        calculated_hops == expected_hops;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-001 HOP COUNT\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Relay Count:        "
              << relay_count << '\n';

    std::cout << "Calculated Hops:    "
              << calculated_hops << '\n';

    std::cout << "Expected Hops:      "
              << expected_hops << '\n';

    std::cout << "RTS-001: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RTS-002 EQUAL RELAY SPACING
// ============================================================================

{
    double total_distance = 1000.0;
    std::size_t relay_count = 3;

    double calculated_spacing =
        RTS.Calculate_Equal_Relay_Spacing(
            total_distance,
            relay_count);

    double expected_spacing = 250.0;

    bool Test_Passed =
        std::abs(
            calculated_spacing -
            expected_spacing) < RTS_Tolerance;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-002 EQUAL RELAY SPACING\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Total Distance:       "
              << total_distance << '\n';

    std::cout << "Relay Count:          "
              << relay_count << '\n';

    std::cout << "Calculated Spacing:   "
              << calculated_spacing << '\n';

    std::cout << "Expected Spacing:     "
              << expected_spacing << '\n';

    std::cout << "RTS-002: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RTS-003 ROUTE RELIABILITY
// ============================================================================

{
    std::vector<double> hop_reliabilities =
    {
        0.90,
        0.80,
        0.95
    };

    double calculated_reliability =
        RTS.Calculate_Route_Reliability(
            hop_reliabilities);

    double expected_reliability =
        0.90 * 0.80 * 0.95;

    bool Test_Passed =
        std::abs(
            calculated_reliability -
            expected_reliability) < RTS_Tolerance;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-003 ROUTE RELIABILITY\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Calculated Reliability: "
              << calculated_reliability << '\n';

    std::cout << "Expected Reliability:   "
              << expected_reliability << '\n';

    std::cout << "RTS-003: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RTS-004 AVERAGE HOP RELIABILITY
// ============================================================================

{
    std::vector<double> hop_reliabilities =
    {
        0.90,
        0.80,
        0.95
    };

    double calculated_average =
        RTS.Calculate_Average_Hop_Reliability(
            hop_reliabilities);

    double expected_average =
        (0.90 + 0.80 + 0.95) / 3.0;

    bool Test_Passed =
        std::abs(
            calculated_average -
            expected_average) < RTS_Tolerance;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-004 AVERAGE HOP RELIABILITY\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Calculated Average: "
              << calculated_average << '\n';

    std::cout << "Expected Average:   "
              << expected_average << '\n';

    std::cout << "RTS-004: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RTS-005 WEAKEST HOP RELIABILITY
// ============================================================================

{
    std::vector<double> hop_reliabilities =
    {
        0.90,
        0.80,
        0.95
    };

    double calculated_weakest =
        RTS.Calculate_Weakest_Hop_Reliability(
            hop_reliabilities);

    double expected_weakest = 0.80;

    bool Test_Passed =
        std::abs(
            calculated_weakest -
            expected_weakest) < RTS_Tolerance;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-005 WEAKEST HOP RELIABILITY\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Calculated Weakest: "
              << calculated_weakest << '\n';

    std::cout << "Expected Weakest:   "
              << expected_weakest << '\n';

    std::cout << "RTS-005: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RTS-006 BOTTLENECK RATE
// ============================================================================

{
    std::vector<double> hop_rates =
    {
        100.0,
        80.0,
        120.0,
        95.0
    };

    double calculated_rate =
        RTS.Calculate_Bottleneck_Rate(
            hop_rates);

    double expected_rate = 80.0;

    bool Test_Passed =
        std::abs(
            calculated_rate -
            expected_rate) < RTS_Tolerance;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-006 BOTTLENECK RATE\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Calculated Bottleneck: "
              << calculated_rate << '\n';

    std::cout << "Expected Bottleneck:   "
              << expected_rate << '\n';

    std::cout << "RTS-006: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RTS-007 THROUGHPUT PER HOP
// ============================================================================

{
    double throughput = 400.0;
    std::size_t relay_count = 3;

    double calculated_throughput =
        RTS.Calculate_Throughput_Per_Hop(
            throughput,
            relay_count);

    double expected_throughput = 100.0;

    bool Test_Passed =
        std::abs(
            calculated_throughput -
            expected_throughput) < RTS_Tolerance;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-007 THROUGHPUT PER HOP\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Total Throughput:        "
              << throughput << '\n';

    std::cout << "Relay Count:             "
              << relay_count << '\n';

    std::cout << "Calculated Per Hop:      "
              << calculated_throughput << '\n';

    std::cout << "Expected Per Hop:        "
              << expected_throughput << '\n';

    std::cout << "RTS-007: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RTS-008 RELIABILITY MARGIN
// ============================================================================

{
    double packet_delivery = 0.97;
    double required_delivery = 0.95;

    double calculated_margin =
        RTS.Calculate_Reliability_Margin(
            packet_delivery,
            required_delivery);

    double expected_margin = 0.02;

    bool Test_Passed =
        std::abs(
            calculated_margin -
            expected_margin) < RTS_Tolerance;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-008 RELIABILITY MARGIN\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Packet Delivery:      "
              << packet_delivery << '\n';

    std::cout << "Required Delivery:    "
              << required_delivery << '\n';

    std::cout << "Calculated Margin:    "
              << calculated_margin << '\n';

    std::cout << "Expected Margin:      "
              << expected_margin << '\n';

    std::cout << "RTS-008: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RTS-009 THROUGHPUT GAIN PERCENTAGE
// ============================================================================

{
    double current_throughput = 120.0;
    double previous_throughput = 100.0;

    double calculated_gain =
        RTS.Calculate_Throughput_Gain_Percentage(
            current_throughput,
            previous_throughput);

    double expected_gain = 20.0;

    bool Test_Passed =
        std::abs(
            calculated_gain -
            expected_gain) < RTS_Tolerance;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-009 THROUGHPUT GAIN PERCENTAGE\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Current Throughput:    "
              << current_throughput << '\n';

    std::cout << "Previous Throughput:   "
              << previous_throughput << '\n';

    std::cout << "Calculated Gain:       "
              << calculated_gain << " %\n";

    std::cout << "Expected Gain:         "
              << expected_gain << " %\n";

    std::cout << "RTS-009: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RTS-010 MARGINAL THROUGHPUT BENEFIT
// ============================================================================

{
    double current_throughput = 120.0;
    double previous_throughput = 100.0;

    double calculated_benefit =
        RTS.Calculate_Marginal_Throughput_Benefit(
            current_throughput,
            previous_throughput);

    double expected_benefit = 20.0;

    bool Test_Passed =
        std::abs(
            calculated_benefit -
            expected_benefit) < RTS_Tolerance;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-010 MARGINAL THROUGHPUT BENEFIT\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Calculated Benefit: "
              << calculated_benefit << '\n';

    std::cout << "Expected Benefit:   "
              << expected_benefit << '\n';

    std::cout << "RTS-010: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RTS-011 DELAY PENALTY
// ============================================================================

{
    double current_delay = 0.45;
    double previous_delay = 0.30;

    double calculated_penalty =
        RTS.Calculate_Delay_Penalty(
            current_delay,
            previous_delay);

    double expected_penalty = 0.15;

    bool Test_Passed =
        std::abs(
            calculated_penalty -
            expected_penalty) < RTS_Tolerance;

    std::cout << "\n----------------------------------------\n";
    std::cout << "RTS-011 DELAY PENALTY\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Current Delay:       "
              << current_delay << '\n';

    std::cout << "Previous Delay:      "
              << previous_delay << '\n';

    std::cout << "Calculated Penalty:  "
              << calculated_penalty << '\n';

    std::cout << "Expected Penalty:    "
              << expected_penalty << '\n';

    std::cout << "RTS-011: "
              << (Test_Passed ? "PASS" : "FAIL")
              << '\n';

    if (Test_Passed)
    {
        ++RTS_Passed;
    }
}


// ============================================================================
// RELAY TOPOLOGY SPACE SUMMARY
// ============================================================================

std::cout << "\n========================================\n";
std::cout << "RELAY TOPOLOGY SPACE SUMMARY\n";
std::cout << "========================================\n";

std::cout << "RESULT: "
          << RTS_Passed
          << " / "
          << RTS_Total
          << " tests passed\n";

std::cout << "BATCH RESULT: "
          << (RTS_Passed == RTS_Total ? "PASS" : "FAIL")
          << '\n';

std::cout << "========================================\n";

























    return 0;
}