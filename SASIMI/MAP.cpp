//
// Created by elab on 18-4-17.
//

#include "MAP.h"

#define nonstr " ="
#define qt N[QN[i]]

MAP::MAP() {
    I=0;
    A=0;
    O=0;
    del=-1;
    sub=-1;
    MaxDelay=0;
    MaxArea=0;
    Omax=false;
    O2max=false;
    memset(ErrFlag,0,sizeof(ErrFlag));
    matrixTime=0;
    errorTime=0;
    areaTime=0;
    findTime=0;
    matrixnode=0;
    errornode=0;
    if(AEMFlag){
        TrueValue = new unsigned long[SimValue];
        AEMTempValue = new unsigned long[SimValue];
        AEMThreshold = 0;
        LastAEM = 0;
    }
}

MAP::~MAP() {
    for(int i=I;i<(int)N.size();i++){
        if(N[i].type!=in&&N[I].V1){
            delete [] N[i].V1;
            if(O>64)
                delete [] N[i].V2;
            if(O>128)
                delete [] N[i].V3;
        }
    }
    for(int i=I;i<I+O;i++){
        delete [] N[i].W;
    }
    if(AEMFlag){
        delete[] TrueValue;
        delete[] AEMTempValue;
    }
}

int MAP::FindNode(const string var) {
    for(int i=0;i<(int)N.size();i++)
        if(N[i].var==var)
            return i;
    return -1;
}

void MAP::InputNode(string s,NODETYPE d) {
    node x;
    x.var=s;
    x.type=d;
    x.area=0;
    x.error=0;
    x.reducearea=0;
    x.P=0;
    x.inv=false;
    x.sub=-1;
    for(int i=0;i<inlong;i++)
        x.inloc[i]=0;
    x.delay=0;
    x.temp=NULL;
    N.push_back(x);
}


void MAP::GatebufNode(string s) {
    int loc,innum;
    string innode;
    string outnode;
    string::size_type index;
    index=s.find_first_of(nonstr);
    innode=s.substr(0,index);
    s=s.substr(index,s.size());
    index=s.find_first_not_of(nonstr);
    s=s.substr(index,s.size());
    outnode=s;
    loc=FindNode(outnode);
    if(loc==-1)
    {
        node x;
        loc=(int)N.size();
        x.var=outnode;
        x.type=intern;
        x.area=0;
        x.error=0;
        x.reducearea=0;
        x.P=0;
        x.inv=false;
        x.sub=-1;
        for(int i=0;i<inlong;i++)
            x.inloc[i]=0;
        x.delay=0;
        N.push_back(x);
    }
    QN.push_back(loc);
    N[loc].QNL = QN.size()-1;
    A++;
    N[loc].gatename="buf";
    N[loc].V1=new unsigned long[SimValue];
    memset(N[loc].V1,0,sizeof(unsigned long)*SimValue);
    if(O>64)
    {
        N[loc].V2=new unsigned long[SimValue];
        memset(N[loc].V2,0,sizeof(unsigned long)*SimValue);
    }
    if(O>128){
        N[loc].V3=new unsigned long[SimValue];
        memset(N[loc].V3,0,sizeof(unsigned long)*SimValue);
    }
    N[loc].g=gbuf;
    N[loc].area=1;
    N[loc].gatedelay=1;
    double maxdelay=0;

    int inloc=FindNode(innode);
    N[loc].in.push_back(inloc);
    N[inloc].out.push_back(loc);
    if(maxdelay<N[inloc].delay)
        maxdelay=N[inloc].delay;
    if(N[inloc].type==in){
        innum=inloc/64;
        N[loc].inloc[innum]|=(unsigned long)1<<(inloc%64);
    }
    else{
        for(int j=0;j<inlong;j++)
            N[loc].inloc[j]|=N[inloc].inloc[j];
    }
    N[loc].delay=maxdelay+N[loc].gatedelay;
    if(MaxDelay<N[loc].delay)
        MaxDelay=N[loc].delay;
    MaxArea+=N[loc].area;
}


void MAP::GateNode(string s) {
    string gatename;
    bool flag=true;
    int loc,innum;
    vector<string> innode;
    string outnode;
    string::size_type index;
    index=s.find_first_of(nonstr);
    gatename=s.substr(0,index);
    s=s.substr(index,s.size());
    index=s.find_first_not_of(nonstr);
    s=s.substr(index,s.size());
    while(flag){
        if(s[0]=='O')
            flag=false;
        string subs;
        index=s.find_first_of(nonstr);
        s=s.substr(index+1,s.size());
        index=s.find_first_of(nonstr);
        subs=s.substr(0,index);
        s=s.substr(index+1,s.size());
        if(flag)
            innode.push_back(subs);
        else
            outnode=subs;
    }
    loc=FindNode(outnode);
    if(loc==-1)
    {
        node x;
        loc=(int)N.size();
        x.var=outnode;
        x.type=intern;
        x.area=0;
        x.error=0;
        x.reducearea=0;
        x.P=0;
        x.inv=false;
        x.sub=-1;
        for(int i=0;i<inlong;i++)
            x.inloc[i]=0;
        x.delay=0;
        x.temp=NULL;
        N.push_back(x);
    }
    QN.push_back(loc);
    N[loc].QNL=QN.size()-1;
    A++;
    N[loc].gatename=gatename;
    N[loc].V1=new unsigned long[SimValue];
    memset(N[loc].V1,0,sizeof(unsigned long)*SimValue);
    if(O>64)
    {
        N[loc].V2=new unsigned long[SimValue];
        memset(N[loc].V2,0,sizeof(unsigned long)*SimValue);
    }
    if(O>128){
        N[loc].V3=new unsigned long[SimValue];
        memset(N[loc].V3,0,sizeof(unsigned long)*SimValue);
    }
    if(gatename.substr(0,3)=="inv"){
        N[loc].g=ginv;
        N[loc].area=gatename[3]-'0';
        N[loc].gatedelay=0.9+0.1*(gatename[3]-'1');
    }
    else if(gatename.substr(0,4)=="nand"){
        N[loc].g=gnand;
        N[loc].area=gatename[4]-'0';
        N[loc].gatedelay=1;
        if(gatename[4]=='4')
            N[loc].gatedelay=1.4;
        else
            N[loc].gatedelay=1+0.1*(gatename[4]-'2');
    }
    else if(gatename.substr(0,3)=="nor"){
        N[loc].g=gnor;
        N[loc].area=gatename[3]-'0';
        if(gatename[3]=='4')
            N[loc].gatedelay=3.8;
        else
            N[loc].gatedelay=1.4+1*(gatename[3]-'2');
    }
    else if(gatename=="and2"){
        N[loc].g=gand;
        N[loc].area=3;
        N[loc].gatedelay=1.9;
    }
    else if(gatename=="or2"){
        N[loc].g=gor;
        N[loc].area=3;
        N[loc].gatedelay=2.4;
    }
    else if(gatename=="xor2a"||gatename=="xor2b"){
        N[loc].g=gxor;
        N[loc].area=5;
        N[loc].gatedelay=1.9;
    }
    else if(gatename=="xnor2a"||gatename=="xnor2b"){
        N[loc].g=gxnor;
        N[loc].area=5;
        N[loc].gatedelay=2.1;
    }
    else if(gatename=="aoi21"){
        N[loc].g=gaoi21;
        N[loc].area=3;
        N[loc].gatedelay=1.6;
    }
    else if(gatename=="aoi22"){
        N[loc].g=gaoi22;
        N[loc].area=4;
        N[loc].gatedelay=2;
    }
    else if(gatename=="oai21"){
        N[loc].g=goai21;
        N[loc].area=3;
        N[loc].gatedelay=1.6;
    }
    else if(gatename=="oai22"){
        N[loc].g=goai22;
        N[loc].area=4;
        N[loc].gatedelay=2;
    }
    else if(gatename=="buf"){
        N[loc].g=gbuf;
        N[loc].area=1;
        N[loc].gatedelay=1;
    }
    else if(gatename=="zero"){
        N[loc].g=gzero;
        N[loc].area=0;
        N[loc].gatedelay=0;
    }
    else if(gatename=="one"){
        N[loc].g=gone;
        N[loc].area=0;
        N[loc].gatedelay=0;
    }
    double maxdelay=0;
    for(int i=0;i<(int)innode.size();i++)
    {
        int inloc=FindNode(innode[i]);
        N[loc].in.push_back(inloc);
        flag=true;
        for(int j=0;j<N[inloc].out.size();j++)
            if(N[inloc].out[j]==loc)
                flag=false;
        if(flag)
            N[inloc].out.push_back(loc);
        if(maxdelay<N[inloc].delay)
            maxdelay=N[inloc].delay;
        if(N[inloc].type==in){
            innum=inloc/64;
            N[loc].inloc[innum]|=(unsigned long)1<<(inloc%64);
        }
        else{
            for(int j=0;j<inlong;j++)
                N[loc].inloc[j]|=N[inloc].inloc[j];
        }
    }
    N[loc].delay=maxdelay+N[loc].gatedelay;
    if(MaxDelay<N[loc].delay)
        MaxDelay=N[loc].delay;
    MaxArea+=N[loc].area;
}

void MAP::InputFile(const char *f) {
    ifstream infile;
    string s,subs;
    string::size_type index;
    infile.open(f);
    node zero;
    zero.var="zero";
    zero.type=in;
    zero.g=gzero;
    zero.gatename="zero";
    zero.delay=0;
    N.push_back(zero);
    node one;
    one.var="one";
    one.type=in;
    one.g=gone;
    one.gatename="one";
    zero.delay=0;
    N.push_back(one);
    I=2;
    while(getline(infile,s)){
        if(s[0]=='.')
        {
            index=s.find_first_of(nonstr);
            subs=s.substr(0,index);
            if(subs==".model")
            {
                s=s.substr(index+1,s.size());
                name=s;
                break;
            }
        }
    }

    while(getline(infile,s)){
        if(s[0]=='.')
        {
            index=s.find_first_of(nonstr);
            subs=s.substr(0,index);
            if(subs==".inputs")
            {
                s=s.substr(index+1,s.size());
                while(true)
                {
                    index=s.find_first_of(nonstr);
                    subs=s.substr(0,index);
                    InputNode(subs,in);
                    I++;
                    s=s.substr(index+1,s.size());
                    if(s[0]=='\\')
                    {
                        getline(infile,s);
                        index=s.find_first_not_of(nonstr);
                        s=s.substr(index,s.size());
                    }
                    if(index==-1)
                        break;
                }
                break;
            }
        }
    }
    while(getline(infile,s)){
        if(s[0]=='.')
        {
            index=s.find_first_of(nonstr);
            subs=s.substr(0,index);
            if(subs==".outputs")
            {
                s=s.substr(index+1,s.size());
                while(true)
                {
                    index=s.find_first_of(nonstr);
                    subs=s.substr(0,index);
                    InputNode(subs,out);
                    O++;
                    s=s.substr(index+1,s.size());
                    if(s[0]=='\\')
                    {
                        getline(infile,s);
                        index=s.find_first_not_of(nonstr);
                        s=s.substr(index,s.size());
                    }
                    if(index==-1)
                        break;
                }
                break;
            }
        }
    }

    for (int i = I; i < I+O; ++i) {
        N[i].W=new bool[SimValue];
        memset(N[i].W,0,sizeof(bool)*SimValue);
    }

    while(getline(infile,s)){
        if(s[0]=='.'){
            index=s.find_first_of(nonstr);
            subs=s.substr(0,index);
            if(subs==".gate"){
                s=s.substr(index+1,s.size());
                GateNode(s);
            }
            else if(subs==".names"){
                s=s.substr(index+1,s.size());
                GatebufNode(s);
            }
            else if(subs==".end")
                break;
        }
    }
    infile.close();
    Omax=O>64;
    O2max=O>128;
    GateNum=(int)QN.size();
    extern float AEMRate;
    if(AEMFlag)
        AEMThreshold=(long)(AEMRate*(((long)1<<O)-1));
}

void MAP::OutputFile(const char *f) {
    int i,j;
    ofstream outfile;
    outfile.open(f);
    outfile<<".model "<<name<<endl;
    outfile<<".inputs";
    for(i=2;i<I;i++)
        outfile<<" "<<N[i].var;
    outfile<<endl;
    outfile<<".outputs";
    for(i=I;i<I+O;i++)
        outfile<<" "<<N[i].var;
    outfile<<endl;
    for(i=0;i<(int)QN.size();i++){
        outfile<<".gate "<<N[QN[i]].gatename;
        for(j=0;j<7-(int)N[QN[i]].gatename.size();j++)
            outfile<<" ";
        char start='a';
        for(j=0;j<(int)N[QN[i]].in.size();j++){
            outfile<<start<<"="<<N[N[QN[i]].in[j]].var<<" ";
            start++;
        }
        outfile<<"O="<<N[QN[i]].var<<endl;
        // if(N[QN[i]].g!=gbuf){
        //     outfile<<".gate "<<N[QN[i]].gatename;
        //     for(j=0;j<7-(int)N[QN[i]].gatename.size();j++)
        //         outfile<<" ";
        //     char start='a';
        //     for(j=0;j<(int)N[QN[i]].in.size();j++){
        //         outfile<<start<<"="<<N[N[QN[i]].in[j]].var<<" ";
        //         start++;
        //     }
        //     outfile<<"O="<<N[QN[i]].var<<endl;
        // }
        // else{
        //     outfile<<".names ";
        //     outfile<<N[N[QN[i]].in[0]].var<<" ";
        //     outfile<<N[QN[i]].var<<endl;
        //     outfile<<"1 1"<<endl;
        // }
    }
    outfile<<".end"<<endl;
    outfile.close();
}

