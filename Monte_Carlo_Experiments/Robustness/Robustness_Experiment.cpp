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
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// RQ4 robustness experiment engine.
// The optical equations, policy equations, event order, and random-number
// construction are copied from the frozen Phase 13 production implementation.
// Only the declared robustness factor is changed in each family.

namespace {
constexpr int PPM_ORDER=16;
constexpr std::size_t PACKET_BITS=1024;
constexpr double DATA_RATE_BPS=1.e8;
const std::array<std::string,4> ROUTES{"Direct_Earth_to_Mars","Earth_Relay0_Mars","Earth_Relay1_Mars","Earth_Relay2_Mars"};
const std::array<std::string,4> POLICIES{"B1_Shortest_Delay","B2_Contact_Aware","B3_Physical_Success","B4_Physical_Multiobjective"};

struct Route {std::string Trial,Seed,Name;bool Available{};int Hops{};double Contact{},PAT{},Propagation{},Bottleneck_NS{},Optical{};};
struct World {std::string Trial,Seed;std::array<Route,4> Routes;};
struct Packet {std::string Trial,Seed;std::size_t Index{};double Release{},Contact_Uniform{},TTL{},Deadline{};};
struct Cell {std::string Name,Base_Scenario,Factor_Name;double Factor_Value{},Signal_Factor{},Background{},PAT_Per_Hop{},TTL{};bool Disable_Relay2{};};
struct Decision {std::array<double,4> Scores{};int Selected{-1};};
struct Outcome {double Optical_Uniform{},Latency{},Arrival{};bool Delivered{},Expired{},Dropped{},Contact_Failure{},Optical_Failure{};};
struct Summary {std::string Trial,Seed,Cell,Base,Factor,Policy,Route;double Factor_Value{};std::size_t Generated{},Delivered{},Expired{},Dropped{},Contact_Failures{},Optical_Failures{};std::uint64_t Bits{};double Latency_Sum{};};

std::vector<std::string> Split(const std::string& line){std::vector<std::string> f;std::string v;bool q=false;for(std::size_t i=0;i<line.size();++i){char c=line[i];if(c=='"'){if(q&&i+1<line.size()&&line[i+1]=='"'){v+='"';++i;}else q=!q;}else if(c==','&&!q){f.push_back(v);v.clear();}else v+=c;}f.push_back(v);return f;}
std::size_t Col(const std::map<std::string,std::size_t>& c,const std::string& n){auto i=c.find(n);if(i==c.end())throw std::runtime_error("Missing column: "+n);return i->second;}
double Num(const std::string& v,const std::string& n){char* e=nullptr;double x=std::strtod(v.c_str(),&e);if(e==v.c_str()||(*e!='\0'&&*e!='\r'))throw std::runtime_error("Invalid "+n+": "+v);return x;}

double Optical_Lower(double ns,double background,int hops){if(ns<=0.||hops<=0)return 0.;double exponent=-std::pow(std::sqrt(ns+background)-std::sqrt(background),2.);double log_ser=std::min(0.,std::log(PPM_ORDER-1.)+exponent);std::size_t symbols=static_cast<std::size_t>(std::ceil(static_cast<double>(PACKET_BITS)/std::log2(static_cast<double>(PPM_ORDER))));double log_failure=std::min(0.,log_ser+std::log(static_cast<double>(symbols))+std::log(static_cast<double>(hops)));return std::clamp(1.-std::exp(log_failure),0.,1.);}

std::vector<World> Read_Worlds(const std::string& file,std::size_t limit){std::ifstream in(file);if(!in)throw std::runtime_error("Could not open world/route CSV: "+file);std::string line;if(!std::getline(in,line))throw std::runtime_error("Empty world/route CSV");auto h=Split(line);std::map<std::string,std::size_t> c;for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;auto g=[&](const std::vector<std::string>& r,const std::string& n)->const std::string&{return r.at(Col(c,n));};std::vector<World> out;
 // Preferred input is the compact Phase 13 route-score file. It contains the
 // exact frozen bottleneck photon values and avoids regenerating orbital state.
 if(c.count("Route_Name")&&c.count("Bottleneck_Photons_Per_Slot")){
  std::map<std::string,World> worlds;std::vector<std::string> order;
  while(std::getline(in,line)){if(line.empty())continue;auto row=Split(line);if(g(row,"Scenario")!="Control"||g(row,"Policy")!="B1_Shortest_Delay")continue;const auto seed=g(row,"Seed");if(!worlds.count(seed)){if(order.size()>=limit)continue;World w;w.Trial=g(row,"Trial");w.Seed=seed;worlds[seed]=w;order.push_back(seed);}auto& w=worlds.at(seed);auto it=std::find(ROUTES.begin(),ROUTES.end(),g(row,"Route_Name"));if(it==ROUTES.end())throw std::runtime_error("Unknown route name");std::size_t i=static_cast<std::size_t>(it-ROUTES.begin());w.Routes[i]={w.Trial,w.Seed,ROUTES[i],std::stoi(g(row,"Available"))!=0,std::stoi(g(row,"Hop_Count")),Num(g(row,"Contact_Probability"),"contact"),Num(g(row,"PAT_s"),"PAT"),Num(g(row,"Propagation_s"),"propagation"),Num(g(row,"Bottleneck_Photons_Per_Slot"),"photons"),0.};}
  for(const auto& seed:order)out.push_back(worlds.at(seed));
  for(const auto& w:out)for(const auto& r:w.Routes)if(r.Name.empty())throw std::runtime_error("Incomplete route-score world");
  return out;
 }
 throw std::runtime_error("Use Phase13_RQ4_Production_Route_Scores.csv as the world input");}

std::vector<Packet> Read_Packets(const std::string& file,const std::set<std::string>& seeds){std::ifstream in(file);if(!in)throw std::runtime_error("Could not open packet template/audit CSV: "+file);std::string line;if(!std::getline(in,line))throw std::runtime_error("Empty packet CSV");auto h=Split(line);std::map<std::string,std::size_t> c;for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;auto g=[&](const std::vector<std::string>& r,const std::string& n)->const std::string&{return r.at(Col(c,n));};bool audit=c.count("Release_s")&&c.count("Scenario");std::string release=audit?"Release_s":"Release_Time_s";std::vector<Packet> out;while(std::getline(in,line)){if(line.empty())continue;auto r=Split(line);if(!seeds.count(g(r,"Seed")))continue;if(audit){if(g(r,"Scenario")!="Control"||g(r,"Policy")!="B1_Shortest_Delay")continue;}else if(g(r,"Policy")!="B1")continue;out.push_back({g(r,"Trial"),g(r,"Seed"),std::stoull(g(r,"Packet_Index")),Num(g(r,release),"release"),Num(g(r,"Shared_Contact_Uniform"),"contact uniform"),Num(g(r,"TTL_s"),"TTL"),Num(g(r,"Deadline_s"),"deadline")});}return out;}

double Minimum(const std::array<Route,4>& r,bool pat){double x=std::numeric_limits<double>::infinity();for(const auto& a:r)if(a.Available)x=std::min(x,pat?a.PAT:a.Propagation);return x;}
Decision Select(const std::array<Route,4>& r,std::size_t p){Decision d;d.Scores.fill(-1.);double best=-1.,min_pat=Minimum(r,true),min_prop=Minimum(r,false);for(std::size_t i=0;i<4;++i){if(!r[i].Available)continue;double physical=r[i].Contact*r[i].Optical;double score=p==0?min_prop/r[i].Propagation:p==1?r[i].Contact:p==2?physical:physical*(min_pat/r[i].PAT)*(min_prop/r[i].Propagation);d.Scores[i]=score;if(score>best+1e-15){best=score;d.Selected=static_cast<int>(i);}}return d;}
std::uint64_t Mix(std::uint64_t x){x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);}
double Optical_Uniform(const std::string& seed,std::size_t index,const std::string& scenario){std::uint64_t tag=1469598103934665603ULL;for(unsigned char c:scenario){tag^=c;tag*=1099511628211ULL;}std::uint64_t x=Mix(std::stoull(seed)^tag^((index+1)*0xd6e8feb86659fd93ULL));return static_cast<double>(x>>11)*(1./9007199254740992.);}
Outcome Evaluate(const Packet& p,const Route& r,const std::string& stream,double ttl){Outcome o;o.Optical_Uniform=Optical_Uniform(p.Seed,p.Index,stream);o.Latency=(static_cast<double>(PACKET_BITS)/DATA_RATE_BPS)*r.Hops+r.PAT+r.Propagation;o.Arrival=p.Release+o.Latency;double deadline=p.Release+ttl;if(p.Contact_Uniform>r.Contact)o.Contact_Failure=true;else if(o.Arrival>deadline)o.Expired=true;else if(o.Optical_Uniform>r.Optical)o.Optical_Failure=true;else o.Delivered=true;o.Dropped=!o.Delivered&&!o.Expired;return o;}

