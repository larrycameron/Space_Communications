#include <algorithm>
#include "Kepler_Physics_Engine.h"
#include "Statistical_Data.h"

#include <iostream>
#include <cmath>
#include <random>
#include <limits>


double Keplers_Physics_Engine::Calculate_Orbital_Eccentricity(double semi_major_a, double semi_major_b)
{
    Semi_Major_a = semi_major_a;
    Semi_Major_b = semi_major_b;

    if (Semi_Major_a <= 0.0 ||  Semi_Major_b < 0.0 || Semi_Major_b > Semi_Major_a)
    {
        Orbital_Eccentricity = std::numeric_limits<double>::quiet_NaN();

        return Orbital_Eccentricity;
    }

    Orbital_Eccentricity = std::sqrt(1.0 -(std::pow(Semi_Major_b, 2) / std::pow(Semi_Major_a, 2)));

    return Orbital_Eccentricity;
}


double Keplers_Physics_Engine::Calculate_Mean_Anomaly( double eccentric_anomaly, double orbital_eccentricity)
{
    Eccentric_Anomaly = eccentric_anomaly;
    Orbital_Eccentricity = orbital_eccentricity;

    if (Orbital_Eccentricity < 0.0 ||  Orbital_Eccentricity >= 1.0)
    {
        Mean_Anomaly =  std::numeric_limits<double>::quiet_NaN();

        return Mean_Anomaly;
    }

    Mean_Anomaly =  Eccentric_Anomaly -  Orbital_Eccentricity *  std::sin(Eccentric_Anomaly);

    return Mean_Anomaly;
}

double Keplers_Physics_Engine::Calculate_Planets_Distance_From_The_Sun(double average_distance_from_sun, double orbital_eccentricity, double eccentric_anomaly)
{
    return Calculate_Orbital_Radius(average_distance_from_sun, orbital_eccentricity, eccentric_anomaly);
}

double Keplers_Physics_Engine::Calculate_Correction(double eccentric_anomaly, double orbital_eccentricity, double mean_anomaly)
{
    Eccentric_Anomaly = eccentric_anomaly;
    Orbital_Eccentricity = orbital_eccentricity;
    Mean_Anomaly = mean_anomaly;

    if (Orbital_Eccentricity < 0.0 || Orbital_Eccentricity >= 1.0)
    {
        Correction =  std::numeric_limits<double>::quiet_NaN();

        return Correction;
    }

    Correction = (Eccentric_Anomaly -  Orbital_Eccentricity * std::sin(Eccentric_Anomaly) -  Mean_Anomaly) /(1.0 - Orbital_Eccentricity * std::cos(Eccentric_Anomaly));

    return Correction;
}

double Keplers_Physics_Engine::Calculate_Eccentric_Anomaly( double mean_anomaly, double orbital_eccentricity)
{
    Mean_Anomaly = mean_anomaly;
    Orbital_Eccentricity = orbital_eccentricity;

    if (Orbital_Eccentricity < 0.0 || Orbital_Eccentricity >= 1.0)
    {
        Eccentric_Anomaly =  std::numeric_limits<double>::quiet_NaN();

        return Eccentric_Anomaly;
    }

    double E = Mean_Anomaly;

    for (std::size_t i = 0; i < 10; ++i)
    {
        Correction = Calculate_Correction( E, Orbital_Eccentricity, Mean_Anomaly);

        E = E - Correction;

        if (std::abs(Correction) < 1e-12)
        {
            break;
        }
    }

    Eccentric_Anomaly = E;

    return Eccentric_Anomaly;
}

double Keplers_Physics_Engine::Calculate_Mean_Motion(double sun_mass, double average_distance_from_center)
{
    Sun_Mass = sun_mass;

    if (Sun_Mass <= 0.0 || average_distance_from_center <= 0.0)
    {
        Mean_Motion = std::numeric_limits<double>::quiet_NaN();

        return Mean_Motion;
    }

    double distance_meters = average_distance_from_center * 1000.0;

    Mean_Motion = std::sqrt((G * Sun_Mass) / std::pow(distance_meters, 3));

    return Mean_Motion;
}


double Keplers_Physics_Engine::Calculate_True_Anomaly(double eccentric_anomaly, double orbital_eccentricity)
{
    Eccentric_Anomaly = eccentric_anomaly;
    Orbital_Eccentricity = orbital_eccentricity;

    if (Orbital_Eccentricity < 0.0 || Orbital_Eccentricity >= 1.0)
    {
        True_Anomaly = std::numeric_limits<double>::quiet_NaN();

        return True_Anomaly;
    }

    True_Anomaly = 2.0 * std::atan2(std::sqrt(1.0 + Orbital_Eccentricity) * std::sin(Eccentric_Anomaly / 2.0),

    std::sqrt(1.0 - Orbital_Eccentricity) * std::cos(Eccentric_Anomaly / 2.0));

    return True_Anomaly;
}


