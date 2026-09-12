#include "../Optical_Communication.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// ============================================================
// MONTE CARLO EXPERIMENT 4 - PHASE 12B
// Optical design-space sweep over the frozen 100 Phase 11B worlds.
//
// Phase 12A showed that the baseline uncoded 16-PPM/8192-bit packet
// configuration makes end-to-end packet success effectively zero on
// long interplanetary hops. Phase 12B changes no Phase 10/11 routing
// mechanics. It searches for a physically usable operating region.
//
// IMPORTANT: Results remain an uncoded photon-counting calibration.
// They are not a claim of CCSDS coded-link performance. Ties in the
// maximum-count PPM detector are conservatively treated as errors.
// ============================================================

namespace
{
constexpr double C = 2.998e8;
constexpr double WAVELENGTH_M = 1.55e-6;
constexpr double FREQUENCY_HZ = C / WAVELENGTH_M;
constexpr double RECEIVER_EFFICIENCY = 0.80;
constexpr std::size_t EXPECTED_WORLDS = 100;

const std::vector<double> BACKGROUNDS{0.0, 0.01, 0.10, 1.0, 4.0};

struct Design
{
    std::string Family;
    std::string Name;
    double Transmit_Power_W{};
    double TX_Diameter_M{};
    double RX_Diameter_M{};
    double Slot_Rate_Hz{};
    int PPM_Order{};
    std::size_t Packet_Bits{};
};

struct World
{
    std::size_t Trial{};
    unsigned long Seed{};
    double Optical_Degradation{};
    bool Relay_Online[3]{};
    double Direct_M{};
    double Relay_Hop1_M[3]{};
    double Relay_Hop2_M[3]{};
};

struct Link_Result
{
    double Diffraction{};
    double Received_Power_W{};
    double Photon_Flux{};
    double Signal_Photons_Per_Slot{};
    double SER{};
    double Packet_Success{};
};

struct Summary
{
    std::size_t N{};
    double Sum{};
    double Minimum{1.0};
    double Maximum{};
    std::size_t At_50{};
    std::size_t At_90{};
    std::size_t At_99{};
    std::size_t At_999{};