std::vector<Cell> Cells(const std::string& family,bool include_null){
 if(family=="pat"){std::vector<Cell> x;for(double pat:{30.,60.,120.}){if(include_null){x.push_back({"PAT"+std::to_string((int)pat)+"__Control","Control","PAT_s_Per_Hop",pat,1.,4.,pat,1500.,false});x.push_back({"PAT"+std::to_string((int)pat)+"__Direct_Background_16","Direct_Background_16","PAT_s_Per_Hop",pat,1.,16.,pat,1500.,false});}x.push_back({"PAT"+std::to_string((int)pat)+"__Direct_Pointing_25pct_Power","Direct_Pointing_25pct_Power","PAT_s_Per_Hop",pat,.25,4.,pat,1500.,false});x.push_back({"PAT"+std::to_string((int)pat)+"__Direct_Combined_25pctPower_BG16","Direct_Combined_25pctPower_BG16","PAT_s_Per_Hop",pat,.25,16.,pat,1500.,false});}return x;}
 if(family=="no-relay2")return {{"NoRelay2__Direct_Pointing_25pct_Power","Direct_Pointing_25pct_Power","Relay2_Enabled",0.,.25,4.,60.,1500.,true},{"NoRelay2__Direct_Combined_25pctPower_BG16","Direct_Combined_25pctPower_BG16","Relay2_Enabled",0.,.25,16.,60.,1500.,true}};
 if(family=="pointing"){std::vector<Cell> x;for(double f:{1.,.75,.50,.25})x.push_back({"PointingFactor"+std::to_string(f),"Direct_Pointing_25pct_Power","Direct_Signal_Factor",f,f,4.,60.,1500.,false});return x;}
 if(family=="ttl"){std::vector<Cell> x;for(double ttl:{750.,1500.,3000.}){x.push_back({"TTL"+std::to_string((int)ttl)+"__Direct_Pointing_25pct_Power","Direct_Pointing_25pct_Power","TTL_s",ttl,.25,4.,60.,ttl,false});x.push_back({"TTL"+std::to_string((int)ttl)+"__Direct_Combined_25pctPower_BG16","Direct_Combined_25pctPower_BG16","TTL_s",ttl,.25,16.,60.,ttl,false});}return x;}
 throw std::runtime_error("Family must be pat, no-relay2, pointing, or ttl");}

