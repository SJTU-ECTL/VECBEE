#include "cktSingSel.h"

using namespace std;
using namespace abc;


Ckt_Sing_Sel_ALC_t::Ckt_Sing_Sel_ALC_t(Ckt_Sop_t * p_ckt_obj, vector <string> _sop, Ckt_Sop_Cat_t _type):
    pCktObj(p_ckt_obj), type(_type), addedER(0)
{
    SOP.assign(_sop.begin(), _sop.end());
}


Ckt_Sing_Sel_ALC_t::Ckt_Sing_Sel_ALC_t(const Ckt_Sing_Sel_ALC_t & other):
    pCktObj(other.pCktObj), type(other.type), addedER(other.addedER)
{
    SOP.assign(other.SOP.begin(), other.SOP.end());
}


Ckt_Sing_Sel_ALC_t::~Ckt_Sing_Sel_ALC_t(void)
{
}


ostream & operator << (ostream & os, const vector <string> & SOP)
{
    for (auto & s : SOP)
        cout << s << "|";
    return os;
}


ostream & operator << (ostream & os, const Ckt_Sing_Sel_ALC_t & ALC)
{
    cout << ALC.pCktObj->GetName() << "\t" << ALC.SOP << "\t" << ALC.type;
    return os;
}


bool Ckt_HasSamePo(Ckt_Sop_Net_t & ckt1, Ckt_Sop_Net_t & ckt2)
{
    if (ckt1.GetPoNum() != ckt2.GetPoNum())
        return false;
    for (int i = 0 ; i < ckt1.GetPoNum(); ++i)
        if (ckt1.GetPo(i)->GetName() != ckt2.GetPo(i)->GetName())
            return false;
    return true;
}


void Ckt_GetALCs(Ckt_Sop_Net_t & ckt, vector <Ckt_Sop_t *> & pOrdObjs, vector < Ckt_Sing_Sel_ALC_t > & ALCs, int maxLiteralNum)
{
    ALCs.clear();
    vector <string> tmp;
    tmp.clear();
    for (auto & pCktObj : pOrdObjs) {
        if (pCktObj->IsPI() || pCktObj->IsPO() || pCktObj->IsConst())
            continue;
        ALCs.emplace_back(Ckt_Sing_Sel_ALC_t(pCktObj, tmp, Ckt_Sop_Cat_t::CONST0));
        ALCs.emplace_back(Ckt_Sing_Sel_ALC_t(pCktObj, tmp, Ckt_Sop_Cat_t::CONST1));
        Ckt_GetALCsRecur(pCktObj, ALCs, 0, 0, 0, maxLiteralNum);
    }
    for (auto & ALC : ALCs)
        cout << ALC << endl;
    cout << ALCs.size() << endl;
}


void Ckt_GetALCsRecur(Ckt_Sop_t * pCktObj, vector < Ckt_Sing_Sel_ALC_t > & ALCs, int i, int j, int n, int maxLiteralNum)
{
    if (n > maxLiteralNum)
        return;
    else if (i >= pCktObj->GetSOPSize()) {
        vector <string> tmp;
        pCktObj->CopySOP(tmp);
        for (auto it = tmp.begin(); it != tmp.end(); ++it) {
            bool isAllDC = true;
            for (auto & ch : *it) {
                if (ch != '-') {
                    isAllDC = false;
                    break;
                }
            }
            if (isAllDC) {
                tmp.erase(it);
                --it;
            }
        }
        if (!tmp.empty() && n)
            ALCs.emplace_back(Ckt_Sing_Sel_ALC_t(pCktObj, tmp, pCktObj->GetType()));
        return;
    }
    else if (j >= pCktObj->GetSOPISize(i))
        Ckt_GetALCsRecur(pCktObj, ALCs, i + 1, 0, n);
    else {
        // do not change
        Ckt_GetALCsRecur(pCktObj, ALCs, i, j + 1, n);
        // change
        if (pCktObj->GetSOPIJ(i, j) != '-') {
            char bak = pCktObj->GetSOPIJ(i, j);
            pCktObj->SetSOPIJ(i, j, '-');
            Ckt_GetALCsRecur(pCktObj, ALCs, i, j + 1, n + 1);
            pCktObj->SetSOPIJ(i, j, bak);
        }
    }
}


void Ckt_EnumerateTest(Ckt_Sop_Net_t & ckt)
{
    Ckt_Sop_Net_t cktRef(ckt);
    assert(Ckt_HasSamePo(ckt, cktRef));
    // get topological sequence
    vector <Ckt_Sop_t *> pOrderedObjs;
    ckt.SortObjects(pOrderedObjs);
    // get base error rate & backup
    cktRef.GenInputDist(314);
    ckt.GenInputDist(314);
    cktRef.FeedForward();
    ckt.FeedForward(pOrderedObjs);
    // get candidate candis
    vector <Ckt_Sing_Sel_ALC_t> candis;
    Ckt_GetALCs(ckt, pOrderedObjs, candis);
    // replace and recover
    // vector <Ckt_Rpl_Info_t> info;
    // for (auto & pr : candis) {
    //     ckt.Replace(*pr.pTS, *pr.pSS, info);
    //     ckt.FeedForward();
    //     cout << pr.pTS->GetName() << "\t" << pr.pSS->GetName() << "\t" << ckt.GetErrorRate(cktRef) << endl;
    //     ckt.RecoverFromRpl(info);
    //     // Ckt_Cec(cktRef, ckt);
    // }
}
