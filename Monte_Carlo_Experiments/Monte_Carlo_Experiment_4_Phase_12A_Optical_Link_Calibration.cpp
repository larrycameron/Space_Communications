#include "Optical_Communication.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================
// MONTE CARLO EXPERIMENT 4 - PHASE 12A
// Optical-link calibration diagnostic for the 100-trial RQ4 pilot.
//
// This is a NEW model-revision file. It does not modify the frozen
// Phase 10 packet mechanics or the Phase 11/11B routing diagnostics.
// It reads one row per matched Phase 11B world (Policy == B1), applies
// the validated free-space optical link implementation, and maps mean
// signal/background photon counts to conservative uncoded M-PPM symbol
// and packet success probabilities.
//
// Scientific boundary: this is an uncoded photon-counting calibration,
// not a final CCSDS coded-link performance claim. Ties are counted as
// errors. Channel coding, interleaving, detector dark counts, dead time,
// turbulence, and pointing-loss distributions require a later model.
// ============================================================

namespace
{
constexpr double SPEED_OF_LIGHT_M_PER_S = 2.998e8;
constexpr double WAVELENGTH_M = 1.55e-6;
constexpr double OPTICAL_FREQUENCY_HZ =
    SPEED_OF_LIGHT_M_PER_S / WAVELENGTH_M;
constexpr double TX_DIAMETER_M = 0.20;
constexpr double RX_DIAMETER_M = 0.20;
constexpr double TRANSMIT_POWER_W = 10.0;
constexpr double RECEIVER_EFFICIENCY = 0.80;
constexpr double SLOT_RATE_HZ = 1.0e6;
constexpr int PPM_ORDER = 16;
constexpr std::size_t PACKET_BITS = 8192;
constexpr std::size_t EXPECTED_PILOT_WORLDS = 100;

const std::vector<double> BACKGROUND_PHOTONS_PER_SLOT{
    0.0, 0.01, 0.10, 1.0, 4.0};

struct Pilot_World
{
    std::size_t Trial{};
    unsigned long Seed{};
    double Optical_Degradation_Factor{};
    bool Relay_Online[3]{};
    double Direct_Distance_M{};
    double Relay_Hop_1_M[3]{};
    double Relay_Hop_2_M[3]{};
};

struct Optical_Calibration
{
    double Diffraction_Efficiency{};
    double Received_Power_W{};
    double Photon_Flux_Per_S{};
    double Signal_Photons_Per_Slot{};
    double Symbol_Error_Probability{};
    double Packet_Success_Probability{};
};

std::vector<std::string> Split_CSV_Row(const std::string& line)
{
    std::vector<std::string> fields;
    std::string field;
    bool in_quotes = false;

    for (std::size_t i = 0; i < line.size(); ++i)
    {
        const char c = line[i];
        if (c == '"')
        {
            if (in_quotes && i + 1 < line.size() && line[i + 1] == '"')
            {
                field.push_back('"');
                ++i;
            }
            else
            {
                in_quotes = !in_quotes;
            }
        }
        else if (c == ',' && !in_quotes)
        {
            fields.push_back(field);
            field.clear();
        }
        else
        {
            field.push_back(c);
        }
    }
    fields.push_back(field);
    return fields;
}

std::size_t Column(const std::map<std::string, std::size_t>& columns,
                   const std::string& name)
{
    const auto found = columns.find(name);
    if (found == columns.end())
    {
        throw std::runtime_error("Missing required Phase 11B column: " + name);
    }
    return found->second;
}

double As_Double(const std::vector<std::string>& row,
                 const std::map<std::string, std::size_t>& columns,
                 const std::string& name)
{
    return std::stod(row.at(Column(columns, name)));
}

std::vector<Pilot_World> Read_Phase11B_Worlds(const std::string& file_name)
{
    std::ifstream input(file_name);
    if (!input)
    {
        throw std::runtime_error("Could not open input CSV: " + file_name);
    }

    std::string line;
    if (!std::getline(input, line))
    {
        throw std::runtime_error("Input CSV has no header: " + file_name);
    }

    const auto header = Split_CSV_Row(line);
    std::map<std::string, std::size_t> columns;
    for (std::size_t i = 0; i < header.size(); ++i)
    {
        columns[header[i]] = i;
    }

    std::vector<Pilot_World> worlds;
    while (std::getline(input, line))
    {
        if (line.empty())
        {
            continue;
        }
        const auto row = Split_CSV_Row(line);
        if (row.at(Column(columns, "Policy")) != "B1")
        {
            continue; // One record per shared world; B2-B4 are duplicates.
        }

        Pilot_World world;
        world.Trial = static_cast<std::size_t>(
            std::stoull(row.at(Column(columns, "Trial"))));
        world.Seed = std::stoul(row.at(Column(columns, "Seed")));
        world.Optical_Degradation_Factor =
            As_Double(row, columns, "Optical_Degradation_Factor");
        world.Direct_Distance_M =
            As_Double(row, columns, "Earth_Mars_Distance_Meters");

        for (int relay = 0; relay < 3; ++relay)
        {
            const std::string number = std::to_string(relay);
            world.Relay_Online[relay] =
                As_Double(row, columns, "Relay_" + number + "_Online") != 0.0;
            world.Relay_Hop_1_M[relay] = As_Double(
                row, columns, "Relay" + number + "_Earth_Hop_Distance_m");
            world.Relay_Hop_2_M[relay] = As_Double(
                row, columns, "Relay" + number + "_Mars_Hop_Distance_m");
        }
        worlds.push_back(world);
    }
    return worlds;
}

// Probability of correct uncoded M-PPM detection with independent Poisson
// counts and a maximum-count receiver. Signal-slot count ~ Pois(ns + nb),
// each empty-slot count ~ Pois(nb). The strict inequality counts ties as
// errors, making this diagnostic conservative:
//   Pc = sum(k=1..infinity) Pois(k;ns+nb) * F_Pois(k-1;nb)^(M-1).
double Conservative_PPM_SER(double signal_photons_per_slot,
                            double background_photons_per_slot,
                            int ppm_order)
{
    if (signal_photons_per_slot <= 0.0 || ppm_order < 2)
    {
        return 1.0;
    }
    if (background_photons_per_slot < 0.0)
    {
        throw std::invalid_argument("Background photons/slot cannot be negative.");
    }

    // For the configured nb <= 4, ns >= 80 makes the omitted error smaller
    // than double-precision reporting can use in this calibration.
    if (signal_photons_per_slot >= 80.0)
    {
        return 0.0;
    }

    const double signal_mean =
        signal_photons_per_slot + background_photons_per_slot;
    double signal_pmf = std::exp(-signal_mean); // k = 0
    double background_pmf = std::exp(-background_photons_per_slot);
    double background_cdf = background_pmf;    // F_nb(0)
    double correct = 0.0;

    for (int k = 1; k <= 400; ++k)
    {
        signal_pmf *= signal_mean / static_cast<double>(k);
        correct += signal_pmf *
            std::pow(background_cdf, static_cast<double>(ppm_order - 1));

        background_pmf *=
            background_photons_per_slot / static_cast<double>(k);
        background_cdf = std::min(1.0, background_cdf + background_pmf);

        if (k > signal_mean + 12.0 * std::sqrt(signal_mean + 1.0) &&
            signal_pmf < 1.0e-16)
        {
            break;
        }
    }
    return std::clamp(1.0 - correct, 0.0, 1.0);
}

double Uncoded_Packet_Success(double symbol_error_probability)
{
    const double bits_per_symbol = std::log2(static_cast<double>(PPM_ORDER));
    const std::size_t symbols = static_cast<std::size_t>(
        std::ceil(static_cast<double>(PACKET_BITS) / bits_per_symbol));
    if (symbol_error_probability <= 0.0)
    {
        return 1.0;
    }
    if (symbol_error_probability >= 1.0)
    {
        return 0.0;
    }
    return std::exp(static_cast<double>(symbols) *
                    std::log1p(-symbol_error_probability));
}

Optical_Calibration Calibrate_Hop(double distance_m,
                                  double optical_degradation_factor,
                                  double background_photons_per_slot)
{
    Optical_Communications optical;
    Optical_Calibration result;
    result.Diffraction_Efficiency = optical.Calcualte_Diffraction_Squence(
        RX_DIAMETER_M, TX_DIAMETER_M, OPTICAL_FREQUENCY_HZ, distance_m);
    result.Received_Power_W = optical.Free_Space_Optical_Link(
        TRANSMIT_POWER_W,
        std::clamp(optical_degradation_factor, 0.0, 1.0),
        result.Diffraction_Efficiency,
        RECEIVER_EFFICIENCY);
    optical.Calculate_Photon_Information_Efficiency(5.0);
    result.Photon_Flux_Per_S = optical.Get_Photon_Flux();
    result.Signal_Photons_Per_Slot =
        optical.Calculate_Average_Signal_Photons_Per_Slot(SLOT_RATE_HZ);
    result.Symbol_Error_Probability = Conservative_PPM_SER(
        result.Signal_Photons_Per_Slot,
        background_photons_per_slot,
        PPM_ORDER);
    result.Packet_Success_Probability =
        Uncoded_Packet_Success(result.Symbol_Error_Probability);
    return result;
}

bool Self_Tests()
{
    bool passed = true;
    const auto check = [&passed](const std::string& name, bool condition)
    {
        std::cout << (condition ? "PASS: " : "FAIL: ") << name << '\n';
        passed = passed && condition;
    };

    const double ser_low_signal = Conservative_PPM_SER(1.0, 0.1, PPM_ORDER);
    const double ser_high_signal = Conservative_PPM_SER(5.0, 0.1, PPM_ORDER);
    const double ser_low_noise = Conservative_PPM_SER(3.0, 0.01, PPM_ORDER);
    const double ser_high_noise = Conservative_PPM_SER(3.0, 4.0, PPM_ORDER);
    check("zero signal fails", Conservative_PPM_SER(0.0, 0.0, PPM_ORDER) == 1.0);
    check("more signal reduces SER", ser_high_signal < ser_low_signal);
    check("more background raises SER", ser_high_noise > ser_low_noise);
    check("packet probability is bounded",
          Uncoded_Packet_Success(ser_high_signal) >= 0.0 &&
          Uncoded_Packet_Success(ser_high_signal) <= 1.0);
    return passed;
}

void Write_Hop(std::ofstream& output,
               const Pilot_World& world,
               const std::string& route,
               bool route_available,
               int hop,
               double distance_m,
               double background_photons_per_slot)
{
    const Optical_Calibration result = Calibrate_Hop(
        distance_m, world.Optical_Degradation_Factor,
        background_photons_per_slot);
    output << world.Trial << ',' << world.Seed << ',' << route << ','
           << (route_available ? 1 : 0) << ',' << hop << ',' << distance_m << ','
           << world.Optical_Degradation_Factor << ','
           << background_photons_per_slot << ','
           << result.Diffraction_Efficiency << ','
           << result.Received_Power_W << ','
           << result.Photon_Flux_Per_S << ','
           << result.Signal_Photons_Per_Slot << ','
           << PPM_ORDER << ',' << PACKET_BITS << ','
           << result.Symbol_Error_Probability << ','
           << result.Packet_Success_Probability << '\n';
}
} // namespace

