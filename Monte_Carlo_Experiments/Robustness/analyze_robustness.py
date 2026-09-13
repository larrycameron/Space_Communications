#!/usr/bin/env python3
import argparse, csv, math, statistics
from collections import Counter, defaultdict
from pathlib import Path

TIE_EPS=1e-12

def read(path):
    with open(path,newline='',encoding='utf-8') as f:return list(csv.DictReader(f))
def f(x):return float(x)
def mean(x):return statistics.fmean(x) if x else math.nan
def sd(x):return statistics.stdev(x) if len(x)>1 else 0.0

def betacf(a,b,x):
    qab=a+b;qap=a+1.;qam=a-1.;c=1.;d=1.-qab*x/qap;d=1e-300 if abs(d)<1e-300 else d;d=1./d;h=d
    for m in range(1,201):
        m2=2*m;aa=m*(b-m)*x/((qam+m2)*(a+m2));d=1.+aa*d;d=1e-300 if abs(d)<1e-300 else d;c=1.+aa/c;c=1e-300 if abs(c)<1e-300 else c;d=1./d;h*=d*c
        aa=-(a+m)*(qab+m)*x/((a+m2)*(qap+m2));d=1.+aa*d;d=1e-300 if abs(d)<1e-300 else d;c=1.+aa/c;c=1e-300 if abs(c)<1e-300 else c;d=1./d;delta=d*c;h*=delta
        if abs(delta-1.)<3e-14:break
    return h
def ibeta(x,a,b):
    if x<=0:return 0.
    if x>=1:return 1.
    bt=math.exp(math.lgamma(a+b)-math.lgamma(a)-math.lgamma(b)+a*math.log(x)+b*math.log1p(-x))
    return bt*betacf(a,b,x)/a if x<(a+1.)/(a+b+2.) else 1.-bt*betacf(b,a,1.-x)/b
def tcdf(t,df):
    if t==0:return .5
    x=df/(df+t*t);tail=.5*ibeta(x,df/2.,.5)
    return 1.-tail if t>0 else tail
def tcrit95(df):
    lo,hi=0.,10.
    for _ in range(90):
        mid=(lo+hi)/2
        if tcdf(mid,df)<.975:lo=mid
        else:hi=mid
    return (lo+hi)/2
def sign_p(w,l):
    n=w+l
    if not n:return 1.
    k=min(w,l)
    return min(1.,2.*sum(math.comb(n,i) for i in range(k+1))/(2**n))

def paired(base,comp,higher=True):
    dif=[b-a for a,b in zip(base,comp)];n=len(dif);m=mean(dif);s=sd(dif);se=s/math.sqrt(n) if n else math.nan;crit=tcrit95(n-1) if n>1 else math.nan
    pref=[d if higher else -d for d in dif];wins=sum(x>TIE_EPS for x in pref);loss=sum(x<-TIE_EPS for x in pref);ties=n-wins-loss
    return {'N':n,'Reference_Mean':mean(base),'Comparator_Mean':mean(comp),'Mean_Paired_Difference':m,'Median_Paired_Difference':statistics.median(dif),'SD_Paired_Difference':s,'CI95_Lower':m-crit*se,'CI95_Upper':m+crit*se,'Cohens_Dz':m/s if s else 0.,'Preferred_Wins':wins,'Preferred_Losses':loss,'Ties':ties,'Exact_Sign_Test_P_Value':sign_p(wins,loss)}