bool Tests(){bool ok=true;auto check=[&](const char* n,bool v){std::cout<<(v?"PASS: ":"FAIL: ")<<n<<'\n';ok&=v;};check("optical monotonic signal",Optical_Lower(40,4,1)>Optical_Lower(10,4,1));check("optical monotonic background",Optical_Lower(35,4,1)>Optical_Lower(35,16,1));check("optical CRN reproducible",Optical_Uniform("1000000",0,"Control")==Optical_Uniform("1000000",0,"Control"));return ok;}
using State_Key=std::tuple<std::string,std::string,std::string>;
using Summary_Key=std::tuple<std::string,std::string,std::string>;
}

int main(int argc,char** argv){if(!Tests())return 1;if(argc<5){std::cerr<<"Usage: Robustness_Experiment FAMILY TRIAL_CSV PACKET_CSV OUTPUT_DIR [WORLD_LIMIT] [--include-null]\n";return 2;}try{
 std::string family=argv[1],trial_file=argv[2],packet_file=argv[3],outdir=argv[4];std::size_t limit=argc>5?std::stoull(argv[5]):1000;bool include_null=argc>6&&std::string(argv[6])=="--include-null";if(limit<1||limit>1000)throw std::runtime_error("WORLD_LIMIT must be 1..1000");auto worlds=Read_Worlds(trial_file,limit);if(worlds.size()!=limit)throw std::runtime_error("World count mismatch");std::set<std::string> seeds;for(const auto& w:worlds)seeds.insert(w.Seed);auto packets=Read_Packets(packet_file,seeds);if(packets.size()!=limit*200)throw std::runtime_error("Expected 200 packet templates per world");auto cells=Cells(family,include_null);std::filesystem::create_directories(outdir);
 std::string prefix=family=="pat"?"PAT":family=="no-relay2"?"No_Relay2":family=="pointing"?"Pointing":"TTL";std::ofstream route(outdir+"/"+prefix+"_Route_Scores.csv"),summary(outdir+"/"+prefix+"_Trial_Summary.csv"),manifest(outdir+"/"+prefix+"_Manifest.txt");if(!route||!summary||!manifest)throw std::runtime_error("Could not create outputs");route<<std::setprecision(17);summary<<std::setprecision(17);
 route<<"Condition,Base_Scenario,Factor_Name,Factor_Value,Trial,Seed,Policy,Route_Name,Available,Hop_Count,Bottleneck_Photons_Per_Slot,Contact_Probability,Optical_Success_Lower,Physical_Success_Lower,PAT_s,Propagation_s,Policy_Score,Selected\n";
 std::map<State_Key,Route> states;
 for(const auto& w:worlds)for(const auto& cell:cells){auto routes=w.Routes;for(std::size_t i=0;i<4;++i){routes[i].PAT=cell.PAT_Per_Hop*routes[i].Hops;if(i==3&&cell.Disable_Relay2)routes[i].Available=false;double signal=i==0?cell.Signal_Factor:1.;double bg=i==0?cell.Background:4.;routes[i].Optical=Optical_Lower(routes[i].Bottleneck_NS*signal,bg,routes[i].Hops);}for(std::size_t p=0;p<4;++p){auto d=Select(routes,p);if(d.Selected<0)throw std::runtime_error("No selected route");states[{w.Seed,cell.Name,POLICIES[p]}]=routes[static_cast<std::size_t>(d.Selected)];for(std::size_t i=0;i<4;++i){const auto& r=routes[i];route<<cell.Name<<','<<cell.Base_Scenario<<','<<cell.Factor_Name<<','<<cell.Factor_Value<<','<<r.Trial<<','<<r.Seed<<','<<POLICIES[p]<<','<<r.Name<<','<<r.Available<<','<<r.Hops<<','<<r.Bottleneck_NS<<','<<r.Contact<<','<<r.Optical<<','<<r.Contact*r.Optical<<','<<r.PAT<<','<<r.Propagation<<','<<d.Scores[i]<<','<<(d.Selected==(int)i)<<'\n';}}}
 summary<<"Condition,Base_Scenario,Factor_Name,Factor_Value,Trial,Seed,Policy,Selected_Route,Generated,Delivered,Expired,Dropped,Contact_Failures,Optical_Failures,Reliability,Delivered_Bits,Mean_Delivered_Latency_s\n";std::map<Summary_Key,Summary> sums;bool accounting=true;for(const auto& p:packets)for(const auto& cell:cells)for(const auto& policy:POLICIES){const auto& r=states.at({p.Seed,cell.Name,policy});auto o=Evaluate(p,r,cell.Base_Scenario,cell.TTL);accounting&=((int)o.Delivered+(int)o.Expired+(int)o.Dropped)==1;auto& s=sums[{p.Seed,cell.Name,policy}];s.Trial=p.Trial;s.Seed=p.Seed;s.Cell=cell.Name;s.Base=cell.Base_Scenario;s.Factor=cell.Factor_Name;s.Factor_Value=cell.Factor_Value;s.Policy=policy;s.Route=r.Name;++s.Generated;s.Delivered+=o.Delivered;s.Expired+=o.Expired;s.Dropped+=o.Dropped;s.Contact_Failures+=o.Contact_Failure;s.Optical_Failures+=o.Optical_Failure;if(o.Delivered){s.Bits+=PACKET_BITS;s.Latency_Sum+=o.Latency;}}
 if(!accounting)throw std::runtime_error("Packet accounting failed");
 for(const auto& kv:sums){const auto& s=kv.second;if(s.Generated!=200||s.Delivered+s.Expired+s.Dropped!=s.Generated||s.Contact_Failures+s.Optical_Failures!=s.Dropped)throw std::runtime_error("Summary accounting failed");summary<<s.Cell<<','<<s.Base<<','<<s.Factor<<','<<s.Factor_Value<<','<<s.Trial<<','<<s.Seed<<','<<s.Policy<<','<<s.Route<<','<<s.Generated<<','<<s.Delivered<<','<<s.Expired<<','<<s.Dropped<<','<<s.Contact_Failures<<','<<s.Optical_Failures<<','<<(double)s.Delivered/s.Generated<<','<<s.Bits<<','<<(s.Delivered?s.Latency_Sum/s.Delivered:0.)<<'\n';}
 manifest<<"RQ4 robustness family: "<<family<<"\nWorlds: "<<limit<<"\nPackets per world: 200\nPolicies: B1, B2, B3, B4\nCells: "<<cells.size()<<"\nFrozen mathematics: Phase 13 optical, routing, event-order, and CRN equations\nPacket audit output: omitted by design; regenerable from frozen inputs\n";for(const auto& c:cells)manifest<<c.Name<<" | "<<c.Factor_Name<<'='<<c.Factor_Value<<" | base_stream="<<c.Base_Scenario<<'\n';
 std::cout<<"PASS: robustness family "<<family<<"\nPASS: "<<worlds.size()<<" matched worlds\nPASS: outcome accounting\nTrial summary: "<<outdir<<"/"<<prefix<<"_Trial_Summary.csv\nRoute scores: "<<outdir<<"/"<<prefix<<"_Route_Scores.csv\n";
 }catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';return 1;}return 0;}
