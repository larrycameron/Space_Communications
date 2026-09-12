#include <algorithm>
#include <array>
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

// Phase 12G: controlled selector stress validation.
// Stress scenarios are diagnostic interventions, not occurrence estimates.
// Configuration 149 and the 100 matched worlds remain fixed.

namespace
{
constexpr int CONFIGURATION_ID=149;
constexpr double BASE_BACKGROUND=4.0;
constexpr int PPM_ORDER=16;
constexpr std::size_t PACKET_BITS=1024;
constexpr double LN10=2.30258509299404568402;
constexpr std::size_t EXPECTED_ROUTES=400;

const std::array<std::string,4> ROUTE_NAMES{
 "Direct_Earth_to_Mars","Earth_Relay0_Mars","Earth_Relay1_Mars","Earth_Relay2_Mars"};
const std::array<std::string,4> POLICY_NAMES{
 "B1_Shortest_Delay","B2_Contact_Aware","B3_Physical_Success","B4_Physical_Multiobjective"};

struct Scenario
{
 std::string Name;
 double Direct_Signal_Factor{};
 double Direct_Background{};
 bool Obstruct_Direct{};
};

const std::array<Scenario,7> SCENARIOS{{
 {"Control",1.0,4.0,false},
 {"Direct_Pointing_50pct_Power",0.50,4.0,false},
 {"Direct_Pointing_25pct_Power",0.25,4.0,false},
 {"Direct_Pointing_10pct_Power",0.10,4.0,false},
 {"Direct_Background_16",1.0,16.0,false},
 {"Direct_Combined_25pctPower_BG16",0.25,16.0,false},
 {"Direct_Hard_Obstruction",0.0,4.0,true}
}};

struct Route
{
 std::string Trial,Seed,Name;
 bool Available{};
 int Hops{};
 double Contact{},Optical{},NS{},PAT{},Propagation{};
};

struct Decision
{
 std::array<double,4> Scores{};
 int Selected{-1};
};

struct Summary
{
 std::size_t Worlds{},Direct{},Relay0{},Relay1{},Relay2{},NoRoute{};
 double Sum_Selected_Physical{};
};

std::vector<std::string> Split(const std::string& line)
{
 std::vector<std::string> f;std::string v;bool q=false;
 for(std::size_t i=0;i<line.size();++i)
 {
  char c=line[i];
  if(c=='"'){if(q&&i+1<line.size()&&line[i+1]=='"'){v+='"';++i;}else q=!q;}
  else if(c==','&&!q){f.push_back(v);v.clear();}else v+=c;
 }
 f.push_back(v);return f;
}

std::size_t Col(const std::map<std::string,std::size_t>& c,const std::string& n)
{auto f=c.find(n);if(f==c.end())throw std::runtime_error("Missing column: "+n);return f->second;}

double Num(const std::string& v,const std::string& n)
{
 char* e=nullptr;double x=std::strtod(v.c_str(),&e);
 if(e==v.c_str()||(*e!='\0'&&*e!='\r'))throw std::runtime_error("Invalid "+n+": "+v);
 return x;
}

using Key=std::pair<std::string,std::string>;

std::map<Key,double> Read_NS(const std::string& file)
{
 std::ifstream in(file);if(!in)throw std::runtime_error("Could not open Phase 12D detail: "+file);
 std::string line;std::getline(in,line);auto h=Split(line);std::map<std::string,std::size_t> c;
 for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;
 auto g=[&](const std::vector<std::string>& r,const std::string& n)->const std::string&{return r.at(Col(c,n));};
 std::map<Key,double> values;
 while(std::getline(in,line))
 {
  if(line.empty()) continue;
  auto r=Split(line);
  if(std::stoi(g(r,"Configuration_ID"))!=CONFIGURATION_ID)continue;
  values[{g(r,"Seed"),g(r,"Route_Name")}]=Num(g(r,"Scaled_Bottleneck_Photons_Per_Slot"),"ns");
 }
 return values;
}

std::map<Key,Route> Read_Routes(const std::string& file,const std::map<Key,double>& ns)
{
 std::ifstream in(file);if(!in)throw std::runtime_error("Could not open Phase 12F scores: "+file);
 std::string line;std::getline(in,line);auto h=Split(line);std::map<std::string,std::size_t> c;
 for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;
 auto g=[&](const std::vector<std::string>& r,const std::string& n)->const std::string&{return r.at(Col(c,n));};
 std::map<Key,Route> routes;
 while(std::getline(in,line))
 {
  if(line.empty()) continue;
  auto row=Split(line);
  if(g(row,"Policy")!="B3_Physical_Success")continue;
  Route r;r.Trial=g(row,"Trial");r.Seed=g(row,"Seed");r.Name=g(row,"Route_Name");
  r.Available=std::stoi(g(row,"Available"))!=0;r.Hops=std::stoi(g(row,"Hop_Count"));
  r.Contact=Num(g(row,"Contact_Probability"),"contact");
  r.Optical=Num(g(row,"Optical_Packet_Success_Lower_Bound"),"optical");
  r.PAT=Num(g(row,"PAT_s"),"PAT");r.Propagation=Num(g(row,"Propagation_s"),"propagation");
  auto f=ns.find({r.Seed,r.Name});if(f==ns.end())throw std::runtime_error("Missing ns state");r.NS=f->second;
  routes[{r.Seed,r.Name}]=r;
 }
 return routes;
}

double Optical_Lower(double ns,double background,int hops)
{
 if(ns<=0)return 0.0;
 const double exponent=-std::pow(std::sqrt(ns+background)-std::sqrt(background),2.0);
 double log_ser=std::min(0.0,std::log(PPM_ORDER-1.0)+exponent);
 const std::size_t symbols=static_cast<std::size_t>(std::ceil(
     static_cast<double>(PACKET_BITS)/std::log2(static_cast<double>(PPM_ORDER))));
 const double log_failure=std::min(0.0,log_ser+std::log(static_cast<double>(symbols))+
     std::log(static_cast<double>(hops)));
 return std::clamp(1.0-std::exp(log_failure),0.0,1.0);
}

std::array<Route,4> Apply(const std::array<Route,4>& base,const Scenario& s)
{
 auto routes=base;
 if(s.Obstruct_Direct){routes[0].Available=false;routes[0].Optical=0.0;}
 else routes[0].Optical=Optical_Lower(routes[0].NS*s.Direct_Signal_Factor,
                                     s.Direct_Background,routes[0].Hops);
 return routes;
}

double Minimum(const std::array<Route,4>& r,bool pat)
{
 double x=std::numeric_limits<double>::infinity();
 for(const auto& a:r)
  if(a.Available) x=std::min(x,pat?a.PAT:a.Propagation);
 return x;
}

Decision Select(const std::array<Route,4>& r,std::size_t policy)
{
 Decision d;d.Scores.fill(-1);double best=-1;
 const double min_pat=Minimum(r,true),min_prop=Minimum(r,false);
 for(std::size_t i=0;i<4;++i)
 {
  if(!r[i].Available)continue;
  const double physical=r[i].Contact*r[i].Optical;
  double score=policy==0?min_prop/r[i].Propagation:
               policy==1?r[i].Contact:
               policy==2?physical:
               physical*(min_pat/r[i].PAT)*(min_prop/r[i].Propagation);
  d.Scores[i]=score;if(score>best+1e-15){best=score;d.Selected=static_cast<int>(i);}
 }
 return d;
}

bool Tests()
{
 bool ok=true;auto check=[&](const char* n,bool v){std::cout<<(v?"PASS: ":"FAIL: ")<<n<<'\n';ok&=v;};
 check("more pointing loss reduces optical success",Optical_Lower(40,.0,1)>Optical_Lower(10,.0,1));
 check("more background reduces optical success",Optical_Lower(35,4,1)>Optical_Lower(35,16,1));
 std::array<Route,4> r{};for(std::size_t i=0;i<4;++i)r[i]={"1","1",ROUTE_NAMES[i],true,i?2:1,i?.8:.9,.999,35,i?120.:60.,i?110.:100.};
 check("control shortest path chooses direct",Select(r,0).Selected==0);
 auto blocked=r;blocked[0].Available=false;
 check("obstruction forces relay selection",Select(blocked,0).Selected>0);
 r[0].Optical=.1;
 check("physical policy responds to optical stress",Select(r,2).Selected>0);
 return ok;
}
} // namespace

