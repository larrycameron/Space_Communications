#include "../Kepler_Physics_Engine.h"

#include <Eigen/Dense>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>


// ============================================================
// SPACECRAFT STATE
// ============================================================

struct Spacecraft_State
{
    Eigen::Vector3d Position;
    Eigen::Vector3d Velocity;
};


// ============================================================
// PROPAGATE SPACECRAFT
//
// Basic constant-velocity kinematics:
//
// Position = Initial Position + Velocity * Time
// ============================================================

Spacecraft_State Propagate_Spacecraft(
    const Spacecraft_State& initial_state,
    double elapsed_time)
{
    Spacecraft_State current_state;

    current_state.Position =
        initial_state.Position
        +
        initial_state.Velocity * elapsed_time;

    current_state.Velocity =
        initial_state.Velocity;

    return current_state;
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    Keplers_Physics_Engine KPE;


    // ============================================================
    // CONSTANTS
    // ============================================================

    const double PI =
        3.141592653589793;

    const double Sun_Mass =
        1.989e30;


    // ============================================================
    // EARTH ORBIT
    // ============================================================

    const double Earth_Semi_Major_Axis =
        149600000.0;      // km

    const double Earth_Eccentricity =
        0.0167;


    // ============================================================
    // MARS ORBIT
    // ============================================================

    const double Mars_Semi_Major_Axis =
        227900000.0;      // km

    const double Mars_Eccentricity =
        0.0934;


    // ============================================================
    // INITIAL MEAN ANOMALIES
    // ============================================================

    const double Earth_Initial_Mean_Anomaly =
        0.0;

    const double Mars_Initial_Mean_Anomaly =
        PI;


    // ============================================================
    // MEAN MOTION
    // ============================================================

    const double Earth_Mean_Motion =
        KPE.Calculate_Mean_Motion(
            Sun_Mass,
            Earth_Semi_Major_Axis);

    const double Mars_Mean_Motion =
        KPE.Calculate_Mean_Motion(
            Sun_Mass,
            Mars_Semi_Major_Axis);


    // ============================================================
    // PRINT MEAN MOTION
    // ============================================================

    std::cout
        << std::setprecision(12);

    std::cout
        << "Earth Mean Motion: "
        << Earth_Mean_Motion
        << " rad/s\n";

    std::cout
        << "Mars Mean Motion: "
        << Mars_Mean_Motion
        << " rad/s\n";


    // ============================================================
    // PROPAGATE ORBITS FORWARD IN TIME
    // ============================================================

    const double Seconds_Per_Day =
        86400.0;

    const double Elapsed_Days =
        30.0;

    const double Elapsed_Time =
        Elapsed_Days *
        Seconds_Per_Day;


    // ============================================================
    // EARTH MEAN ANOMALY AFTER ELAPSED TIME
    // ============================================================

    double Earth_Mean_Anomaly =
        Earth_Initial_Mean_Anomaly
        +
        Earth_Mean_Motion *
        Elapsed_Time;


    // ============================================================
    // MARS MEAN ANOMALY AFTER ELAPSED TIME
    // ============================================================

    double Mars_Mean_Anomaly =
        Mars_Initial_Mean_Anomaly
        +
        Mars_Mean_Motion *
        Elapsed_Time;


    // ============================================================
    // KEEP ANGLES BETWEEN 0 AND 2PI
    // ============================================================

    Earth_Mean_Anomaly =
        std::fmod(
            Earth_Mean_Anomaly,
            2.0 * PI);

    Mars_Mean_Anomaly =
        std::fmod(
            Mars_Mean_Anomaly,
            2.0 * PI);


    if (Earth_Mean_Anomaly < 0.0)
    {
        Earth_Mean_Anomaly +=
            2.0 * PI;
    }


    if (Mars_Mean_Anomaly < 0.0)
    {
        Mars_Mean_Anomaly +=
            2.0 * PI;
    }


    // ============================================================
    // PRINT ORBITAL PROPAGATION
    // ============================================================

    std::cout
        << "\nElapsed Time: "
        << Elapsed_Days
        << " days\n";

    std::cout
        << "Earth Mean Anomaly: "
        << Earth_Mean_Anomaly
        << " radians\n";

    std::cout
        << "Mars Mean Anomaly: "
        << Mars_Mean_Anomaly
        << " radians\n";


    // ============================================================
// CALCULATE ECCENTRIC ANOMALIES
// ============================================================

double Earth_Eccentric_Anomaly =
    KPE.Calculate_Eccentric_Anomaly(
        Earth_Mean_Anomaly,
        Earth_Eccentricity);

double Mars_Eccentric_Anomaly =
    KPE.Calculate_Eccentric_Anomaly(
        Mars_Mean_Anomaly,
        Mars_Eccentricity);


// ============================================================
// CALCULATE TRUE ANOMALIES
// ============================================================

double Earth_True_Anomaly =
    KPE.Calculate_True_Anomaly(
        Earth_Eccentric_Anomaly,
        Earth_Eccentricity);

double Mars_True_Anomaly =
    KPE.Calculate_True_Anomaly(
        Mars_Eccentric_Anomaly,
        Mars_Eccentricity);


// ============================================================
// CALCULATE ORBITAL RADII
// ============================================================

double Earth_Orbital_Radius =
    KPE.Calculate_Orbital_Radius(
        Earth_Semi_Major_Axis,
        Earth_Eccentricity,
        Earth_Eccentric_Anomaly);

double Mars_Orbital_Radius =
    KPE.Calculate_Orbital_Radius(
        Mars_Semi_Major_Axis,
        Mars_Eccentricity,
        Mars_Eccentric_Anomaly);


// ============================================================
// PRINT ORBITAL STATE
// ============================================================

std::cout
    << "\nEarth Eccentric Anomaly: "
    << Earth_Eccentric_Anomaly
    << " radians\n";

std::cout
    << "Mars Eccentric Anomaly: "
    << Mars_Eccentric_Anomaly
    << " radians\n";

std::cout
    << "\nEarth True Anomaly: "
    << Earth_True_Anomaly
    << " radians\n";

std::cout
    << "Mars True Anomaly: "
    << Mars_True_Anomaly
    << " radians\n";

std::cout
    << "\nEarth Orbital Radius: "
    << Earth_Orbital_Radius
    << " km\n";

std::cout
    << "Mars Orbital Radius: "
    << Mars_Orbital_Radius
    << " km\n";

// ============================================================
// CALCULATE HELIOCENTRIC CARTESIAN POSITIONS
//
// For this first Experiment 2 test, we use a 2D orbital plane:
// x = r cos(true anomaly)
// y = r sin(true anomaly)
// z = 0
// ============================================================

Eigen::Vector3d Earth_Position(
    Earth_Orbital_Radius *
        std::cos(Earth_True_Anomaly),

    Earth_Orbital_Radius *
        std::sin(Earth_True_Anomaly),

    0.0);


Eigen::Vector3d Mars_Position(
    Mars_Orbital_Radius *
        std::cos(Mars_True_Anomaly),

    Mars_Orbital_Radius *
        std::sin(Mars_True_Anomaly),

    0.0);


// ============================================================
// EARTH TO MARS RANGE
// ============================================================

Eigen::Vector3d Earth_To_Mars_Vector =
    Mars_Position -
    Earth_Position;

double Earth_To_Mars_Range =
    Earth_To_Mars_Vector.norm();


// ============================================================
// PRINT PLANET POSITIONS
// ============================================================

std::cout
    << "\nEarth Position (km):\n"
    << Earth_Position
    << "\n";

std::cout
    << "\nMars Position (km):\n"
    << Mars_Position
    << "\n";

std::cout
    << "\nEarth-to-Mars Range: "
    << Earth_To_Mars_Range
    << " km\n";


    // ============================================================
    // SPACECRAFT INITIAL STATE
    //
    // These are temporary debugging values.
    // We will later initialize the spacecraft relative to Earth.
    // ============================================================

    Spacecraft_State Spacecraft_Initial;

    // ============================================================
// EARTH POSITION AT DAY 0
// ============================================================

double Earth_Initial_Eccentric_Anomaly =
    KPE.Calculate_Eccentric_Anomaly(
        Earth_Initial_Mean_Anomaly,
        Earth_Eccentricity);

double Earth_Initial_True_Anomaly =
    KPE.Calculate_True_Anomaly(
        Earth_Initial_Eccentric_Anomaly,
        Earth_Eccentricity);

double Earth_Initial_Orbital_Radius =
    KPE.Calculate_Orbital_Radius(
        Earth_Semi_Major_Axis,
        Earth_Eccentricity,
        Earth_Initial_Eccentric_Anomaly);

Eigen::Vector3d Earth_Initial_Position(
    Earth_Initial_Orbital_Radius *
        std::cos(Earth_Initial_True_Anomaly),

    Earth_Initial_Orbital_Radius *
        std::sin(Earth_Initial_True_Anomaly),

    0.0);

// ============================================================
// SPACECRAFT INITIAL STATE
// ============================================================

    Spacecraft_Initial.Position = Earth_Initial_Position;
    // Initial spacecraft position
    //
    // X = 0 meters
    // Y = 0 meters
    // Z = 0 meters

    Spacecraft_Initial.Position =  Earth_Position;


    // Initial spacecraft velocity
    //
    // X direction = 1000 m/s
    //
    // This is a temporary test value.

    Spacecraft_Initial.Velocity =
        Eigen::Vector3d(
            1.0,
            0.0,
            0.0);


    // ============================================================
    // PROPAGATE SPACECRAFT
    // ============================================================

    Spacecraft_State Spacecraft_Current =
        Propagate_Spacecraft(
            Spacecraft_Initial,
            Elapsed_Time);


    // ============================================================
    // PRINT SPACECRAFT STATE
    // ============================================================

    std::cout
        << "\nSpacecraft Position:\n"
        << Spacecraft_Current.Position
        << "\n";

    std::cout
        << "\nSpacecraft Velocity:\n"
        << Spacecraft_Current.Velocity
        << "\n";

    
// ============================================================
// EARTH TO SPACECRAFT RANGE
// ============================================================

Eigen::Vector3d Earth_To_Spacecraft_Vector =
    Spacecraft_Current.Position
    -
    Earth_Position;

double Earth_To_Spacecraft_Range =
    Earth_To_Spacecraft_Vector.norm();


// ============================================================
// SPACECRAFT TO MARS RANGE
// ============================================================

Eigen::Vector3d Spacecraft_To_Mars_Vector =
    Mars_Position
    -
    Spacecraft_Current.Position;

double Spacecraft_To_Mars_Range =
    Spacecraft_To_Mars_Vector.norm();


// ============================================================
// PRINT SPACECRAFT RANGES
// ============================================================

std::cout
    << "\nEarth-to-Spacecraft Range: "
    << Earth_To_Spacecraft_Range
    << " km\n";

std::cout
    << "Spacecraft-to-Mars Range: "
    << Spacecraft_To_Mars_Range
    << " km\n";

// ============================================================
// PROPAGATION DELAY
//
// delay = distance / speed of light
//
// Distances are currently in kilometers, so use the
// speed of light in kilometers per second.
// ============================================================

const double Speed_Of_Light =
    299792.458;   // km/s


// ============================================================
// EARTH TO MARS PROPAGATION DELAY
// ============================================================

double Earth_To_Mars_Delay =
    Earth_To_Mars_Range
    /
    Speed_Of_Light;


// ============================================================
// EARTH TO SPACECRAFT PROPAGATION DELAY
// ============================================================

double Earth_To_Spacecraft_Delay =
    Earth_To_Spacecraft_Range
    /
    Speed_Of_Light;


// ============================================================
// SPACECRAFT TO MARS PROPAGATION DELAY
// ============================================================

double Spacecraft_To_Mars_Delay =
    Spacecraft_To_Mars_Range
    /
    Speed_Of_Light;


// ============================================================
// PRINT PROPAGATION DELAYS
// ============================================================

std::cout
    << "\nEarth-to-Mars Propagation Delay: "
    << Earth_To_Mars_Delay
    << " seconds\n";

std::cout
    << "Earth-to-Spacecraft Propagation Delay: "
    << Earth_To_Spacecraft_Delay
    << " seconds\n";

std::cout
    << "Spacecraft-to-Mars Propagation Delay: "
    << Spacecraft_To_Mars_Delay
    << " seconds\n";

    return 0;
}