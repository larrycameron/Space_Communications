#ifndef SPACECRAFT_GEOMETRY_H
#define SPACECRAFT_GEOMETRY_H

#include <Eigen/Dense>

class Spacecraft_Geometry
{
private:
    double Latitude_Of_Ground_Station{};      // radians
    double Longitude_Of_Ground_Station{};     // radians
    double h_Altitude_of_Ground_Station{};    // meters

    double X_Position_Ground_Station_Components{};
    double Y_Position_Ground_Station_Components{};
    double Z_Position_Ground_Station_Components{};

    double X_Spacecraft_Position_Components_in_Space{};
    double Y_Spacecraft_Position_Components_in_Space{};
    double Z_Spacecraft_Position_Components_in_Space{};

    double Spacecraft_Position_Time_T{};
    double t_Time_Epoch_of_Position_Fix{};

    double a_Earths_Semi_Major_Axis{6378137.0};      // meters
    double e_Earths_Eccentricity{0.0818191908426};  // dimensionless

    double N_Prime_Vertical_Radius_Of_Curvature{};  // meters

    double Range_Scalar_Magnitude{};                 // meters

    double Greenwich_Sidereal_Time_angle{};          // radians

    double South_Component{};                        // meters
    double East_Component{};                         // meters
    double Zenith_Component{};                       // meters

    double Elevation_Angle_Above_Horizon{};          // degrees
    double Azimuth_Angle_From_North{};               // degrees

    static constexpr double PI{3.141592653589793};
    static constexpr double Earth_Rotation_Rate{7.2921150e-5}; // rad/s
    
    Eigen::Vector3d Spacecraft_Position{};
    Eigen::Vector3d Range_Vector{};
    Eigen::Vector3d Line_Of_Sight_Unit_Vector{}; 
    Eigen::Matrix3d R_Rotation_Matrix_Z_Polar_Axis{};
    Eigen::Matrix3d Reference_Frame_Consistency{};
    Eigen::Vector3d Spacecraft_Position_ECEF{};

public:
    Eigen::Vector3d Ground_Station_Positions_long_lat_alt(double longitude,  double latitude, double altitude);

    Eigen::Vector3d Spacecraft_Position_x_y_z(double X, double Y, double Z);

    Eigen::Vector3d Calculate_Range_Vector(const Eigen::Vector3d& spacecraft_position, const Eigen::Vector3d& ground_station_position);

    double Calculate_Range_Magnitude(const Eigen::Vector3d& range_vector);

    Eigen::Vector3d Calculate_Line_Of_Sight(const Eigen::Vector3d& range_vector);

    Eigen::Matrix3d Calculate_ECI_To_ECEF_Rotation(double greenwich_sidereal_time);

    Eigen::Vector3d Convert_ECI_To_ECEF(const Eigen::Vector3d& position_eci, double greenwich_sidereal_time);
    
    Eigen::Vector3d Calculate_SEZ_Components( const Eigen::Vector3d& range_vector_ecef, double latitude, double longitude);
    
    double Calculate_Greenwich_Sidereal_Time(double gmst_at_epoch, double elapsed_seconds);
    
    double Elevation_Angle(double Zenith_Component,double Range_Magnitude);

    double Azimuth_Angle(double East_Component, double South_Component);

    void Print_Spacecraft_Geometry() const;
};

#endif


//Range Vector
//Range (Scalar Magnitude)
//Line-of-Sight
//Elevation
//Azimuth


// X_Position_Ground_Station_Components = (N_Prime_Vertical_Radius_Of_Curvature + h_Altitude_of_Ground_Station) * std::cos(Latitude_Of_Ground_Station) * std::cos(Longitude_Of_Ground_Station)

//Y_Position_Ground_Station_Components = (N_Prime_Vertical_Radius_Of_Curvature + h_Altitude_of_Ground_Station) * std::cos(Latitude_Of_Ground_Station) * std::sin(Longitude_Of_Ground_Station)

//Z_Position_Ground_Station_Components = [N_Prime_Vertical_Radius_Of_Curvature * (1 - std::pow(e_Earths_Eccentricity, 2)) + h_Altitude_of_Ground_Station] * std::sin(Latitude_Of_Ground_Station)

//N_Prime_Vertical_Radius_Of_Curvature = a_Earths_Semi_Major_Axis /  std::sqrt(1.0 - std::pow(e_Earths_Eccentricity, 2) * std::pow(std::sin(Latitude_Of_Ground_Station), 2));

//Spacecraft XYZ 3 x 1 matric spacecraft_position_X, spacecraft_position_y, spacecraft_postion_2
//Reference Frame Consistency  3 X 3 Earth_Centered_Inertial_Frame_fixed_relative_to_stars vs. Earth_Centered_Earth_Fixed_Frame_rotates_with_Earth
//P_Range_Vector_Station_to_Spacecraft = _vectorss_xyz
//Range_Scaler_Magnitude = std::sqrt(std::(X_Spacecraft_Position_Components_in_Space - X_Position_Ground_Station_Components, 2 ) + std::(Y_Spacecraft_Position_Components_in_Space - Y_Position_Ground_Station_Components, 2 ) + std::(Z_Spacecraft_Position_Components_in_Space - Z_Position_Ground_Station_Components, 2 ) )