double Keplers_Physics_Engine::Calculate_Orbital_Radius(double semi_major_a, double orbital_eccentricity, double eccentric_anomaly)
{
    Semi_Major_a = semi_major_a;
    Orbital_Eccentricity = orbital_eccentricity;
    Eccentric_Anomaly = eccentric_anomaly;

    if (Semi_Major_a <= 0.0 ||  Orbital_Eccentricity < 0.0 ||  Orbital_Eccentricity >= 1.0)
    {
        Planets_Distance_From_The_Sun = std::numeric_limits<double>::quiet_NaN();

        return Planets_Distance_From_The_Sun;
    }

    Planets_Distance_From_The_Sun =  Semi_Major_a * (1.0 - Orbital_Eccentricity * std::cos(Eccentric_Anomaly));

    return Planets_Distance_From_The_Sun;
}

double Keplers_Physics_Engine::Calculate_Argument_of_Periapsis(const Eigen::Vector3d& eccentricity_vector, const Eigen::Vector3d& node_vector)
{
    if (node_vector.norm() == 0.0 || eccentricity_vector.norm() == 0.0)
    {
        Argument_of_Periapsis_w = std::numeric_limits<double>::quiet_NaN();

        return Argument_of_Periapsis_w;
    }

    double cosine_value = node_vector.dot(eccentricity_vector) / (node_vector.norm() * eccentricity_vector.norm());

    cosine_value = std::clamp(cosine_value, -1.0, 1.0);

    Argument_of_Periapsis_w = std::acos(cosine_value);

    if (eccentricity_vector.z() < 0.0)
    {
        Argument_of_Periapsis_w = 2.0 * PI - Argument_of_Periapsis_w;
    }

    return Argument_of_Periapsis_w;
}


double Keplers_Physics_Engine::Inclination_Orbit_Tilt_Calculation(const Eigen::Vector3d& angular_momentum_vector_h)
{
    if (angular_momentum_vector_h.norm() == 0.0)
    {
        Inclination_Orbit_Tilt = std::numeric_limits<double>::quiet_NaN();

        return Inclination_Orbit_Tilt;
    }

    double cosine_value =  angular_momentum_vector_h.z() / angular_momentum_vector_h.norm();

    cosine_value = std::clamp(cosine_value, -1.0, 1.0);

    Inclination_Orbit_Tilt = std::acos(cosine_value);
    
    return Inclination_Orbit_Tilt;
}

double Keplers_Physics_Engine::Calculate_Longitude_Ascending_Node(const Eigen::Vector3d& node_vector)
{
    if (node_vector.norm() == 0.0)
    {
        Longitude_Ascending_Node = std::numeric_limits<double>::quiet_NaN();

        return Longitude_Ascending_Node;
    }

    Longitude_Ascending_Node = std::atan2(node_vector.y(), node_vector.x());

    if (Longitude_Ascending_Node < 0.0)
    {
        Longitude_Ascending_Node += 2.0 * PI;
    }

    return Longitude_Ascending_Node;
}


Eigen::Vector3d Keplers_Physics_Engine::Calculate_Cartesian_Position( double orbital_radius, double true_anomaly, double inclination, double longitude_ascending_node, double argument_of_periapsis_w)
{
    X_Position = orbital_radius * (std::cos(longitude_ascending_node) * std::cos(argument_of_periapsis_w + true_anomaly) - std::sin(longitude_ascending_node) * std::sin(argument_of_periapsis_w + true_anomaly) * std::cos(inclination));

    Y_Position = orbital_radius * (std::sin(longitude_ascending_node) * std::cos(argument_of_periapsis_w + true_anomaly) + std::cos(longitude_ascending_node) * std::sin(argument_of_periapsis_w + true_anomaly) * std::cos(inclination));

    Z_Position = orbital_radius * std::sin(argument_of_periapsis_w + true_anomaly) * std::sin(inclination);  
    
    Eigen::Vector3d Position(X_Position, Y_Position, Z_Position);

    return Position;
}


