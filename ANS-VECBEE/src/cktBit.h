#ifndef CKT_BIT_H
#define CKT_BIT_H


#include "header.h"


class Ckt_Bit_Cnt_t
{
private:
    int table[65536];

public:
    explicit                  Ckt_Bit_Cnt_t     (void);
                              ~Ckt_Bit_Cnt_t    (void);

    inline int                GetOneNum         (uint64_t n)                { return table[n >> 48] + table[(n >> 32) & 0xffff] + table[(n >> 16) & 0xffff] +table[n & 0xffff]; }
};


static inline void            Ckt_SetBit        (uint64_t & x, uint64_t f)  { x |= ((uint64_t)1 << (f & (uint64_t)63)); }
static inline void            Ckt_ResetBit      (uint64_t & x, uint64_t f)  { x &= ~((uint64_t)1 << (f & (uint64_t)63)); }
static inline bool            Ckt_GetBit        (uint64_t x, uint64_t f)    { return (bool)((x >> f) & (uint64_t)1); }
static inline int             Ckt_CountOneNum   (uint64_t n)                { static Ckt_Bit_Cnt_t table; return table.GetOneNum(n); }


#endif
