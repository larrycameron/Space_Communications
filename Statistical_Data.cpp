
#include "Statistical_Data.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <boost/math/distributions/students_t.hpp>


bool Statistical_Data::Validate_Samples(const std::vector<double>& samples)
{
    for (double value : samples)
    {
        if (!std::isfinite(value))
        {
            return false;
        }
    }

    return true;
}



void Statistical_Data::Statistics_Sample_Size(const std::vector<double>& samples, const std::string& name, bool convert_to_degrees)
{
    if (samples.empty())
    {
        return;
    }

    if (!Validate_Samples(samples))
    {
        std::cout << "Invalid sample data: NaN or infinity detected."
                  << std::endl;
        return;
    }

    double sum = 0.0;

    double maximum_value = samples[0];
    double minimum_value = samples[0];

    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        sum += samples[i];

        if (samples[i] > maximum_value)
        {
            maximum_value = samples[i];
            
        }

        if (samples[i] < minimum_value)
        {
            minimum_value = samples[i];
            
        }
    }

    double mean = sum / samples.size();

    double range = maximum_value - minimum_value;

    double variance_sum = 0.0;

    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        double difference = samples[i] - mean;
        variance_sum += difference * difference;
    }

    double variance = variance_sum / samples.size();

    double standard_deviation = std::sqrt(variance);

    std::cout
        << "\n================ "
        << name
        << " Statistics ==============================================================================================================="
        << std::endl;

    std::cout
        << "Number of Samples: "
        << samples.size()
        << std::endl;

    if (convert_to_degrees)
    {
        const double PI = 3.141592653589793;

        std::cout
            << "Mean: "
            << mean * (180.0 / PI)
            << " degrees"
            << std::endl;

        std::cout
            << "Maximum Value: "
            << maximum_value * (180.0 / PI)
            << " degrees"
            << std::endl;

        std::cout
            << "Minimum Value: "
            << minimum_value * (180.0 / PI)
            << " degrees"
            << std::endl;

        std::cout
            << "Range: "
            << range * (180.0 / PI)
            << " degrees"
            << std::endl;
    }
    else
    {
        std::cout << "Mean: "
                  << mean
                  << std::endl;

        std::cout << "Maximum Value: "
                  << maximum_value
                  << std::endl;

        std::cout << "Minimum Value: "
                  << minimum_value
                  << std::endl;

        std::cout << "Range: "
                  << range
                  << std::endl;

        std::cout << "Variance: "
                  << variance
                  << std::endl;

        std::cout << "Standard Deviation: "
                  << standard_deviation
                  << std::endl;
    }

    std::cout
        << "========================================================================================================================"
        << std::endl;
}


std::optional<double> Statistical_Data::Calculate_Median(const std::vector<double>& samples)
{
    if (samples.empty())
{
    return std::nullopt;
}

if (!Validate_Samples(samples))
{
    return std::nullopt;
}

std::vector<double> sorted_samples = samples;
    
std::sort(sorted_samples.begin(), sorted_samples.end() );

    std::size_t middle = sorted_samples.size() / 2;

    if (sorted_samples.size() % 2 == 0)
    {
        return ( sorted_samples[middle - 1]  +  sorted_samples[middle]) / 2.0;
    }
    else
    {
        return sorted_samples[middle];
    }
}


std::vector<double> Statistical_Data::Calculate_Mode(const std::vector<double>& samples)
{
    if (samples.empty())
    {
    return {};
    }

    if (!Validate_Samples(samples))
    {
        return {};
    }

    std::vector<double> sorted_samples = samples;

    std::sort( sorted_samples.begin(), sorted_samples.end());

    std::vector<double> modes;

    std::size_t current_count = 1;
    std::size_t highest_count = 1;

    for (std::size_t i = 1; i < sorted_samples.size(); ++i)
    {
        if (sorted_samples[i] == sorted_samples[i - 1])
        {
            ++current_count;
        }
        else
        {
            current_count = 1;
        }

        if (current_count > highest_count)
        {
            highest_count = current_count;

            modes.clear();

            modes.push_back( sorted_samples[i]);
        }
        else if (current_count == highest_count  &&  highest_count > 1)
        {
            modes.push_back( sorted_samples[i] );
        }
    }

    if (highest_count == 1)
    {
        return {};
    }

    return modes;
}