void MAP::ChangeNodeName() {
    for(int i=2; i<I/2+1; i++){
        stringstream ss;
        ss<<"x_"<<I/2-i+1;
        N[i].var = ss.str();
    }
    for(int i=I/2+1; i<I; i++){
        stringstream ss;
        ss<<"y_"<<I-i;
        N[i].var = ss.str();
    }
    for(int i=I; i<I+O; i++){
        stringstream ss;
        ss<<"z_"<<I+O-i;
        N[i].var = ss.str();
    }
    for(int i=I+O; i < N.size(); i++){
        stringstream ss;
        ss<<"n_"<<i;
        N[i].var = ss.str();
    }
}

void MAP::OutputToMATLAB(const char *f, const char * functionname){
    ofstream outfile;
    outfile.open(f);

    outfile<<"function [num] = "<<functionname<<"(num1, num2)"<<endl;
    outfile<<N[0].var<<" = 0;\n";
    outfile<<N[1].var<<" = 1;\n";
    for(int i=2; i<I/2+1; i++){
        outfile<<N[i].var<<" = "<< "num1("<<I/2-i+1<<");\n";
    }
    for(int i=I/2+1; i<I; i++){
        outfile<<N[i].var<<"= num2("<<I-i<<");\n";
    }
    int j,k;
    for(int i=0;i<(int)QN.size(); i++){
        int x=QN[i];
        outfile<<N[x].var<<" = ";
        switch (N[x].g){
            case ginv:
                outfile<<" ~ "<<N[N[x].in[0]].var<<";";
                break;
            case gnand:
                outfile<<" ~ ( ";
                for(j=0; j<N[x].in.size()-1; j++){
                    outfile<<N[N[x].in[j]].var<<" & ";
                }
                outfile<<N[N[x].in[j]].var<<" );";
                break;
            case gnor:
                outfile<<" ~ ( ";
                for(j=0; j<N[x].in.size()-1; j++){
                    outfile<<N[N[x].in[j]].var<<" | ";
                }
                outfile<<N[N[x].in[j]].var<<" );";
                break;
            case gand:
                outfile<<N[N[x].in[0]].var<<" & "<<N[N[x].in[1]].var<<";";
                break;
            case gor:
                outfile<<N[N[x].in[0]].var<<" | "<<N[N[x].in[1]].var<<";";
                break;
            case gxor:
                outfile<<"xor( "<<N[N[x].in[0]].var<<", "<<N[N[x].in[1]].var<<" );";
                break;
            case gxnor:
                outfile<<"~ xor( "<<N[N[x].in[0]].var<<", "<<N[N[x].in[1]].var<<" );";
                break;
            case gaoi21:
                outfile<<" ~ ( "<<N[N[x].in[0]].var<<" & "<<N[N[x].in[1]].var<<" | "<<N[N[x].in[2]].var<<" );";
                break;
            case gaoi22:
                outfile<<" ~ ( "<<N[N[x].in[0]].var<<" & "<<N[N[x].in[1]].var<<" | "<<N[N[x].in[2]].var<<" & "<<N[N[x].in[3]].var<<" );";
                break;
            case goai21:
                outfile<<" ~ ( ( "<<N[N[x].in[0]].var<<" | "<<N[N[x].in[1]].var<<" ) & "<<N[N[x].in[2]].var<<" );";
                break;
            case goai22:
                outfile<<" ~ ( ( "<<N[N[x].in[0]].var<<" | "<<N[N[x].in[1]].var<<" ) & ( "<<N[N[x].in[2]].var<<" | "<<N[N[x].in[3]].var<<" ) );";
                break;
            case gbuf:
                outfile<<N[N[x].in[0]].var<<" ;";
                break;
            case gzero:
                outfile<<N[0].var<<" ;";
                break;
            case gone:
                outfile<<N[1].var<<" ;";
                break;
            default: cout<<"There are no gate for this node!"<<endl;
        }
        outfile<<endl;
    }

    outfile<<"num = [ ";
    for(int i=O-1;i>0;i--){
        outfile<<N[I+i].var<<" , ";
    }
    outfile<<N[I].var<<" ];\n";
    outfile<<"end\n";
    outfile.close();

}


void MAP::RandomInput(unsigned seed) { // Chang modify, add seed
    srand(seed); // Chang modify, use seed
    int i,j;
    for(j=0;j<SimValue;j++) {
        N[0].I[j] = false;
        N[1].I[j]=true;
    }
    for(i=2;i<I;i++)
        for(j=0;j<SimValue;j++){
            N[i].I[j]=(bool)random(2);
        }
}

void MAP::PrintInputPattern(const char *f) {
    FILE* f_out=fopen(f,"w");
    int j;
    for(int i=0;i<SimValue;i++)
    {
        for(j=0;j<I;j++)
        {
            fprintf(f_out,"%d",N[j].I[i]);
        }
        fprintf(f_out,"\n");
    }
    fclose(f_out);
}

void MAP::ReadInputPattern(const char *f) {
    FILE* f_in=fopen(f,"r");
    char s[I+10];
    int i,j;
    for(i=0;i<SimValue;i++)
    {
        fgets(s,I+10,f_in);
        for(j=0;j<I;j++)
        {
            N[j].I[i]=s[j]=='1';
        }
    }
    fclose(f_in);
}

void MAP::SimulateNode() {
    int i,j,k;
    for(i=0;i<(int)QN.size();i++){
        switch (N[QN[i]].g){
            case ginv:
                for(j=0;j<SimValue;j++)
                    qt.I[j]=!N[qt.in[0]].I[j];
                break;
            case gnand:
                for(j=0;j<SimValue;j++)
                {
                    qt.I[j]=N[qt.in[0]].I[j];
                    for(k=1;k<(int)qt.in.size();k++)
                        qt.I[j]=qt.I[j]&&N[qt.in[k]].I[j];
                    qt.I[j]=!qt.I[j];
                }
                break;
            case gnor:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]=N[qt.in[0]].I[j];
                    for(k=1;k<(int)qt.in.size();k++)
                        qt.I[j]=qt.I[j]||N[qt.in[k]].I[j];
                    qt.I[j]=!qt.I[j];
                }
                break;
            case gand:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]=N[qt.in[0]].I[j]&&N[qt.in[1]].I[j];
                }
                break;
            case gor:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]=N[qt.in[0]].I[j]||N[qt.in[1]].I[j];
                }
                break;
            case gxor:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]=N[qt.in[0]].I[j]!=N[qt.in[1]].I[j];
                }
                break;
            case gxnor:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]=N[qt.in[0]].I[j]==N[qt.in[1]].I[j];
                }
                break;
            case gaoi21:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]=!(N[qt.in[0]].I[j]&&N[qt.in[1]].I[j]||N[qt.in[2]].I[j]);
                }
                break;
            case gaoi22:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]=!(N[qt.in[0]].I[j]&&N[qt.in[1]].I[j]||(N[qt.in[2]].I[j]&&N[qt.in[3]].I[j]));
                }
                break;
            case goai21:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]=!((N[qt.in[0]].I[j]||N[qt.in[1]].I[j])&&N[qt.in[2]].I[j]);
                }
                break;
            case goai22:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]=!((N[qt.in[0]].I[j]||N[qt.in[1]].I[j])&&(N[qt.in[2]].I[j]||N[qt.in[3]].I[j]));
                }
                break;
            case gbuf:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]=N[qt.in[0]].I[j];
                }
                break;
            case gzero:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]= false;
                }
                break;
            case gone:
                for (j=0;j<SimValue;j++) {
                    qt.I[j]= true;
                }
                break;
            default: cout<<"There are no gate for this node!"<<endl;
        }
    }
}

int MAP::FindDeleteNode(double nowerror) {
    del = -1;
    int i = 0;
    double P = 0;
    sub = 0;
    ComputeMFFC();
    //ComputeValue(TempValue,false);
    MeasueAdjustVote_Meng(FLOOR);
    //MeasureAllVote_Meng();
    TN.clear();
    for(i=(int)QN.size()-1;i>=0;i--)
        if(N[QN[i]].g!=gzero&&N[QN[i]].g!=gone){
            if(AEMFlag)
                P = MeasureWithAEM(QN[i], nowerror);
            else
                P=Measure(QN[i],nowerror);
            if(P!=0){
                LOC x;
                x.loc=QN[i];
                x.P=P;
                x.reducearea=N[QN[i]].reducearea;
                TN.push_back(x);
            }
        }
    std::sort(TN.begin(),TN.end(),SortTN);
    return del;
}

bool SortTN(const LOC &v1, const LOC &v2) {
    if(v1.P<0&&v2.P>0)
        return true;
    else if(v1.P>0&&v2.P<0)
        return false;
    else if(v1.P>0&&v2.P>0){
        if(v1.P==v2.P)
            return v1.reducearea>v2.reducearea;
        else
            return v1.P>v2.P;
    }
    else{
        if(v1.P==v2.P)
            return v1.reducearea>v2.reducearea;
        else
            return v1.P<v2.P;
    }

}

bool SortOut(const OUTSORT &v1, const OUTSORT &v2){
    if(v1.loc<v2.loc)
        return true;
    else
        return false;
}


bool MAP::CheckJointAdjust(vector< vector<unsigned long> >& isInFanoutCone, map<unsigned long, int>& disjointSet, unsigned long& jNode1, unsigned long& jNode2, const int up){
    for(auto i=disjointSet.begin(); i!=disjointSet.end(); ++i){
        ++i;
        for(auto j=(i--); j!=disjointSet.end(); ++j){
            for(int k=0; k<isInFanoutCone[0].size(); ++k)
                if((isInFanoutCone[QN[i->first]][k]&isInFanoutCone[QN[j->first]][k]) && (i->second<up ||  j->second<up) ){
                    jNode1 = i->first;
                    jNode2 = j->first;
                    return true;
                }
        }
    }
    return false;
}

