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

// Phase 12F: replace the normalized optical-quality routing proxy with
// Configuration 149 end-to-end optical packet-success lower bounds.
// The frozen Phase 11B outputs are inputs and are never modified.

namespace
{
constexpr int CONFIGURATION_ID = 149;
constexpr std::size_t EXPECTED_WORLDS = 100;
constexpr std::size_t EXPECTED_LINKS = 400;

const std::array<std::string,4> ROUTES{
    "Direct_Earth_to_Mars", "Earth_Relay0_Mars",
    "Earth_Relay1_Mars", "Earth_Relay2_Mars"};
const std::array<std::string,4> POLICIES{
    "B1_Shortest_Delay", "B2_Contact_Aware",
    "B3_Physical_Success", "B4_Physical_Multiobjective"};

struct Route_State
{
    std::string Name;
    bool Available{};
    int Hop_Count{};
    double Contact_Probability{};
    double PAT_Seconds{};
    double Propagation_Seconds{};
    double Optical_Success_Lower{};
    double Log10_Optical_Failure_Upper{};
};

struct World
{
    std::string Trial;
    std::string Seed;
    std::array<Route_State,4> Routes;
};

struct Decision
{
    std::array<double,4> Scores{};
    int Selected{-1};
};

struct Link
{
    bool Available{};
    double Success{};
    double Log10_Failure{};
};

std::vector<std::string> Split(const std::string& line)
{
    std::vector<std::string> fields;
    std::string field;
    bool quoted=false;
    for(std::size_t i=0;i<line.size();++i)
    {
        const char c=line[i];
        if(c=='"')
        {
            if(quoted&&i+1<line.size()&&line[i+1]=='"')
            {field.push_back('"');++i;}
            else quoted=!quoted;
        }
        else if(c==','&&!quoted){fields.push_back(field);field.clear();}
        else field.push_back(c);
    }
    fields.push_back(field);
    return fields;
}

std::size_t Col(const std::map<std::string,std::size_t>& c,const std::string& n)
{
    const auto f=c.find(n);
    if(f==c.end())throw std::runtime_error("Missing column: "+n);
    return f->second;
}

double Num(const std::string& v,const std::string& n)
{
    char* end=nullptr;
    const double x=std::strtod(v.c_str(),&end);
    if(end==v.c_str()||(*end!='\0'&&*end!='\r'))
        throw std::runtime_error("Invalid number in "+n+": "+v);
    return x;
}

std::map<std::pair<std::string,std::string>,Link>
Read_Links(const std::string& file_name)
{
    std::ifstream in(file_name);
    if(!in)throw std::runtime_error("Could not open Phase 12D detail: "+file_name);
    std::string line;std::getline(in,line);const auto h=Split(line);
    std::map<std::string,std::size_t> c;
    for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;
    const auto g=[&c](const std::vector<std::string>& r,const std::string& n)->const std::string&
    {return r.at(Col(c,n));};
    std::map<std::pair<std::string,std::string>,Link> links;
    while(std::getline(in,line))
    {
        if(line.empty()) continue;
        const auto r=Split(line);
        if(std::stoi(g(r,"Configuration_ID"))!=CONFIGURATION_ID)continue;
        links[{g(r,"Seed"),g(r,"Route_Name")}]=
        {std::stoi(g(r,"Route_Available"))!=0,
         Num(g(r,"Route_Packet_Success_Lower_Bound"),"optical success"),
         Num(g(r,"Log10_Route_Packet_Failure_Upper_Bound"),"failure bound")};
    }
    return links;
}

std::vector<World> Read_Worlds(
    const std::string& file_name,
    const std::map<std::pair<std::string,std::string>,Link>& links)
{
    std::ifstream in(file_name);
    if(!in)throw std::runtime_error("Could not open Phase 11B trial CSV: "+file_name);
    std::string line;std::getline(in,line);const auto h=Split(line);
    std::map<std::string,std::size_t> c;
    for(std::size_t i=0;i<h.size();++i)c[h[i]]=i;
    const auto g=[&c](const std::vector<std::string>& r,const std::string& n)->const std::string&
    {return r.at(Col(c,n));};
    std::vector<World> worlds;
    while(std::getline(in,line))
    {
        if(line.empty()) continue;
        const auto r=Split(line);
        if(g(r,"Policy")!="B1")continue;
        World w;w.Trial=g(r,"Trial");w.Seed=g(r,"Seed");
        const std::array<std::string,4> prefix{"Direct","Relay0","Relay1","Relay2"};
        for(std::size_t i=0;i<4;++i)
        {
            Route_State s;s.Name=ROUTES[i];
            const auto lf=links.find({w.Seed,s.Name});
            if(lf==links.end())throw std::runtime_error("Missing optical link for "+w.Seed+"/"+s.Name);
            s.Available=lf->second.Available;
            s.Optical_Success_Lower=lf->second.Success;
            s.Log10_Optical_Failure_Upper=lf->second.Log10_Failure;
            s.Hop_Count=static_cast<int>(Num(g(r,prefix[i]+"_Hop_Count"),"hop count"));
            s.Contact_Probability=Num(g(r,prefix[i]+"_Route_Contact_Probability"),"contact");
            s.PAT_Seconds=Num(g(r,prefix[i]+"_PAT_Cost_s"),"PAT");
            s.Propagation_Seconds=Num(g(r,prefix[i]+"_Propagation_Delay_s"),"propagation");
            w.Routes[i]=s;
        }
        worlds.push_back(w);
    }
    return worlds;
}

double Minimum_Available(const World& w,bool pat)
{
    double m=std::numeric_limits<double>::infinity();
    for(const auto& r:w.Routes)if(r.Available)
        m=std::min(m,pat?r.PAT_Seconds:r.Propagation_Seconds);
    return m;
}

Decision Select(const World& w,std::size_t policy)
{
    Decision d;d.Scores.fill(-1.0);
    const double min_pat=Minimum_Available(w,true);
    const double min_prop=Minimum_Available(w,false);
    double best=-1.0;
    for(std::size_t i=0;i<w.Routes.size();++i)
    {
        const auto& r=w.Routes[i];
        if(!r.Available)continue;
        const double physical=r.Contact_Probability*r.Optical_Success_Lower;
        double score=0.0;
        if(policy==0)score=min_prop/r.Propagation_Seconds;
        else if(policy==1)score=r.Contact_Probability;
        else if(policy==2)score=physical;
        else score=physical*(min_pat/r.PAT_Seconds)*(min_prop/r.Propagation_Seconds);
        d.Scores[i]=score;
        if(score>best+1e-15){best=score;d.Selected=static_cast<int>(i);}
    }
    return d;
}

bool Dominates(const Route_State& a,const Route_State& b)
{
    if(!a.Available||!b.Available)return false;
    const double ap=a.Contact_Probability*a.Optical_Success_Lower;
    const double bp=b.Contact_Probability*b.Optical_Success_Lower;
    const bool no_worse=ap>=bp&&a.PAT_Seconds<=b.PAT_Seconds&&
                        a.Propagation_Seconds<=b.Propagation_Seconds;
    const bool better=ap>bp||a.PAT_Seconds<b.PAT_Seconds||
                      a.Propagation_Seconds<b.Propagation_Seconds;
    return no_worse&&better;
}

bool Tests()
{
    bool ok=true;const auto check=[&ok](const char* n,bool v)
    {std::cout<<(v?"PASS: ":"FAIL: ")<<n<<'\n';ok&=v;};
    World w;
    for(std::size_t i=0;i<4;++i)w.Routes[i]={ROUTES[i],true,static_cast<int>(i?2:1),
        i?0.8:0.9,i?120.0:60.0,i?110.0:100.0,i?0.9999:0.999, -4};
    check("unavailable route hard gate",[&w](){w.Routes[0].Available=false;return Select(w,0).Selected!=0;}());
    w.Routes[0].Available=true;
    check("physical score multiplies contact and optical",std::abs(
        w.Routes[0].Contact_Probability*w.Routes[0].Optical_Success_Lower-.8991)<1e-12);
    check("direct dominance recognized",Dominates(w.Routes[0],w.Routes[1]));
    check("physical selector chooses maximum",Select(w,2).Selected==0);
    return ok;
}
} // namespace

