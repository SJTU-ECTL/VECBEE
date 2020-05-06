#include "cktUtil.h"


using namespace std;


// evaluate
void Ckt_EvalASIC(Abc_Ntk_t * pNtk, string fileName, double maxDelay, bool isOutput)
{
    string Command;
    string resyn2 = "strash; balance; rewrite; refactor; balance; rewrite; rewrite -z; balance; refactor -z; rewrite -z; balance;";
    float area, delay;

    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    Abc_FrameReplaceCurrentNetwork(pAbc, Abc_NtkDup(pNtk));

    Command = resyn2 + string("amap;");
    DASSERT(!Cmd_CommandExecute(pAbc, Command.c_str()));

    ostringstream ss;
    ss.str("");
    ss << maxDelay;
    area = Ckt_GetArea(Abc_FrameReadNtk(pAbc));
    delay = Ckt_GetDelay(Abc_FrameReadNtk(pAbc));
    for (int i = 0; i < 10; ++i) {
        if (delay < maxDelay) {
            Command = resyn2 + string("amap;");
            DASSERT(!Cmd_CommandExecute(pAbc, Command.c_str()));
        }
        else {
            Command = resyn2 + string("map -D ") + ss.str() + string(";");
            DASSERT(!Cmd_CommandExecute(pAbc, Command.c_str()));
        }
        area = Ckt_GetArea(Abc_FrameReadNtk(pAbc));
        delay = Ckt_GetDelay(Abc_FrameReadNtk(pAbc));
    }

    if (delay >= maxDelay + 0.1) {
        Command = resyn2 + string("map -D ") + ss.str() + string(";");
        DASSERT(!Cmd_CommandExecute(pAbc, Command.c_str()));
        area = Ckt_GetArea(Abc_FrameReadNtk(pAbc));
        delay = Ckt_GetDelay(Abc_FrameReadNtk(pAbc));
    }
    if (delay >= maxDelay + 0.1)
        cout << "Warning: exceed required delay" << endl;
    cout << "area = " << area << endl;
    cout << "delay = " << delay << endl;
    if (isOutput) {
        ss.str("");
        ss << "write_blif " << fileName << "_" << area << "_" << delay << ".blif";
        DASSERT(!Cmd_CommandExecute(pAbc, ss.str().c_str()));
    }
}


void Ckt_EvalASIC(IN Abc_Ntk_t * pNtk, IN string && fileName, IN double maxDelay, OUT float & area, OUT float & delay)
{
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    Abc_FrameReplaceCurrentNetwork(pAbc, Abc_NtkDup(pNtk));

    string resyn2 = "strash; balance; rewrite; refactor; balance; rewrite; rewrite -z; balance; refactor -z; rewrite -z; balance;";
    string Command = resyn2 + string("amap;");
    DASSERT(!Cmd_CommandExecute(pAbc, Command.c_str()));

    area = Ckt_GetArea(Abc_FrameReadNtk(pAbc));
    delay = Ckt_GetDelay(Abc_FrameReadNtk(pAbc));
    // cout << area << "," << delay << endl;
    Abc_Ntk_t * pNtkBak = nullptr;
    if (delay <= maxDelay)
        pNtkBak = Abc_NtkDup(Abc_FrameReadNtk(pAbc));

    ostringstream ss;
    ss.str("");
    ss << maxDelay;

    Command = resyn2 + string("map -D ") + ss.str() + string(";");
    for (int i = 0; i < 10; ++i)  {
        float oldArea = area;
        float oldDelay = delay;
        DASSERT(!Cmd_CommandExecute(pAbc, Command.c_str()));
        area = Ckt_GetArea(Abc_FrameReadNtk(pAbc));
        delay = Ckt_GetDelay(Abc_FrameReadNtk(pAbc));
        // cout << area << "," << delay << endl;
        if (oldArea < area || (oldArea == area && oldDelay <= delay))
            break;
        if (delay <= maxDelay) {
            Abc_NtkDelete(pNtkBak);
            pNtkBak = Abc_NtkDup(Abc_FrameReadNtk(pAbc));
        }
    }

    if (pNtkBak == nullptr)
        pNtkBak = Abc_NtkDup(Abc_FrameReadNtk(pAbc));
    area = Ckt_GetArea(pNtkBak);
    delay = Ckt_GetDelay(pNtkBak);
    ss.str("");
    ss << fileName << "_" << area << "_" << delay << ".blif";
    Ckt_WriteBlif(pNtkBak, ss.str());
    Abc_NtkDelete(pNtkBak);
}


