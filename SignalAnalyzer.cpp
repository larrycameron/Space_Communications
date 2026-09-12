#include <iostream>
#include <cmath>



#include "SignalAnalyzer.h"

void DigitalSignalProcessing::Generate_Combined_Signal(double amplitudeOne, double frequencyOne, double amplitudeTwo, double frequencyTwo, double samplerate)
{
            AmplitudeOne = amplitudeOne;
            FrequencyOne = frequencyOne;
            AmplitudeTwo = amplitudeTwo;
            FrequencyTwo = frequencyTwo;
            SampleRate = samplerate;

            if (SampleRate <= 0.0)
            {
                Samples.clear();
                return;
            }
                                          
            Samples.resize(TotalSampleNumbers);
                      
            for (std::size_t Sample= 0; Sample < Samples.size(); ++ Sample) 
                {
                    double WaveOneSample = AmplitudeOne * std::sin(2 * PI * FrequencyOne * (static_cast<double>(Sample) / SampleRate)); 
                    
                    double WaveTwoSample = AmplitudeTwo * std::sin(2 * PI * FrequencyTwo * (static_cast<double>(Sample) / SampleRate)); 
                    
                    Samples[Sample] = WaveOneSample + WaveTwoSample;
                }



}

void DigitalSignalProcessing::PrintDigitalSignalProcessing() const
{
        std::cout << "Wave One Amplitude : " << AmplitudeOne << '\n';
        std::cout << "Wave One Frequency : " << FrequencyOne << " Hz\n";
        std::cout << "Wave Two Amplitude : " << AmplitudeTwo << '\n';
        std::cout << "Wave Two Frequency : " << FrequencyTwo << " Hz\n";
        std::cout << "Sample Rate        : " << SampleRate << " Hz\n";
        std::cout << "Total Samples      : " << TotalSampleNumbers << '\n';
            
        std::cout<< "Generated Waveform Samples " << Samples.size() <<  "  samples " << std::endl; 
            
        for (std::size_t i = 0; i < Samples.size(); ++i) 
        {
            std::cout << "Sample    " << i <<  "  instantaneous amplitude: " << Samples[i] << std::endl; 
        }
      
}

double Space_Data_Capture::Calculate_Dynamic_Relative_Velocity( double relative_velocity,  double acceleration, double time_seconds)
{
    double dynamic_relative_velocity = relative_velocity + (acceleration * time_seconds);

    return dynamic_relative_velocity;
}

double Space_Data_Capture::Calculate_First_Order_Doppler_Factor(double radial_velocity)
{
    Warping_Factor = 1.0 - (radial_velocity / Speed_Of_Light);

    return Warping_Factor;
}

double Space_Data_Capture::Calculate_Doppler_Shifted_Frequency(double original_signal, double doppler_factor)
{
    double received_signal =  original_signal * doppler_factor;

    return received_signal;
}

double Space_Data_Capture::Relativistic_Doppler_Factor( double velocity, double theta)
{
    if (std::abs(velocity) >= Speed_Of_Light)
    {
        return 0.0;
    }
    
    double beta = velocity / Speed_Of_Light;

    double einsteinian = std::sqrt(1.0 - beta * beta) /  (1.0 - beta * std::cos(theta));

    return einsteinian;
}

double Space_Data_Capture::Calculate_Radial_Velocity(double orbital_speed, double time_delta, double distance_at_closest_point_of_approach)
{
    double numerator = std::pow(orbital_speed, 2) * time_delta;

    double denominator =  std::sqrt(std::pow(distance_at_closest_point_of_approach, 2) + std::pow(orbital_speed * time_delta, 2));

    if (denominator == 0.0)
    {
        return 0.0;
    }

    double relative_velocity_of_satellite = numerator / denominator;

    return relative_velocity_of_satellite;
}

double Space_Data_Capture::Calculate_Vector_Radial_Velocity(const Eigen::Vector3d& relative_velocity, const Eigen::Vector3d& line_of_sight_unit_vector)
{
    double radial_velocity = relative_velocity.dot(line_of_sight_unit_vector);

    return radial_velocity;
}