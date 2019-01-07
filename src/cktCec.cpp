#include "cktCec.h"


using namespace std;
using namespace abc;


void Ckt_Cec(Ckt_Gate_Net_t & ckt1, Ckt_Gate_Net_t & ckt2)
{
    Abc_Ntk_t * pNtk1 = Abc_NtkDup(ckt1.GetAbcNtk());
    Abc_Ntk_t * pNtk2 = Abc_NtkDup(ckt2.GetAbcNtk());
    Abc_NtkCecFraig(pNtk1, pNtk2, 20, 1);
    Abc_NtkDelete(pNtk1);
    Abc_NtkDelete(pNtk2);
}


void Abc_NtkCecFraig( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int nSeconds, int fVerbose )
{
    abctime clk = Abc_Clock();
    Prove_Params_t Params, * pParams = &Params;
//    Fraig_Params_t Params;
//    Fraig_Man_t * pMan;
    Abc_Ntk_t * pMiter, * pTemp;
    Abc_Ntk_t * pExdc = NULL;
    int RetValue;

    if ( pNtk1->pExdc != NULL || pNtk2->pExdc != NULL )
    {
        if ( pNtk1->pExdc != NULL && pNtk2->pExdc != NULL )
        {
            printf( "Comparing EXDC of the two networks:\n" );
            Abc_NtkCecFraig( pNtk1->pExdc, pNtk2->pExdc, nSeconds, fVerbose );
            printf( "Comparing networks under EXDC of the first network.\n" );
            pExdc = pNtk1->pExdc;
        }
        else if ( pNtk1->pExdc != NULL )
        {
            printf( "Second network has no EXDC. Comparing main networks under EXDC of the first network.\n" );
            pExdc = pNtk1->pExdc;
        }
        else if ( pNtk2->pExdc != NULL )
        {
            printf( "First network has no EXDC. Comparing main networks under EXDC of the second network.\n" );
            pExdc = pNtk2->pExdc;
        }
        else assert( 0 );
    }

    // get the miter of the two networks
    pMiter = Abc_NtkMiter( pNtk1, pNtk2, 1, 0, 0, 0 );
    if ( pMiter == NULL )
    {
        printf( "Miter computation has failed.\n" );
        return;
    }
    // add EXDC to the miter
    if ( pExdc )
    {
        assert( Abc_NtkPoNum(pMiter) == 1 );
        assert( Abc_NtkPoNum(pExdc) == 1 );
        pMiter = Abc_NtkMiter( pTemp = pMiter, pExdc, 1, 0, 1, 0 );
        Abc_NtkDelete( pTemp );
    }
    // handle trivial case
    RetValue = Abc_NtkMiterIsConstant( pMiter );
    if ( RetValue == 0 )
    {
        printf( "Networks are NOT EQUIVALENT after structural hashing.  " );
        // report the error
        pMiter->pModel = Abc_NtkVerifyGetCleanModel( pMiter, 1 );
        Abc_NtkVerifyReportError( pNtk1, pNtk2, pMiter->pModel );
        ABC_FREE( pMiter->pModel );
        Abc_NtkDelete( pMiter );
        Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
        return;
    }
    if ( RetValue == 1 )
    {
        printf( "Networks are equivalent after structural hashing.  " );
        Abc_NtkDelete( pMiter );
        Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
        return;
    }
/*
    // convert the miter into a FRAIG
    Fraig_ParamsSetDefault( &Params );
    Params.fVerbose = fVerbose;
    Params.nSeconds = nSeconds;
//    Params.fFuncRed = 0;
//    Params.nPatsRand = 0;
//    Params.nPatsDyna = 0;
    pMan = (Fraig_Man_t *)Abc_NtkToFraig( pMiter, &Params, 0, 0 );
    Fraig_ManProveMiter( pMan );

    // analyze the result
    RetValue = Fraig_ManCheckMiter( pMan );
    // report the result
    if ( RetValue == -1 )
        printf( "Networks are undecided (SAT solver timed out on the final miter).\n" );
    else if ( RetValue == 1 )
        printf( "Networks are equivalent after fraiging.\n" );
    else if ( RetValue == 0 )
    {
        printf( "Networks are NOT EQUIVALENT after fraiging.\n" );
        Abc_NtkVerifyReportError( pNtk1, pNtk2, Fraig_ManReadModel(pMan) );
    }
    else assert( 0 );
    // delete the fraig manager
    Fraig_ManFree( pMan );
    // delete the miter
    Abc_NtkDelete( pMiter );
*/
    // solve the CNF using the SAT solver
    Prove_ParamsSetDefault( pParams );
    pParams->nItersMax = 5;
//    RetValue = Abc_NtkMiterProve( &pMiter, pParams );
//    pParams->fVerbose = 1;
    RetValue = Abc_NtkIvyProve( &pMiter, pParams );
    if ( RetValue == -1 )
        printf( "Networks are undecided (resource limits is reached).  " );
    else if ( RetValue == 0 )
    {
        int * pSimInfo = Abc_NtkVerifySimulatePattern( pMiter, pMiter->pModel );
        if ( pSimInfo[0] != 1 )
            printf( "ERROR in Abc_NtkMiterProve(): Generated counter-example is invalid.\n" );
        else
            printf( "Networks are NOT EQUIVALENT.  " );
        ABC_FREE( pSimInfo );
    }
    else
        printf( "Networks are equivalent.  " );
    Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
    if ( pMiter->pModel )
        Abc_NtkVerifyReportError( pNtk1, pNtk2, pMiter->pModel );
    Abc_NtkDelete( pMiter );
}