void Ckt_EvalFPGA(Abc_Ntk_t * pNtk, string fileName, string map)
{
    const string resyn2 = "strash; balance; rewrite; refactor; balance; rewrite; rewrite -z; balance; refactor -z; rewrite -z; balance;";

    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    Abc_FrameReplaceCurrentNetwork(pAbc, Abc_NtkDup(pNtk));
    string Command = resyn2 + map;

    // synthesis and mapping
    DASSERT(!Cmd_CommandExecute(pAbc, Command.c_str()));
    int size = Abc_NtkNodeNum(Abc_FrameReadNtk(pAbc));
    int depth = Abc_NtkLevel(Abc_FrameReadNtk(pAbc));
    int bestSize = size;
    int bestDepth = depth;
    bool terminate = false;
    int nRounds = 0;

    // repeat until no improvement
    while (!terminate) {
        terminate = true;
        ++nRounds;

        DASSERT(!Cmd_CommandExecute(pAbc, Command.c_str()));
        size = Abc_NtkNodeNum(Abc_FrameReadNtk(pAbc));
        depth = Abc_NtkLevel(Abc_FrameReadNtk(pAbc));
        if (size < bestSize) {
            bestSize = size;
            bestDepth = depth;
            terminate = false;
        }
        else if (size == bestSize && depth < bestDepth) {
            bestDepth = depth;
            terminate = false;
        }
    }

    // output
    cout << "size = " << size << endl;
    cout << "depth = " << depth << endl;
    Command = string("write_blif ");
    ostringstream ss("");
    ss << fileName << "_" << size << "_" << depth << ".blif";
    string str = ss.str();
    Command += str;
    DASSERT(!Cmd_CommandExecute(pAbc, Command.c_str()));
}


float Ckt_GetArea(Abc_Ntk_t * pNtk)
{
    DASSERT(Abc_NtkHasMapping(pNtk));
    Vec_Ptr_t * vNodes = Abc_NtkDfs(pNtk, 0);
    float area = 0.0;
    Abc_Obj_t * pObj;
    int i;
    Vec_PtrForEachEntry(Abc_Obj_t *, vNodes, pObj, i)
        area += Mio_GateReadArea( (Mio_Gate_t *)pObj->pData );
    Vec_PtrFree(vNodes);
    return area;
}


float Ckt_GetDelay(Abc_Ntk_t * pNtk)
{
    DASSERT(Abc_NtkHasMapping(pNtk));
    return Abc_NtkDelayTrace(pNtk, nullptr, nullptr, 0);
}


int Ckt_GetObjArrivalTime(Abc_Obj_t * pObj, int roundBit)
{
    return round(Abc_NodeReadArrivalWorst(pObj) * pow(10, roundBit));
}


// misc
void Ckt_NtkRename(Abc_Ntk_t * pNtk, const char * name)
{
    free(pNtk->pName);
    pNtk->pName = (char *)malloc(strlen(name) + 1);
    memcpy(pNtk->pName, name, strlen(name) + 1);
}


Abc_Obj_t * Ckt_GetConst(Abc_Ntk_t * pNtk, bool isConst1)
{
    Abc_Obj_t * pNode = nullptr;
    int i = 0;
    Abc_Obj_t * pConst = nullptr;
    if (!isConst1) {
        Abc_NtkForEachNode(pNtk, pNode, i) {
            if (Abc_NodeIsConst0(pNode)) {
                pConst = pNode;
                break;
            }
        }
    }
    else {
        Abc_NtkForEachNode(pNtk, pNode, i) {
            if (Abc_NodeIsConst1(pNode)) {
                pConst = pNode;
                break;
            }
        }
    }
    if (pConst == nullptr) {
        if (!isConst1) {
            pConst = Abc_NtkCreateNodeConst0(pNtk);
            // cout << "create constant 0" << endl;
        }
        else {
            pConst = Abc_NtkCreateNodeConst1(pNtk);
            // cout << "create constant 1" << endl;
        }
    }
    return pConst;
}


