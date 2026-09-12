#include "Spacecraft_Geometry.h"
#include "Contact_Window.h"
#include "PAT_Timing.h"

#include <Eigen/Dense>
#include <iostream>

int main()
{
    Spacecraft_Geometry geometry;
    Contact_Window contact;
    PAT_Timing pat;

    // ---------------------------------------------------------
    // Step 1: Create a simple ground-station position.
    // Longitude = 0 rad
    // Latitude  = 0 rad
    // Altitude  = 0 m
    // ---------------------------------------------------------

    Eigen::Vector3d ground_station =
        geometry.Ground_Station_Positions_long_lat_alt(
            0.0,
            0.0,
            0.0);

    // ---------------------------------------------------------
    // Step 2: Place spacecraft directly above the station.
    //
    // Earth radius is about 6,378,137 m.
    // Add 500 km altitude.
    // ---------------------------------------------------------

    Eigen::Vector3d spacecraft =
        geometry.Spacecraft_Position_x_y_z(
            6878137.0,
            0.0,
            0.0);

    // ---------------------------------------------------------
    // Step 3: Calculate range vector and range magnitude.
    // ---------------------------------------------------------

    Eigen::Vector3d range_vector =
        geometry.Calculate_Range_Vector(
            spacecraft,
            ground_station);

    double range =
        geometry.Calculate_Range_Magnitude(
            range_vector);

    // ---------------------------------------------------------
    // Step 4: Convert the range vector into SEZ coordinates.
    //
    // Because this simple test uses longitude = 0
    // and latitude = 0, no complicated geometry is needed.
    // ---------------------------------------------------------

    Eigen::Vector3d sez =
        geometry.Calculate_SEZ_Components(
            range_vector,
            0.0,
            0.0);

    double zenith = sez.z();

    double elevation =
        geometry.Elevation_Angle(
            zenith,
            range);

    // ---------------------------------------------------------
    // Step 5: Determine whether contact exists.
    // Minimum elevation = 10 degrees.
    // ---------------------------------------------------------

    bool contact_available =
        contact.Is_Contact_Available(
            elevation,
            10.0);

    // ---------------------------------------------------------
    // Step 6: If contact exists, define a 100-second window.
    // ---------------------------------------------------------

    double contact_duration = 0.0;

    if (contact_available)
    {
        contact_duration =
            contact.Calculate_Contact_Window_Duration(
                0.0,
                100.0);
    }

    // ---------------------------------------------------------
    // Step 7: Apply PAT overhead.
    // Acquisition = 10 s
    // Reacquisition = 5 s
    // Retargeting = 5 s
    // Total PAT = 20 s
    // ---------------------------------------------------------

    double total_pat =
        pat.Calculate_Total_PAT_Time(
            10.0,
            5.0,
            5.0);

    double usable_time =
        pat.Calculate_Usable_Contact_Time(
            contact_duration);

    // ---------------------------------------------------------
    // Results
    // ---------------------------------------------------------

    std::cout << "\n==============================\n";
    std::cout << "GEOMETRY -> CONTACT -> PAT TEST\n";
    std::cout << "==============================\n";

    std::cout << "Range: "
              << range
              << " meters\n";

    std::cout << "Elevation: "
              << elevation
              << " degrees\n";

    std::cout << "Contact Available: "
              << (contact_available ? "YES" : "NO")
              << '\n';

    std::cout << "Contact Window: "
              << contact_duration
              << " seconds\n";

    std::cout << "Total PAT Time: "
              << total_pat
              << " seconds\n";

    std::cout << "Usable Contact Time: "
              << usable_time
              << " seconds\n";

    return 0;
}