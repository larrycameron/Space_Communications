#include "PAT_Timing.h"


double PAT_Timing::Calculate_Total_PAT_Time( double acquisition_time, double reacquisition_time, double retargeting_time)
{
    if (acquisition_time < 0.0 ||
        reacquisition_time < 0.0 ||
        retargeting_time < 0.0)
    {
        Acquisition_Time = 0.0;
        Reacquisition_Time = 0.0;
        Retargeting_Time = 0.0;
        Total_PAT_Time = 0.0;

        Usable_Contact_Time = 0.0;
        Contact_Efficiency = 0.0;
        PAT_Overhead_Percentage = 0.0;

        return 0.0;
    }

    Acquisition_Time = acquisition_time;
    Reacquisition_Time = reacquisition_time;
    Retargeting_Time = retargeting_time;

    Total_PAT_Time =
        Acquisition_Time +
        Reacquisition_Time +
        Retargeting_Time;

    return Total_PAT_Time;
}


double PAT_Timing::Calculate_Usable_Contact_Time(
    double contact_window_duration)
{
    if (contact_window_duration <= 0.0)
    {
        Contact_Window_Duration = 0.0;
        Usable_Contact_Time = 0.0;

        return 0.0;
    }

    Contact_Window_Duration = contact_window_duration;

    Usable_Contact_Time =
        Contact_Window_Duration - Total_PAT_Time;

    // PAT cannot create a negative usable contact period.
    if (Usable_Contact_Time < 0.0)
    {
        Usable_Contact_Time = 0.0;
    }

    return Usable_Contact_Time;
}


double PAT_Timing::Calculate_Contact_Efficiency()
{
    if (Contact_Window_Duration <= 0.0)
    {
        Contact_Efficiency = 0.0;
        return 0.0;
    }

    Contact_Efficiency =
        Usable_Contact_Time / Contact_Window_Duration;

    return Contact_Efficiency;
}


double PAT_Timing::Calculate_PAT_Overhead_Percentage()
{
    if (Contact_Window_Duration <= 0.0)
    {
        PAT_Overhead_Percentage = 0.0;
        return 0.0;
    }

    PAT_Overhead_Percentage =
        (Total_PAT_Time / Contact_Window_Duration) * 100.0;

    // Once PAT consumes the entire window, overhead is effectively 100%.
    if (PAT_Overhead_Percentage > 100.0)
    {
        PAT_Overhead_Percentage = 100.0;
    }

    return PAT_Overhead_Percentage;
}


double PAT_Timing::Get_Total_PAT_Time() const
{
    return Total_PAT_Time;
}


double PAT_Timing::Get_Usable_Contact_Time() const
{
    return Usable_Contact_Time;
}


double PAT_Timing::Get_Contact_Efficiency() const
{
    return Contact_Efficiency;
}


double PAT_Timing::Get_PAT_Overhead_Percentage() const
{
    return PAT_Overhead_Percentage;
}