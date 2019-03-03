#ifndef CKT_NTK_H
#define CKT_NTK_H


#include "cktObj.h"


class Ckt_Obj_t;


class Ckt_Ntk_t
: std::enable_shared_from_this <Ckt_Ntk_t>
{
private:
    int                                               nSim;               // nSim = # frame / 64, discard remaining frames
    abc::Abc_Ntk_t  *                                 pAbcNtk;            // the ABC network
    std::vector     < std::weak_ptr   <Ckt_Obj_t> >   pCktObjs;           // objects
    std::vector     < std::shared_ptr <Ckt_Obj_t> >   pCktPis;            // primary inputs
    std::vector     < std::shared_ptr <Ckt_Obj_t> >   pCktPos;            // primary outputs

    Ckt_Ntk_t &                                       operator =          (const Ckt_Ntk_t &);

public:
    explicit                                          Ckt_Ntk_t           (abc::Abc_Ntk_t * p_abc_ntk, int nFrames = 1024);
                                                      Ckt_Ntk_t           (const Ckt_Ntk_t & other);
                                                      ~Ckt_Ntk_t          (void);
};


#endif