std::optional<double> Statistical_Data::Calculate_Q1( const std::vector<double>& samples)
{
    if (samples.size() < 2)
    {
        return std::nullopt;
    }

    if (!Validate_Samples(samples))
    {
        return std::nullopt;
    }

    std::vector<double> sorted_samples = samples;

    std::sort( sorted_samples.begin(), sorted_samples.end());

    std::size_t middle = sorted_samples.size() / 2;

    std::vector<double> lower_half(sorted_samples.begin(), sorted_samples.begin() + middle);

    return Calculate_Median(lower_half);
}


std::optional<double> Statistical_Data::Calculate_Q3(const std::vector<double>& samples)
{
    if (samples.size() < 2)
    {
        return std::nullopt;
    }

    if (!Validate_Samples(samples))
    {
        return std::nullopt;
    }

    std::vector<double> sorted_samples = samples;

    std::sort( sorted_samples.begin(), sorted_samples.end()
    );

    std::size_t middle = sorted_samples.size() / 2;

    std::size_t upper_start = middle;

    if (sorted_samples.size() % 2 != 0)
    {
        ++upper_start;
    }

    std::vector<double> upper_half(sorted_samples.begin() + upper_start, sorted_samples.end());

    return Calculate_Median(upper_half);
}


std::optional<double> Statistical_Data::Calculate_IQR( const std::vector<double>& samples)
{
    auto q1 = Calculate_Q1(samples);

    auto q3 = Calculate_Q3(samples);

    if (!q1.has_value() || !q3.has_value())
    {
        return std::nullopt;
    }

    return q3.value() - q1.value();
}
std::optional<double> Statistical_Data::Calculate_Mean(
    const std::vector<double>& samples)
{
    if (samples.empty())
    {
        return std::nullopt;
    }

    if (!Validate_Samples(samples))
    {
        return std::nullopt;
    }

    double sum = 0.0;

    for (double value : samples)
    {
        sum += value;
    }

    return sum / static_cast<double>(samples.size());
}

std::optional<double> Statistical_Data::Calculate_Population_Variance(const std::vector<double>& samples)
{
    if (samples.empty())
    {
        return std::nullopt;
    }

    if (!Validate_Samples(samples))
    {
        return std::nullopt;
    }

    auto mean = Calculate_Mean(samples);

    if (!mean.has_value())
    {
        return std::nullopt;
    }

    double variance_sum = 0.0;

    for (double value : samples)
    {
        double difference = value - mean.value();

        variance_sum += difference * difference;
    }

    return variance_sum / static_cast<double>(samples.size());
}

std::optional<double> Statistical_Data::Calculate_Sample_Variance(const std::vector<double>& samples)
{
    if (samples.size() < 2)
    {
        return std::nullopt;
    }

    if (!Validate_Samples(samples))
    {
        return std::nullopt;
    }

    auto mean = Calculate_Mean(samples);

    if (!mean.has_value())
    {
        return std::nullopt;
    }

    double variance_sum = 0.0;

    for (double value : samples)
    {
        double difference = value - mean.value();

        variance_sum += difference * difference;
    }

    return variance_sum /
           static_cast<double>(samples.size() - 1);
}

std::optional<double> Statistical_Data::Calculate_Standard_Deviation(const std::vector<double>& samples)
{
    auto variance = Calculate_Sample_Variance(samples);

    if (!variance.has_value())
    {
        return std::nullopt;
    }

    return std::sqrt(variance.value());
}

std::optional<double> Statistical_Data::Calculate_Standard_Error(const std::vector<double>& samples)
{
    auto standard_deviation = Calculate_Standard_Deviation(samples);

    if (!standard_deviation.has_value())
    {
        return std::nullopt;
    }

    return standard_deviation.value() / std::sqrt(static_cast<double>(samples.size()));
}

