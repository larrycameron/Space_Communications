#ifndef SHAPIRO_TIME_DELAY_H
#define SHAPIRO_TIME_DELAY_H

class Shapiro_Time_Delay
{
private:
    double Extra_Time_Delay{};// seconds
    static constexpr double G {6.674e-11}; // m^3*kg^-1*s^-2
    double Sun_Mass{1.989e30};// Planet Mass kg
    static constexpr double c {2.99792458e8};// m/s
    double Distance_Sun_To_Earth{1.496e11};// meters
    double Distance_Sun_To_Target_Planet{2.279e11}; // meter to mars 
    double Closest_Distance_Approach{2.78e9}; //meters

public:
    double Calculate_Extra_Time_Delay(double sun_mass, double distance_sun_to_earth, double distance_sun_to_target_planet, double closest_distance_approach);
    void Print_Calculate_Extra_Time_Delay() const;
};   

#endif