void MAP::MeasueAdjustVote_Meng(const int up){
    clock_t temptime=clock();
    map<unsigned long,int> disjointSet;
    map<unsigned long, int> ComputeSet;
    map<unsigned long, int> TempdisjointSet;
    //bool Visited[N.size()];
    unsigned long outputlen=1, loc;
    bool isJoint;
    unsigned long jNode1, jNode2, expandNode=-1;
    if(O2max)
        outputlen=3;
    else if(Omax)
        outputlen=2;
    vector<vector<unsigned long>> isInFanoutCone(N.size(), vector<unsigned long>(outputlen, 0));
    for(unsigned long i=I; i<I+O; ++i){
        loc = (i-I)/64;
        isInFanoutCone[i][loc] = (unsigned long)1<<((i-I)%64);
    }
    for(int i=QN.size()-1; i>=0; --i){
        loc = QN[i];
        for(int j=0; j<N[loc].in.size(); ++j) {
            for (int k = 0; k < outputlen; ++k)
                isInFanoutCone[N[loc].in[j]][k] |= isInFanoutCone[loc][k];
        }
    }
    for(int i=QN.size()-1; i>=0; --i){
        loc = QN[i];
        disjointSet.clear();
        ComputeSet.clear();
        TempdisjointSet.clear();
        //memset(Visited, false, sizeof(bool)*N.size());
        for(int j=0; j<N[loc].out.size(); ++j){
            int xloc = N[loc].out[j];
            if(TempdisjointSet.find(N[xloc].QNL)==TempdisjointSet.end())
                TempdisjointSet[N[xloc].QNL] = 1;
        }
        isJoint = CheckJointAdjust(isInFanoutCone, TempdisjointSet, jNode1, jNode2, up);
        while(isJoint){
            if(jNode1<jNode2 && TempdisjointSet[jNode1]<up)
                expandNode = jNode1;
            else if(TempdisjointSet[jNode2]<up)
                expandNode = jNode2;
            else
                break;
            if(ComputeSet.find(expandNode)==ComputeSet.end())
                ComputeSet[expandNode] = TempdisjointSet[expandNode];
            if(N[QN[expandNode]].type==out && disjointSet.find(expandNode)==disjointSet.end())
                disjointSet[expandNode]=TempdisjointSet[expandNode];
            int tempfloor=TempdisjointSet[expandNode];
            TempdisjointSet.erase(expandNode);
            for(int j=0; j<N[QN[expandNode]].out.size(); ++j){
                int xloc = N[QN[expandNode]].out[j];
                if(TempdisjointSet.find(N[xloc].QNL)==TempdisjointSet.end())
                    TempdisjointSet[N[xloc].QNL] = tempfloor+1;
            }
            isJoint = CheckJointAdjust(isInFanoutCone, TempdisjointSet, jNode1, jNode2, up);
        }
        for(auto iter=TempdisjointSet.begin(); iter!=TempdisjointSet.end(); ++iter){
            if(ComputeSet.find(iter->first)==ComputeSet.end())
                ComputeSet[iter->first] = iter->second;
            if(disjointSet.find(iter->first)==disjointSet.end())
                disjointSet[iter->first] = iter->second;
        }

        for(auto iter=ComputeSet.begin(); iter!=ComputeSet.end(); ++iter)
            if(iter->second > up)
                cout<<"There are error here for "<<loc<<" and floor is "<<iter->second<<endl;
        memset(N[loc].V1,0,sizeof(unsigned long)*SimValue);
        if(Omax){
            memset(N[loc].V2,0,sizeof(unsigned long)*SimValue);
        }
        if(O2max){
            memset(N[loc].V3,0,sizeof(unsigned long)*SimValue);
        }
        if(N[loc].type==out){
            unsigned  long t=(unsigned long)1<<((loc-I)%64);
            if(O2max){
                if(loc-I>127){
                    for(int j=0;j<SimValue;j++)
                        N[loc].V3[j]=t;
                }
                else if(loc-I>63){
                    for(int j=0;j<SimValue;j++)
                        N[loc].V2[j]=t;
                }
                else{
                    for(int j=0;j<SimValue;j++)
                        N[loc].V1[j]=t;
                }
            }
            else if(Omax)
            {
                if(loc-I>63){
                    for(int j=0;j<SimValue;j++)
                        N[loc].V2[j]=t;
                }
                else{
                    for(int j=0;j<SimValue;j++)
                        N[loc].V1[j]=t;
                }
            }
            else{
                for(int j=0; j<SimValue;j++){
                    N[loc].V1[j]=t;
                }
            }
        }
        N[loc].temp = new bool[SimValue];
        for(int j=0; j<SimValue; ++j)
            N[loc].temp[j] = !N[loc].I[j];
        for(auto iter=ComputeSet.begin(); iter!=ComputeSet.end(); ++iter){
            N[QN[iter->first]].temp = new bool[SimValue];
            ComputeTempNodeValue(QN[iter->first]);
            matrixnode++;
        }
        for(auto iter=disjointSet.begin(); iter!=disjointSet.end(); ++iter){
            int fx=QN[iter->first];
            int k=fx-I;
            if(O2max) {
                for (int j = 0; j < SimValue; j++) {
                    if (N[fx].temp[j] != N[fx].I[j]) {
                        N[loc].V1[j] |= N[fx].V1[j];
                        N[loc].V2[j] |= N[fx].V2[j];
                        N[loc].V3[j] |= N[fx].V3[j];
                    } else if (N[fx].type == out) {
                        if (k < 64)
                            N[loc].V1[j] &= ~(((unsigned long) 1) << (k % 64));
                        else if (k < 128)
                            N[loc].V2[j] &= ~(((unsigned long) 1) << (k % 64));
                        else
                            N[loc].V3[j] &= ~(((unsigned long) 1) << (k % 64));
                    }
                }
            }
            else if(Omax){
                for(int j=0;j<SimValue;j++){
                    if(N[fx].temp[j]!=N[fx].I[j]){
                        N[loc].V1[j]|=N[fx].V1[j];
                        N[loc].V2[j]|=N[fx].V2[j];
                    }
                    else if(N[fx].type==out){
                        if(k<64)
                            N[loc].V1[j]&= ~(((unsigned long)1)<<(k%64));
                        else
                            N[loc].V2[j]&= ~(((unsigned long)1)<<(k%64));
                    }
                }
            }
            else{
                for(int j=0;j<SimValue;j++){
                    if(N[fx].temp[j]!=N[fx].I[j]){
                        N[loc].V1[j]|=N[fx].V1[j];
                    }
                    else if(N[fx].type==out){
                        N[loc].V1[j]&= ~(((unsigned long)1)<<(k%64));
                    }
                }
            }
        }
        delete [] N[loc].temp;
        N[loc].temp=NULL;
        for(auto iter=ComputeSet.begin(); iter!=ComputeSet.end(); ++iter){
            delete [] N[QN[iter->first]].temp;
            N[QN[iter->first]].temp=NULL;
        }
    }
    matrixTime+=(clock()-temptime);
}

bool MAP::CheckJoint(vector< vector<unsigned long> >& isInFanoutCone, map<unsigned long, int>& disjointSet, unsigned long& jNode1, unsigned long& jNode2){
    for(auto i=disjointSet.begin(); i!=disjointSet.end(); ++i){
        ++i;
        for(auto j=(i--); j!=disjointSet.end(); ++j){
            for(int k=0; k<isInFanoutCone[0].size(); ++k)
                if(isInFanoutCone[QN[i->first]][k]&isInFanoutCone[QN[j->first]][k]){
                    jNode1 = i->first;
                    jNode2 = j->first;
                    return true;
                }
        }
    }
    return false;
}

void MAP::MeasureAllVote_Meng() {
    clock_t temptime=clock();
    map<unsigned long,int> disjointSet;
    map<unsigned long, int> ComputeSet;
    map<unsigned long, int> TempdisjointSet;
    bool Visited[N.size()];
    unsigned long outputlen=1, loc;
    bool isJoint;
    unsigned long jNode1, jNode2, expandNode;
    if(O2max)
        outputlen=3;
    else if(Omax)
        outputlen=2;
    vector<vector<unsigned long>> isInFanoutCone(N.size(), vector<unsigned long>(outputlen, 0));
    for(unsigned long i=I; i<I+O; ++i){
        loc = (i-I)/64;
        isInFanoutCone[i][loc] = (unsigned long)1<<((i-I)%64);
    }
    for(int i=QN.size()-1; i>=0; --i){
        loc = QN[i];
        for(int j=0; j<N[loc].in.size(); ++j) {
            for (int k = 0; k < outputlen; ++k)
                isInFanoutCone[N[loc].in[j]][k] |= isInFanoutCone[loc][k];
        }
    }
    for(int i=QN.size()-1; i>=0; --i){
        loc = QN[i];
        if(loc==333)
            loc=333;
        disjointSet.clear();
        ComputeSet.clear();
        TempdisjointSet.clear();
        memset(Visited, false, sizeof(bool)*N.size());
        for(int j=0; j<N[loc].out.size(); ++j){
            int xloc = N[loc].out[j];
            if(TempdisjointSet.find(N[xloc].QNL)==TempdisjointSet.end())
                TempdisjointSet[N[xloc].QNL] = xloc;
        }
        isJoint = CheckJoint(isInFanoutCone, TempdisjointSet, jNode1, jNode2);
        while(isJoint){
            if(jNode1<jNode2)
                expandNode = jNode1;
            else
                expandNode = jNode2;
            if(N[QN[expandNode]].type==out && disjointSet.find(expandNode)==disjointSet.end())
                disjointSet[expandNode] = QN[expandNode];
            TempdisjointSet.erase(expandNode);
            if(ComputeSet.find(expandNode)==ComputeSet.end())
                ComputeSet[expandNode] = QN[expandNode];
            for(int j=0; j<N[QN[expandNode]].out.size(); ++j){
                int xloc = N[QN[expandNode]].out[j];
                if(TempdisjointSet.find(N[xloc].QNL)==TempdisjointSet.end())
                    TempdisjointSet[N[xloc].QNL] = xloc;
            }
            isJoint = CheckJoint(isInFanoutCone, TempdisjointSet, jNode1, jNode2);
        }
        for(auto iter=TempdisjointSet.begin(); iter!=TempdisjointSet.end(); ++iter){
            if(ComputeSet.find(iter->first)==ComputeSet.end())
                ComputeSet[iter->first] = iter->second;
            if(disjointSet.find(iter->first)==disjointSet.end())
                disjointSet[iter->first] = iter->second;
        }

        memset(N[loc].V1,0,sizeof(unsigned long)*SimValue);
        if(Omax){
            memset(N[loc].V2,0,sizeof(unsigned long)*SimValue);
        }
        if(O2max){
            memset(N[loc].V3,0,sizeof(unsigned long)*SimValue);
        }
        if(N[loc].type==out){
            unsigned  long t=(unsigned long)1<<((loc-I)%64);
            if(O2max){
                if(loc-I>127){
                    for(int j=0;j<SimValue;j++)
                        N[loc].V3[j]=t;
                }
                else if(loc-I>63){
                    for(int j=0;j<SimValue;j++)
                        N[loc].V2[j]=t;
                }
                else{
                    for(int j=0;j<SimValue;j++)
                        N[loc].V1[j]=t;
                }
            }
            else if(Omax)
            {
                if(loc-I>63){
                    for(int j=0;j<SimValue;j++)
                        N[loc].V2[j]=t;
                }
                else{
                    for(int j=0;j<SimValue;j++)
                        N[loc].V1[j]=t;
                }
            }
            else{
                for(int j=0; j<SimValue;j++){
                    N[loc].V1[j]=t;
                }
            }
        }
        N[loc].temp = new bool[SimValue];
        for(int j=0; j<SimValue; ++j)
            N[loc].temp[j] = !N[loc].I[j];
        for(auto iter=ComputeSet.begin(); iter!=ComputeSet.end(); ++iter){
            N[iter->second].temp = new bool[SimValue];
            ComputeTempNodeValue(iter->second);
            matrixnode++;
        }
        for(auto iter=disjointSet.begin(); iter!=disjointSet.end(); ++iter){
            int fx=iter->second;
            int k=fx-I;
            if(O2max) {
                for (int j = 0; j < SimValue; j++) {
                    if (N[fx].temp[j] != N[fx].I[j]) {
                        N[loc].V1[j] |= N[fx].V1[j];
                        N[loc].V2[j] |= N[fx].V2[j];
                        N[loc].V3[j] |= N[fx].V3[j];
                    } else if (N[fx].type == out) {
                        if (k < 64)
                            N[loc].V1[j] &= ~(((unsigned long) 1) << (k % 64));
                        else if (k < 128)
                            N[loc].V2[j] &= ~(((unsigned long) 1) << (k % 64));
                        else
                            N[loc].V3[j] &= ~(((unsigned long) 1) << (k % 64));
                    }
                }
            }
            else if(Omax){
                for(int j=0;j<SimValue;j++){
                    if(N[fx].temp[j]!=N[fx].I[j]){
                        N[loc].V1[j]|=N[fx].V1[j];
                        N[loc].V2[j]|=N[fx].V2[j];
                    }
                    else if(N[fx].type==out){
                        if(k<64)
                            N[loc].V1[j]&= ~(((unsigned long)1)<<(k%64));
                        else
                            N[loc].V2[j]&= ~(((unsigned long)1)<<(k%64));
                    }
                }
            }
            else{
                for(int j=0;j<SimValue;j++){
                    if(N[fx].temp[j]!=N[fx].I[j]){
                        N[loc].V1[j]|=N[fx].V1[j];
                    }
                    else if(N[fx].type==out){
                        N[loc].V1[j]&= ~(((unsigned long)1)<<(k%64));
                    }
                }
            }
        }
        delete [] N[loc].temp;
        N[loc].temp=NULL;
        for(auto iter=ComputeSet.begin(); iter!=ComputeSet.end(); ++iter){
            delete [] N[iter->second].temp;
            N[iter->second].temp=NULL;
        }
    }
    matrixTime+=(clock()-temptime);
}