Abc_Obj_t * Ckt_FindNodeByName(Abc_Ntk_t * pNtk, char * nodeName)
{
    int Num = Nm_ManFindIdByName( pNtk->pManName, nodeName, ABC_OBJ_NODE );
    if (Num == -1)
        Num = Nm_ManFindIdByName( pNtk->pManName, nodeName, ABC_OBJ_PI );
    return (Num == -1)? nullptr : Abc_NtkObj(pNtk, Num);
}


void Ckt_PrintNodeFunc(Abc_Obj_t * pNode)
{
    DASSERT(pNode != nullptr);
    DASSERT(Abc_NtkIsAigLogic(pNode->pNtk));
    Vec_Vec_t * vLevels = Vec_VecAlloc( 10 );
    // set the input names
    Abc_Obj_t * pFanin = nullptr;
    int k = 0;
    Abc_ObjForEachFanin( pNode, pFanin, k )
        Hop_IthVar((Hop_Man_t *)pNode->pNtk->pManFunc, k)->pData = Abc_ObjName(pFanin);
    // write the formula
    cout << Abc_ObjName(pNode) << " = ";
    Hop_ObjPrintEqn( stdout, (Hop_Obj_t *)pNode->pData, vLevels, 0 );
    cout << endl;
    Vec_VecFree( vLevels );
}


void Ckt_PrintHopFunc(Hop_Obj_t * pHopObj, Vec_Ptr_t * vFanins)
{
    DASSERT(pHopObj != nullptr);
    DASSERT(vFanins != nullptr);
    Vec_Vec_t * vLevels = Vec_VecAlloc( 10 );
    // set the input names
    int k = 0;
    Abc_Obj_t * pFanin = nullptr;
    Vec_PtrForEachEntry(Abc_Obj_t *, vFanins, pFanin, k)
        Hop_IthVar((Hop_Man_t *)pFanin->pNtk->pManFunc, k)->pData = Abc_ObjName(pFanin);
    // write the formula
    cout << "F = ";
    Hop_ObjPrintEqn( stdout, pHopObj, vLevels, 0 );
    cout << endl;
    // cout << "(";
    // Vec_PtrForEachEntry(Abc_Obj_t *, vFanins, pFanin, k)
    //     cout << Abc_ObjName(pFanin) << ",";
    // cout << ")" << endl;
    Vec_VecFree( vLevels );
}


void Ckt_WriteBlif(Abc_Ntk_t * pNtk, string fileName)
{
    Abc_Ntk_t * pTemp = Abc_NtkDup(pNtk);
    Io_Write(pTemp, const_cast <char *> (fileName.c_str()), IO_FILE_BLIF);
    Abc_NtkDelete(pTemp);
}


void Ckt_WriteDot(Abc_Ntk_t * pNtk0, string fileName, unordered_map <string, int> & name2Level, vector <string> & lastLACs, vector <Abc_Ntk_t *> & pNtks)
{
    int fGateNames = 1;
    int fUseReverse = 0;
    // Abc_NtkShow(pNtk, fGateNames, fSeq, fUseReverse);

    FILE * pFile;
    Abc_Ntk_t * pNtk;
    Abc_Obj_t * pNode;
    Vec_Ptr_t * vNodes;
    int nBarBufs;
    int i;

    assert( Abc_NtkIsStrash(pNtk0) || Abc_NtkIsLogic(pNtk0) );
    if ( Abc_NtkIsStrash(pNtk0) && Abc_NtkGetChoiceNum(pNtk0) )
    {
        printf( "Temporarily visualization of AIGs with choice nodes is disabled.\n" );
        return;
    }
    // check that the file can be opened
    if ( (pFile = fopen( fileName.c_str(), "w" )) == NULL )
    {
        fprintf( stdout, "Cannot open the intermediate file \"%s\".\n", fileName.c_str() );
        return;
    }
    fclose( pFile );


    // convert to logic SOP
    pNtk = Abc_NtkDup( pNtk0 );
    if ( Abc_NtkIsLogic(pNtk) && !Abc_NtkHasMapping(pNtk) )
        Abc_NtkToSop( pNtk, -1, ABC_INFINITY );

    // collect all nodes in the network
    vNodes = Vec_PtrAlloc( 100 );
    Abc_NtkForEachObj( pNtk, pNode, i )
        Vec_PtrPush( vNodes, pNode );

    // write the DOT file
    nBarBufs = pNtk->nBarBufs;
    pNtk->nBarBufs = 0;
    Ckt_WriteDotNtk( pNtk, vNodes, NULL, const_cast <char *> (fileName.c_str()), fGateNames, fUseReverse, name2Level, lastLACs, pNtks );
    pNtk->nBarBufs = nBarBufs;
    Vec_PtrFree( vNodes );

    // convert to png
    // string command = "dot -Tpng " + fileName + " -o " + fileName + ".png -Gdpi=1200";
    // CallSystem(move(command));
}