std::optional<double> Statistical_Data::Calculate_Percentile(const std::vector<double>& samples, double percentile)
{
    if (samples.empty())
    {
        return std::nullopt;
    }

    if (!Validate_Samples(samples))
    {
        return std::nullopt;
    }

    if (percentile < 0.0 || percentile > 100.0)
    {
        return std::nullopt;
    }

    std::vector<double> sorted_samples = samples;

    std::sort(sorted_samples.begin(), sorted_samples.end());

    double position =  (percentile / 100.0) * static_cast<double>(sorted_samples.size() - 1);

    std::size_t lower_index = static_cast<std::size_t>(std::floor(position));

    std::size_t upper_index =  static_cast<std::size_t>(std::ceil(position));

    if (lower_index == upper_index)
    {
        return sorted_samples[lower_index];
    }

    double fraction =  position - static_cast<double>(lower_index);

    return sorted_samples[lower_index] +  fraction * (sorted_samples[upper_index] - sorted_samples[lower_index]);
}

std::optional<Confidence_Interval>
Statistical_Data::Calculate_Confidence_Interval_95(
    const std::vector<double>& samples)
{
    if (samples.size() < 2)
    {
        return std::nullopt;
    }

    if (!Validate_Samples(samples))
    {
        return std::nullopt;
    }

    auto mean = Calculate_Mean(samples);
    auto standard_error = Calculate_Standard_Error(samples);

    if (!mean.has_value() || !standard_error.has_value())
    {
        return std::nullopt;
    }

    double degrees_of_freedom =
        static_cast<double>(samples.size() - 1);

    boost::math::students_t distribution(degrees_of_freedom);

    double t_critical =
        boost::math::quantile(distribution, 0.975);

    double margin_of_error =
        t_critical * standard_error.value();

    Confidence_Interval interval;

    interval.Lower_Bound =
        mean.value() - margin_of_error;

    interval.Upper_Bound =
        mean.value() + margin_of_error;

    return interval;
}

std::optional<Hypothesis_Test_Result>
Statistical_Data::Calculate_Welch_T_Test(
    const std::vector<double>& sample_1,
    const std::vector<double>& sample_2,
    double alpha)
{
    if (sample_1.size() < 2 || sample_2.size() < 2)
    {
        return std::nullopt;
    }

    if (!Validate_Samples(sample_1) ||
        !Validate_Samples(sample_2))
    {
        return std::nullopt;
    }

    if (alpha <= 0.0 || alpha >= 1.0)
    {
        return std::nullopt;
    }

    auto mean_1 = Calculate_Mean(sample_1);
    auto mean_2 = Calculate_Mean(sample_2);

    auto variance_1 = Calculate_Sample_Variance(sample_1);
    auto variance_2 = Calculate_Sample_Variance(sample_2);

    if (!mean_1.has_value() ||
        !mean_2.has_value() ||
        !variance_1.has_value() ||
        !variance_2.has_value())
    {
        return std::nullopt;
    }

    double n1 = static_cast<double>(sample_1.size());
    double n2 = static_cast<double>(sample_2.size());

    double variance_term_1 =
        variance_1.value() / n1;

    double variance_term_2 =
        variance_2.value() / n2;

    double standard_error_squared =
        variance_term_1 + variance_term_2;

    if (standard_error_squared <= 0.0)
    {
        return std::nullopt;
    }

    double t_statistic =
        (mean_1.value() - mean_2.value()) /
        std::sqrt(standard_error_squared);

    double numerator =
        standard_error_squared *
        standard_error_squared;

    double denominator =
        (variance_term_1 * variance_term_1) /
            (n1 - 1.0)
        +
        (variance_term_2 * variance_term_2) /
            (n2 - 1.0);

    if (denominator <= 0.0)
    {
        return std::nullopt;
    }

    double degrees_of_freedom =
        numerator / denominator;

    boost::math::students_t distribution(
        degrees_of_freedom);

    double p_value =
        2.0 *
        boost::math::cdf(
            boost::math::complement(
                distribution,
                std::abs(t_statistic)));

    Hypothesis_Test_Result result;

    result.T_Statistic =
        t_statistic;

    result.Degrees_Of_Freedom =
        degrees_of_freedom;

    result.P_Value =
        p_value;

    result.Statistically_Significant =
        p_value < alpha;

    return result;
}