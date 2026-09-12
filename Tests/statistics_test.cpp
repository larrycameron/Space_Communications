#include "Statistical_Data.h"
#include <limits>
#include <iostream>
#include <vector>

int main()
{
      
    
    Statistical_Data statistics;

   std::vector<double> samples{2.0, 4.0, 6.0, 8.0, 10.0};

    statistics.Statistics_Sample_Size(samples, "Test Data", false);

    // Median
    auto median = statistics.Calculate_Median(samples);

    if (median.has_value())
    {
        std::cout << "Median: " << median.value() << std::endl;
    }
    else
    {
        std::cout << "Median: No median" << std::endl;
    }

    // Mode
    std::vector<double> modes = statistics.Calculate_Mode(samples);

    if (modes.empty())
    {
        std::cout << "Mode: No mode" << std::endl;
    }
    else
    {
        std::cout << "Mode: ";

        for (std::size_t i = 0; i < modes.size(); ++i)
        {
            std::cout << modes[i];

            if (i < modes.size() - 1)
            {
                std::cout << ", ";
            }
        }

        std::cout << std::endl;
    }

    // Q1
    auto q1 = statistics.Calculate_Q1(samples);

    if (q1.has_value())
    {
        std::cout << "Q1: " << q1.value() << std::endl;
    }
    else
    {
        std::cout << "Q1: No Q1" << std::endl;
    }

    // Q3
    auto q3 = statistics.Calculate_Q3(samples);

    if (q3.has_value())
    {
        std::cout << "Q3: " << q3.value() << std::endl;
    }
    else
    {
        std::cout << "Q3: No Q3" << std::endl;
    }

    // IQR
    auto iqr = statistics.Calculate_IQR(samples);

    if (iqr.has_value())
    {
        std::cout << "IQR: " << iqr.value() << std::endl;
    }
    else
    {
        std::cout << "IQR: No IQR" << std::endl;
    }

    return 0;
}