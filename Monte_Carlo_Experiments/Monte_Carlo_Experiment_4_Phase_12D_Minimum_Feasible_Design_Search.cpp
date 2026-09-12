#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// ============================================================
// MONTE CARLO EXPERIMENT 4 - PHASE 12D
// Minimum-feasible optical design boundary search.
//
// Reads Phase 12B detail records for the frozen baseline design at the
// worst tested background (4 photons/slot). Baseline bottleneck photon
// counts are scaled using the validated free-space link relationships:
//   ns scale = (P/10 W)(Dtx/0.2 m)^2(Drx/0.2 m)^2(1 MHz/slot rate).
//
// A conservative Chernoff/union upper bound on packet failure is used,
// as validated in Phase 12C. A configuration is feasible for a route
// only when every AVAILABLE matched world has failure bound <= 0.001.
// Relay-offline worlds remain reported separately and do not masquerade
// as physical-link failures.
// ============================================================

namespace
{
constexpr double BASE_POWER_W = 10.0;
constexpr double BASE_TX_M = 0.20;
constexpr double BASE_RX_M = 0.20;
constexpr double BASE_SLOT_RATE_HZ = 1.0e6;
constexpr double WORST_BACKGROUND = 4.0;
constexpr int PPM_ORDER = 16;
constexpr double LN_10 = 2.30258509299404568402;
constexpr std::size_t EXPECTED_BASE_ROWS = 400;
constexpr std::size_t EXPECTED_CONFIGURATIONS = 300;

struct Baseline_Record
{
    std::string Trial;
    std::string Seed;
    std::string Route;
    bool Available{};
    int Hop_Count{};
    double Total_Distance_M{};
    double Baseline_Bottleneck_NS{};
};

struct Configuration
{
    int ID{};
    double Power_W{};
    double TX_M{};
    double RX_M{};
    double Slot_Rate_Hz{};
    std::size_t Packet_Bits{};
    double Photon_Gain_Factor{};
};

struct Result
{
    double Scaled_Bottleneck_NS{};
    double Log10_Failure_Upper{};
    double Success_Lower{};
    bool Certified_99_9{};
};

struct Summary
{
    std::size_t Total_Worlds{};
    std::size_t Available_Worlds{};
    std::size_t Certified_Worlds{};
    double Worst_Log10{-std::numeric_limits<double>::infinity()};
    double Sum_Success{};
    double Minimum_NS{std::numeric_limits<double>::infinity()};

    void Add(bool available, const Result& result)
    {
        ++Total_Worlds;
        if (!available) return;
        ++Available_Worlds;
        Certified_Worlds += result.Certified_99_9;
        Worst_Log10 = std::max(Worst_Log10, result.Log10_Failure_Upper);
        Sum_Success += result.Success_Lower;
        Minimum_NS = std::min(Minimum_NS, result.Scaled_Bottleneck_NS);
    }
};

std::vector<std::string> Split(const std::string& line)
{
    std::vector<std::string> fields;
    std::string field;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i)
    {
        const char c = line[i];
        if (c == '"')
        {
            if (quoted && i + 1 < line.size() && line[i + 1] == '"')
            {
                field.push_back('"');
                ++i;
            }
            else quoted = !quoted;
        }
        else if (c == ',' && !quoted)
        {
            fields.push_back(field);
            field.clear();
        }
        else field.push_back(c);
    }
    fields.push_back(field);
    return fields;
}

std::size_t Column(const std::map<std::string, std::size_t>& columns,
                   const std::string& name)
{
    const auto found = columns.find(name);
    if (found == columns.end())
        throw std::runtime_error("Missing Phase 12B column: " + name);
    return found->second;
}

double Parse_Double(const std::string& value, const std::string& name)
{
    char* end = nullptr;
    const double result = std::strtod(value.c_str(), &end);
    if (end == value.c_str() || (*end != '\0' && *end != '\r'))
        throw std::runtime_error("Invalid value in " + name + ": " + value);
    return result;
}

std::vector<Baseline_Record> Read_Baseline(const std::string& file_name)
{
    std::ifstream input(file_name);
    if (!input) throw std::runtime_error("Could not open input: " + file_name);
    std::string line;
    if (!std::getline(input, line)) throw std::runtime_error("Empty input CSV.");
    const auto header = Split(line);
    std::map<std::string, std::size_t> columns;
    for (std::size_t i = 0; i < header.size(); ++i) columns[header[i]] = i;
    const auto get = [&columns](const std::vector<std::string>& row,
                                const std::string& name) -> const std::string&
    { return row.at(Column(columns, name)); };

    std::vector<Baseline_Record> records;
    while (std::getline(input, line))
    {
        if (line.empty()) continue;
        const auto row = Split(line);
        if (get(row, "Design_Name") != "Baseline" ||
            std::abs(Parse_Double(get(row, "Background_Photons_Per_Slot"),
                                  "Background") - WORST_BACKGROUND) > 1.0e-12)
            continue;
        Baseline_Record r;
        r.Trial = get(row, "Trial");
        r.Seed = get(row, "Seed");
        r.Route = get(row, "Route_Name");
        r.Available = std::stoi(get(row, "Route_Available")) != 0;
        r.Hop_Count = std::stoi(get(row, "Hop_Count"));
        r.Total_Distance_M = Parse_Double(
            get(row, "Total_Route_Distance_m"), "Total_Route_Distance_m");
        r.Baseline_Bottleneck_NS = Parse_Double(
            get(row, "Bottleneck_Signal_Photons_Per_Slot"),
            "Bottleneck_Signal_Photons_Per_Slot");
        records.push_back(r);
    }
    return records;
}

