#include "Spacecraft_Radiation.h"

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
 

    double Sensor_Environmental_Survivability::Total_Ionizing_Dose(double radition_hardness_capability_component, double predicted_mission_radiation_dose)
    {      
           Radition_Hardness_Capability_Component = radition_hardness_capability_component;
           Predicted_Mission_Radiation_Dose = predicted_mission_radiation_dose;
           
           if (Predicted_Mission_Radiation_Dose <= 0.0)
            {  
                RDMCalculation = 0.0;
                return RDMCalculation;
            }
    
           RDMCalculation = Radition_Hardness_Capability_Component / Predicted_Mission_Radiation_Dose ;

            if (RDMCalculation >= 2.0)
            {
                std::cout<<"Radiation levels are within the standard mission requirements."<<std::endl;  
            }
            else
            {
                std::cout<<"Warning: high levels of radiation."<<std::endl;  
            }
            return RDMCalculation;
    } 

    double Sensor_Environmental_Survivability::Calculate_Radiation_Acceleration_Factor(double lab_dose_rate, double space_dose_rate, std::string component_type_value)
    {
        Lab_Dose_Rate = lab_dose_rate;
        Space_Dose_Rate = space_dose_rate;

        if (Space_Dose_Rate <= 0.0)
        {
            Radiation_Acceleration_Factor = 0.0;
            return Radiation_Acceleration_Factor;
        }

        if (component_type_value == "ideal") // perfect material integrity
        {
            Component_Type_Value = 1.0;
        }
        else if (component_type_value == "cmos_logic")//expands, and softens; a material can not hold anymore it hits saturation.
        {
            Component_Type_Value = 0.8;
        }
        else if (component_type_value == "optoelectronics")   //Electrical Component Degredation
        {
            Component_Type_Value = 1.3;
        }
        else
        {
             Component_Type_Value = 1.0;
        }

        Radiation_Acceleration_Factor = std::pow((Lab_Dose_Rate / Space_Dose_Rate), Component_Type_Value);
        return Radiation_Acceleration_Factor;
    }

    double Sensor_Environmental_Survivability::Stefan_Boltzmann_Law(double material_emissivity, double surface_size_of_spacecraft, double temperature_of_spacecraft, double freezing_temperatures_deepspace)
    {
        Material_Emissivity = material_emissivity;
        Surface_Size_Of_Spacecraft = surface_size_of_spacecraft;
        Temperature_Of_Spacecraft = temperature_of_spacecraft;
        Freezing_Temperatures_Deepspace = freezing_temperatures_deepspace;
        
        
        Total_Heat_Energy = Material_Emissivity * Stefan_Boltzmann_Constant * Surface_Size_Of_Spacecraft * (std::pow(Temperature_Of_Spacecraft, 4) - std::pow(Freezing_Temperatures_Deepspace,4));

        return Total_Heat_Energy;
    }

    double Sensor_Environmental_Survivability::Solar_Radiation_Pressure_Force(double brightness_of_sunlight, double exposed_surface_area, double surface_reflection_coefficient)
    {
        Total_Pushing_Force = brightness_of_sunlight *  exposed_surface_area * (1.0 + surface_reflection_coefficient) / (Speed_Of_Light);
        
        return Total_Pushing_Force;
    }   

    double Sensor_Environmental_Survivability::Calculate_Light_Brightness_Short_Dist( double material_blockage_factor, double starting_light_brightness, double material_glow_factor)
    {
        Change_Of_Light_Brightness_Over_Short_Distance =   - material_blockage_factor * starting_light_brightness +  material_glow_factor;

        return Change_Of_Light_Brightness_Over_Short_Distance;
    }

    void Sensor_Environmental_Survivability::PrintRadiationCalculations() const
    {
        std::cout << "The cumulative exposure time determines how much radiation is absorbed by the materials. \n"<< "Calculated Radiation Design Margin (RDM): " << RDMCalculation << std::endl; 
    }
