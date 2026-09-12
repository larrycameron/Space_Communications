#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
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

// Monte Carlo Experiment 4 - Phase 13
// Full matched-world RQ4 production experiment.
//
// Fixed optical design (Phase 12D Configuration 149):
// 30 W, 0.50 m TX/RX apertures, 500 kHz slot rate,
// 16-PPM, 1024-bit packets, 1.55 um wavelength.
//
// Four routing policies are compared under four predeclared environments.
// Contact and optical random numbers are matched across policies within each
// seed/scenario/packet. The stress environments are experimental conditions,
// not estimates of their real-world occurrence probabilities.

namespace
{
constexpr double C=2.998e8;
constexpr double H=6.626e-34;
constexpr double PI=3.14159265358979323846;
constexpr double WAVELENGTH_M=1.55e-6;
constexpr double FREQUENCY_HZ=C/WAVELENGTH_M;
constexpr double RECEIVER_EFFICIENCY=0.80;
constexpr double POWER_W=30.0;
constexpr double TX_DIAMETER_M=0.50;
constexpr double RX_DIAMETER_M=0.50;
constexpr double SLOT_RATE_HZ=5.0e5;
constexpr int PPM_ORDER=16;
constexpr std::size_t PACKET_BITS=1024;
constexpr double DATA_RATE_BPS=1.0e8;
constexpr std::size_t EXPECTED_WORLDS=1000;
constexpr std::size_t EXPECTED_TEMPLATES=200000;

const std::array<std::string,4> ROUTES{
 "Direct_Earth_to_Mars","Earth_Relay0_Mars","Earth_Relay1_Mars","Earth_Relay2_Mars"};
const std::array<std::string,4> POLICIES{
 "B1_Shortest_Delay","B2_Contact_Aware","B3_Physical_Success","B4_Physical_Multiobjective"};

struct Scenario{std::string Name;double Direct_Signal_Factor;double Direct_Background;};
const std::array<Scenario,4> SCENARIOS{{
 {"Control",1.0,4.0},
 {"Direct_Background_16",1.0,16.0},
 {"Direct_Pointing_25pct_Power",0.25,4.0},
 {"Direct_Combined_25pctPower_BG16",0.25,16.0}
}};

struct Route
{
 std::string Trial,Seed,Name;
 bool Available{};
 int Hops{};
 double Contact{},PAT{},Propagation{},Bottleneck_NS{},Optical{};
};

struct World{std::string Trial,Seed;std::array<Route,4> Routes;};
struct Packet{std::string Trial,Seed;std::size_t Index{};double Release{},Contact_Uniform{},TTL{},Deadline{};};
struct Decision{std::array<double,4> Scores{};int Selected{-1};};
struct Outcome
{
 double Optical_Uniform{},TX_Per_Hop{},Total_TX{},Latency{},Arrival{};
 std::string Name;bool Delivered{},Expired{},Dropped{},Contact_Failure{},Optical_Failure{};
};
struct Summary
{
 std::string Trial,Seed,Scenario,Policy,Route;
 std::size_t Generated{},Delivered{},Expired{},Dropped{},Contact_Failures{},Optical_Failures{};
 std::uint64_t Delivered_Bits{};double Sum_Latency{};
};

std::vector<std::string> Split(const std::string& line)
{
 std::vector<std::string> f;std::string v;bool q=false;
 for(std::size_t i=0;i<line.size();++i){const char c=line[i];if(c=='"'){
  if(q&&i+1<line.size()&&line[i+1]=='"'){v+='"';++i;}else q=!q;}
  else if(c==','&&!q){f.push_back(v);v.clear();}else v+=c;}f.push_back(v);return f;
}

std::size_t Col(const std::map<std::string,std::size_t>& c,const std::string& n)
{const auto f=c.find(n);if(f==c.end())throw std::runtime_error("Missing column: "+n);return f->second;}

double Num(const std::string& v,const std::string& n)
{char* e=nullptr;const double x=std::strtod(v.c_str(),&e);if(e==v.c_str()||(*e!='\0'&&*e!='\r'))
 throw std::runtime_error("Invalid "+n+": "+v);return x;}

double Hop_NS(double distance,double degradation)
{
 if(distance<=0.0)return 0.0;
 const double top=PI*RX_DIAMETER_M*TX_DIAMETER_M*FREQUENCY_HZ;
 const double diffraction=std::pow(top/(4.0*C*distance),2.0);
 const double received=POWER_W*std::clamp(degradation,0.0,1.0)*diffraction*RECEIVER_EFFICIENCY;
 return (received/(H*FREQUENCY_HZ))/SLOT_RATE_HZ;
}

double Optical_Lower(double ns,double background,int hops)
{
 if(ns<=0.0||hops<=0)return 0.0;
 const double exponent=-std::pow(std::sqrt(ns+background)-std::sqrt(background),2.0);
 const double log_ser=std::min(0.0,std::log(PPM_ORDER-1.0)+exponent);
 const std::size_t symbols=static_cast<std::size_t>(std::ceil(
  static_cast<double>(PACKET_BITS)/std::log2(static_cast<double>(PPM_ORDER))));
 const double log_failure=std::min(0.0,log_ser+std::log(static_cast<double>(symbols))+
  std::log(static_cast<double>(hops)));
 return std::clamp(1.0-std::exp(log_failure),0.0,1.0);
}

std::vector<World> Read_Worlds(const std::string& file)
{
 std::ifstream in(file);if(!in)throw std::runtime_error("Could not open production trial CSV: "+file);
 std::string line;if(!std::getline(in,line))throw std::runtime_error("Production trial CSV is empty.");
 const auto h=Split(line);std::map<std::string,std::size_t> c;for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;
 auto g=[&](const std::vector<std::string>& r,const std::string& n)->const std::string&{return r.at(Col(c,n));};
 std::vector<World> worlds;
 while(std::getline(in,line))
 {
  if(line.empty())continue;const auto row=Split(line);if(g(row,"Policy")!="B1")continue;
  World w;w.Trial=g(row,"Trial");w.Seed=g(row,"Seed");const double degradation=Num(g(row,"Optical_Degradation_Factor"),"degradation");
  const std::array<bool,4> available{{
   std::stoi(g(row,"Direct_Available"))!=0,std::stoi(g(row,"Relay0_Route_Available"))!=0,
   std::stoi(g(row,"Relay1_Route_Available"))!=0,std::stoi(g(row,"Relay2_Route_Available"))!=0}};
  const std::array<double,4> contact{{
   Num(g(row,"Direct_Route_Contact_Probability"),"contact"),Num(g(row,"Relay0_Route_Contact_Probability"),"contact"),
   Num(g(row,"Relay1_Route_Contact_Probability"),"contact"),Num(g(row,"Relay2_Route_Contact_Probability"),"contact")}};
  const std::array<double,4> pat{{Num(g(row,"Direct_PAT_Cost_s"),"PAT"),Num(g(row,"Relay0_PAT_Cost_s"),"PAT"),
   Num(g(row,"Relay1_PAT_Cost_s"),"PAT"),Num(g(row,"Relay2_PAT_Cost_s"),"PAT")}};
  const std::array<double,4> prop{{Num(g(row,"Direct_Propagation_Delay_s"),"propagation"),Num(g(row,"Relay0_Propagation_Delay_s"),"propagation"),
   Num(g(row,"Relay1_Propagation_Delay_s"),"propagation"),Num(g(row,"Relay2_Propagation_Delay_s"),"propagation")}};
  const double direct=Num(g(row,"Earth_Mars_Distance_Meters"),"direct distance");
  const std::array<double,3> h1{{Num(g(row,"Relay0_Earth_Hop_Distance_m"),"hop"),Num(g(row,"Relay1_Earth_Hop_Distance_m"),"hop"),Num(g(row,"Relay2_Earth_Hop_Distance_m"),"hop")}};
  const std::array<double,3> h2{{Num(g(row,"Relay0_Mars_Hop_Distance_m"),"hop"),Num(g(row,"Relay1_Mars_Hop_Distance_m"),"hop"),Num(g(row,"Relay2_Mars_Hop_Distance_m"),"hop")}};
  for(std::size_t i=0;i<4;++i){auto& r=w.Routes[i];r.Trial=w.Trial;r.Seed=w.Seed;r.Name=ROUTES[i];r.Available=available[i];
   r.Hops=i==0?1:2;r.Contact=contact[i];r.PAT=pat[i];r.Propagation=prop[i];
   r.Bottleneck_NS=i==0?Hop_NS(direct,degradation):std::min(Hop_NS(h1[i-1],degradation),Hop_NS(h2[i-1],degradation));}
  worlds.push_back(w);
 }
 return worlds;
}

std::vector<Packet> Read_Packets(const std::string& file)
{
 std::ifstream in(file);if(!in)throw std::runtime_error("Could not open production packet CSV: "+file);
 std::string line;if(!std::getline(in,line))throw std::runtime_error("Production packet CSV is empty.");
 const auto h=Split(line);std::map<std::string,std::size_t> c;for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;
 auto g=[&](const std::vector<std::string>& r,const std::string& n)->const std::string&{return r.at(Col(c,n));};
 std::vector<Packet> packets;
 while(std::getline(in,line)){if(line.empty())continue;const auto r=Split(line);if(g(r,"Policy")!="B1")continue;
  packets.push_back({g(r,"Trial"),g(r,"Seed"),std::stoull(g(r,"Packet_Index")),Num(g(r,"Release_Time_s"),"release"),
   Num(g(r,"Shared_Contact_Uniform"),"contact uniform"),Num(g(r,"TTL_s"),"TTL"),Num(g(r,"Deadline_s"),"deadline")});}
 return packets;
}

double Minimum(const std::array<Route,4>& r,bool pat)
{double x=std::numeric_limits<double>::infinity();for(const auto& a:r)if(a.Available)x=std::min(x,pat?a.PAT:a.Propagation);return x;}

Decision Select(const std::array<Route,4>& r,std::size_t policy)
{
 Decision d;d.Scores.fill(-1.0);double best=-1.0;const double min_pat=Minimum(r,true),min_prop=Minimum(r,false);
 for(std::size_t i=0;i<4;++i){if(!r[i].Available)continue;const double physical=r[i].Contact*r[i].Optical;
  const double score=policy==0?min_prop/r[i].Propagation:policy==1?r[i].Contact:policy==2?physical:
   physical*(min_pat/r[i].PAT)*(min_prop/r[i].Propagation);
  d.Scores[i]=score;if(score>best+1e-15){best=score;d.Selected=static_cast<int>(i);}}
 return d;
}

std::uint64_t Mix(std::uint64_t x)
{x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);}

