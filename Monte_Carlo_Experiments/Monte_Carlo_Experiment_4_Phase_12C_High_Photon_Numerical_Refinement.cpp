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
// MONTE CARLO EXPERIMENT 4 - PHASE 12C
// High-photon numerical refinement of the Phase 12B design sweep.
//
// Phase 12B intentionally used SER = 0 above 80 signal photons/slot
// to avoid Poisson underflow. That was suitable for screening, but it
// made strong designs appear identically perfect. Phase 12C reads the
// Phase 12B detail CSV and calculates a conservative, numerically stable
// upper bound on route packet-failure probability in log space.
//
// For independent Poisson counts X~Pois(ns+nb) in the signal slot and
// Y~Pois(nb) in an empty slot, the Chernoff bound is
//   P(Y >= X) <= exp[-(sqrt(ns+nb)-sqrt(nb))^2].
// Union bounds are then applied over M-1 competing PPM slots, packet
// symbols, and route hops. Therefore the reported reliability is a
// LOWER bound, not an optimistic point estimate.
// ============================================================

namespace
{
constexpr std::size_t EXPECTED_DETAIL_ROWS = 42000;
constexpr double LN_10 = 2.30258509299404568402;

struct Record
{
    std::string Trial;
    std::string Seed;
    std::string Family;
    std::string Design;
    double Transmit_Power_W{};
    double TX_Diameter_M{};
    double RX_Diameter_M{};
    double Slot_Rate_Hz{};
    int PPM_Order{};
    std::size_t Packet_Bits{};
    double Background{};
    std::string Route;
    bool Available{};
    int Hop_Count{};
    double Total_Distance_M{};
    double Bottleneck_NS{};
    double Phase12B_Probability{};
};

struct Refined_Result
{
    double Log10_Symbol_Error_Upper_Bound{};
    double Log10_Route_Packet_Failure_Upper_Bound{};
    double Route_Packet_Success_Lower_Bound{};
    bool Double_Precision_Saturated{};
};

struct Summary
{
    std::size_t Total_Worlds{};
    std::size_t Available_Worlds{};
    std::size_t At_99_9{};
    std::size_t At_99_99{};
    double Sum_Lower_Bound{};
    double Worst_Log10_Failure{-std::numeric_limits<double>::infinity()};
    double Best_Log10_Failure{std::numeric_limits<double>::infinity()};
    std::vector<double> Log10_Failures;

    void Add(bool available, const Refined_Result& result)
    {
        ++Total_Worlds;
        if (!available) return;
        ++Available_Worlds;
        Sum_Lower_Bound += result.Route_Packet_Success_Lower_Bound;
        At_99_9 += result.Log10_Route_Packet_Failure_Upper_Bound <= -3.0;
        At_99_99 += result.Log10_Route_Packet_Failure_Upper_Bound <= -4.0;
        Worst_Log10_Failure = std::max(
            Worst_Log10_Failure,
            result.Log10_Route_Packet_Failure_Upper_Bound);
        Best_Log10_Failure = std::min(
            Best_Log10_Failure,
            result.Log10_Route_Packet_Failure_Upper_Bound);
        Log10_Failures.push_back(
            result.Log10_Route_Packet_Failure_Upper_Bound);
    }
};

double Parse_Double(const std::string& value, const std::string& column)
{
    char* end = nullptr;
    const double result = std::strtod(value.c_str(), &end);
    if (end == value.c_str() || (*end != '\0' && *end != '\r'))
    {
        throw std::runtime_error(
            "Invalid numeric value in " + column + ": " + value);
    }
    return result;
}

std::vector<std::string> Split_CSV(const std::string& line)
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
            else
            {
                quoted = !quoted;
            }
        }
        else if (c == ',' && !quoted)
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

