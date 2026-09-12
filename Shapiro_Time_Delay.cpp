#include "Shapiro_Time_Delay.h"

#include <iostream>
#include <cmath>



    double Shapiro_Time_Delay::Calculate_Extra_Time_Delay(double sun_mass,double distance_sun_to_earth, double distance_sun_to_target_planet, double closest_distance_approach)
{
    Sun_Mass = sun_mass;
    Distance_Sun_To_Earth = distance_sun_to_earth;
    Distance_Sun_To_Target_Planet = distance_sun_to_target_planet;
    
    if (Sun_Mass <= 0.0)
    {
        std::cout << "Invalid Sun mass." << std::endl;
        Extra_Time_Delay = 0.0;
        return Extra_Time_Delay;
    }
    
    Closest_Distance_Approach = closest_distance_approach;

    if (Closest_Distance_Approach <= 0.0)
    {
        std::cout << "Invalid closest approach distance."<< std::endl;
        Extra_Time_Delay = 0.0;
        return Extra_Time_Delay;
    }
    
    double log_argument = (4.0 * Distance_Sun_To_Earth *  Distance_Sun_To_Target_Planet) / std::pow(Closest_Distance_Approach, 2);

    if (log_argument <= 1.0)
    {
        std::cout << "Invalid geometry for Shapiro delay approximation." << std::endl;

        Extra_Time_Delay = 0.0;
        return Extra_Time_Delay;
    }

    Extra_Time_Delay =((2.0 * G * Sun_Mass) / std::pow(c, 3)) * std::log(log_argument);
    
    return Extra_Time_Delay;

} 

void Shapiro_Time_Delay::Print_Calculate_Extra_Time_Delay() const
{
    std::cout << "The approximate extra time delay: " << Extra_Time_Delay << " seconds" << std::endl;
}