double Optical_Uniform(const std::string& seed,std::size_t index,const std::string& scenario)
{std::uint64_t tag=1469598103934665603ULL;for(unsigned char c:scenario){tag^=c;tag*=1099511628211ULL;}
 const std::uint64_t x=Mix(std::stoull(seed)^tag^((index+1)*0xd6e8feb86659fd93ULL));
 return static_cast<double>(x>>11)*(1.0/9007199254740992.0);}

Outcome Evaluate(const Packet& p,const Route& r,const std::string& scenario)
{
 Outcome o;o.Optical_Uniform=Optical_Uniform(p.Seed,p.Index,scenario);o.TX_Per_Hop=static_cast<double>(PACKET_BITS)/DATA_RATE_BPS;
 o.Total_TX=o.TX_Per_Hop*r.Hops;o.Latency=o.Total_TX+r.PAT+r.Propagation;o.Arrival=p.Release+o.Latency;
 if(p.Contact_Uniform>r.Contact){o.Name="DROPPED_CONTACT_NOT_REALIZED";o.Contact_Failure=true;}
 else if(o.Arrival>p.Deadline){o.Name="EXPIRED_TTL";o.Expired=true;}
 else if(o.Optical_Uniform>r.Optical){o.Name="DROPPED_OPTICAL_LINK_FAILURE";o.Optical_Failure=true;}
 else{o.Name="DELIVERED_PHYSICAL_LINK";o.Delivered=true;}o.Dropped=!o.Delivered&&!o.Expired;return o;
}