int main(int argc,char* argv[])
{
    if(!Tests())return 1;
    const std::string trial_input=argc>1?argv[1]:
        "Monte_Carlo_Experiments/Phase11B_Trial_1_Monte_Carlo_Experiment_4.csv";
    const std::string link_input=argc>2?argv[2]:
        "Monte_Carlo_Experiments/Phase12D_Results/Phase12D_Minimum_Feasible_Design_Detail.csv";
    const std::string output_dir=argc>3?argv[3]:
        "Monte_Carlo_Experiments/Phase12F_Results";
    try
    {
        const auto links=Read_Links(link_input);
        const auto worlds=Read_Worlds(trial_input,links);
        if(links.size()!=EXPECTED_LINKS)throw std::runtime_error("Expected 400 link states; read "+std::to_string(links.size()));
        if(worlds.size()!=EXPECTED_WORLDS)throw std::runtime_error("Expected 100 worlds; read "+std::to_string(worlds.size()));
        std::filesystem::create_directories(output_dir);
        const std::string detail_name=output_dir+"/Phase12F_Physical_Routing_Scores.csv";
        const std::string decision_name=output_dir+"/Phase12F_Policy_Decisions.csv";
        const std::string dominance_name=output_dir+"/Phase12F_Route_Dominance.csv";
        std::ofstream detail(detail_name),decisions(decision_name),dominance(dominance_name);
        if(!detail||!decisions||!dominance)throw std::runtime_error("Could not create Phase 12F outputs.");
        detail<<std::setprecision(17);decisions<<std::setprecision(17);
        detail<<"Trial,Seed,Policy,Route_Index,Route_Name,Available,Hop_Count,"
                "Contact_Probability,Optical_Packet_Success_Lower_Bound,"
                "Physical_End_To_End_Success_Lower_Bound,PAT_s,Propagation_s,"
                "Policy_Score,Selected\n";
        decisions<<"Trial,Seed,Policy,Selected_Route_Index,Selected_Route_Name,"
                   "Selected_Score,Selected_Physical_Success_Lower_Bound\n";
        dominance<<"Trial,Seed,Dominating_Route,Dominated_Route\n";
        std::map<std::pair<std::string,std::string>,std::size_t> selections;
        std::size_t dominance_rows=0;
        for(const World& w:worlds)
        {
            for(std::size_t p=0;p<POLICIES.size();++p)
            {
                const Decision d=Select(w,p);
                if(d.Selected<0)throw std::runtime_error("No route selected in trial "+w.Trial);
                const auto& selected=w.Routes[static_cast<std::size_t>(d.Selected)];
                ++selections[{POLICIES[p],selected.Name}];
                for(std::size_t i=0;i<w.Routes.size();++i)
                {
                    const auto& r=w.Routes[i];
                    detail<<w.Trial<<','<<w.Seed<<','<<POLICIES[p]<<','<<i<<','
                          <<r.Name<<','<<(r.Available?1:0)<<','<<r.Hop_Count<<','
                          <<r.Contact_Probability<<','<<r.Optical_Success_Lower<<','
                          <<r.Contact_Probability*r.Optical_Success_Lower<<','
                          <<r.PAT_Seconds<<','<<r.Propagation_Seconds<<','
                          <<d.Scores[i]<<','<<(d.Selected==static_cast<int>(i)?1:0)<<'\n';
                }
                decisions<<w.Trial<<','<<w.Seed<<','<<POLICIES[p]<<','<<d.Selected
                         <<','<<selected.Name<<','<<d.Scores[d.Selected]<<','
                         <<selected.Contact_Probability*selected.Optical_Success_Lower<<'\n';
            }
            for(std::size_t i=0;i<4;++i)for(std::size_t j=0;j<4;++j)
                if(i!=j&&Dominates(w.Routes[i],w.Routes[j]))
                {dominance<<w.Trial<<','<<w.Seed<<','<<w.Routes[i].Name<<','<<w.Routes[j].Name<<'\n';++dominance_rows;}
        }
        detail.close();decisions.close();dominance.close();
        std::cout<<"Phase 12F complete.\nMatched worlds: "<<worlds.size()
                 <<"\nRoute-score rows: "<<worlds.size()*POLICIES.size()*ROUTES.size()
                 <<"\nPolicy-decision rows: "<<worlds.size()*POLICIES.size()
                 <<"\nDominance rows: "<<dominance_rows<<"\nSelection counts:\n";
        for(const auto& x:selections)
            std::cout<<"  "<<x.first.first<<" / "<<x.first.second<<": "<<x.second<<'\n';
        std::cout<<"Scores: "<<detail_name<<"\nDecisions: "<<decision_name
                 <<"\nDominance: "<<dominance_name<<'\n';
    }
    catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';return 1;}
    return 0;
}