void MAP::MeasureAllVote(const int x){
    int i,j,k;
    memset(N[x].V1,0,sizeof(unsigned long)*SimValue);
    if(Omax){
        memset(N[x].V2,0,sizeof(unsigned long)*SimValue);
    }
    if(O2max){
        //memset(N[x].V2,0,sizeof(unsigned long)*SimValue);
        memset(N[x].V3,0,sizeof(unsigned long)*SimValue);
    }
    if(N[x].type==out)
    {
        unsigned  long t=(unsigned long)1<<((x-I)%64);
        if(O2max){
            if(x-I>127){
                for(i=0;i<SimValue;i++)
                    N[x].V3[i]=t;
            }
            else if(x-I>63){
                for(i=0;i<SimValue;i++)
                    N[x].V2[i]=t;
            }
            else{
                for(i=0;i<SimValue;i++)
                    N[x].V1[i]=t;
            }
        }
        else if(Omax)
        {
            if(x-I>63){
                for(i=0;i<SimValue;i++)
                    N[x].V2[i]=t;
            }
            else{
                for(i=0;i<SimValue;i++)
                    N[x].V1[i]=t;
            }
        }
        else{
            for(i=0;i<SimValue;i++){
                N[x].V1[i]=t;
            }
        }
    }
    if(!N[x].out.empty()){
        vector<int> OutNode;
        N[x].temp=new bool[SimValue];
        SN.clear();
        int fx;
        int fo;
        for(i=0;i<SimValue;i++){
            N[x].temp[i]=!N[x].I[i];
        }
        for(i=0;i<N[x].out.size();i++){
            OUTSORT node;
            node.x=N[x].out[i];
            node.loc=N[N[x].out[i]].QNL;
            SN.push_back(node);
        }
        for(i=0;i<N[x].out.size();i++){
            RecursiveFindOutput(N[x].out[i]);
        }
        std::sort(SN.begin(),SN.end(),SortOut);
        for(i=0;i<SN.size();i++){
            fx=SN[i].x;
            if(N[fx].type==out)
                OutNode.push_back(fx);
            N[fx].temp=new bool[SimValue];
            ComputeTempNodeValue(fx);
        }
        for(i=0;i<OutNode.size();i++){
            fx=OutNode[i];
            fo=fx-I;
            if(fo<64){
                for(j=0;j<SimValue;j++)
                    if(N[fx].temp[j]!=N[fx].I[j]){
                        N[x].V1[j]|=(unsigned long)1<<fo;
                    }
            }
            else if(fo<128){
                fo=fo-64;
                for(j=0;j<SimValue;j++)
                    if(N[fx].temp[j]!=N[fx].I[j]){
                        N[x].V2[j]|=(unsigned long)1<<fo;
                    }
            }
            else{
                fo=fo-128;
                for(j=0;j<SimValue;j++)
                    if(N[fx].temp[j]!=N[fx].I[j]){
                        N[x].V3[j]|=(unsigned long)1<<fo;
                    }
            }
        }
        delete [] N[x].temp;
        N[x].temp=NULL;
        for(i=0;i<SN.size();i++){
            delete [] N[SN[i].x].temp;
            N[SN[i].x].temp=NULL;
        }
    }
}

void MAP::RecursiveFindOutput(const int x) {
    int fx,j;
    for(int i=0;i<N[x].out.size();i++){
        fx=N[x].out[i];
        for(j=0;j<SN.size();j++)  //Can put SN into Hashmap
            if(SN[j].x==fx){
                break;
            }
        if(j>=SN.size()){
            OUTSORT node;
            node.x=fx;
            node.loc=N[fx].QNL;
            SN.push_back(node);
            RecursiveFindOutput(fx);
        }
    }
}

void MAP::MeasureAdjustVote(const int x, const int up) {
    clock_t temptime=clock();
    int i,j,k;
    memset(N[x].V1,0,sizeof(unsigned long)*SimValue);
    if(Omax){
        memset(N[x].V2,0,sizeof(unsigned long)*SimValue);
    }
    if(O2max){
        //memset(N[x].V2,0,sizeof(unsigned long)*SimValue);
        memset(N[x].V3,0,sizeof(unsigned long)*SimValue);
    }
    if(N[x].type==out)
    {
        unsigned  long t=(unsigned long)1<<((x-I)%64);
        if(O2max){
            if(x-I>127){
                for(i=0;i<SimValue;i++)
                    N[x].V3[i]=t;
            }
            else if(x-I>63){
                for(i=0;i<SimValue;i++)
                    N[x].V2[i]=t;
            }
            else{
                for(i=0;i<SimValue;i++)
                    N[x].V1[i]=t;
            }
        }
        else if(Omax)
        {
            if(x-I>63){
                for(i=0;i<SimValue;i++)
                    N[x].V2[i]=t;
            }
            else{
                for(i=0;i<SimValue;i++)
                    N[x].V1[i]=t;
            }
        }
        else{
            for(i=0;i<SimValue;i++){
                N[x].V1[i]=t;
            }
        }
    }
    map<int,PATH> Hmap;
    FindPathNode(x,up,0,Hmap,false);
    int recovHeight=0;
    for(map<int,PATH>::iterator iter=Hmap.begin();iter!=Hmap.end();iter++){
        if(iter->second.n>1&&iter->second.h>recovHeight)
            recovHeight=iter->second.h;
    }
    if(recovHeight>0){
        int fx;
        SN.clear();
        vector<int> OUT;
        for(map<int,PATH>::iterator iter=Hmap.begin();iter!=Hmap.end();iter++){
            if((iter->second.h)<=recovHeight){
                OUTSORT ns;
                ns.x=iter->first;
                ns.loc=N[ns.x].QNL;
                SN.push_back(ns);
            }
        }
        std::sort(SN.begin(),SN.end(),SortOut);
        for(i=0;i<SN.size();i++){
            map<int,PATH>::iterator iter=Hmap.find(SN[i].x);
            if((iter->second.h)==recovHeight||(iter->second.h<recovHeight&&N[iter->first].type==out))
                OUT.push_back(iter->first);
        }
        N[x].temp=new bool[SimValue];
        for(i=0;i<SimValue;i++){
            N[x].temp[i]=!N[x].I[i];
        }
        for(i=0;i<SN.size();i++){
            fx=SN[i].x;
            N[fx].temp=new bool[SimValue];
            ComputeTempNodeValue(fx);
            matrixnode++;
        }
        for(i=0;i<OUT.size();i++){
            fx=OUT[i];
            k=fx-I;
            if(O2max){
                for(j=0;j<SimValue;j++){
                    if(N[fx].temp[j]!=N[fx].I[j]){
                        N[x].V1[j]|=N[fx].V1[j];
                        N[x].V2[j]|=N[fx].V2[j];
                        N[x].V3[j]|=N[fx].V3[j];
                    }
                    else if(N[fx].type==out){
                        if(k<64)
                            N[x].V1[j]&= ~(((unsigned long)1)<<(k%64));
                        else if(k<128)
                            N[x].V2[j]&= ~(((unsigned long)1)<<(k%64));
                        else
                            N[x].V3[j]&= ~(((unsigned long)1)<<(k%64));
                    }
                }
            }
            else if(Omax){
                for(j=0;j<SimValue;j++){
                    if(N[fx].temp[j]!=N[fx].I[j]){
                        N[x].V1[j]|=N[fx].V1[j];
                        N[x].V2[j]|=N[fx].V2[j];
                    }
                    else if(N[fx].type==out){
                        if(k<64)
                            N[x].V1[j]&= ~(((unsigned long)1)<<(k%64));
                        else
                            N[x].V2[j]&= ~(((unsigned long)1)<<(k%64));
                    }
                }
            }
            else{
                for(j=0;j<SimValue;j++){
                    if(N[fx].temp[j]!=N[fx].I[j]){
                        N[x].V1[j]|=N[fx].V1[j];
                    }
                    else if(N[fx].type==out){
                        N[x].V1[j]&= ~(((unsigned long)1)<<(k%64));
                    }
                }
            }
        }
        delete [] N[x].temp;
        N[x].temp=NULL;
        for(i=0;i<SN.size();i++){
            delete [] N[SN[i].x].temp;
            N[SN[i].x].temp=NULL;
        }
    }
    else{
        bool BD[SimValue];
        int fx;
        for(i=0;i<SimValue;i++){
            N[x].I[i]=!N[x].I[i];
        }
        for(i=0;i<N[x].out.size();i++){
            fx=N[x].out[i];
            ComputeBooleanDifference(BD,fx,N);
            matrixnode++;
            if(O2max){
                for(j=0;j<SimValue;j++){
                    if(BD[j]){
                        N[x].V1[j] |= N[fx].V1[j];
                        N[x].V2[j] |= N[fx].V2[j];
                        N[x].V3[j] |= N[fx].V3[j];
                    }
                }
            }
            else if(Omax){
                for(j=0;j<SimValue;j++){
                    if(BD[j]){
                        N[x].V1[j] |= N[fx].V1[j];
                        N[x].V2[j] |= N[fx].V2[j];
                    }
                }
            }
            else{
                for(j=0;j<SimValue;j++){
                    if(BD[j]){
                        N[x].V1[j] |= N[fx].V1[j];
                    }
                }
            }

        }
        for(i=0;i<SimValue;i++){
            N[x].I[i]=!N[x].I[i];
        }
    }
    matrixTime+=(clock()-temptime);
}


void MAP::FindPathNode(const int x, const int up, int cur, map<int, PATH> &Hmap, bool F) {
    if(cur>=up)
        return;
    int fx;
    if(F){
        for(int i=0;i<N[x].out.size();i++){
            fx=N[x].out[i];
            map<int,PATH>::iterator iter=Hmap.find(fx);
            if((iter->second.h)<cur+1){
                iter->second.h=cur+1;
                FindPathNode(fx,up,cur+1,Hmap,true);
            }
        }
    }
    else{
        for(int i=0;i<N[x].out.size();i++){
            fx=N[x].out[i];
            if(Hmap.find(fx)!=Hmap.end()){
                map<int,PATH>::iterator iter=Hmap.find(fx);
                (iter->second.n)++;
                if((iter->second.h)<(cur+1)){
                    iter->second.h=cur+1;
                    FindPathNode(fx,up,cur+1,Hmap,true);
                }
            }
            else{
                PATH p;
                p.h=cur+1;
                p.n=1;
                Hmap[fx]=p;
                FindPathNode(fx,up,cur+1,Hmap, false);
            }
        }
    }
}

double MAP::AccurateMeasureError(const int x, const int sub) {
    clock_t tempTime=clock();
    double sum=0,invertsum=0;
    int i,j;
    int fx;
    vector<int> OutNode;
    if(N[x].type==out)
        OutNode.push_back(x);
    N[x].temp=new bool[SimValue];
    for(i=0;i<SimValue;i++){
        N[x].temp[i]=N[sub].I[i];
    }
    SN.clear();
    for(i=0;i<N[x].out.size();i++){
        OUTSORT node;
        node.x=N[x].out[i];
        node.loc=N[N[x].out[i]].QNL;
        SN.push_back(node);
    }
    for(i=0;i<N[x].out.size();i++){
        RecursiveFindOutput(N[x].out[i]);
    }
    std::sort(SN.begin(),SN.end(),SortOut);
    for(i=0;i<SN.size();i++){
        fx=SN[i].x;
        if(N[fx].type==out)
            OutNode.push_back(fx);
        N[fx].temp=new bool[SimValue];
        ComputeTempNodeValue(fx);
    }
    for(i=0;i<SimValue;i++){
        if(!ErrFlag[i]){
            for(j=0;j<OutNode.size();j++){
                if(N[OutNode[j]].temp[i]!=N[OutNode[j]].I[i]){
                    sum++;
                    break;
                }
            }
        }
        else{
            for(j=I;j<I+O;j++){
                if(N[j].W[i]&&(N[j].temp==NULL||(N[j].temp!=NULL&&N[j].temp[i]==N[j].I[i]))){
                    break;
                }
            }
            if(j>=I+O)
                sum--;
        }
    }
    for(i=0;i<SN.size();i++){
        delete [] N[SN[i].x].temp;
        N[SN[i].x].temp=NULL;
    }
    if(sub!=0&&sub!=1){
        for(i=0;i<SimValue;i++){
            N[x].temp[i]=!N[sub].I[i];
        }
        for(i=0;i<SN.size();i++){
            fx=SN[i].x;
            N[fx].temp=new bool[SimValue];
            ComputeTempNodeValue(fx);
        }
        for(i=0;i<SimValue;i++){
            if(!ErrFlag[i]){
                for(j=0;j<OutNode.size();j++){
                    if(N[OutNode[j]].temp[i]!=N[OutNode[j]].I[i]){
                        invertsum++;
                        break;
                    }
                }
            }
            else{
                for(j=I;j<I+O;j++){
                    if(N[j].W[i]&&(N[j].temp==NULL||(N[j].temp!=NULL&&N[j].temp[i]==N[j].I[i]))){
                        break;
                    }
                }
                if(j>=I+O)
                    invertsum--;
            }
        }
        N[x].inverterror=invertsum/SimValue;
        for(i=0;i<SN.size();i++){
            delete [] N[SN[i].x].temp;
            N[SN[i].x].temp=NULL;
        }
        delete [] N[x].temp;
        N[x].temp=NULL;
    }
    errorTime+=(clock()-tempTime);
    return sum/SimValue;
}