std::size_t Required_Column(
    const std::map<std::string, std::size_t>& columns,
    const std::string& name)
{
    const auto found = columns.find(name);
    if (found == columns.end())
    {
        throw std::runtime_error("Missing Phase 12B column: " + name);
    }
    return found->second;
}

std::vector<Record> Read_Phase12B_Detail(const std::string& file_name)
{
    std::ifstream input(file_name);
    if (!input)
    {
        throw std::runtime_error("Could not open Phase 12B detail CSV: " + file_name);
    }

    std::string line;
    if (!std::getline(input, line))
    {
        throw std::runtime_error("Phase 12B detail CSV has no header.");
    }
    const auto header = Split_CSV(line);
    std::map<std::string, std::size_t> columns;
    for (std::size_t i = 0; i < header.size(); ++i)
    {
        columns[header[i]] = i;
    }
    const auto text = [&columns](const std::vector<std::string>& row,
                                 const std::string& name) -> const std::string&
    {
        return row.at(Required_Column(columns, name));
    };

    std::vector<Record> records;
    while (std::getline(input, line))
    {
        if (line.empty()) continue;
        const auto row = Split_CSV(line);
        Record r;
        r.Trial = text(row, "Trial");
        r.Seed = text(row, "Seed");
        r.Family = text(row, "Experiment_Family");
        r.Design = text(row, "Design_Name");
        r.Transmit_Power_W = Parse_Double(text(row, "Transmit_Power_W"), "Transmit_Power_W");
        r.TX_Diameter_M = Parse_Double(text(row, "TX_Diameter_m"), "TX_Diameter_m");
        r.RX_Diameter_M = Parse_Double(text(row, "RX_Diameter_m"), "RX_Diameter_m");
        r.Slot_Rate_Hz = Parse_Double(text(row, "Slot_Rate_Hz"), "Slot_Rate_Hz");
        r.PPM_Order = std::stoi(text(row, "PPM_Order"));
        r.Packet_Bits = std::stoull(text(row, "Packet_Bits"));
        r.Background = Parse_Double(text(row, "Background_Photons_Per_Slot"), "Background_Photons_Per_Slot");
        r.Route = text(row, "Route_Name");
        r.Available = std::stoi(text(row, "Route_Available")) != 0;
        r.Hop_Count = std::stoi(text(row, "Hop_Count"));
        r.Total_Distance_M = Parse_Double(text(row, "Total_Route_Distance_m"), "Total_Route_Distance_m");
        r.Bottleneck_NS = Parse_Double(
            text(row, "Bottleneck_Signal_Photons_Per_Slot"), "Bottleneck_Signal_Photons_Per_Slot");
        r.Phase12B_Probability = Parse_Double(
            text(row, "End_To_End_Uncoded_Packet_Success_Probability"),
            "End_To_End_Uncoded_Packet_Success_Probability");
        records.push_back(r);
    }
    return records;
}

Refined_Result Refine(const Record& record)
{
    Refined_Result result;
    if (!record.Available || record.Bottleneck_NS <= 0.0 ||
        record.PPM_Order < 2 || record.Hop_Count < 1)
    {
        result.Log10_Symbol_Error_Upper_Bound = 0.0;
        result.Log10_Route_Packet_Failure_Upper_Bound = 0.0;
        result.Route_Packet_Success_Lower_Bound = 0.0;
        return result;
    }

    const double signal_slot_mean =
        record.Bottleneck_NS + record.Background;
    const double exponent = -std::pow(
        std::sqrt(signal_slot_mean) - std::sqrt(record.Background), 2.0);

    // Union bound across the M-1 empty PPM slots.
    double log_symbol_error_upper =
        std::log(static_cast<double>(record.PPM_Order - 1)) + exponent;
    log_symbol_error_upper = std::min(0.0, log_symbol_error_upper);

    const double bits_per_symbol =
        std::log2(static_cast<double>(record.PPM_Order));
    const std::size_t symbols_per_packet = static_cast<std::size_t>(
        std::ceil(static_cast<double>(record.Packet_Bits) / bits_per_symbol));

    // Union bound across all packet symbols and all route hops.
    double log_route_failure_upper = log_symbol_error_upper +
        std::log(static_cast<double>(symbols_per_packet)) +
        std::log(static_cast<double>(record.Hop_Count));
    log_route_failure_upper = std::min(0.0, log_route_failure_upper);

    result.Log10_Symbol_Error_Upper_Bound =
        log_symbol_error_upper / LN_10;
    result.Log10_Route_Packet_Failure_Upper_Bound =
        log_route_failure_upper / LN_10;

    // exp(log bound) may underflow harmlessly. log10 remains available
    // for ranking even when the success lower bound rounds to exactly 1.
    const double failure_upper = std::exp(log_route_failure_upper);
    result.Route_Packet_Success_Lower_Bound =
        std::clamp(1.0 - failure_upper, 0.0, 1.0);
    result.Double_Precision_Saturated =
        result.Route_Packet_Success_Lower_Bound == 1.0;
    return result;
}

