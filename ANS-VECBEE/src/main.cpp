#include "header.h"
#include "cmdline.h"
#include "cktSASIMI.h"
#include "cktSingSel.h"
#include "cktSynthesis.h"
#include "cktBlif.h"
#include "cktNtk.h"


using namespace std;
using namespace abc;
using namespace cmdline;


parser Cmdline_Parser(int argc, char * argv[])
{
    parser option;
    option.add <string> ("file",   'f', "Circuit file",       false, "data/su/c880_rem.blif");
    option.add <string> ("genlib", 'g', "Map libarary file",  false, "data/genlib/mcnc.genlib");
    option.add <float>  ("error",  'e', "Error rate",         false, 0.01f, range(0.0f, 1.0f));
    option.add <int>    ("number", 'n', "Frame number",       false, 100000, range(1, INT_MAX));
    option.parse_check(argc, argv);
    return option;
}


void Execute_Gate_Net(Abc_Ntk_t * pAbcNtk, int number)
{
    Ckt_Gate_Net_t ckt(pAbcNtk, number);
    Ckt_Gate_Net_t cktRef(ckt);

    // ckt.PrintInfo();
    cout << "brute" << endl;
    Ckt_EnumerateTest(ckt);
    cout << "batch" << endl;
    Ckt_BatchErrorEstimation(ckt, cktRef);
    // ckt.TestSimulatorSpeed();
}


void Execute_Sop_Net(Abc_Ntk_t * pAbcNtk, int number, float ERThres, string genlib)
{
    Ckt_Sop_Net_t ckt(pAbcNtk, number);
    Ckt_Sop_Net_t cktRef(ckt);
    int EThres = static_cast <int> (ERThres * number);

    cout << "fileName = " << ckt.GetName() << endl;
    cout << "original #literals = " << ckt.CountLiteralNum() << endl;
    int round = 0;
    cout << "--------- round " << round++ << " ---------" << endl;
    auto error = Ckt_SingleSelectionOnce(ckt, cktRef, EThres);
    while (error <= EThres) {
        cout << "--------- round " << round++ << " ---------" << endl;
        error = Ckt_SingleSelectionOnce(ckt, cktRef, EThres);
    }

    // assert(system("if [ ! -d approx ]; then mkdir approx; fi") != -1);
    // string fileName("");
    // fileName += ckt.GetName();
    // fileName += string("_");
    // random_device rd;
    // unsigned seed = static_cast <unsigned> (rd());
    // ckt.GenInputDist(seed);
    // ckt.FeedForward();
    // cktRef.GenInputDist(seed);
    // cktRef.FeedForward();
    // float er = ckt.GetErrorRate(cktRef) / static_cast <float> (number);
    // stringstream str;
    // str << er;
    // fileName += str.str();
    // fileName += string(".blif");

    // Ckt_Synthesis2(ckt, fileName);

    // string command;
    // Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    // Abc_FrameReplaceCurrentNetwork(pAbc, Abc_NtkDup(ckt.GetAbcNtk()));
    // command = "sweep";
    // assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );
    // command = "write approx/" + fileName;
    // assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );
    // command = "read approx/" + fileName;
    // assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );

    // Ckt_Sop_Net_t appCkt(Abc_FrameReadNtk(pAbc), number);
    // cout << "error rate = " << er << endl;
    // cout << "#literals = " << appCkt.CountLiteralNum() << endl;

    // FILE * fp = nullptr;
    // assert(fp = fopen("./script/temp.rug", "w"));
    // fprintf(fp, "read_library %s\n", genlib.c_str());
    // fprintf(fp, "read_blif mapped/%s\n", fileName.c_str());
    // fprintf(fp, "print_map_stats\n");
    // fclose(fp);
    // assert(system("sis -f ./script/temp.rug -t none | grep -e \"Most Negative Slack\" -e \"Total Area\"") != -1);
}


void Aig_Test(void)
{
    Abc_Ntk_t * pAig;
    pAig = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    string ntkName("test");
    pAig->pName = Extra_UtilStrsav(ntkName.c_str());
    int nPrimaryInputs = 3;
    for (int i = 0; i < nPrimaryInputs; ++i) {
        Abc_Obj_t * pObj = Abc_NtkCreatePi(pAig);
        stringstream ss;
        string str;
        ss << static_cast <char> ('A' + i);
        ss >> str;
        Abc_ObjAssignName(pObj, const_cast <char *> (str.c_str()), nullptr);
    }
    for (int i = 0; i < nPrimaryInputs; ++i)
        cout << Abc_ObjName(Abc_NtkPi(pAig, i)) << endl;
    cout << Abc_NtkPiNum(pAig) << endl;
    Abc_Obj_t * pObjA = Abc_NtkPi(pAig, 0);
    Abc_Obj_t * pObjB = Abc_NtkPi(pAig, 1);
    Abc_Obj_t * pObjC = Abc_NtkPi(pAig, 2);
    Abc_Obj_t * pObjAnd = Abc_AigAnd((Abc_Aig_t *)pAig->pManFunc, pObjA, pObjB);
    Abc_Obj_t * pObjF = Abc_AigOr((Abc_Aig_t *)pAig->pManFunc, pObjAnd, pObjC);
    Abc_Obj_t * pPo = Abc_NtkCreatePo(pAig);
    Abc_ObjAddFanin(pPo, pObjF);
    assert(Abc_NtkCheck(pAig));
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    Abc_FrameReplaceCurrentNetwork(pAbc, pAig);
    string command("show");
    assert( !Cmd_CommandExecute(pAbc, command.c_str()) );
}


int main(int argc, char * argv[])
{
    parser option = Cmdline_Parser(argc, argv);
    string file = option.get <string> ("file");
    string genlib = option.get <string> ("genlib");
    int number = option.get <int> ("number");
    float ERThres = option.get <float> ("error");

    Abc_Start();
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    string command = "read_genlib -v " + genlib;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );
    command = "read_blif " + file;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );

    Abc_Ntk_t * pAbcNtk = Abc_FrameReadNtk(pAbc);

    if (Abc_NtkIsSopLogic(pAbcNtk))
        Execute_Sop_Net(pAbcNtk, number, ERThres, genlib);
    else if (Abc_NtkIsMappedLogic(pAbcNtk))
        Execute_Gate_Net(pAbcNtk, number);

    Abc_Stop();

    return 0;
}
