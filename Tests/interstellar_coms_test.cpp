#include <iostream>
#include "Interstellar_Communications_Network.h"

int main()
{
    Interstellar_Communications_Network network;

    std::cout << "Propagation Delay: "
              << network.Calculate_Propagation_Delay(299800000.0)
              << " seconds\n";

    std::cout << "Transmission Time: "
              << network.Calculate_Transmission_Time(1000.0, 100.0)
              << " seconds\n";

    std::cout << "Transmission Delay: "
              << network.Calculate_Transmission_Delay(2000.0, 100.0)
              << " seconds\n";

    std::cout << "Round Trip Time: "
              << network.Calculate_Round_Trip_Time(10.0)
              << " seconds\n";

    std::cout << "Utilization Efficiency Parameter: "
              << network.Calculate_Utilization_Efficiency_Parameter(20.0, 10.0)
              << '\n';

    return 0;
}