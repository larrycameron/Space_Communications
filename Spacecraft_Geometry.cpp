#include "Spacecraft_Geometry.h"

#include <Eigen/Dense>
#include <cmath>
#include <iostream>


Eigen::Vector3d Spacecraft_Geometry::Ground_Station_Positions_long_lat_alt(double longitude, double latitude, double altitude)
{
    Longitude_Of_Ground_Station = longitude;
    Latitude_Of_Ground_Station = latitude;
    h_Altitude_of_Ground_Station = altitude;

    N_Prime_Vertical_Radius_Of_Curvature = a_Earths_Semi_Major_Axis / std::sqrt(1.0 - std::pow(e_Earths_Eccentricity, 2) * std::pow(std::sin(latitude), 2));

    X_Position_Ground_Station_Components = (N_Prime_Vertical_Radius_Of_Curvature + altitude) *  std::cos(latitude) *  std::cos(longitude);

    Y_Position_Ground_Station_Components = (N_Prime_Vertical_Radius_Of_Curvature + altitude) *  std::cos(latitude) *  std::sin(longitude);

    Z_Position_Ground_Station_Components = (N_Prime_Vertical_Radius_Of_Curvature * (1.0 - std::pow(e_Earths_Eccentricity, 2)) + altitude) * std::sin(latitude);

    return Eigen::Vector3d( X_Position_Ground_Station_Components, Y_Position_Ground_Station_Components, Z_Position_Ground_Station_Components);
}


Eigen::Vector3d Spacecraft_Geometry::Spacecraft_Position_x_y_z( double spacecraft_position_X, double spacecraft_position_Y, double spacecraft_position_Z)
{
    Spacecraft_Position = Eigen::Vector3d(spacecraft_position_X, spacecraft_position_Y, spacecraft_position_Z);

    return Spacecraft_Position;
}


Eigen::Vector3d Spacecraft_Geometry::Calculate_Range_Vector( const Eigen::Vector3d& spacecraft_position, const Eigen::Vector3d& ground_station_position)
{
    Range_Vector = spacecraft_position - ground_station_position;

    return Range_Vector;
}


double Spacecraft_Geometry::Calculate_Range_Magnitude(const Eigen::Vector3d& range_vector)
{
    Range_Scalar_Magnitude = range_vector.norm();

    return Range_Scalar_Magnitude;
}

Eigen::Vector3d Spacecraft_Geometry::Calculate_Line_Of_Sight( const Eigen::Vector3d& range_vector)
{
    double magnitude = range_vector.norm();

    if (magnitude < 1e-9)
    {
        Line_Of_Sight_Unit_Vector =  Eigen::Vector3d(0.0, 0.0, 0.0);

        return Line_Of_Sight_Unit_Vector;
    }

    Line_Of_Sight_Unit_Vector = range_vector / magnitude;

    return Line_Of_Sight_Unit_Vector;
}

Eigen::Matrix3d Spacecraft_Geometry::Calculate_ECI_To_ECEF_Rotation( double greenwich_sidereal_time)
{
    double theta = greenwich_sidereal_time;

    R_Rotation_Matrix_Z_Polar_Axis <<
         std::cos(theta),  std::sin(theta), 0.0,
        -std::sin(theta),  std::cos(theta), 0.0,
         0.0,              0.0,             1.0;

    return R_Rotation_Matrix_Z_Polar_Axis;
}

Eigen::Vector3d Spacecraft_Geometry::Convert_ECI_To_ECEF(const Eigen::Vector3d& position_eci, double greenwich_sidereal_time)
{
    Reference_Frame_Consistency = Calculate_ECI_To_ECEF_Rotation(greenwich_sidereal_time );

    Spacecraft_Position_ECEF = Reference_Frame_Consistency *  position_eci;

    return Spacecraft_Position_ECEF;
}

