#include "Statistical_Data.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <optional>
#include <string>


bool Nearly_Equal(
    double actual,
    double expected,
    double tolerance = 1e-9)
{
    return std::abs(actual - expected) <= tolerance;
}


void Check_Value(
    const std::string& test_name,
    const std::optional<double>& result,
    double expected)
{
    std::cout << test_name << ": ";

    if (result.has_value() &&
        Nearly_Equal(result.value(), expected))
    {
        std::cout
            << "PASS"
            << " | Actual: " << result.value()
            << " | Expected: " << expected
            << '\n';
    }
    else
    {
        std::cout << "FAIL";

        if (result.has_value())
        {
            std::cout
                << " | Actual: "
                << result.value();
        }
        else
        {
            std::cout
                << " | Actual: nullopt";
        }

        std::cout
            << " | Expected: "
            << expected
            << '\n';
    }
}


void Check_Nullopt(
    const std::string& test_name,
    const std::optional<double>& result)
{
    std::cout << test_name << ": ";

    if (!result.has_value())
    {
        std::cout
            << "PASS"
            << " | Actual: nullopt"
            << " | Expected: nullopt"
            << '\n';
    }
    else
    {
        std::cout
            << "FAIL"
            << " | Actual: "
            << result.value()
            << " | Expected: nullopt"
            << '\n';
    }
}