bool Run_Tests()
{
    bool passed = true;
    const auto check = [&passed](const char* name, bool condition)
    {
        std::cout << (condition ? "PASS: " : "FAIL: ") << name << '\n';
        passed = passed && condition;
    };

    Record base;
    base.Available = true;
    base.Bottleneck_NS = 80.0;
    base.Background = 4.0;
    base.PPM_Order = 16;
    base.Packet_Bits = 8192;
    base.Hop_Count = 1;
    const Refined_Result a = Refine(base);
    Record stronger = base;
    stronger.Bottleneck_NS = 160.0;
    const Refined_Result b = Refine(stronger);
    Record noisier = base;
    noisier.Background = 8.0;
    const Refined_Result c = Refine(noisier);
    Record two_hops = base;
    two_hops.Hop_Count = 2;
    const Refined_Result d = Refine(two_hops);

    check("high-photon result remains finite",
          std::isfinite(a.Log10_Route_Packet_Failure_Upper_Bound));
    check("more signal improves failure bound",
          b.Log10_Route_Packet_Failure_Upper_Bound <
          a.Log10_Route_Packet_Failure_Upper_Bound);
    check("more background worsens failure bound",
          c.Log10_Route_Packet_Failure_Upper_Bound >
          a.Log10_Route_Packet_Failure_Upper_Bound);
    check("additional hop worsens failure bound",
          d.Log10_Route_Packet_Failure_Upper_Bound >
          a.Log10_Route_Packet_Failure_Upper_Bound);
    return passed;
}

using Key = std::tuple<std::string, std::string, double>;

double Median(std::vector<double> values)
{
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const std::size_t middle = values.size() / 2;
    if (values.size() % 2 != 0) return values[middle];
    return 0.5 * (values[middle - 1] + values[middle]);
}
} // namespace