def main():
    ap=argparse.ArgumentParser();ap.add_argument('summary');ap.add_argument('routes');ap.add_argument('output_dir');ap.add_argument('--phase13-summary');a=ap.parse_args();out=Path(a.output_dir);out.mkdir(parents=True,exist_ok=True);s=read(a.summary);r=read(a.routes);errors=[]
    skeys=[(x['Condition'],x['Seed'],x['Policy']) for x in s];rkeys=[(x['Condition'],x['Seed'],x['Policy'],x['Route_Name']) for x in r]
    if len(skeys)!=len(set(skeys)):errors.append('duplicate trial-summary key')
    if len(rkeys)!=len(set(rkeys)):errors.append('duplicate route-score key')
    for x in s:
        g,d,e,dr,c,o=map(int,[x['Generated'],x['Delivered'],x['Expired'],x['Dropped'],x['Contact_Failures'],x['Optical_Failures']])
        if d+e+dr!=g or c+o!=dr or not math.isclose(f(x['Reliability']),d/g,abs_tol=1e-14) or int(x['Delivered_Bits'])!=d*1024:errors.append('summary accounting: '+str((x['Condition'],x['Seed'],x['Policy'])))
    rg=defaultdict(list)
    for x in r:rg[(x['Condition'],x['Seed'],x['Policy'])].append(x)
    for k,rows in rg.items():
        selected=[x for x in rows if x['Selected']=='1']
        if len(selected)!=1:errors.append('selected count: '+str(k));continue
        available=[x for x in rows if x['Available']=='1'];mx=max(f(x['Policy_Score']) for x in available)
        if selected[0]['Available']!='1' or not math.isclose(f(selected[0]['Policy_Score']),mx,abs_tol=1e-14):errors.append('selected route is not available maximum: '+str(k))
    # PAT leakage: physical inputs must be invariant across PAT values for each base route.
    if s and s[0]['Factor_Name']=='PAT_s_Per_Hop':
        leak=defaultdict(set)
        for x in r:leak[(x['Base_Scenario'],x['Seed'],x['Policy'],x['Route_Name'])].add((x['Contact_Probability'],x['Optical_Success_Lower'],x['Propagation_s']))
        if any(len(v)!=1 for v in leak.values()):errors.append('PAT factor leakage into contact, optical, or propagation term')
    by=defaultdict(dict)
    for x in s:by[(x['Condition'],x['Seed'])][x['Policy']]=x
    comparisons=[]
    for condition in sorted({x['Condition'] for x in s}):
        pairs=[v for (c,_),v in by.items() if c==condition]
        for ref,comp in [('B1_Shortest_Delay','B4_Physical_Multiobjective'),('B3_Physical_Success','B4_Physical_Multiobjective')]:
            b=[f(x[ref]['Reliability']) for x in pairs];q=[f(x[comp]['Reliability']) for x in pairs];z={'Condition':condition,'Metric':'Reliability','Reference_Policy':ref,'Comparator_Policy':comp};z.update(paired(b,q,True))
            if ref=='B1_Shortest_Delay':
                if z['SD_Paired_Difference']==0 and z['Mean_Paired_Difference']==0:z['Interpretation']='No_Change'
                elif z['Mean_Paired_Difference']>=.10 and z['CI95_Lower']>0:z['Interpretation']='Preserved'
                elif z['Mean_Paired_Difference']>0 and z['CI95_Lower']>0:z['Interpretation']='Qualified_Remainder'
                elif z['Mean_Paired_Difference']<0 and z['CI95_Upper']<0:z['Interpretation']='Reversal'
                else:z['Interpretation']='Inconclusive'
            comparisons.append(z)
            eligible=[x for x in pairs if int(x[ref]['Delivered'])>0 and int(x[comp]['Delivered'])>0];b=[f(x[ref]['Mean_Delivered_Latency_s']) for x in eligible];q=[f(x[comp]['Mean_Delivered_Latency_s']) for x in eligible]
            if len(b)>1:z={'Condition':condition,'Metric':'Conditional_Latency_s','Reference_Policy':ref,'Comparator_Policy':comp,'Excluded_Worlds':len(pairs)-len(eligible)};z.update(paired(b,q,False));comparisons.append(z)
    fields=['Condition','Metric','Reference_Policy','Comparator_Policy','Interpretation','Excluded_Worlds','N','Reference_Mean','Comparator_Mean','Mean_Paired_Difference','Median_Paired_Difference','SD_Paired_Difference','CI95_Lower','CI95_Upper','Cohens_Dz','Preferred_Wins','Preferred_Losses','Ties','Exact_Sign_Test_P_Value']
    prefix=Path(a.summary).name.replace('_Trial_Summary.csv','')
    with open(out/f'{prefix}_Paired_Comparisons.csv','w',newline='') as fobj:w=csv.DictWriter(fobj,fields);w.writeheader();w.writerows(comparisons)
    route_counts=Counter((x['Condition'],x['Policy'],x['Selected_Route']) for x in s);worlds=Counter((x['Condition'],x['Policy']) for x in s)
    with open(out/f'{prefix}_Route_Selection_Summary.csv','w',newline='') as fobj:
        fields2=['Condition','Policy','Route','Selection_Count','Selection_Percentage'];w=csv.DictWriter(fobj,fields2);w.writeheader()
        for c,p in sorted(worlds):
            for route in ['Direct_Earth_to_Mars','Earth_Relay0_Mars','Earth_Relay1_Mars','Earth_Relay2_Mars']:
                n=route_counts[(c,p,route)];w.writerow({'Condition':c,'Policy':p,'Route':route,'Selection_Count':n,'Selection_Percentage':100*n/worlds[(c,p)]})
    # Exact Phase 13 replication for PAT=60 on common seeds and scenarios.
    replicated='not requested'
    if a.phase13_summary:
        old=read(a.phase13_summary);old={(x['Base_Scenario'] if 'Base_Scenario' in x else x['Scenario'],x['Seed'],x['Policy']):x for x in old};checked=0
        fields3=['Selected_Route','Generated','Delivered','Expired','Dropped','Contact_Failures','Optical_Failures','Reliability','Delivered_Bits','Mean_Delivered_Latency_s']
        for x in s:
            if x['Factor_Name']!='PAT_s_Per_Hop' or not math.isclose(f(x['Factor_Value']),60.):continue
            y=old.get((x['Base_Scenario'],x['Seed'],x['Policy']))
            if y is None:errors.append('missing Phase 13 reference row');continue
            checked+=1
            for field in fields3:
                if field=='Mean_Delivered_Latency_s' or field=='Reliability':
                    if not math.isclose(f(x[field]),f(y[field]),rel_tol=0,abs_tol=1e-12):errors.append('Phase 13 mismatch '+field+str((x['Base_Scenario'],x['Seed'],x['Policy'])))
                elif x[field]!=y[field]:errors.append('Phase 13 mismatch '+field+str((x['Base_Scenario'],x['Seed'],x['Policy'])))
        replicated=f'{checked} PAT=60 rows checked exactly'
    report=out/f'{prefix}_Validation_Report.txt'
    with open(report,'w') as fobj:
        fobj.write(('FAIL' if errors else 'PASS')+'\n')
        fobj.write(f'Trial summary rows: {len(s)}\nRoute score rows: {len(r)}\nPhase 13 replication: {replicated}\n')
        fobj.write('Checks: unique keys, accounting, probability-derived reliability, delivered bits, one available maximum-score route, PAT leakage, paired statistics\n')
        for e in errors:fobj.write('ERROR: '+e+'\n')
    print(('FAIL' if errors else 'PASS')+f': {report}')
    if errors:raise SystemExit(1)
if __name__=='__main__':main()