static char * Abc_NtkPrintSop( char * pSop )
{
    static char Buffer[1000];
    char * pGet, * pSet;
    pSet = Buffer;
    for ( pGet = pSop; *pGet; pGet++ )
    {
        if ( *pGet == '\n' )
        {
            *pSet++ = '\\';
            *pSet++ = 'n';
        }
        else
            *pSet++ = *pGet;
    }
    *(pSet-2) = 0;
    return Buffer;
}


static int Abc_NtkCountLogicNodes( Vec_Ptr_t * vNodes )
{
    Abc_Obj_t * pObj;
    int i, Counter = 0;
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pObj, i )
    {
        if ( !Abc_ObjIsNode(pObj) )
            continue;
        if ( Abc_ObjFaninNum(pObj) == 0 && Abc_ObjFanoutNum(pObj) == 0 )
            continue;
        Counter ++;
    }
    return Counter;
}


void Ckt_WriteDotNtk( Abc_Ntk_t * pNtk, Vec_Ptr_t * vNodes, Vec_Ptr_t * vNodesShow, char * pFileName, int fGateNames, int fUseReverse, unordered_map <string, int> & name2Level, vector <string> & lastLACs, vector <Abc_Ntk_t *> & pNtks)
{
    FILE * pFile;
    Abc_Obj_t * pNode, * pFanin;
    char * pSopString;
    int LevelMin, LevelMax, fHasCos, Level, i, k, fHasBdds, fCompl, Prev;
    int Limit = 500;

    assert( Abc_NtkIsStrash(pNtk) || Abc_NtkIsLogic(pNtk) );

    if ( vNodes->nSize < 1 )
    {
        printf( "The set has no nodes. DOT file is not written.\n" );
        return;
    }

    if ( vNodes->nSize > Limit )
    {
        printf( "The set has more than %d nodes. DOT file is not written.\n", Limit );
        return;
    }

    // start the stream
    if ( (pFile = fopen( pFileName, "w" )) == NULL )
    {
        fprintf( stdout, "Cannot open the intermediate file \"%s\".\n", pFileName );
        return;
    }

    // transform logic functions from BDD to SOP
    if ( (fHasBdds = Abc_NtkIsBddLogic(pNtk)) )
    {
        if ( !Abc_NtkBddToSop(pNtk, -1, ABC_INFINITY) )
        {
            printf( "Io_WriteDotNtk(): Converting to SOPs has failed.\n" );
            return;
        }
    }

    // mark the nodes from the set
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
        pNode->fMarkC = 1;
    if ( vNodesShow )
        Vec_PtrForEachEntry( Abc_Obj_t *, vNodesShow, pNode, i )
            pNode->fMarkB = 1;

    // get the levels of nodes
    LevelMax = Abc_NtkLevel( pNtk );
    if ( fUseReverse )
    {
        LevelMin = Abc_NtkLevelReverse( pNtk );
        assert( LevelMax == LevelMin );
        Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
            if ( Abc_ObjIsNode(pNode) )
                pNode->Level = LevelMax - pNode->Level + 1;
    }

    // find the largest and the smallest levels
    LevelMin = 10000;
    LevelMax = -1;
    fHasCos  = 0;
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
    {
        if ( Abc_ObjIsCo(pNode) )
        {
            fHasCos = 1;
            continue;
        }
        if ( LevelMin > (int)pNode->Level )
            LevelMin = pNode->Level;
        if ( LevelMax < (int)pNode->Level )
            LevelMax = pNode->Level;
    }

    // set the level of the CO nodes
    if ( fHasCos )
    {
        LevelMax++;
        Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
        {
            if ( Abc_ObjIsCo(pNode) )
                pNode->Level = LevelMax;
        }
    }

    // write the DOT header
    fprintf( pFile, "# %s\n",  "Network structure generated by ABC" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "digraph network {\n" );
    fprintf( pFile, "size = \"7.5,10\";\n" );
//    fprintf( pFile, "size = \"10,8.5\";\n" );
//    fprintf( pFile, "size = \"14,11\";\n" );
//    fprintf( pFile, "page = \"8,11\";\n" );
//  fprintf( pFile, "ranksep = 0.5;\n" );
//  fprintf( pFile, "nodesep = 0.5;\n" );
    fprintf( pFile, "center = true;\n" );
//    fprintf( pFile, "orientation = landscape;\n" );
//  fprintf( pFile, "edge [fontsize = 10];\n" );
//  fprintf( pFile, "edge [dir = none];\n" );
    fprintf( pFile, "edge [dir = back];\n" );
    fprintf( pFile, "\n" );

    // labels on the left of the picture
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  node [shape = plaintext];\n" );
    fprintf( pFile, "  edge [style = invis];\n" );
    fprintf( pFile, "  LevelTitle1 [label=\"\"];\n" );
    fprintf( pFile, "  LevelTitle2 [label=\"\"];\n" );
    // generate node names with labels
    for ( Level = LevelMax; Level >= LevelMin; Level-- )
    {
        // the visible node name
        fprintf( pFile, "  Level%d", Level );
        fprintf( pFile, " [label = " );
        // label name
        fprintf( pFile, "\"" );
        fprintf( pFile, "\"" );
        fprintf( pFile, "];\n" );
    }

    // genetate the sequence of visible/invisible nodes to mark levels
    fprintf( pFile, "  LevelTitle1 ->  LevelTitle2 ->" );
    for ( Level = LevelMax; Level >= LevelMin; Level-- )
    {
        // the visible node name
        fprintf( pFile, "  Level%d",  Level );
        // the connector
        if ( Level != LevelMin )
            fprintf( pFile, " ->" );
        else
            fprintf( pFile, ";" );
    }
    fprintf( pFile, "\n" );
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate title box on top
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  rank = same;\n" );
    fprintf( pFile, "  LevelTitle1;\n" );
    fprintf( pFile, "  title1 [shape=plaintext,\n" );
    fprintf( pFile, "          fontsize=20,\n" );
    fprintf( pFile, "          fontname = \"Times-Roman\",\n" );
    fprintf( pFile, "          label=\"" );
    fprintf( pFile, "%s", "Network structure visualized by ABC" );
    fprintf( pFile, "\\n" );
    fprintf( pFile, "Benchmark \\\"%s\\\". ", pNtk->pName );
    fprintf( pFile, "Time was %s. ",  Extra_TimeStamp() );
    fprintf( pFile, "\"\n" );
    fprintf( pFile, "         ];\n" );
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate statistics box
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  rank = same;\n" );
    fprintf( pFile, "  LevelTitle2;\n" );
    fprintf( pFile, "  title2 [shape=plaintext,\n" );
    fprintf( pFile, "          fontsize=18,\n" );
    fprintf( pFile, "          fontname = \"Times-Roman\",\n" );
    fprintf( pFile, "          label=\"" );
    if ( Abc_NtkObjNum(pNtk) == Vec_PtrSize(vNodes) )
        fprintf( pFile, "The network contains %d logic nodes and %d latches.", Abc_NtkNodeNum(pNtk), Abc_NtkLatchNum(pNtk) );
    else
        fprintf( pFile, "The set contains %d logic nodes and spans %d levels.", Abc_NtkCountLogicNodes(vNodes), LevelMax - LevelMin + 1 );
    fprintf( pFile, "\\n" );
    fprintf( pFile, "\"\n" );
    fprintf( pFile, "         ];\n" );
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate the POs
    if ( fHasCos )
    {
        fprintf( pFile, "{\n" );
        fprintf( pFile, "  rank = same;\n" );
        // the labeling node of this level
        fprintf( pFile, "  Level%d;\n",  LevelMax );
        // generate the PO nodes
        Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
        {
            if ( !Abc_ObjIsCo(pNode) )
                continue;
            fprintf( pFile, "  Node%d [label = \"%s%s\"",
                pNode->Id,
                (Abc_ObjIsBi(pNode)? Abc_ObjName(Abc_ObjFanout0(pNode)):Abc_ObjName(pNode)),
                (Abc_ObjIsBi(pNode)? "_in":"") );
            fprintf( pFile, ", shape = %s", (Abc_ObjIsBi(pNode)? "box":"invtriangle") );
            if ( pNode->fMarkB )
                fprintf( pFile, ", style = filled" );
            fprintf( pFile, ", color = coral, fillcolor = coral" );
            fprintf( pFile, "];\n" );
        }
        fprintf( pFile, "}" );
        fprintf( pFile, "\n" );
        fprintf( pFile, "\n" );
    }

    // generate nodes of each rank
    for ( Level = LevelMax - fHasCos; Level >= LevelMin && Level > 0; Level-- )
    {
        fprintf( pFile, "{\n" );
        fprintf( pFile, "  rank = same;\n" );
        // the labeling node of this level
        fprintf( pFile, "  Level%d;\n",  Level );
        Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
        {
            if ( (int)pNode->Level != Level )
                continue;
            if ( Abc_ObjFaninNum(pNode) == 0 )
                continue;

            if ( Abc_NtkIsStrash(pNtk) )
                pSopString = const_cast <char *> ("");
            else if ( Abc_NtkHasMapping(pNtk) && fGateNames )
                pSopString = Mio_GateReadName((Mio_Gate_t *)pNode->pData);
            else if ( Abc_NtkHasMapping(pNtk) )
                pSopString = Abc_NtkPrintSop(Mio_GateReadSop((Mio_Gate_t *)pNode->pData));
            else
                pSopString = Abc_NtkPrintSop((char *)pNode->pData);

            DASSERT(!pNode->fMarkB);

            vector <int> positions;
            for (int i = 0; i < static_cast <int> (lastLACs.size()); ++i) {
                if (lastLACs[i] == string(Abc_ObjName(pNode)))
                    positions.emplace_back(i);
            }
            if (positions.size()) {
                cout << "here " << Abc_ObjName(pNode) << endl;
                fprintf( pFile, "  Node%d [label = \"%s\\n%s", pNode->Id, Abc_ObjName(pNode), pSopString );
                for (int idx: positions)
                    fprintf( pFile, ", last %d,", idx );
                fprintf( pFile, "\"" );
                fprintf( pFile, ", shape = ellipse" );
                fprintf( pFile, ", style = filled" );
                fprintf( pFile, ", fillcolor = \"0.3333 1.0 1.0\"" );
                fprintf( pFile, "];\n" );
            }
            else if (name2Level.find(string(Abc_ObjName(pNode))) != name2Level.end()) {
                fprintf( pFile, "  Node%d [label = \"%s\\n%s\"", pNode->Id, Abc_ObjName(pNode), pSopString );
                fprintf( pFile, ", shape = ellipse" );
                int lev = name2Level[string(Abc_ObjName(pNode))];
                float saturation = lev / 50.0;
                // cout << Abc_ObjName(pNode) << "\t" << lev << "\t" << saturation << endl;
                saturation = min(saturation, 1.0f);
                fprintf( pFile, ", style = filled" );
                if (lev == 0)
                    fprintf( pFile, ", fillcolor = \"0.6667 1.0 1.0\"" );
                else
                    fprintf( pFile, ", fillcolor = \"1.0 %.2f 1.0\"", saturation );
                fprintf( pFile, "];\n" );
            }
            else {
                fprintf( pFile, "  Node%d [label = \"%s\\n%s\"", pNode->Id, Abc_ObjName(pNode), pSopString );
                fprintf( pFile, ", shape = ellipse" );
                fprintf( pFile, "];\n" );
            }
        }
        fprintf( pFile, "}" );
        fprintf( pFile, "\n" );
        fprintf( pFile, "\n" );
    }

    // generate the PI nodes if any
    if ( LevelMin == 0 )
    {
        fprintf( pFile, "{\n" );
        fprintf( pFile, "  rank = same;\n" );
        // the labeling node of this level
        fprintf( pFile, "  Level%d;\n",  LevelMin );
        // generate the PO nodes
        Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
        {
            if ( !Abc_ObjIsCi(pNode) )
            {
                // check if the costant node is present
                if ( Abc_ObjFaninNum(pNode) == 0 && Abc_ObjFanoutNum(pNode) > 0 )
                {
                    fprintf( pFile, "  Node%d [label = \"Const%d\"", pNode->Id, Abc_NtkIsStrash(pNode->pNtk) || Abc_NodeIsConst1(pNode) );
                    fprintf( pFile, ", shape = ellipse" );
                    if ( pNode->fMarkB )
                        fprintf( pFile, ", style = filled" );
                    fprintf( pFile, ", color = coral, fillcolor = coral" );
                    fprintf( pFile, "];\n" );
                }
                continue;
            }
            fprintf( pFile, "  Node%d [label = \"%s\"",
                pNode->Id,
                (Abc_ObjIsBo(pNode)? Abc_ObjName(Abc_ObjFanin0(pNode)):Abc_ObjName(pNode)) );
            fprintf( pFile, ", shape = %s", (Abc_ObjIsBo(pNode)? "box":"triangle") );
            if ( pNode->fMarkB )
                fprintf( pFile, ", style = filled" );
            fprintf( pFile, ", color = coral, fillcolor = coral" );
            fprintf( pFile, "];\n" );
        }
        fprintf( pFile, "}" );
        fprintf( pFile, "\n" );
        fprintf( pFile, "\n" );
    }

    // generate invisible edges from the square down
    fprintf( pFile, "title1 -> title2 [style = invis];\n" );
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
    {
        if ( (int)pNode->Level != LevelMax )
            continue;
        fprintf( pFile, "title2 -> Node%d [style = invis];\n", pNode->Id );
    }
    // generate invisible edges among the COs
    Prev = -1;
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
    {
        if ( (int)pNode->Level != LevelMax )
            continue;
        if ( !Abc_ObjIsPo(pNode) )
            continue;
        if ( Prev >= 0 )
            fprintf( pFile, "Node%d -> Node%d [style = invis];\n", Prev, pNode->Id );
        Prev = pNode->Id;
    }

    // generate edges
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
    {
        if ( Abc_ObjIsLatch(pNode) )
            continue;
        Abc_ObjForEachFanin( pNode, pFanin, k )
        {
            if ( Abc_ObjIsLatch(pFanin) )
                continue;
            fCompl = 0;
            if ( Abc_NtkIsStrash(pNtk) )
                fCompl = Abc_ObjFaninC(pNode, k);
            // generate the edge from this node to the next
            fprintf( pFile, "Node%d",  pNode->Id );
            fprintf( pFile, " -> " );
            fprintf( pFile, "Node%d",  pFanin->Id );
            fprintf( pFile, " [style = %s", fCompl? "dotted" : "solid" );
            fprintf( pFile, "]" );
            fprintf( pFile, ";\n" );
        }
    }

    // recover deleted nodes
    DASSERT(lastLACs.size() == pNtks.size());
    if (pNtks.size() > 1) {
        for (int i = 0; i < static_cast <int> (pNtks.size()) - 1; ++i) {
            Abc_Obj_t * pDelNd = Abc_NtkFindNode(pNtks[i], const_cast <char *>(lastLACs[i].c_str()));
            DASSERT(pDelNd != nullptr && string(Abc_ObjName(pDelNd)) == lastLACs[i]);
            cout << "traverse " << Abc_ObjName(pDelNd) << ":";

            // collect nodes
            vector <Abc_Obj_t *> delCone;
            Abc_NtkIncrementTravId(pNtks[i]);
            delCone.emplace_back(pDelNd);
            Abc_NodeSetTravIdCurrent(pDelNd);
            uint32_t head = 0;
            while (1) {
                if (head >= delCone.size())
                    break;
                Abc_Obj_t * pTmpNd = Ckt_FindNodeByName(pNtk, Abc_ObjName(delCone[head]));
                if (pTmpNd == nullptr) {
                    Abc_Obj_t * pFanin = nullptr;
                    int j = 0;
                    Abc_ObjForEachFanin(delCone[head], pFanin, j) {
                        if (!Abc_NodeIsTravIdCurrent(pFanin)) {
                            delCone.emplace_back(pFanin);
                            Abc_NodeSetTravIdCurrent(pFanin);
                        }
                    }
                }
                ++head;
            }
            for (Abc_Obj_t * & pConeNd: delCone)
                cout << Abc_ObjName(pConeNd) << ",";
            cout << endl;

            // recover node
            for (int k = 0; k < static_cast <int>(delCone.size()); ++k ) {
                Abc_Obj_t * pTmpNd = Ckt_FindNodeByName(pNtk, Abc_ObjName(delCone[k]));
                if (pTmpNd == nullptr) {
                    fprintf( pFile, "  Node%s [label = \"%s, last %d\"", Abc_ObjName(delCone[k]), Abc_ObjName(delCone[k]), i );
                    fprintf( pFile, ", shape = ellipse" );
                    fprintf( pFile, ", style = filled" );
                    if (k == 0)
                        fprintf( pFile, ", fillcolor = \"0.3333 1.0 1.0\"" );
                    else
                        fprintf( pFile, ", fillcolor = \"0.3333 0.1 1.0\"" );
                    fprintf( pFile, "];\n" );
                }
            }

            // recover edge
            for (int k = 0; k < static_cast <int>(delCone.size()); ++k ) {
                Abc_Obj_t * pFanin = nullptr;
                int j = 0;
                Abc_ObjForEachFanin(delCone[k], pFanin, j) {
                    // generate the edge from this node to the next
                    Abc_Obj_t * pTmpNd = Ckt_FindNodeByName(pNtk, Abc_ObjName(delCone[k]));
                    Abc_Obj_t * pTmpNd2 = Ckt_FindNodeByName(pNtk, Abc_ObjName(pFanin));
                    if (pTmpNd == nullptr || pTmpNd2 == nullptr) {
                        if (pTmpNd == nullptr)
                            fprintf( pFile, "Node%s",  Abc_ObjName(delCone[k]) );
                        else
                            fprintf( pFile, "Node%d",  pTmpNd->Id );
                        fprintf( pFile, " -> " );
                        if (pTmpNd2 == nullptr)
                            fprintf( pFile, "Node%s",  Abc_ObjName(pFanin) );
                        else
                            fprintf( pFile, "Node%d",  pTmpNd2->Id );
                        fprintf( pFile, " [style = solid];\n" );
                    }
                }
            }
        }
    }

    // EOF
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );
    fclose( pFile );

    // unmark the nodes from the set
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
        pNode->fMarkC = 0;
    if ( vNodesShow )
        Vec_PtrForEachEntry( Abc_Obj_t *, vNodesShow, pNode, i )
            pNode->fMarkB = 0;

    // convert the network back into BDDs if this is how it was
    if ( fHasBdds )
        Abc_NtkSopToBdd(pNtk);
}