int main(int argc,char* argv[])
{
 if(!Tests())return 1;
 const std::string ns_file=argc>1?argv[1]:"Monte_Carlo_Experiments/Phase12D_Results/Phase12D_Minimum_Feasible_Design_Detail.csv";
 const std::string route_file=argc>2?argv[2]:"Monte_Carlo_Experiments/Phase12F_Results/Phase12F_Physical_Routing_Scores.csv";
 const std::string output_dir=argc>3?argv[3]:"Monte_Carlo_Experiments/Phase12G_Results";
 try
 {
  auto ns=Read_NS(ns_file);auto route_map=Read_Routes(route_file,ns);
  if(ns.size()!=EXPECTED_ROUTES||route_map.size()!=EXPECTED_ROUTES)throw std::runtime_error("Expected 400 matched route states.");
  std::map<std::string,std::array<Route,4>> worlds;
  for(const auto& x:route_map)
  {
   auto f=std::find(ROUTE_NAMES.begin(),ROUTE_NAMES.end(),x.second.Name);
   worlds[x.second.Seed][static_cast<std::size_t>(f-ROUTE_NAMES.begin())]=x.second;
  }
  std::filesystem::create_directories(output_dir);
  const std::string score_name=output_dir+"/Phase12G_Stress_Route_Scores.csv";
  const std::string decision_name=output_dir+"/Phase12G_Stress_Policy_Decisions.csv";
  const std::string summary_name=output_dir+"/Phase12G_Stress_Summary.csv";
  std::ofstream scores(score_name),decisions(decision_name),summary_file(summary_name);
  if(!scores||!decisions||!summary_file)throw std::runtime_error("Could not create outputs.");
  scores<<std::setprecision(17);
  decisions<<std::setprecision(17);
  summary_file<<std::setprecision(17);
  scores<<"Trial,Seed,Scenario,Policy,Route_Name,Available,Contact_Probability,"
          "Stressed_Optical_Success_Lower,Physical_Success_Lower,PAT_s,"
          "Propagation_s,Policy_Score,Selected\n";
  decisions<<"Trial,Seed,Scenario,Policy,Selected_Route,Selected_Score,"
             "Selected_Physical_Success_Lower,Switched_From_Direct\n";
  std::map<std::pair<std::string,std::string>,Summary> summaries;
  for(const auto& world:worlds)
  for(const Scenario& scenario:SCENARIOS)
  {
   const auto stressed=Apply(world.second,scenario);
   for(std::size_t p=0;p<4;++p)
   {
    const Decision d=Select(stressed,p);auto& sm=summaries[{scenario.Name,POLICY_NAMES[p]}];++sm.Worlds;
    if(d.Selected<0){++sm.NoRoute;continue;}
    const auto& selected=stressed[static_cast<std::size_t>(d.Selected)];
    if(d.Selected==0)++sm.Direct;else if(d.Selected==1)++sm.Relay0;else if(d.Selected==2)++sm.Relay1;else ++sm.Relay2;
    sm.Sum_Selected_Physical+=selected.Contact*selected.Optical;
    for(std::size_t i=0;i<4;++i)
    {
     const auto& r=stressed[i];scores<<r.Trial<<','<<r.Seed<<','<<scenario.Name<<','
      <<POLICY_NAMES[p]<<','<<r.Name<<','<<(r.Available?1:0)<<','<<r.Contact<<','
      <<r.Optical<<','<<r.Contact*r.Optical<<','<<r.PAT<<','<<r.Propagation<<','
      <<d.Scores[i]<<','<<(d.Selected==static_cast<int>(i)?1:0)<<'\n';
    }
    decisions<<selected.Trial<<','<<selected.Seed<<','<<scenario.Name<<','
     <<POLICY_NAMES[p]<<','<<selected.Name<<','<<d.Scores[d.Selected]<<','
     <<selected.Contact*selected.Optical<<','<<(d.Selected==0?0:1)<<'\n';
   }
  }
  scores.close();decisions.close();
  summary_file<<"Scenario,Policy,Worlds,Direct,Relay0,Relay1,Relay2,No_Route,"
                "Switch_Rate_From_Direct,Mean_Selected_Physical_Success_Lower\n";
  for(const auto& x:summaries)
  {
   const auto& s=x.second;const std::size_t switched=s.Relay0+s.Relay1+s.Relay2;
   summary_file<<x.first.first<<','<<x.first.second<<','<<s.Worlds<<','<<s.Direct
    <<','<<s.Relay0<<','<<s.Relay1<<','<<s.Relay2<<','<<s.NoRoute<<','
    <<static_cast<double>(switched)/s.Worlds<<','<<s.Sum_Selected_Physical/s.Worlds<<'\n';
  }
  summary_file.close();
  std::cout<<"Phase 12G complete.\nMatched worlds: "<<worlds.size()
   <<"\nStress scenarios: "<<SCENARIOS.size()
   <<"\nRoute-score rows: "<<worlds.size()*SCENARIOS.size()*4*4
   <<"\nDecision rows: "<<worlds.size()*SCENARIOS.size()*4
   <<"\nSummary rows: "<<summaries.size()
   <<"\nScores: "<<score_name<<"\nDecisions: "<<decision_name
   <<"\nSummary: "<<summary_name<<'\n';
 }
 catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';return 1;}
 return 0;
}
