
#ifndef SIGNALANALYZER_H
#define SIGNALANALYZER_H

#include <vector>
#include <cstddef>
#include <Eigen/Dense>

class DigitalSignalProcessing
{
private:
    double AmplitudeOne{};
    double FrequencyOne{};
    double AmplitudeTwo{};
    double FrequencyTwo{};
    double SampleRate{};
    std::size_t TotalSampleNumbers{500};
    static constexpr double PI{3.14159265358979};

    std::vector<double> Samples;

public:
    void Generate_Combined_Signal(double amplitudeOne, double frequencyOne, double amplitudeTwo, double frequencyTwo,double samplerate);
    void PrintDigitalSignalProcessing() const;
};

class Space_Data_Capture
{
private:
    static constexpr double Speed_Of_Light{2.998e8};

    double Warping_Factor{1.0};

public:
    double Calculate_Dynamic_Relative_Velocity(double relative_velocity, double acceleration, double time_seconds);

    double Calculate_First_Order_Doppler_Factor(double dynamic_relative_velocity);

    double Calculate_Doppler_Shifted_Frequency(double original_signal, double doppler_factor);

    double Relativistic_Doppler_Factor( double velocity,  double theta);

    double Calculate_Radial_Velocity( double orbital_speed, double time_delta, double distance_at_closest_point_of_approach);

    double Calculate_Vector_Radial_Velocity(const Eigen::Vector3d& relative_velocity,const Eigen::Vector3d& line_of_sight_unit_vector);
};


#endif