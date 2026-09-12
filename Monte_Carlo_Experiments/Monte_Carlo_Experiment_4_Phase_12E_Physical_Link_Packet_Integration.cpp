#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// ============================================================
// MONTE CARLO EXPERIMENT 4 - PHASE 12E
// Configuration 149 physical-link packet integration.
//
// Inputs:
//   1. Frozen Phase 11B packet audit (80,000 matched records).
//   2. Phase 12D detail output (Configuration 149 link bounds).
//
// Preserved: policy selections, releases, contact uniform/outcome, PAT,
// propagation, TTL, and matched-world structure.
// Revised: packet unit = 1024 bits; transmission time is recalculated;
// Configuration 149 route packet-success lower bound is applied using a
// deterministic policy-independent optical uniform. This keeps common
// random numbers across B1-B4 while recording optical failures separately.
// ============================================================

namespace
{
constexpr int CONFIGURATION_ID = 149;
constexpr std::size_t NEW_PACKET_BITS = 1024;
constexpr std::size_t OLD_PACKET_BITS = 8192;
constexpr std::size_t EXPECTED_PACKETS = 80000;
constexpr std::size_t EXPECTED_LINK_ROWS = 400;

using Link_Key = std::pair<std::string, std::string>; // seed, route

struct Link_State
{
    bool Available{};
    double Packet_Success_Lower_Bound{};
    double Log10_Failure_Upper_Bound{};
};

struct Packet
{
    std::string Trial;
    std::string Seed;
    std::string Policy;
    std::size_t Index{};
    double Release{};
    int Route_Index{};
    std::string Route;
    double Contact_Probability{};
    double Contact_Uniform{};
    bool Contact_Realized{};
    std::string Original_Outcome;
    double Old_Transmission_Per_Hop{};
    double Old_Total_Transmission{};
    double PAT{};
    double Propagation{};
    double TTL{};
    double Deadline{};
};

struct Integrated
{
    double Optical_Uniform{};
    double Optical_Probability{};
    double Log10_Failure_Bound{};
    double Transmission_Per_Hop{};
    double Total_Transmission{};
    double Latency{};
    double Arrival{};
    std::string Outcome;
    bool Delivered{};
    bool Expired{};
    bool Dropped{};
    bool Contact_Failure{};
    bool Optical_Failure{};
};

struct Trial_Summary
{
    std::string Trial;
    std::string Seed;
    std::string Policy;
    std::size_t Generated{};
    std::size_t Delivered{};
    std::size_t Expired{};
    std::size_t Dropped{};
    std::size_t Contact_Failures{};
    std::size_t Optical_Failures{};
    std::uint64_t Delivered_Bits{};
    double Sum_Delivered_Latency{};
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
            { field.push_back('"'); ++i; }
            else quoted = !quoted;
        }
        else if (c == ',' && !quoted)
        { fields.push_back(field); field.clear(); }
        else field.push_back(c);
    }
    fields.push_back(field);
    return fields;
}

std::size_t Column(const std::map<std::string,std::size_t>& c,
                   const std::string& name)
{
    const auto f = c.find(name);
    if (f == c.end()) throw std::runtime_error("Missing column: " + name);
    return f->second;
}

double Number(const std::string& value, const std::string& name)
{
    char* end = nullptr;
    const double result = std::strtod(value.c_str(), &end);
    if (end == value.c_str() || (*end != '\0' && *end != '\r'))
        throw std::runtime_error("Invalid number in " + name + ": " + value);
    return result;
}