bool Tests()
{
 bool ok=true;auto check=[&](const char* n,bool v){std::cout<<(v?"PASS: ":"FAIL: ")<<n<<'\n';ok&=v;};
 check("more signal improves optical bound",Optical_Lower(40,4,1)>Optical_Lower(10,4,1));
 check("more background worsens optical bound",Optical_Lower(35,4,1)>Optical_Lower(35,16,1));
 check("optical CRN reproducible",Optical_Uniform("1",2,"Control")==Optical_Uniform("1",2,"Control"));
 std::array<Route,4> r{};for(std::size_t i=0;i<4;++i)r[i]={"1","1",ROUTES[i],true,i?2:1,i?.8:.9,i?120.:60.,i?110.:100.,35,.999};
 check("shortest-delay control chooses direct",Select(r,0).Selected==0);r[0].Optical=.1;
 check("physical policy responds to stress",Select(r,2).Selected>0);return ok;
}

using State_Key=std::tuple<std::string,std::string,std::string>;
using Summary_Key=std::tuple<std::string,std::string,std::string,std::string>;
}

int main(int argc,char* argv[])
{
 if(!Tests())return 1;
 const std::string trial_file=argc>1?argv[1]:"Monte_Carlo_Experiments/RQ4_Production_Trial_1_Monte_Carlo_Experiment_4.csv";
 const std::string packet_file=argc>2?argv[2]:"Monte_Carlo_Experiments/RQ4_Production_Packet_1_Monte_Carlo_Experiment_4.csv";
 const std::string output_dir=argc>3?argv[3]:"Monte_Carlo_Experiments/Phase13_RQ4_Production_Results";
 try
 {
  const auto worlds=Read_Worlds(trial_file);const auto packets=Read_Packets(packet_file);
  if(worlds.size()!=EXPECTED_WORLDS)throw std::runtime_error("Expected 1000 B1 worlds; read "+std::to_string(worlds.size()));
  if(packets.size()!=EXPECTED_TEMPLATES)throw std::runtime_error("Expected 200000 B1 packet templates; read "+std::to_string(packets.size()));
  std::map<State_Key,Route> states;std::filesystem::create_directories(output_dir);
  const std::string score_name=output_dir+"/Phase13_RQ4_Production_Route_Scores.csv";
  const std::string audit_name=output_dir+"/Phase13_RQ4_Production_Packet_Audit.csv";
  const std::string summary_name=output_dir+"/Phase13_RQ4_Production_Trial_Summary.csv";
  std::ofstream scores(score_name),audit(audit_name),summary(summary_name);if(!scores||!audit||!summary)throw std::runtime_error("Could not create output files.");
  scores<<std::setprecision(17);audit<<std::setprecision(17);summary<<std::setprecision(17);
  scores<<"Trial,Seed,Scenario,Policy,Route_Name,Available,Hop_Count,Bottleneck_Photons_Per_Slot,Contact_Probability,Optical_Success_Lower,Physical_Success_Lower,PAT_s,Propagation_s,Policy_Score,Selected\n";
  for(const auto& w:worlds)for(const auto& scenario:SCENARIOS)
  {
   auto routes=w.Routes;for(std::size_t i=0;i<4;++i){const double factor=i==0?scenario.Direct_Signal_Factor:1.0;
    const double background=i==0?scenario.Direct_Background:4.0;routes[i].Optical=Optical_Lower(routes[i].Bottleneck_NS*factor,background,routes[i].Hops);}
   for(std::size_t p=0;p<4;++p){const auto d=Select(routes,p);if(d.Selected<0)throw std::runtime_error("No route for seed "+w.Seed);
    states[{w.Seed,scenario.Name,POLICIES[p]}]=routes[static_cast<std::size_t>(d.Selected)];
    for(std::size_t i=0;i<4;++i){const auto& r=routes[i];scores<<r.Trial<<','<<r.Seed<<','<<scenario.Name<<','<<POLICIES[p]<<','<<r.Name<<','<<r.Available<<','<<r.Hops<<','<<r.Bottleneck_NS<<','<<r.Contact<<','<<r.Optical<<','<<r.Contact*r.Optical<<','<<r.PAT<<','<<r.Propagation<<','<<d.Scores[i]<<','<<(d.Selected==static_cast<int>(i))<<'\n';}}
  }
  scores.close();
  if(states.size()!=EXPECTED_WORLDS*SCENARIOS.size()*POLICIES.size())throw std::runtime_error("Selected-state count mismatch.");
  audit<<"Trial,Seed,Scenario,Policy,Packet_Index,Selected_Route,Hop_Count,Packet_Bits,Release_s,Contact_Probability,Shared_Contact_Uniform,Optical_Success_Lower,Shared_Optical_Uniform,Transmission_Per_Hop_s,Total_Transmission_s,PAT_s,Propagation_s,Latency_s,Arrival_s,TTL_s,Deadline_s,Outcome,Delivered,Expired,Dropped,Contact_Failure,Optical_Failure\n";
  std::map<Summary_Key,Summary> summaries;bool accounting=true,matching=true;
  std::map<std::tuple<std::string,std::string,std::size_t>,std::pair<double,double>> crn;
  for(const auto& p:packets)for(const auto& scenario:SCENARIOS)for(const auto& policy:POLICIES)
  {
   const auto f=states.find({p.Seed,scenario.Name,policy});if(f==states.end())throw std::runtime_error("Missing selected state for seed "+p.Seed);
   const auto& r=f->second;const auto o=Evaluate(p,r,scenario.Name);accounting&=(static_cast<int>(o.Delivered)+static_cast<int>(o.Expired)+static_cast<int>(o.Dropped)==1);
   const auto ck=std::make_tuple(p.Seed,scenario.Name,p.Index);const auto old=crn.find(ck);if(old==crn.end())crn[ck]={p.Contact_Uniform,o.Optical_Uniform};
   else matching&=(old->second.first==p.Contact_Uniform&&old->second.second==o.Optical_Uniform);
   auto& s=summaries[{p.Trial,p.Seed,scenario.Name,policy}];s.Trial=p.Trial;s.Seed=p.Seed;s.Scenario=scenario.Name;s.Policy=policy;s.Route=r.Name;
   ++s.Generated;s.Delivered+=o.Delivered;s.Expired+=o.Expired;s.Dropped+=o.Dropped;s.Contact_Failures+=o.Contact_Failure;s.Optical_Failures+=o.Optical_Failure;
   if(o.Delivered){s.Delivered_Bits+=PACKET_BITS;s.Sum_Latency+=o.Latency;}
   audit<<p.Trial<<','<<p.Seed<<','<<scenario.Name<<','<<policy<<','<<p.Index<<','<<r.Name<<','<<r.Hops<<','<<PACKET_BITS<<','<<p.Release<<','<<r.Contact<<','<<p.Contact_Uniform<<','<<r.Optical<<','<<o.Optical_Uniform<<','<<o.TX_Per_Hop<<','<<o.Total_TX<<','<<r.PAT<<','<<r.Propagation<<','<<o.Latency<<','<<o.Arrival<<','<<p.TTL<<','<<p.Deadline<<','<<o.Name<<','<<o.Delivered<<','<<o.Expired<<','<<o.Dropped<<','<<o.Contact_Failure<<','<<o.Optical_Failure<<'\n';
  }
  audit.close();if(!accounting||!matching)throw std::runtime_error("Matched-CRN or outcome-accounting validation failed.");
  summary<<"Trial,Seed,Scenario,Policy,Selected_Route,Generated,Delivered,Expired,Dropped,Contact_Failures,Optical_Failures,Reliability,Delivered_Bits,Mean_Delivered_Latency_s\n";
  for(const auto& x:summaries){const auto& s=x.second;summary<<s.Trial<<','<<s.Seed<<','<<s.Scenario<<','<<s.Policy<<','<<s.Route<<','<<s.Generated<<','<<s.Delivered<<','<<s.Expired<<','<<s.Dropped<<','<<s.Contact_Failures<<','<<s.Optical_Failures<<','<<static_cast<double>(s.Delivered)/s.Generated<<','<<s.Delivered_Bits<<','<<(s.Delivered?s.Sum_Latency/s.Delivered:0.0)<<'\n';}
  summary.close();
  std::cout<<"PASS: 1000 matched production worlds\nPASS: post-run outcome accounting\nPASS: matched contact/optical CRN streams\nPhase 13 RQ4 production complete.\n"
   <<"Packet comparisons: "<<packets.size()*SCENARIOS.size()*POLICIES.size()<<"\nTrial-policy-scenario summaries: "<<summaries.size()<<"\nRoute scores: "<<score_name<<"\nPacket audit: "<<audit_name<<"\nTrial summary: "<<summary_name<<'\n';
 }
 catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';return 1;}return 0;
}