double MAP::MeasureWithSASIMI(int x) {
    int i = 0, j;
    double P = 0;
    double error = 0;
    double TemP;
    double area = 0;
    bool flag;
    for (i = 0; i < 2; i++) { //consider 0 and 1
        error = AddErrorWithSASIMI(x, i);
        //error=AccurateMeasureError(x,i);
        if (error  <= ERROR) {
            area = MeasureArea(x, i);
            TemP = area / error;
            if ((error < 0 && P > TemP) || (error >= 0 && P >= 0 && P < TemP)) {
                P = TemP;
                N[x].sub = i;
                N[x].inv = false;
                N[x].error = error;
                N[x].reducearea = area;
            }
        }
    }
    for (i = 0; i < (int) QN.size(); i++){
        if (N[QN[i]].delay <= (N[x].delay) && QN[i] != x) {
            flag = false;
            for (j = 0; j < inlong; j++){
                if ((N[QN[i]].inloc[j] & N[x].inloc[j]) > 0) {
                    flag = true;
                    break;
                }
            }

            if (N[x].g == gbuf && N[x].in[0] == QN[i])
                flag = false;
            for (j = 0; j < N[x].in.size(); j++)
                if (N[x].in[j] == QN[i])
                    flag = false;
            if (flag) {
                error = AddErrorWithSASIMI(x, QN[i]);
                //error=AccurateMeasureError(x,QN[i]);
                if (error  <= ERROR ||
                    ((N[x].inverterror)  <= ERROR && N[x].delay >= N[QN[i]].delay + 1)) {
                    area = MeasureArea(x, QN[i]);
                    if (error <= N[x].inverterror) {
                        if (N[QN[i]].type == out)
                            area = area - 1;
                        TemP = area / error;
                        if ((error < 0 && P > TemP) || (error >= 0 && P >= 0 && P < TemP)) {
                            P = TemP;
                            N[x].sub = QN[i];
                            N[x].inv = false;
                            N[x].error = error;
                            N[x].reducearea = area;
                        }
                    } else {
                        area = area - 1;
                        TemP = area / (N[x].inverterror);
                        if ((N[x].inverterror < 0 && P > TemP) || (N[x].inverterror >= 0 && P >= 0 && P < TemP)) {
                            P = TemP;
                            N[x].sub = QN[i];
                            N[x].inv = true;
                            N[x].error = N[x].inverterror;
                            N[x].reducearea = area;
                        }
                    }

                }
            }
        }
    }
    if(N[x].g==gbuf&&N[N[x].in[0]].type==in){
        N[x].P=P;
        return P;
    }
    int innum=0;
    for(i=2;i<I;i++){
        innum=i/64;
        if(((N[x].inloc[innum]>>(i%64)))%2){
            error=AddErrorWithSASIMI(x,i);
            //error=AccurateMeasureError(x,i);
            if(error<=ERROR||(N[x].inverterror)<=ERROR){
                area=MeasureArea(x,i)-1;
                if(error<=(N[x].inverterror)){
                    TemP=area/error;
                    if((error<0&&P>TemP)||(error>=0&&P>=0&&P<TemP)){
                        P=TemP;
                        N[x].sub=i;
                        N[x].inv=false;
                        N[x].error=error;
                        N[x].reducearea=area;
                    }
                }
                else{
                    TemP=area/(N[x].inverterror);
                    if((N[x].inverterror<0&&P>TemP)||(N[x].inverterror>=0&&P>=0&&P<TemP)){
                        P=TemP;
                        N[x].sub=i;
                        N[x].inv=true;
                        N[x].error=N[x].inverterror;
                        N[x].reducearea=area;
                    }
                }

            }
        }
    }
    N[x].P=P;
    return P;
}


double MAP::Measure(int x,double nowerror) {
    int i = 0, j;
    double P = 0;
    double error = 0;
    double TemP;
    double area = 0;
    bool flag;
    //MeasureTwoVote(x);
    //MeasureAdjustVote(x,FLOOR);
    nowerror = 0;
    for (i = 0; i < 2; i++) { //consider 0 and 1
        error = AddError(x, i);
        //error=AccurateMeasureError(x,i);
        if (error + nowerror <= ERROR) {
            area = MeasureArea(x, i);
            TemP = area / error;
            if ((error < 0 && P > TemP) || (error >= 0 && P >= 0 && P < TemP)) {
                P = TemP;
                N[x].sub = i;
                N[x].inv = false;
                N[x].error = error;
                N[x].reducearea = area;
            }
        }
    }
    for (i = 0; i < (int) QN.size(); i++){
        if (N[QN[i]].delay <= (N[x].delay) && QN[i] != x) {
            flag = false;
            for (j = 0; j < inlong; j++){
                if ((N[QN[i]].inloc[j] & N[x].inloc[j]) > 0) {
                    flag = true;
                    break;
                }
            }

            if (N[x].g == gbuf && N[x].in[0] == QN[i])
                flag = false;
            for (j = 0; j < N[x].in.size(); j++)
                if (N[x].in[j] == QN[i])
                    flag = false;
            if (flag) {
                error = AddError(x, QN[i]);
                //error=AccurateMeasureError(x,QN[i]);
                if (error + nowerror <= ERROR ||
                    ((N[x].inverterror) + nowerror <= ERROR && N[x].delay >= N[QN[i]].delay + 1)) {
                    area = MeasureArea(x, QN[i]);
                    if (error <= N[x].inverterror) {
                        if (N[QN[i]].type == out)
                            area = area - 1;
                        TemP = area / error;
                        if ((error < 0 && P > TemP) || (error >= 0 && P >= 0 && P < TemP)) {
                            P = TemP;
                            N[x].sub = QN[i];
                            N[x].inv = false;
                            N[x].error = error;
                            N[x].reducearea = area;
                        }
                    } else if(N[x].delay>=N[QN[i]].delay+1) {
                        area = area - 1;
                        TemP = area / (N[x].inverterror);
                        if ((N[x].inverterror < 0 && P > TemP) || (N[x].inverterror >= 0 && P >= 0 && P < TemP)) {
                            P = TemP;
                            N[x].sub = QN[i];
                            N[x].inv = true;
                            N[x].error = N[x].inverterror;
                            N[x].reducearea = area;
                        }
                    }

                }
            }
        }
    }
    if(N[x].g==gbuf&&N[N[x].in[0]].type==in){
        N[x].P=P;
        return P;
    }
    int innum=0;
    for(i=2;i<I;i++){
        innum=i/64;
        if(((N[x].inloc[innum]>>(i%64)))%2){
            error=AddError(x,i);
            //error=AccurateMeasureError(x,i);
            if(error+nowerror<=ERROR||(N[x].inverterror)+nowerror<=ERROR){
                area=MeasureArea(x,i)-1;
                if(error<=(N[x].inverterror)){
                    TemP=area/error;
                    if((error<0&&P>TemP)||(error>=0&&P>=0&&P<TemP)){
                        P=TemP;
                        N[x].sub=i;
                        N[x].inv=false;
                        N[x].error=error;
                        N[x].reducearea=area;
                    }
                }
                else{
                    TemP=area/(N[x].inverterror);
                    if((N[x].inverterror<0&&P>TemP)||(N[x].inverterror>=0&&P>=0&&P<TemP)){
                        P=TemP;
                        N[x].sub=i;
                        N[x].inv=true;
                        N[x].error=N[x].inverterror;
                        N[x].reducearea=area;
                    }
                }

            }
        }
    }
    N[x].P=P;
    return P;
}


double MAP::MeasureWithAEM(int x,double nowerror) {
    int i = 0, j;
    double P = 0;
    double error = 0;
    double TemP;
    double area = 0;
    bool flag;
    //MeasureTwoVote(x);
    MeasureAdjustVote(x,FLOOR);
    nowerror = 0;
    if(HIGESTORDER <= 64){
        unsigned long maxvalue = ((unsigned long)1<<HIGESTORDER) - 1;
        for(i=0; i<SimValue; i++){
            if(N[x].V1[i] > maxvalue)
                return 0;
        }
    }

    for (i = 0; i < 2; i++) { //consider 0 and 1
        error = AEMAddError(x, i);
        //error=AccurateMeasureError(x,i);
        if (error + nowerror <= AEMThreshold) {
            area = MeasureArea(x, i);
            TemP = area / error;
            if ((error < 0 && P > TemP) || (error >= 0 && P >= 0 && P < TemP)) {
                P = TemP;
                N[x].sub = i;
                N[x].inv = false;
                N[x].error = error;
                N[x].reducearea = area;
            }
        }
    }
    for (i = 0; i < (int) QN.size(); i++){
        if (N[QN[i]].delay <= (N[x].delay) && QN[i] != x) {
            flag = false;
            for (j = 0; j < inlong; j++){
                if ((N[QN[i]].inloc[j] & N[x].inloc[j]) > 0) {
                    flag = true;
                    break;
                }
            }

            if (N[x].g == gbuf && N[x].in[0] == QN[i])
                flag = false;
            for (j = 0; j < N[x].in.size(); j++)
                if (N[x].in[j] == QN[i])
                    flag = false;
            if (flag) {
                error = AEMAddError(x, QN[i]);
                //error=AccurateMeasureError(x,QN[i]);
                if (error + nowerror <= AEMThreshold ||
                    ((N[x].inverterror) + nowerror <= AEMThreshold && N[x].delay >= N[QN[i]].delay + 1)) {
                    area = MeasureArea(x, QN[i]);
                    if (error <= N[x].inverterror) {
                        if (N[QN[i]].type == out)
                            area = area - 1;
                        TemP = area / error;
                        if ((error < 0 && P > TemP) || (error >= 0 && P >= 0 && P < TemP)) {
                            P = TemP;
                            N[x].sub = QN[i];
                            N[x].inv = false;
                            N[x].error = error;
                            N[x].reducearea = area;
                        }
                    } else if(N[x].delay>=N[QN[i]].delay+1){
                        area = area - 1;
                        TemP = area / (N[x].inverterror);
                        if ((N[x].inverterror < 0 && P > TemP) || (N[x].inverterror >= 0 && P >= 0 && P < TemP)) {
                            P = TemP;
                            N[x].sub = QN[i];
                            N[x].inv = true;
                            N[x].error = N[x].inverterror;
                            N[x].reducearea = area;
                        }
                    }

                }
            }
        }
    }
    if(N[x].g==gbuf&&N[N[x].in[0]].type==in){
        N[x].P=P;
        return P;
    }
    int innum=0;
    for(i=2;i<I;i++){
        innum=i/64;
        if(((N[x].inloc[innum]>>(i%64)))%2){
            error=AEMAddError(x,i);
            //error=AccurateMeasureError(x,i);
            if(error+nowerror<=AEMThreshold||(N[x].inverterror)+nowerror<=AEMThreshold){
                area=MeasureArea(x,i)-1;
                if(error<=(N[x].inverterror)){
                    TemP=area/error;
                    if((error<0&&P>TemP)||(error>=0&&P>=0&&P<TemP)){
                        P=TemP;
                        N[x].sub=i;
                        N[x].inv=false;
                        N[x].error=error;
                        N[x].reducearea=area;
                    }
                }
                else{
                    TemP=area/(N[x].inverterror);
                    if((N[x].inverterror<0&&P>TemP)||(N[x].inverterror>=0&&P>=0&&P<TemP)){
                        P=TemP;
                        N[x].sub=i;
                        N[x].inv=true;
                        N[x].error=N[x].inverterror;
                        N[x].reducearea=area;
                    }
                }

            }
        }
    }
    N[x].P=P;
    return P;
}



void MAP::MeasureOneVote(const int x) {
    int i,j;
    memset(N[x].V1,0,sizeof(unsigned long)*SimValue);
    if(Omax){
        memset(N[x].V2,0,sizeof(unsigned long)*SimValue);
    }
    if(O2max){
        memset(N[x].V2,0,sizeof(unsigned long)*SimValue);
        memset(N[x].V3,0,sizeof(unsigned long)*SimValue);
    }
    if(N[x].type==out)
    {
        unsigned  long t=(unsigned long)1<<((x-I)%64);
        if(O2max){
            if(x-I>127){
                for(i=0;i<SimValue;i++)
                    N[x].V3[i]=t;
            }
            else if(x-I>63){
                for(i=0;i<SimValue;i++)
                    N[x].V2[i]=t;
            }
            else{
                for(i=0;i<SimValue;i++)
                    N[x].V1[i]=t;
            }
        }
        else if(Omax)
        {
            if(x-I>63){
                for(i=0;i<SimValue;i++)
                    N[x].V2[i]=t;
            }
            else{
                for(i=0;i<SimValue;i++)
                    N[x].V1[i]=t;
            }
        }
        else{
            for(i=0;i<SimValue;i++){
                N[x].V1[i]=t;
            }
        }
    }
    int fx;
    bool BD[SimValue];
    for(i=0;i<SimValue;i++){
        N[x].I[i]=!N[x].I[i];
    }
    for(i=0;i<N[x].out.size();i++){
        fx=N[x].out[i];
        ComputeBooleanDifference(BD,fx,N);
        if(O2max){
            for(j=0;j<SimValue;j++){
                if(BD[j]){
                    N[x].V1[j] |= N[fx].V1[j];
                    N[x].V2[j] |= N[fx].V2[j];
                    N[x].V3[j] |= N[fx].V3[j];
                }
            }
        }
        else if(Omax){
            for(j=0;j<SimValue;j++){
                if(BD[j]){
                    N[x].V1[j] |= N[fx].V1[j];
                    N[x].V2[j] |= N[fx].V2[j];
                }
            }
        }
        else{
            for(j=0;j<SimValue;j++){
                if(BD[j]){
                    N[x].V1[j] |= N[fx].V1[j];
                }
            }
        }
    }
    for(i=0;i<SimValue;i++){
        N[x].I[i]=!N[x].I[i];
    }
}