double Gain(double power, double tx, double rx, double slot_rate)
{
    return (power / BASE_POWER_W) *
           std::pow(tx / BASE_TX_M, 2.0) *
           std::pow(rx / BASE_RX_M, 2.0) *
           (BASE_SLOT_RATE_HZ / slot_rate);
}

std::vector<Configuration> Build_Configurations()
{
    std::vector<Configuration> configurations;
    int id = 1;
    for (double power : {10.0, 20.0, 30.0, 40.0, 50.0})
    for (double diameter : {0.30, 0.40, 0.50, 0.60, 0.70})
    for (double slot_rate : {2.5e5, 5.0e5, 1.0e6})
    for (std::size_t packet_bits : {1024U, 2048U, 4096U, 8192U})
    {
        configurations.push_back({id++, power, diameter, diameter,
            slot_rate, packet_bits,
            Gain(power, diameter, diameter, slot_rate)});
    }
    return configurations;
}

Result Evaluate(const Baseline_Record& baseline,
                const Configuration& configuration)
{
    Result result;
    if (!baseline.Available)
    {
        result.Log10_Failure_Upper = 0.0;
        return result;
    }
    result.Scaled_Bottleneck_NS =
        baseline.Baseline_Bottleneck_NS * configuration.Photon_Gain_Factor;
    const double exponent = -std::pow(
        std::sqrt(result.Scaled_Bottleneck_NS + WORST_BACKGROUND) -
        std::sqrt(WORST_BACKGROUND), 2.0);
    double log_ser_upper =
        std::log(static_cast<double>(PPM_ORDER - 1)) + exponent;
    log_ser_upper = std::min(0.0, log_ser_upper);
    const std::size_t symbols = static_cast<std::size_t>(std::ceil(
        static_cast<double>(configuration.Packet_Bits) /
        std::log2(static_cast<double>(PPM_ORDER))));
    double log_failure_upper = log_ser_upper +
        std::log(static_cast<double>(symbols)) +
        std::log(static_cast<double>(baseline.Hop_Count));
    log_failure_upper = std::min(0.0, log_failure_upper);
    result.Log10_Failure_Upper = log_failure_upper / LN_10;
    result.Success_Lower = std::clamp(
        1.0 - std::exp(log_failure_upper), 0.0, 1.0);
    result.Certified_99_9 = result.Log10_Failure_Upper <= -3.0;
    return result;
}

bool Tests()
{
    bool ok = true;
    const auto check = [&ok](const char* name, bool value)
    { std::cout << (value ? "PASS: " : "FAIL: ") << name << '\n'; ok &= value; };
    const auto c = Build_Configurations();
    check("configuration count", c.size() == EXPECTED_CONFIGURATIONS);
    check("gain increases with power",
          Gain(20, .5, .5, 1e6) > Gain(10, .5, .5, 1e6));
    check("gain increases with aperture",
          Gain(10, .5, .5, 1e6) > Gain(10, .4, .4, 1e6));
    check("lower slot rate increases photons per slot",
          Gain(10, .5, .5, 2.5e5) > Gain(10, .5, .5, 1e6));
    Baseline_Record b{"1", "1", "Direct", true, 1, 1.0, 0.2};
    Configuration weak{1,10,.3,.3,1e6,8192,Gain(10,.3,.3,1e6)};
    Configuration strong{2,50,.7,.7,2.5e5,1024,Gain(50,.7,.7,2.5e5)};
    check("strong design improves failure bound",
          Evaluate(b,strong).Log10_Failure_Upper <
          Evaluate(b,weak).Log10_Failure_Upper);
    return ok;
}

using Key = std::pair<int, std::string>;
} // namespace