std::map<Link_Key,Link_State> Read_Links(const std::string& file_name)
{
    std::ifstream in(file_name);
    if (!in) throw std::runtime_error("Could not open Phase 12D detail: " + file_name);
    std::string line;
    std::getline(in,line);
    const auto h=Split(line);
    std::map<std::string,std::size_t> c;
    for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;
    const auto get=[&c](const std::vector<std::string>& r,const std::string& n)->const std::string&
    {return r.at(Column(c,n));};
    std::map<Link_Key,Link_State> links;
    while(std::getline(in,line))
    {
        if(line.empty())continue;
        const auto r=Split(line);
        if(std::stoi(get(r,"Configuration_ID"))!=CONFIGURATION_ID)continue;
        Link_State state;
        state.Available=std::stoi(get(r,"Route_Available"))!=0;
        state.Packet_Success_Lower_Bound=Number(
            get(r,"Route_Packet_Success_Lower_Bound"),"Packet success");
        state.Log10_Failure_Upper_Bound=Number(
            get(r,"Log10_Route_Packet_Failure_Upper_Bound"),"Failure bound");
        links[{get(r,"Seed"),get(r,"Route_Name")}]=state;
    }
    return links;
}

std::vector<Packet> Read_Packets(const std::string& file_name)
{
    std::ifstream in(file_name);
    if(!in)throw std::runtime_error("Could not open Phase 11B packet CSV: "+file_name);
    std::string line;
    std::getline(in,line);
    const auto h=Split(line);
    std::map<std::string,std::size_t> c;
    for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;
    const auto get=[&c](const std::vector<std::string>& r,const std::string& n)->const std::string&
    {return r.at(Column(c,n));};
    std::vector<Packet> packets;
    while(std::getline(in,line))
    {
        if(line.empty())continue;
        const auto r=Split(line);
        Packet p;
        p.Trial=get(r,"Trial"); p.Seed=get(r,"Seed"); p.Policy=get(r,"Policy");
        p.Index=std::stoull(get(r,"Packet_Index"));
        p.Release=Number(get(r,"Release_Time_s"),"Release");
        p.Route_Index=std::stoi(get(r,"Selected_Route_Index"));
        p.Route=get(r,"Selected_Route_Name");
        p.Contact_Probability=Number(get(r,"Route_Contact_Probability"),"Contact probability");
        p.Contact_Uniform=Number(get(r,"Shared_Contact_Uniform"),"Contact uniform");
        p.Contact_Realized=std::stoi(get(r,"Contact_Realized"))!=0;
        p.Original_Outcome=get(r,"Outcome");
        p.Old_Transmission_Per_Hop=Number(get(r,"Transmission_Time_Per_Hop_s"),"TX/hop");
        p.Old_Total_Transmission=Number(get(r,"Total_Transmission_Time_s"),"Total TX");
        p.PAT=Number(get(r,"PAT_Time_s"),"PAT");
        p.Propagation=Number(get(r,"Propagation_Time_s"),"Propagation");
        p.TTL=Number(get(r,"TTL_s"),"TTL");
        p.Deadline=Number(get(r,"Deadline_s"),"Deadline");
        packets.push_back(p);
    }
    return packets;
}