void MAP::MeasureTwoVote(const int x) {
    int i,j;
    memset(N[x].V1,0,sizeof(unsigned long)*SimValue);
    if(Omax){
        memset(N[x].V2,0,sizeof(unsigned long)*SimValue);
    }
    if(O2max){
        memset(N[x].V2,0,sizeof(unsigned long)*SimValue);
        memset(N[x].V3,0,sizeof(unsigned long)*SimValue);
    }
    if(N[x].type==out)
    {
        unsigned  long t=(unsigned long)1<<((x-I)%64);
        if(O2max){
            if(x-I>127){
                for(i=0;i<SimValue;i++)
                    N[x].V3[i]=t;
            }
            else if(x-I>63){
                for(i=0;i<SimValue;i++)
                    N[x].V2[i]=t;
            }
            else{
                for(i=0;i<SimValue;i++)
                    N[x].V1[i]=t;
            }
        }
        else if(Omax)
        {
            if(x-I>63){
                for(i=0;i<SimValue;i++)
                    N[x].V2[i]=t;
            }
            else{
                for(i=0;i<SimValue;i++)
                    N[x].V1[i]=t;
            }
        }
        else{
            for(i=0;i<SimValue;i++){
                N[x].V1[i]=t;
            }
        }
    }
    int fx;
    bool BD[SimValue];

    vector<int> reconvparent;
    map<int,int> Hmap;
    for(i=0;i<N[x].out.size();i++){
        fx=N[x].out[i];
        if(Hmap.find(fx)!=Hmap.end()){
            (Hmap.find(fx)->second)++;
        }
        else{
            Hmap[fx]=1;
        }
        for(j=0;j<N[fx].out.size();j++){
            if(Hmap.find(N[fx].out[j])!=Hmap.end()){
                (Hmap.find(N[fx].out[j])->second)++;
            }
            else{
                Hmap[N[fx].out[j]]=1;
            }
        }
    }
    for(map<int,int>::iterator iter=Hmap.begin();iter!=Hmap.end();iter++){
        if(iter->second>1)
            reconvparent.push_back(iter->first);
        //if(iter->second>2)
        //printf("There are %d paths\n",iter->second);
    }

    if(!reconvparent.empty())
        ReconvergeTwoPath(x,Hmap);
    else{
        for(i=0;i<SimValue;i++){
            N[x].I[i]=!N[x].I[i];
        }
        for(i=0;i<N[x].out.size();i++){
            fx=N[x].out[i];
            ComputeBooleanDifference(BD,fx,N);
            if(O2max){
                for(j=0;j<SimValue;j++){
                    if(BD[j]){
                        N[x].V1[j] |= N[fx].V1[j];
                        N[x].V2[j] |= N[fx].V2[j];
                        N[x].V3[j] |= N[fx].V3[j];
                    }
                }
            }
            else if(Omax){
                for(j=0;j<SimValue;j++){
                    if(BD[j]){
                        N[x].V1[j] |= N[fx].V1[j];
                        N[x].V2[j] |= N[fx].V2[j];
                    }
                }
            }
            else{
                for(j=0;j<SimValue;j++){
                    if(BD[j]){
                        N[x].V1[j] |= N[fx].V1[j];
                    }
                }
            }

        }
        for(i=0;i<SimValue;i++){
            N[x].I[i]=!N[x].I[i];
        }
    }
}

void MAP::ReconvergeTwoPath(int x,map<int,int> &Hmap) {
    int fx,ffx;
    bool Flag=true;
    map<int,int> Tempmap;
    N[x].temp=new bool[SimValue];
    vector<int> Parx;
    for(int i=0;i<SimValue;i++)
        N[x].temp[i]=!N[x].I[i];
    for(map<int,int>::iterator iter=Hmap.begin();iter!=Hmap.end();iter++){
        Tempmap[iter->first]=0;
    }
    for(int i=0;i<N[x].out.size();i++){
        Flag=false;
        fx=N[x].out[i];
        Tempmap.find(fx)->second++;
        if(Tempmap.find(fx)->second==Hmap.find(fx)->second){
            N[fx].temp=new bool[SimValue];
            ComputeTempNodeValue(fx);
            if(Hmap.find(fx)->second>1){
                Parx.push_back(fx);
                Flag=true;
            }

        }
        for(int j=0;j<N[fx].out.size();j++){
            ffx=N[fx].out[j];
            Tempmap.find(ffx)->second++;
            if(Tempmap.find(ffx)->second==Hmap.find(ffx)->second&&(!Flag||Hmap.find(ffx)->second>1)){
                N[ffx].temp=new bool[SimValue];
                ComputeTempNodeValue(ffx);
                Parx.push_back(ffx);
            }
        }
    }
    if(O2max){
        for(int i=0;i<Parx.size();i++){
            fx=Parx[i];
            for(int j=0;j<SimValue;j++){
                if(N[fx].I[j]!=N[fx].temp[j]){
                    N[x].V1[j] |= N[fx].V1[j];
                    N[x].V2[j] |= N[fx].V2[j];
                    N[x].V3[j] |= N[fx].V3[j];
                }
            }
        }
    }
    else if(Omax){
        for(int i=0;i<Parx.size();i++){
            fx=Parx[i];
            for(int j=0;j<SimValue;j++){
                if(N[fx].I[j]!=N[fx].temp[j]){
                    N[x].V1[j] |= N[fx].V1[j];
                    N[x].V2[j] |= N[fx].V2[j];
                }
            }
        }
    }
    else{
        for(int i=0;i<Parx.size();i++){
            fx=Parx[i];
            for(int j=0;j<SimValue;j++){
                if(N[fx].I[j]!=N[fx].temp[j]){
                    N[x].V1[j] |= N[fx].V1[j];
                }
            }
        }
    }
    for(map<int,int>::iterator iter=Hmap.begin();iter!=Hmap.end();iter++){
        delete [] N[iter->first].temp;
        N[iter->first].temp=NULL;
    }
    delete [] N[x].temp;
    N[x].temp=NULL;
}

void MAP::ComputeTempNodeValue(int x) {
    int i,j,k;
    errornode++;
    switch (N[x].g){
        case ginv:
            if(N[N[x].in[0]].temp==NULL)
                for(j=0;j<SimValue;j++)
                    N[x].temp[j]=!N[N[x].in[0]].I[j];
            else
                for(j=0;j<SimValue;j++)
                    N[x].temp[j]=!N[N[x].in[0]].temp[j];
            break;
        case gnand:
            for(j=0;j<SimValue;j++)
            {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
                for(k=1;k<(int)N[x].in.size();k++)
                    N[x].temp[j]=N[x].temp[j]&&(N[N[x].in[k]].temp==NULL?N[N[x].in[k]].I[j]:N[N[x].in[k]].temp[j]);
                N[x].temp[j]=!N[x].temp[j];
            }
            break;
        case gnor:
            for (j=0;j<SimValue;j++) {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
                for(k=1;k<(int)N[x].in.size();k++)
                    N[x].temp[j]=N[x].temp[j]||(N[N[x].in[k]].temp==NULL?N[N[x].in[k]].I[j]:N[N[x].in[k]].temp[j]);
                N[x].temp[j]=!N[x].temp[j];
            }
            break;
        case gand:
            for (j=0;j<SimValue;j++) {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
                N[x].temp[j]=N[x].temp[j]&&(N[N[x].in[1]].temp==NULL?N[N[x].in[1]].I[j]:N[N[x].in[1]].temp[j]);
            }
            break;
        case gor:
            for (j=0;j<SimValue;j++) {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
                N[x].temp[j]=N[x].temp[j]||(N[N[x].in[1]].temp==NULL?N[N[x].in[1]].I[j]:N[N[x].in[1]].temp[j]);
            }
            break;
        case gxor:
            for (j=0;j<SimValue;j++) {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
                N[x].temp[j]=N[x].temp[j]!=(N[N[x].in[1]].temp==NULL?N[N[x].in[1]].I[j]:N[N[x].in[1]].temp[j]);
            }
            break;
        case gxnor:
            for (j=0;j<SimValue;j++) {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
                N[x].temp[j]=N[x].temp[j]==(N[N[x].in[1]].temp==NULL?N[N[x].in[1]].I[j]:N[N[x].in[1]].temp[j]);
            }
            break;
        case gaoi21:
            for (j=0;j<SimValue;j++) {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
                N[x].temp[j]=N[x].temp[j]&&(N[N[x].in[1]].temp==NULL?N[N[x].in[1]].I[j]:N[N[x].in[1]].temp[j]);
                N[x].temp[j]=!(N[x].temp[j]||(N[N[x].in[2]].temp==NULL?N[N[x].in[2]].I[j]:N[N[x].in[2]].temp[j]));
            }
            break;
        case gaoi22:
            for (j=0;j<SimValue;j++) {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
                N[x].temp[j]=N[x].temp[j]&&(N[N[x].in[1]].temp==NULL?N[N[x].in[1]].I[j]:N[N[x].in[1]].temp[j]);
                N[x].temp[j]=!(N[x].temp[j]||((N[N[x].in[2]].temp==NULL?N[N[x].in[2]].I[j]:N[N[x].in[2]].temp[j])&&(N[N[x].in[3]].temp==NULL?N[N[x].in[3]].I[j]:N[N[x].in[3]].temp[j])));
            }
            break;
        case goai21:
            for (j=0;j<SimValue;j++) {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
                N[x].temp[j]=N[x].temp[j]||(N[N[x].in[1]].temp==NULL?N[N[x].in[1]].I[j]:N[N[x].in[1]].temp[j]);
                N[x].temp[j]=!(N[x].temp[j]&&(N[N[x].in[2]].temp==NULL?N[N[x].in[2]].I[j]:N[N[x].in[2]].temp[j]));
            }
            break;
        case goai22:
            for (j=0;j<SimValue;j++) {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
                N[x].temp[j]=N[x].temp[j]||(N[N[x].in[1]].temp==NULL?N[N[x].in[1]].I[j]:N[N[x].in[1]].temp[j]);
                N[x].temp[j]=!(N[x].temp[j]&&((N[N[x].in[2]].temp==NULL?N[N[x].in[2]].I[j]:N[N[x].in[2]].temp[j])||(N[N[x].in[3]].temp==NULL?N[N[x].in[3]].I[j]:N[N[x].in[3]].temp[j])));
            }
            break;
        case gbuf:
            for (j=0;j<SimValue;j++) {
                N[x].temp[j]=N[N[x].in[0]].temp==NULL?N[N[x].in[0]].I[j]:N[N[x].in[0]].temp[j];
            }
            break;
        default: cout<<"There are no gate for this node!"<<endl;
    }
}

static inline void ComputeBooleanDifference(bool *BD, const int x, const TYPE & N) {
    int j,k;
    bool temp;
    switch (N[x].g){
        case ginv:
            for(j=0;j<SimValue;j++)
                BD[j]=N[x].I[j]==N[N[x].in[0]].I[j];
            break;
        case gnand:
            for(j=0;j<SimValue;j++)
            {
                temp=N[N[x].in[0]].I[j];
                for(k=1;k<(int)N[x].in.size();k++)
                    temp=temp&&N[N[x].in[k]].I[j];
                BD[j]=N[x].I[j]==temp;
            }
            break;
        case gnor:
            for (j=0;j<SimValue;j++) {
                temp=N[N[x].in[0]].I[j];
                for(k=1;k<(int)N[x].in.size();k++)
                    temp=temp||N[N[x].in[k]].I[j];
                BD[j]=N[x].I[j]==temp;
            }
            break;
        case gand:
            for (j=0;j<SimValue;j++) {
                BD[j]=N[x].I[j]!=(N[N[x].in[0]].I[j]&&N[N[x].in[1]].I[j]);
            }
            break;
        case gor:
            for (j=0;j<SimValue;j++) {
                BD[j]=N[x].I[j]!=(N[N[x].in[0]].I[j]||N[N[x].in[1]].I[j]);
            }
            break;
        case gxor:
            for (j=0;j<SimValue;j++) {
                BD[j]=N[x].I[j]!=(N[N[x].in[0]].I[j]!=N[N[x].in[1]].I[j]);
            }
            break;
        case gxnor:
            for (j=0;j<SimValue;j++) {
                BD[j]=N[x].I[j]!=(N[N[x].in[0]].I[j]==N[N[x].in[1]].I[j]);
            }
            break;
        case gaoi21:
            for (j=0;j<SimValue;j++) {
                BD[j]=N[x].I[j]==(N[N[x].in[0]].I[j]&&N[N[x].in[1]].I[j]||N[N[x].in[2]].I[j]);
            }
            break;
        case gaoi22:
            for (j=0;j<SimValue;j++) {
                BD[j]=N[x].I[j]==(N[N[x].in[0]].I[j]&&N[N[x].in[1]].I[j]||(N[N[x].in[2]].I[j]&&N[N[x].in[3]].I[j]));
            }
            break;
        case goai21:
            for (j=0;j<SimValue;j++) {
                BD[j]=N[x].I[j]==((N[N[x].in[0]].I[j]||N[N[x].in[1]].I[j])&&N[N[x].in[2]].I[j]);
            }
            break;
        case goai22:
            for (j=0;j<SimValue;j++) {
                BD[j]=N[x].I[j]==((N[N[x].in[0]].I[j]||N[N[x].in[1]].I[j])&&(N[N[x].in[2]].I[j]||N[N[x].in[3]].I[j]));
            }
            break;
        case gbuf:
            for (j=0;j<SimValue;j++) {
                BD[j]=true;
            }
            break;
        case gzero:
            for (j=0;j<SimValue;j++) {
                BD[j]= false;
            }
            break;
        case gone:
            for (j=0;j<SimValue;j++) {
                BD[j]= false;
            }
            break;
        default: cout<<"There are no gate for this node!"<<endl;
    }
}

