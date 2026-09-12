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
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// Phase 12H: final matched packet-level pilot before RQ4 production.
// Uses Configuration 149 (1024-bit packets, 30 W, 0.5 m TX/RX,
// 500 kHz, 16-PPM) and four predeclared Phase 12G environments.

namespace
{
constexpr std::size_t PACKET_BITS=1024;
constexpr double DATA_RATE_BPS=1.0e8;
constexpr std::size_t EXPECTED_TEMPLATE_PACKETS=20000;
constexpr std::size_t EXPECTED_SELECTED_STATES=1600; // 100*4 scenarios*4 policies

const std::array<std::string,4> SCENARIOS{
 "Control","Direct_Background_16","Direct_Pointing_25pct_Power",
 "Direct_Combined_25pctPower_BG16"};
const std::array<std::string,4> POLICIES{
 "B1_Shortest_Delay","B2_Contact_Aware","B3_Physical_Success",
 "B4_Physical_Multiobjective"};

struct Packet_Template
{
 std::string Trial,Seed;
 std::size_t Index{};
 double Release{},Contact_Uniform{},TTL{},Deadline{};
};

struct Selected_State
{
 std::string Trial,Seed,Scenario,Policy,Route;
 int Hops{};
 double Contact{},Optical{},PAT{},Propagation{};
};

struct Outcome
{
 double Optical_Uniform{},TX_Per_Hop{},Total_TX{},Latency{},Arrival{};
 std::string Name;
 bool Delivered{},Expired{},Dropped{},Contact_Failure{},Optical_Failure{};
};

struct Summary
{
 std::string Trial,Seed,Scenario,Policy,Route;
 std::size_t Generated{},Delivered{},Expired{},Dropped{},Contact_Failures{},Optical_Failures{};
 std::uint64_t Delivered_Bits{};
 double Sum_Latency{};
};

std::vector<std::string> Split(const std::string& line)
{
 std::vector<std::string> f;std::string v;bool q=false;
 for(std::size_t i=0;i<line.size();++i){char c=line[i];if(c=='"'){
  if(q&&i+1<line.size()&&line[i+1]=='"'){v+='"';++i;}else q=!q;}
  else if(c==','&&!q){f.push_back(v);v.clear();}else v+=c;}f.push_back(v);return f;
}

std::size_t Col(const std::map<std::string,std::size_t>& c,const std::string& n)
{auto f=c.find(n);if(f==c.end())throw std::runtime_error("Missing column: "+n);return f->second;}

double Num(const std::string& v,const std::string& n)
{
 char* e=nullptr;
 double x=std::strtod(v.c_str(),&e);
 if(e==v.c_str()||(*e!='\0'&&*e!='\r'))
  throw std::runtime_error("Invalid "+n+": "+v);
 return x;
}

std::vector<Packet_Template> Read_Templates(const std::string& file)
{
 std::ifstream in(file);if(!in)throw std::runtime_error("Could not open Phase 11B packets: "+file);
 std::string line;std::getline(in,line);auto h=Split(line);std::map<std::string,std::size_t> c;
 for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;
 auto g=[&](const std::vector<std::string>& r,const std::string& n)->const std::string&{return r.at(Col(c,n));};
 std::vector<Packet_Template> packets;
 while(std::getline(in,line))
 {
  if(line.empty()) continue;
  auto r=Split(line);
  if(g(r,"Policy")!="B1") continue;
  packets.push_back({g(r,"Trial"),g(r,"Seed"),std::stoull(g(r,"Packet_Index")),
   Num(g(r,"Release_Time_s"),"release"),Num(g(r,"Shared_Contact_Uniform"),"contact uniform"),
   Num(g(r,"TTL_s"),"TTL"),Num(g(r,"Deadline_s"),"deadline")});
 }
 return packets;
}

using State_Key=std::tuple<std::string,std::string,std::string>;

std::map<State_Key,Selected_State> Read_Selected(const std::string& file)
{
 std::ifstream in(file);if(!in)throw std::runtime_error("Could not open Phase 12G scores: "+file);
 std::string line;std::getline(in,line);auto h=Split(line);std::map<std::string,std::size_t> c;
 for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;
 auto g=[&](const std::vector<std::string>& r,const std::string& n)->const std::string&{return r.at(Col(c,n));};
 std::map<State_Key,Selected_State> states;
 while(std::getline(in,line))
 {
  if(line.empty()) continue;
  auto r=Split(line);
  if(std::find(SCENARIOS.begin(),SCENARIOS.end(),g(r,"Scenario"))==SCENARIOS.end())continue;
  if(std::stoi(g(r,"Selected"))!=1)continue;
  Selected_State s;s.Trial=g(r,"Trial");s.Seed=g(r,"Seed");s.Scenario=g(r,"Scenario");
  s.Policy=g(r,"Policy");s.Route=g(r,"Route_Name");
  s.Contact=Num(g(r,"Contact_Probability"),"contact");
  s.Optical=Num(g(r,"Stressed_Optical_Success_Lower"),"optical");
  s.PAT=Num(g(r,"PAT_s"),"PAT");s.Propagation=Num(g(r,"Propagation_s"),"propagation");
  s.Hops=s.Route=="Direct_Earth_to_Mars"?1:2;
  states[{s.Seed,s.Scenario,s.Policy}]=s;
 }
 return states;
}

std::uint64_t Mix(std::uint64_t x)
{x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;
 x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);}

