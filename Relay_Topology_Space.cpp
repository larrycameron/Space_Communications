#include "Relay_Topology_Space.h"


std::size_t Relay_Topology_Space::Calculate_Hop_Count(
    std::size_t relay_count)
{
    return relay_count + 1;
}


double Relay_Topology_Space::Calculate_Equal_Relay_Spacing(
    double total_route_distance,
    std::size_t relay_count)
{
    if (total_route_distance <= 0.0)
    {
        return 0.0;
    }

    std::size_t hop_count =
        Calculate_Hop_Count(relay_count);

    return
        total_route_distance /
        static_cast<double>(hop_count);
}


double Relay_Topology_Space::Calculate_Route_Reliability(
    const std::vector<double>& hop_reliabilities)
{
    if (hop_reliabilities.empty())
    {
        return 0.0;
    }

    double Route_Reliability = 1.0;

    for (double Reliability : hop_reliabilities)
    {
        if (Reliability < 0.0 || Reliability > 1.0)
        {
            return 0.0;
        }

        Route_Reliability *= Reliability;
    }

    return Route_Reliability;
}


double Relay_Topology_Space::Calculate_Average_Hop_Reliability(
    const std::vector<double>& hop_reliabilities)
{
    if (hop_reliabilities.empty())
    {
        return 0.0;
    }

    double Sum =
        std::accumulate(
            hop_reliabilities.begin(),
            hop_reliabilities.end(),
            0.0);

    return
        Sum /
        static_cast<double>(hop_reliabilities.size());
}


double Relay_Topology_Space::Calculate_Weakest_Hop_Reliability(
    const std::vector<double>& hop_reliabilities)
{
    if (hop_reliabilities.empty())
    {
        return 0.0;
    }

    return
        *std::min_element(
            hop_reliabilities.begin(),
            hop_reliabilities.end());
}


double Relay_Topology_Space::Calculate_Bottleneck_Rate(
    const std::vector<double>& hop_rates)
{
    if (hop_rates.empty())
    {
        return 0.0;
    }

    return
        *std::min_element(
            hop_rates.begin(),
            hop_rates.end());
}


double Relay_Topology_Space::Calculate_Throughput_Per_Hop(
    double throughput,
    std::size_t relay_count)
{
    if (throughput <= 0.0)
    {
        return 0.0;
    }

    std::size_t hop_count =
        Calculate_Hop_Count(relay_count);

    return
        throughput /
        static_cast<double>(hop_count);
}


double Relay_Topology_Space::Calculate_Reliability_Margin(
    double packet_delivery_percentage,
    double required_packet_delivery)
{
    return
        packet_delivery_percentage -
        required_packet_delivery;
}


double Relay_Topology_Space::Calculate_Throughput_Gain_Percentage(
    double current_throughput,
    double previous_throughput)
{
    if (previous_throughput <= 0.0)
    {
        return 0.0;
    }

    return
        ((current_throughput - previous_throughput)
        / previous_throughput) * 100.0;
}


double Relay_Topology_Space::Calculate_Marginal_Throughput_Benefit(
    double current_throughput,
    double previous_throughput)
{
    return
        current_throughput -
        previous_throughput;
}


double Relay_Topology_Space::Calculate_Delay_Penalty(
    double current_delay,
    double previous_delay)
{
    return
        current_delay -
        previous_delay;
}