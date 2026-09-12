#ifndef PAT_TIMING_H
#define PAT_TIMING_H

class PAT_Timing
{
private:
    double Acquisition_Time{};
    double Reacquisition_Time{};
    double Retargeting_Time{};

    double Total_PAT_Time{};
    double Contact_Window_Duration{};
    double Usable_Contact_Time{};
    double Contact_Efficiency{};
    double PAT_Overhead_Percentage{};

public:
    double Calculate_Total_PAT_Time(
        double acquisition_time,
        double reacquisition_time,
        double retargeting_time);

    double Calculate_Usable_Contact_Time(
        double contact_window_duration);

    double Calculate_Contact_Efficiency();

    double Calculate_PAT_Overhead_Percentage();

    double Get_Total_PAT_Time() const;
    double Get_Usable_Contact_Time() const;
    double Get_Contact_Efficiency() const;
    double Get_PAT_Overhead_Percentage() const;
};

#endif