double Optical_Uniform(const std::string& seed,std::size_t index,const std::string& scenario)
{
 std::uint64_t tag=1469598103934665603ULL;
 for(unsigned char c:scenario){tag^=c;tag*=1099511628211ULL;}
 const std::uint64_t x=Mix(std::stoull(seed)^tag^((index+1)*0xd6e8feb86659fd93ULL));
 return static_cast<double>(x>>11)*(1.0/9007199254740992.0);
}

Outcome Evaluate(const Packet_Template& p,const Selected_State& s)
{
 Outcome o;o.Optical_Uniform=Optical_Uniform(p.Seed,p.Index,s.Scenario);
 o.TX_Per_Hop=static_cast<double>(PACKET_BITS)/DATA_RATE_BPS;
 o.Total_TX=o.TX_Per_Hop*s.Hops;o.Latency=o.Total_TX+s.PAT+s.Propagation;o.Arrival=p.Release+o.Latency;
 if(p.Contact_Uniform>s.Contact){o.Name="DROPPED_CONTACT_NOT_REALIZED";o.Contact_Failure=true;}
 else if(o.Arrival>p.Deadline){o.Name="EXPIRED_TTL";o.Expired=true;}
 else if(o.Optical_Uniform>s.Optical){o.Name="DROPPED_OPTICAL_LINK_FAILURE";o.Optical_Failure=true;}
 else{o.Name="DELIVERED_PHYSICAL_LINK";o.Delivered=true;}
 o.Dropped=!o.Delivered&&!o.Expired;return o;
}

bool Tests()
{
 bool ok=true;auto check=[&](const char* n,bool v){std::cout<<(v?"PASS: ":"FAIL: ")<<n<<'\n';ok&=v;};
 check("scenario count",SCENARIOS.size()==4);
 check("optical CRN reproducible",Optical_Uniform("1",2,"Control")==Optical_Uniform("1",2,"Control"));
 check("scenario streams distinct",Optical_Uniform("1",2,"Control")!=Optical_Uniform("1",2,SCENARIOS[1]));
 Packet_Template p{"1","1",0,0,.1,1500,1500};Selected_State s{"1","1","Control","B1","Direct",1,.9,1,60,100};
 check("feasible packet delivers",Evaluate(p,s).Delivered);s.Contact=.05;
 check("contact failure classified",Evaluate(p,s).Contact_Failure);return ok;
}

using Summary_Key=std::tuple<std::string,std::string,std::string,std::string>;
} // namespace