void Ckt_PrintSop(std::string sop)
{
    for (auto &ch: sop) {
        if (ch == '\n')
            cout << ";";
        else
            cout << ch;
    }
}


void Ckt_PrintNodes(Vec_Ptr_t * vFanins)
{
    Abc_Obj_t * pObj = nullptr;
    int i = 0;
    Vec_PtrForEachEntry(Abc_Obj_t *, vFanins, pObj, i)
        cout << Abc_ObjName(pObj) << ",";
}


void Ckt_PrintFanins(Abc_Obj_t * pObj)
{
    Abc_Obj_t * pFanin = nullptr;
    int i = 0;
    Abc_ObjForEachFanin(pObj, pFanin, i)
        cout << Abc_ObjName(pFanin) << ",";
}


void Ckt_ReplaceByName(Abc_Ntk_t * pNtk, std::string tsName, std::string ssName)
{
    cout << tsName << "," << ssName << endl;
    Abc_Obj_t * pTS = Ckt_FindNodeByName(pNtk, const_cast <char *> (tsName.c_str()));
    Abc_Obj_t * pSS = Ckt_FindNodeByName(pNtk, const_cast <char *> (ssName.c_str()));
    DASSERT(pTS != nullptr);
    DASSERT(pSS != nullptr);
    Abc_ObjReplace(pTS, pSS);
}


int CallSystem(const std::string && cmd)
{
    pid_t status;
    cout << cmd << endl;
    status = system(cmd.c_str());
    if (-1 == status)
    {
        printf("system error!");
        return 0;
    }
    else
    {
        printf("exit status value = [0x%x]\n", status);
        if (WIFEXITED(status))
        {
            if (0 == WEXITSTATUS(status))
            {
                printf("run shell script successfully.\n");
                return 1;
            }
            else
            {
                printf("run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
                return 0;
            }
        }
        else
        {
            printf("exit status = [%d]\n", WEXITSTATUS(status));
            return 0;
        }
    }
}
