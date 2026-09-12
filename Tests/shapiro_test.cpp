#include <iostream>
#include <iomanip>
#include "Shapiro_Time_Delay.h"

int main()
{
    std::cout << std::setprecision(12);

    std::cout << "\n=== STD-007 through STD-012: Final Shapiro Batch ===\n";

    // STD-007: Zero Sun-to-Earth distance
    {
        Shapiro_Time_Delay shapiro;

        double result =
            shapiro.Calculate_Extra_Time_Delay(
                1.989e30,
                0.0,
                2.279e11,
                2.78e9
            );

        std::cout << "STD-007 Zero Sun-Earth Distance: "
                  << result << " seconds\n";
    }

    // STD-008: Negative Sun-to-Earth distance
    {
        Shapiro_Time_Delay shapiro;

        double result =
            shapiro.Calculate_Extra_Time_Delay(
                1.989e30,
                -1.496e11,
                2.279e11,
                2.78e9
            );

        std::cout << "STD-008 Negative Sun-Earth Distance: "
                  << result << " seconds\n";
    }

    // STD-009: Zero Sun-to-target distance
    {
        Shapiro_Time_Delay shapiro;

        double result =
            shapiro.Calculate_Extra_Time_Delay(
                1.989e30,
                1.496e11,
                0.0,
                2.78e9
            );

        std::cout << "STD-009 Zero Sun-Target Distance: "
                  << result << " seconds\n";
    }

    // STD-010: Negative Sun-to-target distance
    {
        Shapiro_Time_Delay shapiro;

        double result =
            shapiro.Calculate_Extra_Time_Delay(
                1.989e30,
                1.496e11,
                -2.279e11,
                2.78e9
            );

        std::cout << "STD-010 Negative Sun-Target Distance: "
                  << result << " seconds\n";
    }

    // STD-011: Very large closest approach
    {
        Shapiro_Time_Delay shapiro;

        double result =
            shapiro.Calculate_Extra_Time_Delay(
                1.989e30,
                1.496e11,
                2.279e11,
                1.0e12
            );

        std::cout << "STD-011 Very Large Closest Approach: "
                  << result << " seconds\n";
    }

    // STD-012: Final nominal regression
    {
        Shapiro_Time_Delay shapiro;

        double result =
            shapiro.Calculate_Extra_Time_Delay(
                1.989e30,
                1.496e11,
                2.279e11,
                2.78e9
            );

        std::cout << "STD-012 Final Nominal Regression: "
                  << result << " seconds\n";
    }

    return 0;
}