double MAP::ReduceArea(const int l) {
    double area=0;
    if(N[l].out.size()==1&&N[l].type==intern)
    {
        area++;
        for(int i=0;i<(int)N[l].in.size();i++)
            area+=ReduceArea(N[l].in[i]);
    }
    return area;
}


bool MAP::IterateArea(int sub, int x) {
    if(sub==x)
        return false;
    bool flag=true;
    for(int i=0;i<N[sub].in.size();i++){
        flag=IterateArea(N[sub].in[i],x);
        if(!flag)
            return false;
    }
    return true;
}

double MAP::MeasureArea(const int x, const int sub) {  //The delay and area should be reconsidered
    clock_t tempTime=clock();
    double area=N[x].area;
    int i,j;
    bool Flag;
    bool subflag=false;
    for(i=0;i<(int)N[x].mffc.size();i++){
        Flag=true;
        if(N[x].mffc[i]==sub){
            Flag= false;
            subflag=true;
        }
        if(subflag&&Flag){  //in case that sub is also x's mffc and its input is also x's mffc
            for(j=0;j<N[sub].in.size();j++){
                if(!IterateArea(N[sub].in[j],N[x].mffc[i])){
                    Flag=false;
                    break;
                }
            }
        }
        for(j=0;j<(int)N[sub].mffc.size()&&Flag;j++){
            if(N[x].mffc[i]==N[sub].mffc[j]){
                Flag= false;
                break;
            }
        }
        if(Flag){
            area+=N[N[x].mffc[i]].area;
        }

    }
    //area=lambda*(area/MaxArea)+(1-lambda)*(N[x].delay-N[sub].delay)/MaxDelay;
    areaTime+=(clock()-tempTime);
    return area;
}


double MAP::AddErrorWithSASIMI(const int x, const int sub) {
    clock_t tempTime=clock();
    errornode++;
    int sum=0,j;
    bool flag;
    N[x].inverterror=1;
    int invertsum=0;
    for(int i=0; i<SimValue; i++){
        if(N[x].I[i] != N[sub].I[i])
            sum++;
        else
            invertsum++;
    }
    N[x].inverterror=(double)invertsum/SimValue;
    errorTime+=(clock()-tempTime);
    return (double)sum/SimValue;
}


double MAP::AEMAddError(const int x, const int sub) {
    clock_t tempTime=clock();
    errornode++;
    long sum=0;
    int j;
    N[x].inverterror=1;
    long invertsum=0;
    if(O2max){
        cout<<"Error! Output more than 128!\n";
    }
    else if(Omax){
        cout<<"Error! Output more than 64!\n";
    }
    else{
        for(int i=0;i<SimValue;i++){
            if(N[x].V1[i]){
                if(N[x].I[i]!=N[sub].I[i]){
                    if(ErrFlag[i]){
                        sum-=abs((long)(TrueValue[i]-AEMTempValue[i]));
                        sum+=abs((long)((AEMTempValue[i]^N[x].V1[i])-TrueValue[i]));
                    }
                    else
                        sum+=abs((long)((TrueValue[i]^N[x].V1[i])-TrueValue[i]));
                }
                else{
                    if(ErrFlag[i]){
                        invertsum-=abs((long)(TrueValue[i]-AEMTempValue[i]));
                        invertsum+=abs((long)((AEMTempValue[i]^N[x].V1[i])-TrueValue[i]));
                    }
                    else
                        invertsum+=abs((long)((TrueValue[i]^N[x].V1[i])-TrueValue[i]));
                }
            }
        }
    }
    N[x].inverterror=(double)invertsum/SimValue;
    errorTime+=(clock()-tempTime);
    return (double)sum/SimValue;
}


double MAP::AddError(const int x, const int sub) {
    clock_t tempTime=clock();
    errornode++;
    int sum=0,j;
    bool flag;
    N[x].inverterror=1;
    int invertsum=0;
    if(O2max){
        for(int i=0;i<SimValue;i++){
            if((N[x].V1[i]||N[x].V2[i]||N[x].V3[i])) {
                if (N[x].I[i] != N[sub].I[i]) {
                    if (ErrFlag[i]) {
                        flag = true;
                        for (j = 0; j < 64; j++)
                            if ((N[x].V1[i] >> j) % 2 ^ N[I + j].W[i])
                                flag = false;
                        for (j = 0; j < 64; j++)
                            if ((N[x].V2[i] >> j) % 2 ^ N[I + 64 + j].W[i])
                                flag = false;
                        for (j = 0; j < O - 128; j++)
                            if ((N[x].V3[i] >> j) % 2 ^ N[I + 128 + j].W[i])
                                flag = false;
                        if (flag)
                            sum--;
                    } else {
                        sum++;
                    }
                }
                else if(sub!=0&&sub!=1){
                    if(ErrFlag[i]){
                        flag=true;
                        for (j = 0; j < 64; j++)
                            if ((N[x].V1[i] >> j) % 2 ^ N[I + j].W[i])
                                flag = false;
                        for (j = 0; j < 64; j++)
                            if ((N[x].V2[i] >> j) % 2 ^ N[I + 64 + j].W[i])
                                flag = false;
                        for (j = 0; j < O - 128; j++)
                            if ((N[x].V3[i] >> j) % 2 ^ N[I + 128 + j].W[i])
                                flag = false;
                        if(flag)
                            invertsum--;
                    }
                    else{
                        invertsum++;
                    }
                }
            }
        }
    }
    else if(Omax){
        for(int i=0;i<SimValue;i++) {
            if (N[x].V1[i] || N[x].V2[i]){
                if(N[x].I[i] != N[sub].I[i]) {
                    if (ErrFlag[i]) {
                        flag = true;
                        for (j = 0; j < 64; j++)
                            if ((N[x].V1[i] >> j) % 2 ^ N[I + j].W[i])
                                flag = false;
                        for (j = 0; j < O - 64; j++)
                            if ((N[x].V2[i] >> j) % 2 ^ N[I + 64 + j].W[i])
                                flag = false;
                        if (flag){
                            sum--;
                        }
                    } else {
                        sum++;
                    }
                }
                else if(sub!=0&&sub!=1){
                    if (ErrFlag[i]) {
                        flag = true;
                        for (j = 0; j < 64; j++)
                            if ((N[x].V1[i] >> j) % 2 ^ N[I + j].W[i])
                                flag = false;
                        for (j = 0; j < O - 64; j++)
                            if ((N[x].V2[i] >> j) % 2 ^ N[I + 64 + j].W[i])
                                flag = false;
                        if (flag)
                            invertsum--;
                    } else {
                        invertsum++;
                    }
                }
            }
        }

    }
    else{
        for(int i=0;i<SimValue;i++) {
            if (N[x].V1[i]) {
                if (N[x].I[i] != N[sub].I[i]) {
                    if (ErrFlag[i]) {
                        flag = true;
                        for (j = 0; j < O; j++)
                            if ((N[x].V1[i] >> j) % 2 ^ N[I + j].W[i])
                                flag = false;
                        if (flag)
                            sum--;
                    } else {
                        sum++;
                    }
                } else if (sub != 0 && sub != 1) {
                    if (ErrFlag[i]) {
                        flag = true;
                        for (j = 0; j < O; j++)
                            if ((N[x].V1[i] >> j) % 2 ^ N[I + j].W[i])
                                flag = false;
                        if (flag)
                            invertsum--;
                    } else {
                        invertsum++;
                    }
                }
            }
        }


    }
    /*for(int i=0;i<SimValue;i++){
	if(N[x].I[i]!=N[sub].I[i])
		sum++;
    }*/
    N[x].inverterror=(double)invertsum/SimValue;
    errorTime+=(clock()-tempTime);
    return (double)sum/SimValue;
}

void MAP::DeleteNode(const int x) {
    int sub=N[x].sub;
    int i,j,p,sx,ssx;
    bool subflag=false;
    for(i=0;i<N[x].mffc.size();i++){
        for(p=0;p<N[sub].mffc.size();p++){
            if(N[sub].mffc[p]==N[x].mffc[i])
                break;
        }
        if(N[x].mffc[i]==sub){
            p=-1;
            subflag=true;
        }
        if(subflag&&p>=(int)N[sub].mffc.size()){
            for(j=0;j<N[sub].in.size();j++)
                if(!IterateArea(N[sub].in[j],N[x].mffc[i]))
                    p=-1;
        }
        if(p>=(int)N[sub].mffc.size()){
            sx=N[x].mffc[i];
            N[sx].type=in;
            N[sx].g=gzero;
            N[sx].out.clear();
            GateNum--;
            for(j=0;j<N[sx].in.size();j++){
                ssx=N[sx].in[j];
                for(vector<int>::iterator k=N[ssx].out.begin();k!=N[ssx].out.end();k++)
                    if((*k)==sx){
                        N[ssx].out.erase(k);
                        break;
                    }
            }
            N[sx].in.clear();
        }
    }
    for(i=0;i<N[x].in.size();i++){
        sx=N[x].in[i];
        for(vector<int>::iterator k=N[sx].out.begin();k!=N[sx].out.end();k++)
            if((*k)==x){
                N[sx].out.erase(k);
                break;
            }
    }
    if(N[x].inv){
        N[x].in.clear();
        N[x].g=ginv;
        N[x].gatename="inv1";
        N[x].in.push_back(sub);
        N[x].area=1;
        N[x].gatedelay=1;
        for(i=0;i<N[sub].out.size();i++){
            if(N[sub].out[i]==x)
                break;
        }
        if(i>=N[sub].out.size())
            N[sub].out.push_back(x);

        if(N[sub].type==in){
            for(i=0;i<inlong;i++)
                N[x].inloc[i]=0;
            int l=sub/64;
            N[x].inloc[l]=(unsigned long)1<<(sub%64);
        }
        else{
            for(i=0;i<inlong;i++)
                N[x].inloc[i]=N[sub].inloc[i];
        }
    }
    else{
        if(N[sub].type!=intern&&sub>1){
            if(N[sub].type==in){
                for(i=0;i<inlong;i++)
                    N[x].inloc[i]=0;
                int l=sub/64;
                N[x].inloc[l]=(unsigned long)1<<(sub%64);
            }
            else{
                for(i=0;i<inlong;i++)
                    N[x].inloc[i]=N[sub].inloc[i];
            }
            N[x].in.clear();
            N[x].g=gbuf;
            N[x].gatename="buf";
            N[x].in.push_back(sub);
            N[x].area=1;
            N[x].gatedelay=1;
            for(i=0;i<N[sub].out.size();i++){
                if(N[sub].out[i]==x)
                    break;
            }
            if(i>=N[sub].out.size())
                N[sub].out.push_back(x);
        }
        else if(sub<=1){
            N[x].in.clear();
            N[x].g=N[sub].g;
            N[x].gatename=N[sub].gatename;
            N[x].area=0;
            N[x].gatedelay=0;
            N[x].delay=0;
            for(i=0;i<inlong;i++)
                N[x].inloc[i]=0;
        }
        else{
            N[x].g=N[sub].g;
            N[x].area=N[sub].area;
            N[x].gatedelay=N[sub].gatedelay;
            N[x].gatename=N[sub].gatename;
            N[x].in.clear();
            for(i=0;i<N[sub].in.size();i++){
                sx=N[sub].in[i];
                N[x].in.push_back(sx);
                for(vector<int>::iterator k=N[sx].out.begin();k!=N[sx].out.end();k++){
                    if((*k)==sub){
                        N[sx].out.erase(k);
                        break;
                    }
                }
                for(j=0;j<N[sx].out.size();j++)
                    if(N[sx].out[j]==x)
                        break;
                if(j>=N[sx].out.size())
                    N[sx].out.push_back(x);
            }
            N[sub].in.clear();
            for(i=0;i<N[sub].out.size();i++){
                sx=N[sub].out[i];
                for(j=0;j<N[sx].in.size();j++)
                    if(N[sx].in[j]==x)
                        break;
                if(j>=N[sx].in.size())
                    N[x].out.push_back(sx);
                for(j=0;j<N[sx].in.size();j++)
                    if(N[sx].in[j]==sub){
                        N[sx].in[j]=x;
                    }
            }
            N[sub].out.clear();
            N[sub].type=in;
            for(i=0;i<inlong;i++)
                N[x].inloc[i]=N[sub].inloc[i];
            GateNum--;
        }
    }
}