int main(int argc, char* argv[])
{
    if (!Tests()) return 1;
    const std::string input = argc > 1 ? argv[1]
        : "Monte_Carlo_Experiments/Phase12B_Results/"
          "Phase12B_Optical_Design_Sweep_Detail.csv";
    const std::string output_dir = argc > 2 ? argv[2]
        : "Monte_Carlo_Experiments/Phase12D_Results";

    try
    {
        const auto baseline = Read_Baseline(input);
        const auto configurations = Build_Configurations();
        if (baseline.size() != EXPECTED_BASE_ROWS)
        {
            std::cerr << "ERROR: Expected 400 baseline rows; read "
                      << baseline.size() << ".\n";
            return 1;
        }
        std::filesystem::create_directories(output_dir);
        const std::string detail_name = output_dir +
            "/Phase12D_Minimum_Feasible_Design_Detail.csv";
        const std::string summary_name = output_dir +
            "/Phase12D_Minimum_Feasible_Design_Summary.csv";
        std::ofstream detail(detail_name), summary_file(summary_name);
        if (!detail || !summary_file)
            throw std::runtime_error("Could not create Phase 12D outputs.");
        detail << std::setprecision(17);
        summary_file << std::setprecision(17);
        detail << "Trial,Seed,Configuration_ID,Transmit_Power_W,TX_Diameter_m,"
                  "RX_Diameter_m,Slot_Rate_Hz,Packet_Bits,PPM_Order,"
                  "Background_Photons_Per_Slot,Photon_Gain_Factor,Route_Name,"
                  "Route_Available,Hop_Count,Total_Route_Distance_m,"
                  "Baseline_Bottleneck_Photons_Per_Slot,"
                  "Scaled_Bottleneck_Photons_Per_Slot,"
                  "Log10_Route_Packet_Failure_Upper_Bound,"
                  "Route_Packet_Success_Lower_Bound,Certified_99_9pct\n";

        std::map<Key, Summary> summaries;
        for (const Configuration& c : configurations)
        for (const Baseline_Record& b : baseline)
        {
            const Result r = Evaluate(b, c);
            summaries[{c.ID, b.Route}].Add(b.Available, r);
            detail << b.Trial << ',' << b.Seed << ',' << c.ID << ','
                   << c.Power_W << ',' << c.TX_M << ',' << c.RX_M << ','
                   << c.Slot_Rate_Hz << ',' << c.Packet_Bits << ',' << PPM_ORDER
                   << ',' << WORST_BACKGROUND << ',' << c.Photon_Gain_Factor
                   << ',' << b.Route << ',' << (b.Available ? 1 : 0) << ','
                   << b.Hop_Count << ',' << b.Total_Distance_M << ','
                   << b.Baseline_Bottleneck_NS << ',' << r.Scaled_Bottleneck_NS
                   << ',' << r.Log10_Failure_Upper << ',' << r.Success_Lower
                   << ',' << (r.Certified_99_9 ? 1 : 0) << '\n';
        }
        detail.close();

        summary_file << "Configuration_ID,Transmit_Power_W,TX_Diameter_m,"
                        "RX_Diameter_m,Slot_Rate_Hz,Packet_Bits,PPM_Order,"
                        "Background_Photons_Per_Slot,Photon_Gain_Factor,"
                        "Route_Name,Total_Worlds,Available_Worlds,"
                        "Certified_Worlds,All_Available_Worlds_Certified,"
                        "Minimum_Scaled_Bottleneck_Photons_Per_Slot,"
                        "Worst_Log10_Route_Packet_Failure_Upper_Bound,"
                        "Mean_Route_Packet_Success_Lower_Bound_Available\n";
        std::size_t globally_feasible = 0;
        for (const Configuration& c : configurations)
        {
            bool all_routes = true;
            for (const std::string route : {"Direct_Earth_to_Mars",
                 "Earth_Relay0_Mars", "Earth_Relay1_Mars",
                 "Earth_Relay2_Mars"})
            {
                const Summary& s = summaries.at({c.ID, route});
                const bool feasible = s.Available_Worlds > 0 &&
                    s.Certified_Worlds == s.Available_Worlds;
                all_routes &= feasible;
                summary_file << c.ID << ',' << c.Power_W << ',' << c.TX_M
                    << ',' << c.RX_M << ',' << c.Slot_Rate_Hz << ','
                    << c.Packet_Bits << ',' << PPM_ORDER << ','
                    << WORST_BACKGROUND << ',' << c.Photon_Gain_Factor << ','
                    << route << ',' << s.Total_Worlds << ',' << s.Available_Worlds
                    << ',' << s.Certified_Worlds << ',' << (feasible ? 1 : 0)
                    << ',' << s.Minimum_NS << ',' << s.Worst_Log10 << ','
                    << s.Sum_Success / static_cast<double>(s.Available_Worlds)
                    << '\n';
            }
            globally_feasible += all_routes;
        }
        summary_file.close();

        std::cout << "Phase 12D complete.\n"
                  << "Baseline matched-world route rows: " << baseline.size() << '\n'
                  << "Configurations searched: " << configurations.size() << '\n'
                  << "Detail rows: " << baseline.size()*configurations.size() << '\n'
                  << "Summary rows: " << summaries.size() << '\n'
                  << "Configurations feasible across every available route/world: "
                  << globally_feasible << '\n'
                  << "Detail output: " << detail_name << '\n'
                  << "Summary output: " << summary_name << '\n';
    }
    catch (const std::exception& error)
    {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