int main()
{
    Statistical_Data statistics;

    std::vector<double> samples =
    {
        2.0,
        4.0,
        6.0,
        8.0,
        10.0
    };

    std::vector<double> empty_samples = {};

    std::vector<double> one_sample =
    {
        5.0
    };


    std::cout
        << "\n========== STATISTICAL DATA EXPANSION VALIDATION ==========\n\n";


    // =================================================
    // MEAN
    // =================================================

    Check_Value(
        "ST-001 Mean",
        statistics.Calculate_Mean(samples),
        6.0);


    // =================================================
    // POPULATION VARIANCE
    // =================================================

    Check_Value(
        "ST-002 Population Variance",
        statistics.Calculate_Population_Variance(samples),
        8.0);


    // =================================================
    // SAMPLE VARIANCE
    // =================================================

    Check_Value(
        "ST-003 Sample Variance",
        statistics.Calculate_Sample_Variance(samples),
        10.0);


    // =================================================
    // SAMPLE STANDARD DEVIATION
    // =================================================

    Check_Value(
        "ST-004 Sample Standard Deviation",
        statistics.Calculate_Standard_Deviation(samples),
        std::sqrt(10.0));


    // =================================================
    // STANDARD ERROR
    // =================================================

    Check_Value(
        "ST-005 Standard Error",
        statistics.Calculate_Standard_Error(samples),
        std::sqrt(10.0) / std::sqrt(5.0));


    // =================================================
    // PERCENTILES
    // =================================================

    Check_Value(
        "ST-006 0th Percentile",
        statistics.Calculate_Percentile(
            samples,
            0.0),
        2.0);

    Check_Value(
        "ST-007 25th Percentile",
        statistics.Calculate_Percentile(
            samples,
            25.0),
        4.0);

    Check_Value(
        "ST-008 50th Percentile",
        statistics.Calculate_Percentile(
            samples,
            50.0),
        6.0);

    Check_Value(
        "ST-009 75th Percentile",
        statistics.Calculate_Percentile(
            samples,
            75.0),
        8.0);

    Check_Value(
        "ST-010 100th Percentile",
        statistics.Calculate_Percentile(
            samples,
            100.0),
        10.0);


    // =================================================
    // EMPTY SAMPLE TESTS
    // =================================================

    Check_Nullopt(
        "ST-011 Empty Mean",
        statistics.Calculate_Mean(
            empty_samples));

    Check_Nullopt(
        "ST-012 Empty Population Variance",
        statistics.Calculate_Population_Variance(
            empty_samples));


    // =================================================
    // ONE-SAMPLE TESTS
    // =================================================

    Check_Nullopt(
        "ST-013 One-Sample Sample Variance",
        statistics.Calculate_Sample_Variance(
            one_sample));

    Check_Nullopt(
        "ST-014 One-Sample Standard Deviation",
        statistics.Calculate_Standard_Deviation(
            one_sample));

    Check_Nullopt(
        "ST-015 One-Sample Standard Error",
        statistics.Calculate_Standard_Error(
            one_sample));


    // =================================================
    // INVALID PERCENTILES
    // =================================================

    Check_Nullopt(
        "ST-016 Percentile Below Zero",
        statistics.Calculate_Percentile(
            samples,
            -1.0));

    Check_Nullopt(
        "ST-017 Percentile Above 100",
        statistics.Calculate_Percentile(
            samples,
            101.0));


    // =================================================
    // 95% CONFIDENCE INTERVAL
    // =================================================

    auto confidence_interval =
        statistics.Calculate_Confidence_Interval_95(
            samples);

    std::cout
        << "ST-018 95% Confidence Interval: ";

    if (confidence_interval.has_value())
    {
        double expected_lower = 2.0735;
        double expected_upper = 9.9265;

        bool lower_pass =
            Nearly_Equal(
                confidence_interval->Lower_Bound,
                expected_lower,
                0.0001);

        bool upper_pass =
            Nearly_Equal(
                confidence_interval->Upper_Bound,
                expected_upper,
                0.0001);

        if (lower_pass && upper_pass)
        {
            std::cout << "PASS";
        }
        else
        {
            std::cout << "FAIL";
        }

        std::cout
            << " | Actual: ["
            << confidence_interval->Lower_Bound
            << ", "
            << confidence_interval->Upper_Bound
            << "]"
            << " | Expected: ["
            << expected_lower
            << ", "
            << expected_upper
            << "]\n";
    }
    else
    {
        std::cout
            << "FAIL"
            << " | Actual: nullopt\n";
    }


    // =================================================
    // CONFIDENCE INTERVAL EDGE CASES
    // =================================================

    auto empty_ci =
        statistics.Calculate_Confidence_Interval_95(
            empty_samples);

    std::cout
        << "ST-019 Empty Confidence Interval: "
        << (!empty_ci.has_value()
                ? "PASS"
                : "FAIL")
        << '\n';


    auto one_sample_ci =
        statistics.Calculate_Confidence_Interval_95(
            one_sample);

    std::cout
        << "ST-020 One-Sample Confidence Interval: "
        << (!one_sample_ci.has_value()
                ? "PASS"
                : "FAIL")
        << '\n';


    // =================================================
    // WELCH TWO-SAMPLE T-TEST
    // =================================================

    std::vector<double> welch_sample_1 =
    {
        10.0,
        12.0,
        14.0,
        16.0,
        18.0
    };

    std::vector<double> welch_sample_2 =
    {
        20.0,
        22.0,
        24.0,
        26.0,
        28.0
    };


    auto welch_result =
        statistics.Calculate_Welch_T_Test(
            welch_sample_1,
            welch_sample_2);

    std::cout
        << "ST-021 Welch T-Test: ";

    if (welch_result.has_value())
    {
        double expected_t = -5.0;
        double expected_df = 8.0;
        double expected_p = 0.00105283;

        bool t_pass =
            Nearly_Equal(
                welch_result->T_Statistic,
                expected_t,
                1e-9);

        bool df_pass =
            Nearly_Equal(
                welch_result->Degrees_Of_Freedom,
                expected_df,
                1e-9);

        bool p_pass =
            Nearly_Equal(
                welch_result->P_Value,
                expected_p,
                1e-7);

        bool significance_pass =
            welch_result->Statistically_Significant;

        if (t_pass &&
            df_pass &&
            p_pass &&
            significance_pass)
        {
            std::cout << "PASS";
        }
        else
        {
            std::cout << "FAIL";
        }

        std::cout
            << " | t = "
            << welch_result->T_Statistic
            << " | Expected t = "
            << expected_t
            << " | df = "
            << welch_result->Degrees_Of_Freedom
            << " | Expected df = "
            << expected_df
            << " | p = "
            << welch_result->P_Value
            << " | Expected p = "
            << expected_p
            << " | Significant = "
            << welch_result->Statistically_Significant
            << '\n';
    }
    else
    {
        std::cout
            << "FAIL"
            << " | Unexpected nullopt\n";
    }


    // =================================================
    // IDENTICAL SAMPLES
    // =================================================

    auto identical_result =
        statistics.Calculate_Welch_T_Test(
            welch_sample_1,
            welch_sample_1);

    std::cout
        << "ST-022 Identical Samples: ";

    if (identical_result.has_value() &&
        Nearly_Equal(
            identical_result->T_Statistic,
            0.0) &&
        Nearly_Equal(
            identical_result->P_Value,
            1.0) &&
        !identical_result->Statistically_Significant)
    {
        std::cout
            << "PASS"
            << " | t = "
            << identical_result->T_Statistic
            << " | p = "
            << identical_result->P_Value
            << '\n';
    }
    else
    {
        std::cout << "FAIL\n";
    }


    // =================================================
    // INSUFFICIENT SAMPLE SIZE
    // =================================================

    auto small_result =
        statistics.Calculate_Welch_T_Test(
            one_sample,
            welch_sample_2);

    std::cout
        << "ST-023 Insufficient Sample Size: "
        << (!small_result.has_value()
                ? "PASS"
                : "FAIL")
        << '\n';


    // =================================================
    // INVALID ALPHA = 0
    // =================================================

    auto invalid_alpha_low =
        statistics.Calculate_Welch_T_Test(
            welch_sample_1,
            welch_sample_2,
            0.0);

    std::cout
        << "ST-024 Invalid Alpha 0.0: "
        << (!invalid_alpha_low.has_value()
                ? "PASS"
                : "FAIL")
        << '\n';


    // =================================================
    // INVALID ALPHA = 1
    // =================================================

    auto invalid_alpha_high =
        statistics.Calculate_Welch_T_Test(
            welch_sample_1,
            welch_sample_2,
            1.0);

    std::cout
        << "ST-025 Invalid Alpha 1.0: "
        << (!invalid_alpha_high.has_value()
                ? "PASS"
                : "FAIL")
        << '\n';


    std::cout
        << "\n===========================================================\n";

    return 0;
}