    void Add(double p)
    {
        ++N;
        Sum += p;
        Minimum = std::min(Minimum, p);
        Maximum = std::max(Maximum, p);
        At_50 += p >= 0.50;
        At_90 += p >= 0.90;
        At_99 += p >= 0.99;
        At_999 += p >= 0.999;
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

std::size_t Col(const std::map<std::string, std::size_t>& columns,
                const std::string& name)
{
    const auto found = columns.find(name);
    if (found == columns.end())
    {
        throw std::runtime_error("Missing required Phase 11B column: " + name);
    }
    return found->second;
}

double Number(const std::vector<std::string>& row,
              const std::map<std::string, std::size_t>& columns,
              const std::string& name)
{
    return std::stod(row.at(Col(columns, name)));
}

std::vector<World> Read_Worlds(const std::string& file_name)
{
    std::ifstream input(file_name);
    if (!input)
    {
        throw std::runtime_error("Could not open input CSV: " + file_name);
    }

    std::string line;
    if (!std::getline(input, line))
    {
        throw std::runtime_error("Input CSV has no header.");
    }
    const auto header = Split(line);
    std::map<std::string, std::size_t> columns;
    for (std::size_t i = 0; i < header.size(); ++i)
    {
        columns[header[i]] = i;
    }

    std::vector<World> worlds;
    while (std::getline(input, line))
    {
        if (line.empty()) continue;
        const auto row = Split(line);
        if (row.at(Col(columns, "Policy")) != "B1") continue;

        World w;
        w.Trial = std::stoull(row.at(Col(columns, "Trial")));
        w.Seed = std::stoul(row.at(Col(columns, "Seed")));
        w.Optical_Degradation = Number(row, columns,
                                               "Optical_Degradation_Factor");
        w.Direct_M = Number(row, columns, "Earth_Mars_Distance_Meters");
        for (int relay = 0; relay < 3; ++relay)
        {
            const std::string n = std::to_string(relay);
            w.Relay_Online[relay] =
                Number(row, columns, "Relay_" + n + "_Online") != 0.0;
            w.Relay_Hop1_M[relay] = Number(
                row, columns, "Relay" + n + "_Earth_Hop_Distance_m");
            w.Relay_Hop2_M[relay] = Number(
                row, columns, "Relay" + n + "_Mars_Hop_Distance_m");
        }
        worlds.push_back(w);
    }
    return worlds;
}

std::vector<Design> Build_Designs()
{
    const Design base{"Baseline", "Baseline", 10.0, 0.20, 0.20,
                      1.0e6, 16, 8192};
    std::vector<Design> d{base};

    for (double value : {25.0, 50.0, 100.0})
    {
        Design x = base;
        x.Family = "Transmit_Power";
        x.Name = "Power_" + std::to_string(static_cast<int>(value)) + "W";
        x.Transmit_Power_W = value;
        d.push_back(x);
    }
    for (double value : {0.30, 0.50, 1.00})
    {
        Design x = base;
        x.Family = "Aperture";
        x.Name = "TX_RX_" + std::to_string(static_cast<int>(value * 100.0)) + "cm";
        x.TX_Diameter_M = value;
        x.RX_Diameter_M = value;
        d.push_back(x);
    }
    for (double value : {1.0e5, 2.5e5, 5.0e5, 2.0e6})
    {
        Design x = base;
        x.Family = "Slot_Rate";
        x.Name = "Slot_Rate_" + std::to_string(static_cast<int>(value)) + "Hz";
        x.Slot_Rate_Hz = value;
        d.push_back(x);
    }
    for (int value : {4, 8, 32, 64})
    {
        Design x = base;
        x.Family = "PPM_Order";
        x.Name = "PPM_" + std::to_string(value);
        x.PPM_Order = value;
        d.push_back(x);
    }
    for (std::size_t value : {256U, 1024U, 2048U, 4096U})
    {
        Design x = base;
        x.Family = "Packet_Size";
        x.Name = "Packet_" + std::to_string(value) + "bits";
        x.Packet_Bits = value;
        d.push_back(x);
    }

    d.push_back({"Combined", "Candidate_Moderate", 50.0, 0.50, 0.50,
                 2.5e5, 16, 1024});
    d.push_back({"Combined", "Candidate_High_Margin", 100.0, 1.00, 1.00,
                 1.0e5, 16, 256});
    return d;
}

double PPM_SER(double ns, double nb, int order)
{
    if (ns <= 0.0 || order < 2) return 1.0;
    if (nb < 0.0) throw std::invalid_argument("Negative background.");
    if (ns >= 80.0) return 0.0;

    const double mean = ns + nb;
    double signal_pmf = std::exp(-mean);
    double noise_pmf = std::exp(-nb);
    double noise_cdf = noise_pmf;
    double correct = 0.0;
    for (int k = 1; k <= 400; ++k)
    {
        signal_pmf *= mean / static_cast<double>(k);
        correct += signal_pmf *
            std::pow(noise_cdf, static_cast<double>(order - 1));
        noise_pmf *= nb / static_cast<double>(k);
        noise_cdf = std::min(1.0, noise_cdf + noise_pmf);
        if (k > mean + 12.0 * std::sqrt(mean + 1.0) &&
            signal_pmf < 1.0e-16) break;
    }
    return std::clamp(1.0 - correct, 0.0, 1.0);
}

double Packet_Success(double ser, const Design& d)
{
    const double bits_per_symbol = std::log2(static_cast<double>(d.PPM_Order));
    const std::size_t symbols = static_cast<std::size_t>(
        std::ceil(static_cast<double>(d.Packet_Bits) / bits_per_symbol));
    if (ser <= 0.0) return 1.0;
    if (ser >= 1.0) return 0.0;
    return std::exp(static_cast<double>(symbols) * std::log1p(-ser));
}

Link_Result Evaluate_Hop(double distance_m, double degradation,
                         double background, const Design& d)
{
    Optical_Communications optical;
    Link_Result r;
    r.Diffraction = optical.Calcualte_Diffraction_Squence(
        d.RX_Diameter_M, d.TX_Diameter_M, FREQUENCY_HZ, distance_m);
    r.Received_Power_W = optical.Free_Space_Optical_Link(
        d.Transmit_Power_W, std::clamp(degradation, 0.0, 1.0),
        r.Diffraction, RECEIVER_EFFICIENCY);
    optical.Calculate_Photon_Information_Efficiency(5.0);
    r.Photon_Flux = optical.Get_Photon_Flux();
    r.Signal_Photons_Per_Slot =
        optical.Calculate_Average_Signal_Photons_Per_Slot(d.Slot_Rate_Hz);
    r.SER = PPM_SER(r.Signal_Photons_Per_Slot, background, d.PPM_Order);
    r.Packet_Success = Packet_Success(r.SER, d);
    return r;
}

bool Tests()
{
    bool ok = true;
    const auto check = [&ok](const char* name, bool value)
    {
        std::cout << (value ? "PASS: " : "FAIL: ") << name << '\n';
        ok = ok && value;
    };
    check("increased photons reduce SER", PPM_SER(5.0, 0.1, 16) < PPM_SER(1.0, 0.1, 16));
    check("background raises SER", PPM_SER(3.0, 4.0, 16) > PPM_SER(3.0, 0.01, 16));
    const auto designs = Build_Designs();
    check("design names are unique", [&designs]()
    {
        std::map<std::string, int> names;
        for (const auto& d : designs) ++names[d.Name];
        for (const auto& item : names) if (item.second != 1) return false;
        return true;
    }());
    check("baseline retained", designs.front().Transmit_Power_W == 10.0 &&
          designs.front().Packet_Bits == 8192);
    return ok;
}

using Summary_Key = std::tuple<std::string, std::string, double>;

void Write_Detail(std::ofstream& out, const World& w, const Design& d,
                  double background, const std::string& route,
                  bool available, const std::vector<double>& distances,
                  std::map<Summary_Key, Summary>& summaries)
{
    double route_probability = available ? 1.0 : 0.0;
    double bottleneck_ns = 1.0e300;
    double total_distance = 0.0;
    for (double distance : distances)
    {
        const Link_Result hop = Evaluate_Hop(
            distance, w.Optical_Degradation, background, d);
        route_probability *= hop.Packet_Success;
        bottleneck_ns = std::min(bottleneck_ns, hop.Signal_Photons_Per_Slot);
        total_distance += distance;
    }
    route_probability = std::clamp(route_probability, 0.0, 1.0);
    summaries[{d.Name, route, background}].Add(route_probability);

    out << w.Trial << ',' << w.Seed << ',' << d.Family << ',' << d.Name << ','
        << d.Transmit_Power_W << ',' << d.TX_Diameter_M << ','
        << d.RX_Diameter_M << ',' << d.Slot_Rate_Hz << ',' << d.PPM_Order
        << ',' << d.Packet_Bits << ',' << background << ',' << route << ','
        << (available ? 1 : 0) << ',' << distances.size() << ','
        << total_distance << ',' << bottleneck_ns << ','
        << route_probability << '\n';
}
} // namespace

int main(int argc, char* argv[])
{
    if (!Tests()) return 1;
    const std::string input = argc > 1 ? argv[1]
        : "Monte_Carlo_Experiments/Phase11B_Trial_1_Monte_Carlo_Experiment_4.csv";
    const std::string output_dir = argc > 2 ? argv[2]
        : "Monte_Carlo_Experiments/Phase12B_Results";

    try
    {
        const auto worlds = Read_Worlds(input);
        if (worlds.size() != EXPECTED_WORLDS)
        {
            std::cerr << "ERROR: Expected 100 B1 worlds; read "
                      << worlds.size() << ".\n";
            return 1;
        }
        const auto designs = Build_Designs();
        std::filesystem::create_directories(output_dir);
        const std::string detail_name = output_dir +
            "/Phase12B_Optical_Design_Sweep_Detail.csv";
        const std::string summary_name = output_dir +
            "/Phase12B_Optical_Design_Sweep_Summary.csv";
        std::ofstream detail(detail_name);
        std::ofstream summary(summary_name);
        if (!detail || !summary)
        {
            std::cerr << "ERROR: Could not create Phase 12B output files.\n";
            return 1;
        }
        detail << std::setprecision(17);
        summary << std::setprecision(17);
        detail << "Trial,Seed,Experiment_Family,Design_Name,Transmit_Power_W,"
                  "TX_Diameter_m,RX_Diameter_m,Slot_Rate_Hz,PPM_Order,"
                  "Packet_Bits,Background_Photons_Per_Slot,Route_Name,"
                  "Route_Available,Hop_Count,Total_Route_Distance_m,"
                  "Bottleneck_Signal_Photons_Per_Slot,"
                  "End_To_End_Uncoded_Packet_Success_Probability\n";

        std::map<Summary_Key, Summary> summaries;
        std::size_t detail_rows = 0;
        for (const World& w : worlds)
        {
            for (const Design& d : designs)
            {
                for (double bg : BACKGROUNDS)
                {
                    Write_Detail(detail, w, d, bg, "Direct_Earth_to_Mars",
                                 true, {w.Direct_M}, summaries);
                    ++detail_rows;
                    for (int relay = 0; relay < 3; ++relay)
                    {
                        Write_Detail(detail, w, d, bg,
                            "Earth_Relay" + std::to_string(relay) + "_Mars",
                            w.Relay_Online[relay],
                            {w.Relay_Hop1_M[relay], w.Relay_Hop2_M[relay]},
                            summaries);
                        ++detail_rows;
                    }
                }
            }
        }
        detail.close();

        summary << "Experiment_Family,Design_Name,Transmit_Power_W,"
                   "TX_Diameter_m,RX_Diameter_m,Slot_Rate_Hz,PPM_Order,"
                   "Packet_Bits,Background_Photons_Per_Slot,Route_Name,N_Worlds,"
                   "Mean_End_To_End_Packet_Success,Minimum,Maximum,"
                   "Worlds_At_Least_50pct,Worlds_At_Least_90pct,"
                   "Worlds_At_Least_99pct,Worlds_At_Least_99_9pct\n";
        for (const Design& d : designs)
        {
            for (double bg : BACKGROUNDS)
            {
                for (const std::string route : {"Direct_Earth_to_Mars",
                     "Earth_Relay0_Mars", "Earth_Relay1_Mars",
                     "Earth_Relay2_Mars"})
                {
                    const Summary& s = summaries.at({d.Name, route, bg});
                    summary << d.Family << ',' << d.Name << ','
                            << d.Transmit_Power_W << ',' << d.TX_Diameter_M << ','
                            << d.RX_Diameter_M << ',' << d.Slot_Rate_Hz << ','
                            << d.PPM_Order << ',' << d.Packet_Bits << ',' << bg
                            << ',' << route << ',' << s.N << ','
                            << s.Sum / static_cast<double>(s.N) << ','
                            << s.Minimum << ',' << s.Maximum << ','
                            << s.At_50 << ',' << s.At_90 << ',' << s.At_99 << ','
                            << s.At_999 << '\n';
                }
            }
        }
        summary.close();

        std::cout << "Phase 12B complete.\n"
                  << "Matched worlds: " << worlds.size() << '\n'
                  << "Design configurations: " << designs.size() << '\n'
                  << "Detail rows: " << detail_rows << '\n'
                  << "Detail output: " << detail_name << '\n'
                  << "Summary output: " << summary_name << '\n'
                  << "Interpretation: uncoded calibration sweep only.\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
