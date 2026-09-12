#ifndef SPACECRAFT_RADIATION_H
#define SPACECRAFT_RADIATION_H


#include <string>
#include <vector>
 

struct RadiationSample {double lab_dose_rate; double space_dose_rate; std::string component_type;};

class Sensor_Environmental_Survivability
{
private:

    double Radition_Hardness_Capability_Component{};
    double Predicted_Mission_Radiation_Dose{};
    double RDMCalculation{};
    double Component_Type_Value{};
    double Space_Dose_Rate{};
    double Lab_Dose_Rate{};
    static constexpr double  Stefan_Boltzmann_Constant{5.67e-8};// W/m²K⁴
    static constexpr double  Speed_Of_Light{2.998e8};// meters per second
    double Surface_Size_Of_Spacecraft{};
    double Temperature_Of_Spacecraft{};
    double Freezing_Temperatures_Deepspace{};
    double Material_Emissivity{};
    double Total_Heat_Energy{};
    double Total_Pushing_Force{};
    double Change_Of_Light_Brightness_Over_Short_Distance{};
    double Radiation_Acceleration_Factor{};
   
public:

    double Total_Ionizing_Dose(double radition_hardness_capability_component, double predicted_mission_radiation_dose);
    double Calculate_Radiation_Acceleration_Factor(double lab_dose_rate, double space_dose_rate, std::string component_type_value);
    double Stefan_Boltzmann_Law(double material_emissivity, double surface_size_of_spacecraft, double temperature_of_spacecraft, double freezing_temperatures_deepspace);
    double Solar_Radiation_Pressure_Force(double brightness_of_sunlight, double exposed_surface_area, double surface_reflection_coefficient);
    double Calculate_Light_Brightness_Short_Dist( double material_blockage_factor, double starting_light_brightness, double material_glow_factor);
  
    
    void PrintRadiationCalculations() const;
   

};

#endif