Eigen::Vector3d Spacecraft_Geometry::Calculate_SEZ_Components(const Eigen::Vector3d& range_vector_ecef, double latitude, double longitude)
{
    double dx = range_vector_ecef.x();
    double dy = range_vector_ecef.y();
    double dz = range_vector_ecef.z();

    South_Component = std::sin(latitude) * std::cos(longitude) * dx + std::sin(latitude) * std::sin(longitude) * dy  - std::cos(latitude) * dz;

    East_Component =  -std::sin(longitude) * dx  +  std::cos(longitude) * dy;

    Zenith_Component = std::cos(latitude) * std::cos(longitude) * dx  + std::cos(latitude) * std::sin(longitude) * dy  + std::sin(latitude) * dz;

    return Eigen::Vector3d( South_Component, East_Component, Zenith_Component);

}

double Spacecraft_Geometry::Calculate_Greenwich_Sidereal_Time(double gmst_at_epoch, double elapsed_seconds)
{
    Spacecraft_Position_Time_T = elapsed_seconds;

    Greenwich_Sidereal_Time_angle =  gmst_at_epoch +  Earth_Rotation_Rate * elapsed_seconds;

    Greenwich_Sidereal_Time_angle = std::fmod(Greenwich_Sidereal_Time_angle, 2.0 * PI);

    if (Greenwich_Sidereal_Time_angle < 0.0)
    {
        Greenwich_Sidereal_Time_angle += 2.0 * PI;
    }

    return Greenwich_Sidereal_Time_angle;
}

double Spacecraft_Geometry::Elevation_Angle( double Zenith_Component,  double Range_Magnitude)
{
    if (Range_Magnitude < 1e-9)
    {
        return 0.0;
    }

    double ratio = Zenith_Component / Range_Magnitude;

    if (ratio > 1.0)
    {
        ratio = 1.0;
    }

    if (ratio < -1.0)
    {
        ratio = -1.0;
    }

    double elevation_radians = std::asin(ratio);

    Elevation_Angle_Above_Horizon = elevation_radians * (180.0 / PI);

    return Elevation_Angle_Above_Horizon;
}


double Spacecraft_Geometry::Azimuth_Angle(double East_Component, double South_Component)
{
    double azimuth_radians = std::atan2(East_Component,-South_Component);

    double azimuth_degrees = azimuth_radians *  (180.0 / PI);

    if (azimuth_degrees < 0.0)
    {
        azimuth_degrees += 360.0;
    }

    Azimuth_Angle_From_North = azimuth_degrees;

    return Azimuth_Angle_From_North;
}


void Spacecraft_Geometry::Print_Spacecraft_Geometry() const
{
    std::cout<< "\n=========================== Spacecraft Geometry =========================================================================================================================================" << std::endl;
    std::cout<< "Ground station longitude: "<< Longitude_Of_Ground_Station<< " radians" << std::endl;
    std::cout<< "Ground station latitude: "<< Latitude_Of_Ground_Station << " radians" << std::endl;
    std::cout<< "Ground station altitude: " << h_Altitude_of_Ground_Station << " meters" << std::endl;
    std::cout << "Ground station Cartesian coordinates (X, Y, Z): " << X_Position_Ground_Station_Components  << ", " << Y_Position_Ground_Station_Components << ", " << Z_Position_Ground_Station_Components << std::endl;
    std::cout<< "Range vector from ground station to spacecraft: " << Range_Vector.transpose() << std::endl;
    std::cout<< "Straight-line range magnitude: " << Range_Scalar_Magnitude << " meters" << std::endl;
    std::cout<< "Elevation angle above horizon: " << Elevation_Angle_Above_Horizon<< " degrees" << std::endl;
    std::cout<< "Azimuth angle from North: " << Azimuth_Angle_From_North << " degrees" << std::endl;
    std::cout<< "Line-of-sight unit vector: " << Line_Of_Sight_Unit_Vector.transpose() << std::endl;
    std::cout<< "Spacecraft position (X, Y, Z): " << Spacecraft_Position.transpose() << std::endl;
    std::cout << "Greenwich sidereal angle: " << Greenwich_Sidereal_Time_angle << " radians" << std::endl;
    std::cout << "South component: " << South_Component << " meters" << std::endl;
    std::cout << "East component: " << East_Component << " meters" << std::endl;
    std::cout << "Zenith component: " << Zenith_Component << " meters" << std::endl;

    std::cout<< "==============================================================================================================================================================================================="<< std::endl;
}