std::uint64_t SplitMix64(std::uint64_t x)
{
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

double Optical_Uniform(const std::string& seed, std::size_t packet_index)
{
    const std::uint64_t s=std::stoull(seed);
    const std::uint64_t mixed=SplitMix64(s ^
        (static_cast<std::uint64_t>(packet_index)+1ULL)*0xd6e8feb86659fd93ULL);
    return static_cast<double>(mixed >> 11) * (1.0/9007199254740992.0);
}

Integrated Evaluate(const Packet& p,const Link_State& link)
{
    Integrated r;
    r.Optical_Uniform=Optical_Uniform(p.Seed,p.Index);
    r.Optical_Probability=link.Packet_Success_Lower_Bound;
    r.Log10_Failure_Bound=link.Log10_Failure_Upper_Bound;
    const double ratio=static_cast<double>(NEW_PACKET_BITS)/OLD_PACKET_BITS;
    r.Transmission_Per_Hop=p.Old_Transmission_Per_Hop*ratio;
    r.Total_Transmission=p.Old_Total_Transmission*ratio;
    r.Latency=r.Total_Transmission+p.PAT+p.Propagation;
    r.Arrival=p.Release+r.Latency;

    if(!link.Available || p.Route_Index<0)
        r.Outcome="DROPPED_ROUTE_UNAVAILABLE";
    else if(!p.Contact_Realized)
    { r.Outcome="DROPPED_CONTACT_NOT_REALIZED"; r.Contact_Failure=true; }
    else if(r.Arrival>p.Deadline)
    { r.Outcome="EXPIRED_TTL"; r.Expired=true; }
    else if(r.Optical_Uniform>r.Optical_Probability)
    { r.Outcome="DROPPED_OPTICAL_LINK_FAILURE"; r.Optical_Failure=true; }
    else
    { r.Outcome="DELIVERED_PHYSICAL_LINK"; r.Delivered=true; }
    r.Dropped=!r.Delivered&&!r.Expired;
    return r;
}

bool Tests()
{
    bool ok=true;
    const auto check=[&ok](const char* n,bool v){std::cout<<(v?"PASS: ":"FAIL: ")<<n<<'\n';ok&=v;};
    check("optical CRN reproducible",Optical_Uniform("100000",7)==Optical_Uniform("100000",7));
    check("packet index changes optical CRN",Optical_Uniform("100000",7)!=Optical_Uniform("100000",8));
    check("new packet is one eighth original",NEW_PACKET_BITS*8==OLD_PACKET_BITS);
    Packet p; p.Seed="1";p.Index=1;p.Route_Index=0;p.Route="Direct";
    p.Contact_Realized=true;p.Old_Transmission_Per_Hop=8;p.Old_Total_Transmission=8;
    p.PAT=1;p.Propagation=1;p.Release=0;p.Deadline=100;
    Link_State certain{true,1.0,-10};
    check("certain optical link delivers",Evaluate(p,certain).Delivered);
    Link_State impossible{true,0.0,0};
    check("impossible optical link drops",Evaluate(p,impossible).Optical_Failure);
    return ok;
}

using Summary_Key=std::tuple<std::string,std::string,std::string>;
} // namespace

