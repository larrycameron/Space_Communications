#ifndef KEPLER_PHYSICS_ENGINE_H
#define KEPLER_PHYSICS_ENGINE_H

#include <vector>
#include <string>
#include <Eigen/Dense>


class Keplers_Physics_Engine
{
private:
    std::vector<double> Eccentricity_Samples;
    std::vector<double> Mean_Anomaly_Samples;
    std::vector<double> Eccentric_Anomaly_Samples;
    std::vector<double> True_Anomaly_Samples;
    std::vector<double> Orbital_Radius_Samples;
    std::vector<double> Mean_Motion_Samples;
    double Mean_Anomaly{};
    double Eccentric_Anomaly{};
    double Orbital_Eccentricity{};// e
    double Semi_Major_a{};
    double Semi_Major_b{};
    double Planets_Distance_From_The_Sun{};
    double Correction{}; 
    double Mean_Motion{};
    double True_Anomaly{};
    double Argument_of_Periapsis_w{};
    double Inclination_Orbit_Tilt{};
    double Longitude_Ascending_Node{};
    double X_Position{};
    double Y_Position{};
    double Z_Position{};
    
    
    static constexpr double G {6.674e-11}; // m^3*kg^-1*s^-2
    static constexpr double PI {3.141592653589793};
    
    double Sun_Mass{1.989e30};// Sun Mass kg
    double Planet_Mass{};// kg
    double Average_Distance_From_Center{};
    double Eccentricity_Vector{};
    double Node_Vector{};
    double Angular_Momentum_Vector_h{};
    double Z_Component_h_z{};  
    
    std::size_t Sample_Size{10};

public:
    
    double Calculate_Orbital_Eccentricity(double semi_major_a, double semi_major_b);
    double Calculate_Mean_Anomaly(double eccentric_anomaly, double orbital_eccentricity);
    double Calculate_Planets_Distance_From_The_Sun(double average_distance_from_sun, double orbital_eccentricity, double eccentric_anomaly);
    double Calculate_Correction(double eccentric_anomaly, double orbital_eccentricity, double mean_anomaly);
    double Calculate_Eccentric_Anomaly(double mean_anomaly, double orbital_eccentricity);
    double Calculate_Mean_Motion(double sun_mass, double average_distance_from_center);
    double Calculate_True_Anomaly( double eccentric_anomaly, double orbital_eccentricity);
    double Calculate_Orbital_Radius( double semi_major_axis, double orbital_eccentricity, double eccentric_anomaly);
    double Calculate_Argument_of_Periapsis(const Eigen::Vector3d& Eccentricity_Vector, const Eigen::Vector3d& Node_Vector);
    double Inclination_Orbit_Tilt_Calculation(const Eigen::Vector3d& Angular_Momentum_Vector_h);
    double Calculate_Longitude_Ascending_Node(const Eigen::Vector3d& Node_Vector);
    Eigen::Vector3d Calculate_Cartesian_Position(double orbital_radius, double True_Anomaly, double inclination, double longitude_ascending_node, double Argument_of_Periapis_w);
    void Run_Experiments();
    void Run_All_Statistics();
    void Print_Kepler_Physics_Engine_Calculations() const;
   

};    

#endif