void Keplers_Physics_Engine::Run_Experiments()
{
    
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> semi_major_a_dist(149500000.0 , 149700000.0);

    std::uniform_real_distribution<double> eccentricity_dist(0.0160, 0.0175);

    std::uniform_real_distribution<double> mean_anomaly_dist(0.0, 2.0 * PI );

    Eccentricity_Samples.clear();
    Mean_Anomaly_Samples.clear();
    Eccentric_Anomaly_Samples.clear();
    True_Anomaly_Samples.clear();
    Orbital_Radius_Samples.clear();
    Mean_Motion_Samples.clear();

    for (std::size_t i = 0; i < Sample_Size; ++i)
    {
        double Mean_Anomaly_Result = mean_anomaly_dist(gen);

        double semi_major_a = semi_major_a_dist(gen);

        double orbital_eccentricity = eccentricity_dist(gen);

        double semi_major_b = semi_major_a * std::sqrt(1.0 - orbital_eccentricity * orbital_eccentricity);

        double Eccentricity_Result = Calculate_Orbital_Eccentricity( semi_major_a, semi_major_b);
        double Eccentric_Anomaly_Result = Calculate_Eccentric_Anomaly( Mean_Anomaly_Result, Eccentricity_Result);
        double True_Anomaly_Result = Calculate_True_Anomaly(Eccentric_Anomaly_Result, Eccentricity_Result);
        double Orbital_Radius_Result = Calculate_Orbital_Radius( semi_major_a, Eccentricity_Result, Eccentric_Anomaly_Result);
        double Mean_Motion_Result =  Calculate_Mean_Motion( Sun_Mass, semi_major_a);


        Eccentricity_Samples.push_back(Eccentricity_Result);
        Mean_Anomaly_Samples.push_back(Mean_Anomaly_Result);
        Eccentric_Anomaly_Samples.push_back(Eccentric_Anomaly_Result);
        True_Anomaly_Samples.push_back(True_Anomaly_Result);
        Orbital_Radius_Samples.push_back(Orbital_Radius_Result);
        Mean_Motion_Samples.push_back(Mean_Motion_Result);


        std::cout<< "================================Run Experiment=============================================================="<< std::endl;
        std::cout<< "Experiment " << i + 1 << std::endl;
        std::cout<< "Semi-major axis: " << semi_major_a<< " km" << std::endl;
        std::cout<< "Semi-minor axis: " << semi_major_b << " km"<< std::endl;
        std::cout<< "Eccentricity: " << Eccentricity_Result << std::endl;
        std::cout<< "Mean Anomaly: " << Mean_Anomaly_Result * (180.0 / PI) << " degrees" << std::endl;
        std::cout<< "Eccentric Anomaly: " << Eccentric_Anomaly_Result * (180.0 / PI) << " degrees"<< std::endl;
        std::cout<< "True Anomaly: "<< True_Anomaly_Result * (180.0 / PI)<< " degrees"<< std::endl;
        std::cout<< "Orbital Radius: "<< Orbital_Radius_Result<< " km"<< std::endl;
        std::cout<< "Mean Motion: " << Mean_Motion_Result << " rad/s" << std::endl;
        std::cout<< "================================================================================================================"<< std::endl;
    }
}


void Keplers_Physics_Engine::Run_All_Statistics()
{
    Statistical_Data stats;

    stats.Statistics_Sample_Size(Eccentricity_Samples, "Eccentricity", false);
    stats.Statistics_Sample_Size(Mean_Anomaly_Samples, "Mean Anomaly", true);
    stats.Statistics_Sample_Size(Eccentric_Anomaly_Samples, "Eccentric Anomaly", true);
    stats.Statistics_Sample_Size(True_Anomaly_Samples, "True Anomaly", true);
    stats.Statistics_Sample_Size( Orbital_Radius_Samples, "Orbital Radius", false);
    stats.Statistics_Sample_Size(Mean_Motion_Samples, "Mean Motion", false);
}


void Keplers_Physics_Engine::Print_Kepler_Physics_Engine_Calculations() const
{
    std::cout<< "===================================Summary of Kepler Physics Calculations=============================================="<< std::endl;
    std::cout<< "The shape of the elliptical orbit: " << Orbital_Eccentricity << std::endl;
    std::cout<< "The uniform time-based angle: "<< Mean_Anomaly * (180.0 / PI) << " degrees" << std::endl;
    std::cout<< "The orbital distance from the sun: " << Planets_Distance_From_The_Sun << " km" << std::endl;
    std::cout << "Newton-Raphson eccentric anomaly correction: " << Correction << " rad" << std::endl;
    std::cout<< "The position of a planet along a perfect auxiliary circle: "<< Eccentric_Anomaly * (180.0 / PI)<< " degrees"<< std::endl;
    std::cout << "The average angular speed of the orbital body: "<< Mean_Motion<< " rad/s"<< std::endl;
    std::cout<< "The actual physical direction of the planet as seen directly from the Sun: "<< True_Anomaly * (180.0 / PI)<< " degrees"<< std::endl;
    std::cout<< "The planet's distance from the sun: "<< Planets_Distance_From_The_Sun<< " km"<< std::endl;
    std::cout << "The orientation of the orbit: " << Argument_of_Periapsis_w * (180.0 / PI) << " degrees" << std::endl;
    std::cout << "The vertical tilt of the object's orbital plane: " << Inclination_Orbit_Tilt * (180.0 / PI)<< " degrees" << std::endl;
    std::cout << "The horizontal orientation of the orbital plane: " << Longitude_Ascending_Node * (180.0 / PI)<< " degrees" << std::endl;
    std::cout<< "The planetary position: " << X_Position << " X, " << Y_Position << " Y, " << Z_Position << " Z" << std::endl;
    std::cout<< "===================================Summary of Kepler Physics Calculations=============================================="<< std::endl;
}