int main(int argc,char* argv[])
{
 if(!Tests())return 1;
 const std::string packet_file=argc>1?argv[1]:"Monte_Carlo_Experiments/Phase11B_Packet_1_Monte_Carlo_Experiment_4.csv";
 const std::string score_file=argc>2?argv[2]:"Monte_Carlo_Experiments/Phase12G_Results/Phase12G_Stress_Route_Scores.csv";
 const std::string output_dir=argc>3?argv[3]:"Monte_Carlo_Experiments/Phase12H_Results";
 try
 {
  const auto packets=Read_Templates(packet_file);const auto states=Read_Selected(score_file);
  if(packets.size()!=EXPECTED_TEMPLATE_PACKETS)throw std::runtime_error("Expected 20000 B1 packet templates; read "+std::to_string(packets.size()));
  if(states.size()!=EXPECTED_SELECTED_STATES)throw std::runtime_error("Expected 1600 selected states; read "+std::to_string(states.size()));
  std::filesystem::create_directories(output_dir);
  const std::string audit_name=output_dir+"/Phase12H_Matched_Packet_Audit.csv";
  const std::string summary_name=output_dir+"/Phase12H_Trial_Policy_Summary.csv";
  std::ofstream out(audit_name),sumout(summary_name);if(!out||!sumout)throw std::runtime_error("Could not create outputs.");
  out<<std::setprecision(17);sumout<<std::setprecision(17);
  out<<"Trial,Seed,Scenario,Policy,Packet_Index,Selected_Route,Hop_Count,Packet_Bits,"
       "Release_s,Contact_Probability,Shared_Contact_Uniform,Optical_Success_Lower,"
       "Shared_Optical_Uniform,Transmission_Per_Hop_s,Total_Transmission_s,PAT_s,"
       "Propagation_s,Latency_s,Arrival_s,TTL_s,Deadline_s,Outcome,Delivered,Expired,"
       "Dropped,Contact_Failure,Optical_Failure\n";
  std::map<Summary_Key,Summary> summaries;bool accounting=true,matching=true;
  std::map<std::tuple<std::string,std::string,std::size_t>,std::pair<double,double>> crn;
  for(const auto& p:packets)for(const auto& scenario:SCENARIOS)for(const auto& policy:POLICIES)
  {
   const auto f=states.find({p.Seed,scenario,policy});if(f==states.end())throw std::runtime_error("Missing selected state");
   const auto& st=f->second;const Outcome o=Evaluate(p,st);
   accounting&=(static_cast<int>(o.Delivered)+static_cast<int>(o.Expired)+static_cast<int>(o.Dropped)==1);
   const auto ck=std::make_tuple(p.Seed,scenario,p.Index);const auto old=crn.find(ck);
   if(old==crn.end())crn[ck]={p.Contact_Uniform,o.Optical_Uniform};
   else matching&=(old->second.first==p.Contact_Uniform&&old->second.second==o.Optical_Uniform);
   auto& sm=summaries[{p.Trial,p.Seed,scenario,policy}];sm.Trial=p.Trial;sm.Seed=p.Seed;
   sm.Scenario=scenario;sm.Policy=policy;sm.Route=st.Route;++sm.Generated;sm.Delivered+=o.Delivered;
   sm.Expired+=o.Expired;sm.Dropped+=o.Dropped;sm.Contact_Failures+=o.Contact_Failure;
   sm.Optical_Failures+=o.Optical_Failure;if(o.Delivered){sm.Delivered_Bits+=PACKET_BITS;sm.Sum_Latency+=o.Latency;}
   out<<p.Trial<<','<<p.Seed<<','<<scenario<<','<<policy<<','<<p.Index<<','<<st.Route<<','
      <<st.Hops<<','<<PACKET_BITS<<','<<p.Release<<','<<st.Contact<<','<<p.Contact_Uniform
      <<','<<st.Optical<<','<<o.Optical_Uniform<<','<<o.TX_Per_Hop<<','<<o.Total_TX<<','
      <<st.PAT<<','<<st.Propagation<<','<<o.Latency<<','<<o.Arrival<<','<<p.TTL<<','
      <<p.Deadline<<','<<o.Name<<','<<o.Delivered<<','<<o.Expired<<','<<o.Dropped<<','
      <<o.Contact_Failure<<','<<o.Optical_Failure<<'\n';
  }
  out.close();if(!accounting||!matching)throw std::runtime_error("Post-run validation failed.");
  sumout<<"Trial,Seed,Scenario,Policy,Selected_Route,Generated,Delivered,Expired,Dropped,"
          "Contact_Failures,Optical_Failures,Reliability,Delivered_Bits,Mean_Delivered_Latency_s\n";
  for(const auto& x:summaries){const auto& s=x.second;sumout<<s.Trial<<','<<s.Seed<<','
   <<s.Scenario<<','<<s.Policy<<','<<s.Route<<','<<s.Generated<<','<<s.Delivered<<','
   <<s.Expired<<','<<s.Dropped<<','<<s.Contact_Failures<<','<<s.Optical_Failures<<','
   <<static_cast<double>(s.Delivered)/s.Generated<<','<<s.Delivered_Bits<<','
   <<(s.Delivered?s.Sum_Latency/s.Delivered:0)<<'\n';}sumout.close();
  std::cout<<"PASS: post-run outcome accounting\nPASS: matched contact/optical CRN streams\n"
   <<"Phase 12H complete.\nPacket comparisons: "<<packets.size()*SCENARIOS.size()*POLICIES.size()
   <<"\nTrial-policy-scenario summaries: "<<summaries.size()
   <<"\nPacket output: "<<audit_name<<"\nSummary output: "<<summary_name<<'\n';
 }
 catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';return 1;}return 0;
}