void Abc_NtkVerifyReportError( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int * pModel )
{
    Vec_Ptr_t * vNodes;
    Abc_Obj_t * pNode;
    int * pValues1, * pValues2;
    int nErrors, nPrinted, i, iNode = -1;

    assert( Abc_NtkCiNum(pNtk1) == Abc_NtkCiNum(pNtk2) );
    assert( Abc_NtkCoNum(pNtk1) == Abc_NtkCoNum(pNtk2) );
    // get the CO values under this model
    pValues1 = Abc_NtkVerifySimulatePattern( pNtk1, pModel );
    pValues2 = Abc_NtkVerifySimulatePattern( pNtk2, pModel );
    // count the mismatches
    nErrors = 0;
    for ( i = 0; i < Abc_NtkCoNum(pNtk1); i++ )
        nErrors += (int)( pValues1[i] != pValues2[i] );
    printf( "Verification failed for at least %d outputs: ", nErrors );
    // print the first 3 outputs
    nPrinted = 0;
    for ( i = 0; i < Abc_NtkCoNum(pNtk1); i++ )
        if ( pValues1[i] != pValues2[i] )
        {
            if ( iNode == -1 )
                iNode = i;
            printf( " %s", Abc_ObjName(Abc_NtkCo(pNtk1,i)) );
            if ( ++nPrinted == 3 )
                break;
        }
    if ( nPrinted != nErrors )
        printf( " ..." );
    printf( "\n" );
    // report mismatch for the first output
    if ( iNode >= 0 )
    {
        printf( "Output %s: Value in Network1 = %d. Value in Network2 = %d.\n",
            Abc_ObjName(Abc_NtkCo(pNtk1,iNode)), pValues1[iNode], pValues2[iNode] );
        printf( "Input pattern: " );
        // collect PIs in the cone
        pNode = Abc_NtkCo(pNtk1,iNode);
        vNodes = Abc_NtkNodeSupport( pNtk1, &pNode, 1 );
        // set the PI numbers
        Abc_NtkForEachCi( pNtk1, pNode, i )
            pNode->pCopy = (Abc_Obj_t *)(ABC_PTRINT_T)i;
        // print the model
        pNode = (Abc_Obj_t *)Vec_PtrEntry( vNodes, 0 );
        if ( Abc_ObjIsCi(pNode) )
        {
            Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pNode, i )
            {
                assert( Abc_ObjIsCi(pNode) );
                printf( " %s=%d", Abc_ObjName(pNode), pModel[(int)(ABC_PTRINT_T)pNode->pCopy] );
            }
        }
        printf( "\n" );
        Vec_PtrFree( vNodes );
    }
    ABC_FREE( pValues1 );
    ABC_FREE( pValues2 );
}