int main(int argc, char* argv[])
{
    if (!Self_Tests())
    {
        std::cerr << "Phase 12A self-test failure; no evidence file produced.\n";
        return 1;
    }

    const std::string input_file = argc > 1
        ? argv[1]
        : "Monte_Carlo_Experiments/Phase11B_Trial_1_Monte_Carlo_Experiment_4.csv";
    const std::string output_directory = argc > 2
        ? argv[2]
        : "Monte_Carlo_Experiments/Phase12A_Results";

    try
    {
        const auto worlds = Read_Phase11B_Worlds(input_file);
        if (worlds.size() != EXPECTED_PILOT_WORLDS)
        {
            std::cerr << "ERROR: Expected " << EXPECTED_PILOT_WORLDS
                      << " unique B1 pilot worlds, read " << worlds.size() << ".\n";
            return 1;
        }

        std::filesystem::create_directories(output_directory);
        const std::string output_file = output_directory +
            "/Phase12A_Optical_Link_Calibration.csv";
        std::ofstream output(output_file);
        if (!output)
        {
            std::cerr << "ERROR: Could not create " << output_file << '\n';
            return 1;
        }

        output << std::setprecision(17);
        output << "Trial,Seed,Route_Name,Route_Available,Hop_Number,"
                  "Hop_Distance_m,Optical_Degradation_Factor,"
                  "Background_Photons_Per_Slot,Diffraction_Efficiency,"
                  "Received_Power_W,Photon_Flux_Per_s,Signal_Photons_Per_Slot,"
                  "PPM_Order,Packet_Bits,Conservative_Uncoded_SER,"
                  "Uncoded_Packet_Success_Probability\n";

        std::size_t rows = 0;
        for (const Pilot_World& world : worlds)
        {
            for (double background : BACKGROUND_PHOTONS_PER_SLOT)
            {
                Write_Hop(output, world, "Direct_Earth_to_Mars", true,
                          1, world.Direct_Distance_M, background);
                ++rows;
                for (int relay = 0; relay < 3; ++relay)
                {
                    const std::string route =
                        "Earth_Relay" + std::to_string(relay) + "_Mars";
                    Write_Hop(output, world, route, world.Relay_Online[relay],
                              1, world.Relay_Hop_1_M[relay], background);
                    Write_Hop(output, world, route, world.Relay_Online[relay],
                              2, world.Relay_Hop_2_M[relay], background);
                    rows += 2;
                }
            }
        }
        output.close();

        std::cout << "Phase 12A complete.\n"
                  << "Matched pilot worlds: " << worlds.size() << '\n'
                  << "Calibration rows: " << rows << '\n'
                  << "Output: " << output_file << '\n'
                  << "Interpretation: calibration only; uncoded PPM, ties are errors.\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