int main(int argc, char* argv[])
{
    if (!Run_Tests())
    {
        std::cerr << "Phase 12C self-test failure.\n";
        return 1;
    }

    const std::string input = argc > 1 ? argv[1]
        : "Monte_Carlo_Experiments/Phase12B_Results/"
          "Phase12B_Optical_Design_Sweep_Detail.csv";
    const std::string output_directory = argc > 2 ? argv[2]
        : "Monte_Carlo_Experiments/Phase12C_Results";

    try
    {
        const auto records = Read_Phase12B_Detail(input);
        if (records.size() != EXPECTED_DETAIL_ROWS)
        {
            std::cerr << "ERROR: Expected 42000 Phase 12B rows; read "
                      << records.size() << ".\n";
            return 1;
        }

        std::filesystem::create_directories(output_directory);
        const std::string detail_name = output_directory +
            "/Phase12C_High_Photon_Refinement_Detail.csv";
        const std::string summary_name = output_directory +
            "/Phase12C_High_Photon_Refinement_Summary.csv";
        std::ofstream detail(detail_name);
        std::ofstream summary_file(summary_name);
        if (!detail || !summary_file)
        {
            std::cerr << "ERROR: Could not create Phase 12C outputs.\n";
            return 1;
        }
        detail << std::setprecision(17);
        summary_file << std::setprecision(17);
        detail << "Trial,Seed,Experiment_Family,Design_Name,Transmit_Power_W,"
                  "TX_Diameter_m,RX_Diameter_m,Slot_Rate_Hz,PPM_Order,"
                  "Packet_Bits,Background_Photons_Per_Slot,Route_Name,"
                  "Route_Available,Hop_Count,Total_Route_Distance_m,"
                  "Bottleneck_Signal_Photons_Per_Slot,Phase12B_Probability,"
                  "Log10_Symbol_Error_Upper_Bound,"
                  "Log10_Route_Packet_Failure_Upper_Bound,"
                  "Route_Packet_Success_Lower_Bound,"
                  "Double_Precision_Saturated\n";

        std::map<Key, Summary> summaries;
        for (const Record& record : records)
        {
            const Refined_Result refined = Refine(record);
            summaries[{record.Design, record.Route, record.Background}].Add(
                record.Available, refined);
            detail << record.Trial << ',' << record.Seed << ','
                   << record.Family << ',' << record.Design << ','
                   << record.Transmit_Power_W << ',' << record.TX_Diameter_M
                   << ',' << record.RX_Diameter_M << ',' << record.Slot_Rate_Hz
                   << ',' << record.PPM_Order << ',' << record.Packet_Bits << ','
                   << record.Background << ',' << record.Route << ','
                   << (record.Available ? 1 : 0) << ',' << record.Hop_Count << ','
                   << record.Total_Distance_M << ',' << record.Bottleneck_NS << ','
                   << record.Phase12B_Probability << ','
                   << refined.Log10_Symbol_Error_Upper_Bound << ','
                   << refined.Log10_Route_Packet_Failure_Upper_Bound << ','
                   << refined.Route_Packet_Success_Lower_Bound << ','
                   << (refined.Double_Precision_Saturated ? 1 : 0) << '\n';
        }
        detail.close();

        summary_file << "Design_Name,Route_Name,Background_Photons_Per_Slot,"
                        "Total_Worlds,Available_Worlds,"
                        "Mean_Route_Packet_Success_Lower_Bound_Available,"
                        "Worst_Log10_Route_Packet_Failure_Upper_Bound,"
                        "Median_Log10_Route_Packet_Failure_Upper_Bound,"
                        "Best_Log10_Route_Packet_Failure_Upper_Bound,"
                        "Available_Worlds_Certified_At_Least_99_9pct,"
                        "Available_Worlds_Certified_At_Least_99_99pct\n";
        for (const auto& item : summaries)
        {
            const auto& key = item.first;
            const Summary& s = item.second;
            summary_file << std::get<0>(key) << ',' << std::get<1>(key) << ','
                         << std::get<2>(key) << ',' << s.Total_Worlds << ','
                         << s.Available_Worlds << ','
                         << (s.Available_Worlds == 0 ? 0.0 :
                             s.Sum_Lower_Bound /
                             static_cast<double>(s.Available_Worlds)) << ','
                         << s.Worst_Log10_Failure << ','
                         << Median(s.Log10_Failures) << ','
                         << s.Best_Log10_Failure << ','
                         << s.At_99_9 << ',' << s.At_99_99 << '\n';
        }
        summary_file.close();

        std::cout << "Phase 12C complete.\n"
                  << "Refined detail rows: " << records.size() << '\n'
                  << "Summary rows: " << summaries.size() << '\n'
                  << "Detail output: " << detail_name << '\n'
                  << "Summary output: " << summary_name << '\n'
                  << "Interpretation: conservative reliability lower bounds; "
                     "log10 failure bounds retain high-photon resolution.\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
