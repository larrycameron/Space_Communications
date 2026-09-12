#ifndef CONTACT_WINDOW_H
#define CONTACT_WINDOW_H

class Contact_Window
{
private:
    bool Contact_Available{};
    double Contact_Start_Time{};
    double Contact_End_Time{};
    double Contact_Window_Duration{};

public:
    bool Is_Contact_Available(
        double elevation_angle,
        double minimum_elevation_angle);

    double Calculate_Contact_Window_Duration(
        double contact_start_time,
        double contact_end_time);

    bool Get_Contact_Available() const;
    double Get_Contact_Window_Duration() const;
};

#endif