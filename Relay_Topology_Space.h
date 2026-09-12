#ifndef RELAY_TOPOLOGY_SPACE_H
#define RELAY_TOPOLOGY_SPACE_H

#include <vector>
#include <algorithm>
#include <numeric>
#include <cstddef>

class Relay_Topology_Space
{
public:
    // Topology
    std::size_t Calculate_Hop_Count(std::size_t relay_count);

    double Calculate_Equal_Relay_Spacing(
        double total_route_distance,
        std::size_t relay_count);

    // Reliability
    double Calculate_Route_Reliability(
        const std::vector<double>& hop_reliabilities);

    double Calculate_Average_Hop_Reliability(
        const std::vector<double>& hop_reliabilities);

    double Calculate_Weakest_Hop_Reliability(
        const std::vector<double>& hop_reliabilities);

    // Performance
    double Calculate_Bottleneck_Rate(
        const std::vector<double>& hop_rates);

    double Calculate_Throughput_Per_Hop(
        double throughput,
        std::size_t relay_count);

    // Optimization
    double Calculate_Reliability_Margin(
        double packet_delivery_percentage,
        double required_packet_delivery);

    double Calculate_Throughput_Gain_Percentage(
        double current_throughput,
        double previous_throughput);

    double Calculate_Marginal_Throughput_Benefit(
        double current_throughput,
        double previous_throughput);

    double Calculate_Delay_Penalty(
        double current_delay,
        double previous_delay);
};
#endif