int main(int argc,char* argv[])
{
    if(!Tests())return 1;
    const std::string packet_input=argc>1?argv[1]:
        "Monte_Carlo_Experiments/Phase11B_Packet_1_Monte_Carlo_Experiment_4.csv";
    const std::string link_input=argc>2?argv[2]:
        "Monte_Carlo_Experiments/Phase12D_Results/Phase12D_Minimum_Feasible_Design_Detail.csv";
    const std::string output_dir=argc>3?argv[3]:
        "Monte_Carlo_Experiments/Phase12E_Results";
    try
    {
        const auto packets=Read_Packets(packet_input);
        const auto links=Read_Links(link_input);
        if(packets.size()!=EXPECTED_PACKETS)
            throw std::runtime_error("Expected 80000 packets; read "+std::to_string(packets.size()));
        if(links.size()!=EXPECTED_LINK_ROWS)
            throw std::runtime_error("Expected 400 Configuration 149 link states; read "+std::to_string(links.size()));
        std::filesystem::create_directories(output_dir);
        const std::string packet_name=output_dir+"/Phase12E_Physical_Link_Packet_Audit.csv";
        const std::string summary_name=output_dir+"/Phase12E_Physical_Link_Trial_Summary.csv";
        std::ofstream out(packet_name),sumout(summary_name);
        if(!out||!sumout)throw std::runtime_error("Could not create Phase 12E outputs.");
        out<<std::setprecision(17);sumout<<std::setprecision(17);
        out<<"Trial,Seed,Policy,Packet_Index,Release_Time_s,Selected_Route_Index,"
              "Selected_Route_Name,Contact_Probability,Shared_Contact_Uniform,"
              "Contact_Realized,Optical_Success_Lower_Bound,Shared_Optical_Uniform,"
              "Log10_Optical_Failure_Upper_Bound,Packet_Bits,"
              "Transmission_Time_Per_Hop_s,Total_Transmission_Time_s,PAT_Time_s,"
              "Propagation_Time_s,End_To_End_Latency_s,Arrival_Time_s,TTL_s,"
              "Deadline_s,Outcome,Delivered,Expired,Dropped,Contact_Failure,"
              "Optical_Link_Failure\n";
        std::map<Summary_Key,Trial_Summary> summaries;
        bool accounting=true,matched=true;
        std::map<std::tuple<std::string,std::size_t,std::string>,std::pair<double,std::string>> crn_check;
        for(const Packet& p:packets)
        {
            const auto found=links.find({p.Seed,p.Route});
            if(found==links.end())throw std::runtime_error("Missing link state for seed/route: "+p.Seed+"/"+p.Route);
            const Integrated r=Evaluate(p,found->second);
            accounting&=(static_cast<int>(r.Delivered)+static_cast<int>(r.Expired)+static_cast<int>(r.Dropped)==1);
            const auto ck=std::make_tuple(p.Seed,p.Index,p.Route);
            const auto prior=crn_check.find(ck);
            if(prior==crn_check.end())crn_check[ck]={r.Optical_Uniform,r.Outcome};
            else matched&=(prior->second.first==r.Optical_Uniform&&prior->second.second==r.Outcome);
            auto& s=summaries[{p.Trial,p.Seed,p.Policy}];
            s.Trial=p.Trial;s.Seed=p.Seed;s.Policy=p.Policy;++s.Generated;
            s.Delivered+=r.Delivered;s.Expired+=r.Expired;s.Dropped+=r.Dropped;
            s.Contact_Failures+=r.Contact_Failure;s.Optical_Failures+=r.Optical_Failure;
            if(r.Delivered){s.Delivered_Bits+=NEW_PACKET_BITS;s.Sum_Delivered_Latency+=r.Latency;}
            out<<p.Trial<<','<<p.Seed<<','<<p.Policy<<','<<p.Index<<','<<p.Release
               <<','<<p.Route_Index<<','<<p.Route<<','<<p.Contact_Probability<<','
               <<p.Contact_Uniform<<','<<(p.Contact_Realized?1:0)<<','
               <<r.Optical_Probability<<','<<r.Optical_Uniform<<','
               <<r.Log10_Failure_Bound<<','<<NEW_PACKET_BITS<<','
               <<r.Transmission_Per_Hop<<','<<r.Total_Transmission<<','<<p.PAT
               <<','<<p.Propagation<<','<<r.Latency<<','<<r.Arrival<<','<<p.TTL
               <<','<<p.Deadline<<','<<r.Outcome<<','<<(r.Delivered?1:0)<<','
               <<(r.Expired?1:0)<<','<<(r.Dropped?1:0)<<','
               <<(r.Contact_Failure?1:0)<<','<<(r.Optical_Failure?1:0)<<'\n';
        }
        out.close();
        if(!accounting||!matched)throw std::runtime_error("Post-run accounting or CRN matching validation failed.");
        sumout<<"Trial,Seed,Policy,Generated,Delivered,Expired,Dropped,"
                "Contact_Failures,Optical_Link_Failures,Reliability,Delivered_Bits,"
                "Mean_Delivered_Latency_s\n";
        for(const auto& item:summaries)
        {
            const Trial_Summary& s=item.second;
            sumout<<s.Trial<<','<<s.Seed<<','<<s.Policy<<','<<s.Generated<<','
                  <<s.Delivered<<','<<s.Expired<<','<<s.Dropped<<','
                  <<s.Contact_Failures<<','<<s.Optical_Failures<<','
                  <<static_cast<double>(s.Delivered)/s.Generated<<','
                  <<s.Delivered_Bits<<','
                  <<(s.Delivered?s.Sum_Delivered_Latency/s.Delivered:0.0)<<'\n';
        }
        sumout.close();
        std::cout<<"PASS: post-run outcome accounting\n"
                 <<"PASS: same-route policy optical CRN matching\n"
                 <<"Phase 12E complete.\nPackets integrated: "<<packets.size()
                 <<"\nTrial-policy summaries: "<<summaries.size()
                 <<"\nPacket output: "<<packet_name
                 <<"\nSummary output: "<<summary_name<<'\n';
    }
    catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';return 1;}
    return 0;
}