void MAP::ComputeMFFC()
{
    // nodes should be arranged in topological order

    // init vars
    int * outDegree = (int *) malloc( N.size() * sizeof( int ) );
    if ( !outDegree )
        assert( 0 );
    memset( outDegree, 0, N.size() * sizeof( int ) );

    int * storeDegree = (int *) malloc( N.size() * sizeof( int ) );
    if ( !storeDegree )
        assert( 0 );
    memset( storeDegree, 0, N.size() * sizeof( int ) );

    int x, fanin;

    for ( unsigned int j = 0; j < QN.size(); j ++ ) {
        for ( unsigned int k = 0; k < N[QN[j]].in.size(); k ++ ) {
            fanin = N[QN[j]].in[k];
            storeDegree[fanin] ++;
        }
    }

    // compute MFFC, POs' MFFC will be included, n's MFFC excludes n and PIs
    for ( unsigned int i = 0; i < QN.size(); i ++ ) {
        x = QN[i];
        N[x].mffc.clear();
        // init out-degree for each nodes, do not use out.size() in order to deal with ( A and A = B ).
        /*memset( outDegree, 0, N.size() * sizeof( int ) );
        for ( unsigned int j = 0; j < QN.size(); j ++ ) {
            for ( unsigned int k = 0; k < N[QN[j]].in.size(); k ++ ) {
                fanin = N[QN[j]].in[k];
                outDegree[fanin] ++;
            }
        }*/
        for(unsigned int j = 0; j< N.size(); j++)
            outDegree[j] = storeDegree[j];
        IterationMFFC( x, x, outDegree, 0 );
    }

    // free memory
    free( outDegree );
    free(storeDegree);
//    for ( unsigned int i = 0; i < N.size(); i ++ ) {
//        if ( N[i].mffc.size() ) {
//            cout << N[i].var << ":";
//            for ( unsigned int j = 0; j < N[i].mffc.size(); j ++ )
//                cout << N[N[i].mffc[j]].var << " ";
//            cout << endl;
//        }
//    }

}

void MAP::IterationMFFC( int src, int x, int * outDegree, bool choice )
{
    int fanin;
    if ( N[x].type == in || ( N[x].type == out && src != x ) )
        return;
    if ( src != x )
        N[src].mffc.push_back( x );
    for ( unsigned int i = 0; i < N[x].in.size(); i ++ ) {
        fanin = N[x].in[i];
        assert( outDegree[fanin] > 0 );
        if ( --outDegree[fanin] == 0 ) {
            IterationMFFC(src, fanin, outDegree, 0);
        }
    }
}

/*void MAP::ComputeMFFC() {
    int num[N.size()-I];
    int i,j,x,k;
    vector<int> temp;
    for(i=0;i<QN.size();i++){
        memset(num,0,sizeof(int)*(N.size()-I));
        x=QN[i];
        N[x].mffc.clear();
        temp.clear();
        for(j=0;j<(int)N[x].in.size();j++)
            if(N[N[x].in[j]].type!=in){
                for(k=0;k<temp.size();k++)
                    if(temp[k]==N[x].in[j])
                        break;
                if(k>=temp.size())
                    num[N[x].in[j]-I]++;
                temp.push_back(N[x].in[j]);  // whether put temp into the if-part
            if(num[N[x].in[j]-I]==N[N[x].in[j]].out.size()&&N[N[x].in[j]].type!=out){
                N[x].mffc.push_back(N[x].in[j]);
                IterationMFFC(N[x].in[j],num,x);
            }
        }
    }
}

void MAP::IterationMFFC(int x, int *num,int fx) {
    vector<int> temp;
    int k;
    for(int j=0;j<(int)N[x].in.size();j++)
        if(N[N[x].in[j]].type!=in){
            for(k=0;k<temp.size();k++)
                if(temp[k]==N[x].in[j])
                    break;
            if(k>=temp.size())
                num[N[x].in[j]-I]++;
            temp.push_back(N[x].in[j]);
        if(num[N[x].in[j]-I]==N[N[x].in[j]].out.size()&&N[N[x].in[j]].type!=out){
            N[fx].mffc.push_back(N[x].in[j]);
            IterationMFFC(N[x].in[j],num,fx);
        }
    }
}*/

void MAP::EvaluateTI() {
    TI.clear(); // Chang modify, for reusing
    for(int i=I;i<I+O;i++){
        vector<bool> x(SimValue);
        for(int j=0;j<SimValue;j++)
            x[j]=N[i].I[j];
        TI.push_back(x);
    }
}

void MAP::CollectTI(MAP & map0) {
    this->TI = map0.TI; // Chang modify, copy vec <vec <bool> >
}

double MAP::SimError(const int x) {
    int sub=N[x].sub;
    NODEGATE gate=N[x].g;
    vector<int> in;
    bool Flag;
    double sum=0;
    for(int i=0;i<N[x].in.size();i++)
        in.push_back(N[x].in[i]);
    N[x].in.clear();
    if(N[x].inv)
        N[x].g=ginv;
    else
        N[x].g=gbuf;
    N[x].in.push_back(sub);
    SimulateNode();
    for(int j=0;j<SimValue;j++){
        Flag=false;
        for(int i=0;i<O;i++){
            if(N[i+I].I[j]!=TI[i][j]){
                N[i+I].W[j]=true;
                Flag=true;
            }
        }
        ErrFlag[j]=Flag;
        if(Flag){
            sum++;
        }
    }

    N[x].g=gate;
    N[x].in.clear();
    for(int i=0;i<in.size();i++)
        N[x].in.push_back(in[i]);
    double p = sum/SimValue;
    double up_bound=0;
    //double up_bound = Binomal_95 * sqrt(p*(1-p)/SimValue);
    //printf("The sample error is %lf, difference of upper bound is %lf, upper bound of 95%% confidence is  %lf\n", p, up_bound, up_bound+p);
    return p+up_bound;
}

double MAP::NewSimError(const int x, double pasterror) {
    bool TempErrFlag[SimValue];
    int sub=N[x].sub;
    NODEGATE gate=N[x].g;
    vector<int> in;
    bool Flag;
    double sum=0;
    for(int i=0;i<N[x].in.size();i++)
        in.push_back(N[x].in[i]);
    N[x].in.clear();
    if(N[x].inv)
        N[x].g=ginv;
    else
        N[x].g=gbuf;
    N[x].in.push_back(sub);
    SimulateNode();
    for(int j=0;j<SimValue;j++){
        Flag=false;
        for(int i=0;i<O;i++){
            if(N[i+I].I[j]!=TI[i][j]){
                N[i+I].W[j]=true;
                Flag=true;
            }
        }
        TempErrFlag[j]=ErrFlag[j];
        ErrFlag[j]=Flag;
        if(Flag){
            sum++;
        }
    }
    N[x].g=gate;
    N[x].in.clear();
    for(int i=0;i<in.size();i++)
        N[x].in.push_back(in[i]);
    double p = sum/SimValue;



    double up_bound=0;
    //double up_bound = Binomal_95 * sqrt(p*(1-p)/SimValue);
    //printf("The sample error is %lf, difference of upper bound is %lf, upper bound of 95%% confidence is  %lf\n", p, up_bound, up_bound+p);
    return p+up_bound;
}

double MAP::AccurateError() {
    SimulateNode();
    double sum=0;
    for(int j=0;j<SimValue;j++)
        for(int i=0;i<O;i++)
            if(N[i+I].I[j]!=TI[i][j]){
                sum++;
                break;
            }
    return sum/SimValue;
}

void MAP::ComputeDelayAndArea() {
    MaxDelay=0;
    MaxArea=0;
    int x,i,j;
    for(i=0;i<QN.size();i++){
        x=QN[i];
        N[x].delay=0;
        MaxArea+=N[x].area;
        for(j=0;j<N[x].in.size();j++){
            if(N[x].delay<(N[N[x].in[j]].delay+N[x].gatedelay))
                N[x].delay=N[N[x].in[j]].delay+N[x].gatedelay;
        }
        if(MaxDelay<N[x].delay)
            MaxDelay=N[x].delay;
    }
}

void MAP::TopologicalSorting() {
    queue<int> Q;
    QN.clear();
    int num[N.size()];
    int x,i,j,index;
    memset(num,0,sizeof(int)*N.size());
    for(i=0;i<I;i++){
        Q.push(i);
    }
    for(i=I;i<N.size();i++){
        if((N[i].g==gzero||N[i].g==gone)&&N[i].type!=in){
            QN.push_back(i);
            N[i].QNL=QN.size()-1;
            Q.push(i);
        }
    }
    while(!Q.empty()){
        x=Q.front();
        Q.pop();
        for(i=0;i<N[x].out.size();i++){
            index=N[x].out[i];
            for(j=0;j<N[index].in.size();j++)
                if(N[index].in[j]==x)
                    num[index]++;
            if(num[index]==N[index].in.size()){
                Q.push(index);
                if(N[index].type!=in){
                    QN.push_back(index);
                    N[index].QNL=QN.size()-1;
                }

            }

        }
    }
}


void MAP::EvaluateTrueValue() {
    for(int i=0;i<SimValue;i++){
        TrueValue[i]=0;
        for(int j=I;j<I+O;j++)
            TrueValue[i]+=(N[j].I[i]?(unsigned long)1<<(j-I):0);
        AEMTempValue[i] = TrueValue[i];
    }
}


void MAP::CollectTrueValue(MAP & map0) {
    for(int i=0;i<SimValue;i++)
        this->TrueValue[i] = map0.TrueValue[i];
}


double MAP::AEMSimError(const int x) {
    int sub=N[x].sub;
    NODEGATE gate=N[x].g;
    vector<int> in;
    unsigned long sum=0;
    double AEMDiffer[SimValue];
    for(int i=0;i<N[x].in.size();i++)
        in.push_back(N[x].in[i]);
    N[x].in.clear();
    if(N[x].inv)
        N[x].g=ginv;
    else
        N[x].g=gbuf;
    N[x].in.push_back(sub);
    SimulateNode();
    for(int j=0;j<SimValue;j++){
        AEMTempValue[j]=0;
        for(int i=0;i<O;i++){
            if(N[i+I].I[j]!=TI[i][j]){
                N[i+I].W[j]=true;
                ErrFlag[j]=true;
            }
            AEMTempValue[j]+=(N[i+I].I[j]?(unsigned long)1<<i:0);
        }
        AEMDiffer[j] = AEMTempValue[j]>TrueValue[j]?AEMTempValue[j]-TrueValue[j]:TrueValue[j]-AEMTempValue[j];
        sum += AEMTempValue[j]>TrueValue[j]?AEMTempValue[j]-TrueValue[j]:TrueValue[j]-AEMTempValue[j];
    }
    N[x].g=gate;
    N[x].in.clear();
    for(int i=0;i<in.size();i++)
        N[x].in.push_back(in[i]);
    LastAEM = (double)sum/SimValue;
    double up_bound=0;
    //for(int j=0; j<SimValue; j++){
    //    up_bound += (AEMDiffer[j]-LastAEM) * (AEMDiffer[j]-LastAEM);
    //}
    //up_bound = Binomal_95 * sqrt(up_bound/SimValue/(SimValue-1));
    //printf("The sample AEM is %lf, difference of upper bound is %lf, upper bound of 95%% confidence is  %lf\n", LastAEM, up_bound, up_bound+LastAEM);
    return up_bound+LastAEM;
}

void MAP::SetZeroOne(void){
    for(int i=0; i<SimValue; i++){
        N[0].I[i] = false;
        N[1].I[i] = true;
    }
}

void MAP::SingleInputPattern(int val, int loc){
    for(unsigned long  i=0; i<I-2; i++){
        N[i+2].I[loc] = (val>>i) % 2;
    }
}
unsigned long MAP::ERComparedWithAccurate(MAP &map, const int num){
    int j;
    unsigned long  sum = 0;
    for(int i=0; i < num; i++){
        for(j=I; j<I+O; j++){
            if(map.N[j].I[i]!=N[j].I[i]){
                sum++;
                break;
            }
        }
    }
    return sum;
}

unsigned long MAP::AEMComparedWithAccurate(MAP &map, const int num) {
    int j;
    unsigned long sum = 0;
    unsigned long tempOri = 0;
    unsigned long tempAppr = 0;
    for(int i=0;i < num; i++){
        tempAppr = 0;
        tempOri = 0;
        for(j=I; j<I+O; j++){
            tempOri += map.N[j].I[i] ? (unsigned long)1<<(j-I) : 0;
            tempAppr += N[j].I[i] ? (unsigned long)1<<(j-I) : 0;
        }
        sum += ( tempOri>tempAppr ? tempOri - tempAppr : tempAppr - tempOri );
    }
    return sum;
}

void MAP::ResetInLoc() {
    for(int i=0; i<QN.size(); ++i){
        int loc = QN[i];
        for(int j=0; j<inlong; ++j)
            N[loc].inloc[j] = 0;
        for(int j=0; j<N[loc].in.size(); ++j){
            for(int k=0; k<inlong; ++k){
                N[loc].inloc[k] |= N[N[loc].in[j]].inloc[k];
            }
        }
    }
    return;
}
