#include "Contact_Window.h"

bool Contact_Window::Is_Contact_Available(double elevation_angle, double minimum_elevation_angle)
{
    Contact_Available = elevation_angle >= minimum_elevation_angle;

    return Contact_Available;
}

double Contact_Window::Calculate_Contact_Window_Duration( double contact_start_time, double contact_end_time)
{
    if (contact_start_time < 0.0 ||
        contact_end_time < 0.0 ||
        contact_end_time < contact_start_time)
    {
        Contact_Start_Time = 0.0;
        Contact_End_Time = 0.0;
        Contact_Window_Duration = 0.0;

        return 0.0;
    }

    Contact_Start_Time = contact_start_time;
    Contact_End_Time = contact_end_time;

    Contact_Window_Duration =
        Contact_End_Time - Contact_Start_Time;

    return Contact_Window_Duration;
}

bool Contact_Window::Get_Contact_Available() const
{
    return Contact_Available;
}

double Contact_Window::Get_Contact_Window_Duration() const
{
    return Contact_Window_Duration;
    
}