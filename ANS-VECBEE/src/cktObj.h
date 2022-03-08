#ifndef CKT_OBJ_H
#define CKT_OBJ_H


#include "header.h"
#include "abcApi.h"
#include "cktBit.h"
#include "cktNtk.h"


class Ckt_Ntk_t;


class Ckt_Obj_t
{
private:
    abc::Abc_Obj_t  *                               pAbcObj;            // the ABC object
    std::weak_ptr   < Ckt_Ntk_t >                   pCktNtk;            // the CKT network
    std::vector     < uint64_t >                    simValues;          // simluation value
    std::vector     < std::shared_ptr <Ckt_Obj_t> > pCktFanins;         // fanins
    std::vector     < std::shared_ptr <Ckt_Obj_t> > pCktFanouts;        // fanouts

    Ckt_Obj_t &                                     operator =          (const Ckt_Obj_t &);

public:
    explicit                                        Ckt_Obj_t           (abc::Abc_Obj_t * p_abc_obj, std::shared_ptr <Ckt_Ntk_t> p_ckt_ntk);
                                                    Ckt_Obj_t           (const Ckt_Obj_t & other);
                                                    ~Ckt_Obj_t          (void);
};


#endif
