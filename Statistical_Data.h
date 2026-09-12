#ifndef STATISTICALDATA_H
#define STATISTICALDATA_H

#include <vector>
#include <string>
#include <optional>

struct Confidence_Interval
{
    double Lower_Bound{};
    double Upper_Bound{};
};

struct Hypothesis_Test_Result
{
    double T_Statistic{};
    double Degrees_Of_Freedom{};
    double P_Value{};
    bool Statistically_Significant{};
};


class Statistical_Data
{
public:

    std::optional<double> Calculate_Mean( const std::vector<double>& samples);

    std::optional<double> Calculate_Population_Variance(const std::vector<double>& samples);

    std::optional<double> Calculate_Sample_Variance(const std::vector<double>& samples);

    std::optional<double> Calculate_Standard_Deviation(const std::vector<double>& samples);

    std::optional<double> Calculate_Standard_Error(const std::vector<double>& samples);

    std::optional<double> Calculate_Percentile(const std::vector<double>& samples, double percentile);

    bool Validate_Samples(const std::vector<double>& samples);

    void Statistics_Sample_Size(const std::vector<double>& samples, const std::string& name, bool convert_to_degrees);

    std::optional<double> Calculate_Median(const std::vector<double>& samples);

    std::vector<double> Calculate_Mode(const std::vector<double>& samples);

    std::optional<double> Calculate_Q1(const std::vector<double>& samples);

    std::optional<double> Calculate_Q3(const std::vector<double>& samples);

    std::optional<double> Calculate_IQR( const std::vector<double>& samples);

    std::optional<Confidence_Interval>Calculate_Confidence_Interval_95(const std::vector<double>& samples);

    std::optional<Hypothesis_Test_Result>Calculate_Welch_T_Test(const std::vector<double>& sample_1, const std::vector<double>& sample_2, double alpha = 0.05);

};

#endif
