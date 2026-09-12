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
#include <numeric>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// Monte Carlo Experiment 4 - Phase 13B
// Matched trial-level statistical analysis for RQ4.
//
// Experimental unit: one Monte Carlo world/seed.
// Reference policy: B1 shortest delay.
// Primary adaptive policy: B4 physical multiobjective.
// B2 and B3 are retained as secondary policy ablations.
// Packet observations are not treated as independent replicates.

namespace
{
constexpr std::size_t EXPECTED_ROWS=16000;
constexpr std::size_t EXPECTED_SCORE_ROWS=64000;
constexpr std::size_t EXPECTED_WORLDS=1000;
constexpr double ALPHA=0.05;

const std::array<std::string,4> SCENARIOS{
 "Control","Direct_Background_16","Direct_Pointing_25pct_Power",
 "Direct_Combined_25pctPower_BG16"};
const std::array<std::string,4> POLICIES{
 "B1_Shortest_Delay","B2_Contact_Aware","B3_Physical_Success",
 "B4_Physical_Multiobjective"};
const std::array<std::string,4> ROUTES{
 "Direct_Earth_to_Mars","Earth_Relay0_Mars","Earth_Relay1_Mars","Earth_Relay2_Mars"};

struct Record
{
 std::string Trial,Seed,Scenario,Policy,Route;
 double Generated{},Delivered{},Expired{},Dropped{},Contact_Failures{},Optical_Failures{};
 double Reliability{},Delivered_Bits{},Latency{};
};

struct Selected_Score
{
 std::string Route;
 double Physical{},Score{};
};

struct Descriptive
{
 std::size_t N{};double Mean{},SD{},Minimum{},Median{},Maximum{};
};

struct Paired_Result
{
 std::size_t N{},Wins{},Losses{},Ties{};
 double Baseline_Mean{},Comparator_Mean{},Mean_Difference{},Median_Difference{},SD_Difference{};
 double CI_Lower{},CI_Upper{},Percent_Change{},Cohens_Dz{},T{},P_T{},P_Sign{};
};

std::vector<std::string> Split(const std::string& line)
{
 std::vector<std::string> fields;std::string value;bool quoted=false;
 for(std::size_t i=0;i<line.size();++i){const char c=line[i];if(c=='"'){
  if(quoted&&i+1<line.size()&&line[i+1]=='"'){value+='"';++i;}else quoted=!quoted;}
  else if(c==','&&!quoted){fields.push_back(value);value.clear();}else value+=c;}
 fields.push_back(value);return fields;
}

std::size_t Col(const std::map<std::string,std::size_t>& c,const std::string& name)
{const auto f=c.find(name);if(f==c.end())throw std::runtime_error("Missing column: "+name);return f->second;}

double Number(const std::string& value,const std::string& name)
{char* end=nullptr;const double x=std::strtod(value.c_str(),&end);
 if(end==value.c_str()||(*end!='\0'&&*end!='\r'))throw std::runtime_error("Invalid "+name+": "+value);return x;}

using Key=std::tuple<std::string,std::string,std::string>;

std::map<Key,Record> Read_Summaries(const std::string& file,std::size_t& rows)
{
 std::ifstream in(file);if(!in)throw std::runtime_error("Could not open Phase 13 trial summary: "+file);
 std::string line;if(!std::getline(in,line))throw std::runtime_error("Trial summary is empty.");
 const auto header=Split(line);std::map<std::string,std::size_t> c;for(std::size_t i=0;i<header.size();++i)c[header[i]]=i;
 auto g=[&](const std::vector<std::string>& r,const std::string& n)->const std::string&{return r.at(Col(c,n));};
 std::map<Key,Record> data;rows=0;
 while(std::getline(in,line)){if(line.empty())continue;++rows;const auto row=Split(line);Record r;
  r.Trial=g(row,"Trial");r.Seed=g(row,"Seed");r.Scenario=g(row,"Scenario");r.Policy=g(row,"Policy");r.Route=g(row,"Selected_Route");
  r.Generated=Number(g(row,"Generated"),"Generated");r.Delivered=Number(g(row,"Delivered"),"Delivered");
  r.Expired=Number(g(row,"Expired"),"Expired");r.Dropped=Number(g(row,"Dropped"),"Dropped");
  r.Contact_Failures=Number(g(row,"Contact_Failures"),"Contact_Failures");r.Optical_Failures=Number(g(row,"Optical_Failures"),"Optical_Failures");
  r.Reliability=Number(g(row,"Reliability"),"Reliability");r.Delivered_Bits=Number(g(row,"Delivered_Bits"),"Delivered_Bits");
  r.Latency=Number(g(row,"Mean_Delivered_Latency_s"),"Mean_Delivered_Latency_s");
  const auto key=Key{r.Seed,r.Scenario,r.Policy};if(!data.emplace(key,r).second)throw std::runtime_error("Duplicate summary key.");}
 return data;
}

std::map<Key,Selected_Score> Read_Selected_Scores(const std::string& file,std::size_t& rows)
{
 std::ifstream in(file);if(!in)throw std::runtime_error("Could not open Phase 13 route scores: "+file);
 std::string line;if(!std::getline(in,line))throw std::runtime_error("Route score file is empty.");
 const auto header=Split(line);std::map<std::string,std::size_t> c;for(std::size_t i=0;i<header.size();++i)c[header[i]]=i;
 auto g=[&](const std::vector<std::string>& r,const std::string& n)->const std::string&{return r.at(Col(c,n));};
 std::map<Key,Selected_Score> selected;rows=0;
 while(std::getline(in,line)){if(line.empty())continue;++rows;const auto row=Split(line);if(std::stoi(g(row,"Selected"))!=1)continue;
  const Key key{g(row,"Seed"),g(row,"Scenario"),g(row,"Policy")};Selected_Score s{g(row,"Route_Name"),
   Number(g(row,"Physical_Success_Lower"),"Physical_Success_Lower"),Number(g(row,"Policy_Score"),"Policy_Score")};
  if(!selected.emplace(key,s).second)throw std::runtime_error("Multiple selected routes for one key.");}
 return selected;
}

double Median(std::vector<double> x)
{if(x.empty())return 0.0;std::sort(x.begin(),x.end());const std::size_t n=x.size();return n%2?x[n/2]:(x[n/2-1]+x[n/2])/2.0;}

Descriptive Describe(const std::vector<double>& x)
{
 if(x.empty())throw std::runtime_error("Cannot describe empty vector.");Descriptive d;d.N=x.size();
 d.Mean=std::accumulate(x.begin(),x.end(),0.0)/x.size();double ss=0.0;for(double v:x)ss+=(v-d.Mean)*(v-d.Mean);
 d.SD=x.size()>1?std::sqrt(ss/(x.size()-1)):0.0;d.Minimum=*std::min_element(x.begin(),x.end());d.Maximum=*std::max_element(x.begin(),x.end());d.Median=Median(x);return d;
}

double Beta_CF(double a,double b,double x)
{
 constexpr int MAXIT=200;constexpr double EPS=3.0e-14;constexpr double FPMIN=1.0e-300;
 const double qab=a+b,qap=a+1.0,qam=a-1.0;double c=1.0,d=1.0-qab*x/qap;if(std::abs(d)<FPMIN)d=FPMIN;
 d=1.0/d;double h=d;
 for(int m=1;m<=MAXIT;++m){const int m2=2*m;double aa=m*(b-m)*x/((qam+m2)*(a+m2));d=1.0+aa*d;if(std::abs(d)<FPMIN)d=FPMIN;
  c=1.0+aa/c;if(std::abs(c)<FPMIN)c=FPMIN;d=1.0/d;h*=d*c;aa=-(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
  d=1.0+aa*d;if(std::abs(d)<FPMIN)d=FPMIN;c=1.0+aa/c;if(std::abs(c)<FPMIN)c=FPMIN;d=1.0/d;const double del=d*c;h*=del;if(std::abs(del-1.0)<EPS)break;}
 return h;
}

double Regularized_Beta(double x,double a,double b)
{
 if(x<=0.0)return 0.0;if(x>=1.0)return 1.0;const double bt=std::exp(std::lgamma(a+b)-std::lgamma(a)-std::lgamma(b)+a*std::log(x)+b*std::log1p(-x));
 return x<(a+1.0)/(a+b+2.0)?bt*Beta_CF(a,b,x)/a:1.0-bt*Beta_CF(b,a,1.0-x)/b;
}

double Two_Sided_T_P(double t,double df)
{if(!std::isfinite(t))return 0.0;if(t==0.0)return 1.0;const double x=df/(df+t*t);return std::clamp(Regularized_Beta(x,df/2.0,0.5),0.0,1.0);}

double Sign_Test_P(std::size_t wins,std::size_t losses)
{
 const std::size_t n=wins+losses;if(n==0)return 1.0;const std::size_t k=std::min(wins,losses);
 long double term=std::exp(-static_cast<long double>(n)*std::log(2.0L)),sum=term;
 for(std::size_t i=0;i<k;++i){term*=static_cast<long double>(n-i)/static_cast<long double>(i+1);sum+=term;}
 return static_cast<double>(std::min(1.0L,2.0L*sum));
}

Paired_Result Compare(const std::vector<double>& baseline,const std::vector<double>& comparator,bool higher_is_better)
{
 if(baseline.size()!=comparator.size()||baseline.size()<2)throw std::runtime_error("Invalid paired sample.");
 Paired_Result r;r.N=baseline.size();std::vector<double> diff;diff.reserve(r.N);
 for(std::size_t i=0;i<r.N;++i){const double d=comparator[i]-baseline[i];diff.push_back(d);const double preferred=higher_is_better?d:-d;
  if(preferred>1e-12)++r.Wins;else if(preferred<-1e-12)++r.Losses;else ++r.Ties;}
 const auto b=Describe(baseline),c=Describe(comparator),d=Describe(diff);r.Baseline_Mean=b.Mean;r.Comparator_Mean=c.Mean;r.Mean_Difference=d.Mean;
 r.Median_Difference=d.Median;r.SD_Difference=d.SD;const double se=d.SD/std::sqrt(static_cast<double>(r.N));
 r.T=se>0.0?d.Mean/se:0.0;r.P_T=se>0.0?Two_Sided_T_P(r.T,static_cast<double>(r.N-1)):(std::abs(d.Mean)>0.0?0.0:1.0);
 r.CI_Lower=d.Mean-1.96234*se;r.CI_Upper=d.Mean+1.96234*se;r.Percent_Change=std::abs(b.Mean)>1e-15?100.0*d.Mean/b.Mean:0.0;
 r.Cohens_Dz=d.SD>0.0?d.Mean/d.SD:0.0;r.P_Sign=Sign_Test_P(r.Wins,r.Losses);return r;
}

double Metric(const Record& r,const std::string& name)
{
 if(name=="Reliability")return r.Reliability;if(name=="Delivered_Packets")return r.Delivered;
 if(name=="Delivered_Bits")return r.Delivered_Bits;if(name=="Mean_Delivered_Latency_s")return r.Latency;
 if(name=="Contact_Failures")return r.Contact_Failures;if(name=="Optical_Failures")return r.Optical_Failures;
 throw std::runtime_error("Unknown metric: "+name);
}

bool Higher_Is_Better(const std::string& metric)
{return metric=="Reliability"||metric=="Delivered_Packets"||metric=="Delivered_Bits";}

bool Tests()
{
 bool ok=true;auto check=[&](const char* n,bool v){std::cout<<(v?"PASS: ":"FAIL: ")<<n<<'\n';ok&=v;};
 check("median odd",Median({3,1,2})==2.0);check("median even",Median({4,1,3,2})==2.5);
 const auto x=Compare({1,2,3,4},{2,3,4,5},true);check("paired difference",std::abs(x.Mean_Difference-1.0)<1e-12);
 check("sign test bounded",Sign_Test_P(10,0)>=0.0&&Sign_Test_P(10,0)<=1.0);check("t p-value bounded",Two_Sided_T_P(2.0,20)>=0.0&&Two_Sided_T_P(2.0,20)<=1.0);return ok;
}
}

int main(int argc,char* argv[])
{
 if(!Tests())return 1;
 const std::string summary_file=argc>1?argv[1]:"Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/Phase13_RQ4_Production_Trial_Summary.csv";
 const std::string scores_file=argc>2?argv[2]:"Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/Phase13_RQ4_Production_Route_Scores.csv";
 const std::string output_dir=argc>3?argv[3]:"Monte_Carlo_Experiments/Phase13B_RQ4_Statistical_Results";
 try
 {
  std::size_t summary_rows=0,score_rows=0;const auto data=Read_Summaries(summary_file,summary_rows);const auto selected=Read_Selected_Scores(scores_file,score_rows);
  if(summary_rows!=EXPECTED_ROWS||data.size()!=EXPECTED_ROWS)throw std::runtime_error("Expected 16000 unique trial summaries; read "+std::to_string(summary_rows));
  if(score_rows!=EXPECTED_SCORE_ROWS||selected.size()!=EXPECTED_ROWS)throw std::runtime_error("Expected 64000 score rows and 16000 selections.");
  std::size_t route_matches=0;for(const auto& x:data){const auto f=selected.find(x.first);if(f==selected.end()||f->second.Route!=x.second.Route)throw std::runtime_error("Route selection mismatch.");++route_matches;}
  std::filesystem::create_directories(output_dir);
  const std::string aggregate_name=output_dir+"/Phase13B_Scenario_Policy_Descriptive_Statistics.csv";
  const std::string paired_name=output_dir+"/Phase13B_Paired_Policy_Comparisons.csv";
  const std::string primary_name=output_dir+"/Phase13B_B4_vs_B1_Primary_Trial_Differences.csv";
  const std::string route_name=output_dir+"/Phase13B_Route_Selection_Summary.csv";
  std::ofstream aggregate(aggregate_name),paired(paired_name),primary(primary_name),routeout(route_name);
  if(!aggregate||!paired||!primary||!routeout)throw std::runtime_error("Could not create Phase 13B outputs.");
  aggregate<<std::setprecision(17);paired<<std::setprecision(17);primary<<std::setprecision(17);routeout<<std::setprecision(17);
  const std::array<std::string,6> metrics{"Reliability","Delivered_Packets","Delivered_Bits","Mean_Delivered_Latency_s","Contact_Failures","Optical_Failures"};
  aggregate<<"Scenario,Policy,Metric,N,Mean,SD,Minimum,Median,Maximum\n";
  paired<<"Scenario,Reference_Policy,Comparator_Policy,Comparison_Role,Metric,Higher_Is_Better,N,Reference_Mean,Comparator_Mean,Mean_Paired_Difference,Median_Paired_Difference,SD_Paired_Difference,CI95_Lower,CI95_Upper,Percent_Change_From_Reference,Cohens_Dz,Paired_T,Paired_T_P_Value,Preferred_Wins,Preferred_Losses,Ties,Exact_Sign_Test_P_Value,Significant_At_0_05,Direction\n";
  primary<<"Trial,Seed,Scenario,B1_Route,B4_Route,B1_Reliability,B4_Reliability,Reliability_Difference,B1_Delivered_Bits,B4_Delivered_Bits,Delivered_Bits_Difference,B1_Mean_Latency_s,B4_Mean_Latency_s,Latency_Difference_s,B1_Contact_Failures,B4_Contact_Failures,Contact_Failure_Difference,B1_Optical_Failures,B4_Optical_Failures,Optical_Failure_Difference\n";
  routeout<<"Scenario,Policy,Route,Selection_Count,Selection_Percentage,Mean_Selected_Physical_Success_Lower,Mean_Selected_Policy_Score\n";

  for(const auto& scenario:SCENARIOS)for(const auto& policy:POLICIES)for(const auto& metric:metrics)
  {std::vector<double> values;for(const auto& x:data)if(x.second.Scenario==scenario&&x.second.Policy==policy)
   {if(metric=="Mean_Delivered_Latency_s"&&x.second.Delivered<=0.0)continue;values.push_back(Metric(x.second,metric));}
   if(metric!="Mean_Delivered_Latency_s"&&values.size()!=EXPECTED_WORLDS)throw std::runtime_error("Incomplete descriptive cell.");
   if(values.empty())throw std::runtime_error("No valid observations for descriptive cell.");const auto d=Describe(values);
   aggregate<<scenario<<','<<policy<<','<<metric<<','<<d.N<<','<<d.Mean<<','<<d.SD<<','<<d.Minimum<<','<<d.Median<<','<<d.Maximum<<'\n';}

  for(const auto& scenario:SCENARIOS)for(std::size_t p=1;p<POLICIES.size();++p)for(const auto& metric:metrics)
  {std::vector<double> b,c;for(std::size_t seed=1000000;seed<1000000+EXPECTED_WORLDS;++seed){const std::string s=std::to_string(seed);
    const auto fb=data.find({s,scenario,POLICIES[0]}),fc=data.find({s,scenario,POLICIES[p]});if(fb==data.end()||fc==data.end())throw std::runtime_error("Missing matched policy pair.");
    if(metric=="Mean_Delivered_Latency_s"&&(fb->second.Delivered<=0.0||fc->second.Delivered<=0.0))continue;
    b.push_back(Metric(fb->second,metric));c.push_back(Metric(fc->second,metric));}
   const bool hib=Higher_Is_Better(metric);const auto r=Compare(b,c,hib);const double preferred=hib?r.Mean_Difference:-r.Mean_Difference;
   const std::string direction=std::abs(r.Mean_Difference)<1e-15?"No_Change":preferred>0?"Comparator_Better":"Comparator_Worse";
   paired<<scenario<<','<<POLICIES[0]<<','<<POLICIES[p]<<','<<(p==3?"Primary":"Secondary_Ablation")<<','<<metric<<','<<hib<<','<<r.N<<','<<r.Baseline_Mean<<','<<r.Comparator_Mean<<','<<r.Mean_Difference<<','<<r.Median_Difference<<','<<r.SD_Difference<<','<<r.CI_Lower<<','<<r.CI_Upper<<','<<r.Percent_Change<<','<<r.Cohens_Dz<<','<<r.T<<','<<r.P_T<<','<<r.Wins<<','<<r.Losses<<','<<r.Ties<<','<<r.P_Sign<<','<<(r.P_T<ALPHA)<<','<<direction<<'\n';}

  for(std::size_t seed=1000000;seed<1000000+EXPECTED_WORLDS;++seed)for(const auto& scenario:SCENARIOS)
  {const std::string s=std::to_string(seed);const auto& b=data.at({s,scenario,POLICIES[0]});const auto& a=data.at({s,scenario,POLICIES[3]});
   primary<<b.Trial<<','<<s<<','<<scenario<<','<<b.Route<<','<<a.Route<<','<<b.Reliability<<','<<a.Reliability<<','<<a.Reliability-b.Reliability<<','<<b.Delivered_Bits<<','<<a.Delivered_Bits<<','<<a.Delivered_Bits-b.Delivered_Bits<<','<<b.Latency<<','<<a.Latency<<','<<a.Latency-b.Latency<<','<<b.Contact_Failures<<','<<a.Contact_Failures<<','<<a.Contact_Failures-b.Contact_Failures<<','<<b.Optical_Failures<<','<<a.Optical_Failures<<','<<a.Optical_Failures-b.Optical_Failures<<'\n';}

  for(const auto& scenario:SCENARIOS)for(const auto& policy:POLICIES)for(const auto& route:ROUTES)
  {std::size_t count=0;double physical=0.0,score=0.0;for(const auto& x:data)if(x.second.Scenario==scenario&&x.second.Policy==policy&&x.second.Route==route)
   {const auto& s=selected.at(x.first);++count;physical+=s.Physical;score+=s.Score;}
   routeout<<scenario<<','<<policy<<','<<route<<','<<count<<','<<100.0*count/EXPECTED_WORLDS<<','<<(count?physical/count:0.0)<<','<<(count?score/count:0.0)<<'\n';}
  aggregate.close();paired.close();primary.close();routeout.close();
  std::cout<<"PASS: production row counts\nPASS: 16000 route selections matched across inputs\nPASS: matched trial pairing validated\nPASS: undefined zero-delivery latency excluded\nPhase 13B complete.\n"
   <<"Descriptive rows: "<<SCENARIOS.size()*POLICIES.size()*metrics.size()<<"\nPaired comparison rows: "<<SCENARIOS.size()*3*metrics.size()<<"\nPrimary paired trial rows: "<<SCENARIOS.size()*EXPECTED_WORLDS<<"\n"
   <<"Descriptive output: "<<aggregate_name<<"\nPaired output: "<<paired_name<<"\nPrimary differences: "<<primary_name<<"\nRoute selection output: "<<route_name<<'\n';
 }
 catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';return 1;}return 0;
}
