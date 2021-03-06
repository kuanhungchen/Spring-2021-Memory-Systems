#ifndef REPL_STATE_H
#define REPL_STATE_H

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cassert>
#include "utils.h"
#include "crc_cache_defs.h"

// Replacement Policies Supported
typedef enum
{
    CRC_REPL_LRU        = 0,
    CRC_REPL_RANDOM     = 1,
    CRC_REPL_CONTESTANT = 2
} ReplacemntPolicy;

// Replacement State Per Cache Line
typedef struct
{
    UINT32  LRUstackposition;

    // CONTESTANTS: Add extra state per cache line here
    UINT32 blockRRPV;
    UINT32 blockOutcome;
    UINT32 blockSignature;

} LINE_REPLACEMENT_STATE;


// The implementation for the cache replacement policy
class CACHE_REPLACEMENT_STATE
{

  private:
    UINT32 numsets;
    UINT32 assoc;
    UINT32 replPolicy;

    LINE_REPLACEMENT_STATE   **repl;

    COUNTER mytimer;  // tracks # of references to the cache

    // CONTESTANTS:  Add extra state for cache here
    UINT32 *mySHCT;
    bool isOJ;

  public:

    UINT32 numSHCT;
    // The constructor CAN NOT be changed
    CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol );

    INT32  GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType );
    void   UpdateReplacementState(UINT32 setIndex, INT32 updateWayID,
                                  Addr_t PC, bool cacheHit);

    void   SetReplacementPolicy( UINT32 _pol ) { replPolicy = _pol; }
    void   IncrementTimer() { mytimer++; }
    void   SetOJ(bool x) { isOJ = x; }

    void   InitSHCT(UINT32 _num_shct);

    INT32  GetVictimInSet(UINT32 setIndex);
    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine,
                                   UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit );

    ostream&   PrintStats( ostream &out);

  private:

    void   InitReplacementState();
    INT32  Get_Random_Victim( UINT32 setIndex );

    INT32  Get_LRU_Victim( UINT32 setIndex );
    void   UpdateLRU( UINT32 setIndex, INT32 updateWayID );

    INT32  Get_SRRIP_Victim(UINT32 setIndex);
    void UpdateSRRIP(UINT32 setIndex, INT32 updateWayID, Addr_t PC, bool cacheHit);

    void PrintRequest(UINT32 setIndex, INT32 updateWayID, Addr_t PC,
                      bool cacheHit);
    void PrintCacheReplStatus(const LINE_REPLACEMENT_STATE *currLine, INT32 updateWayID,
                              bool cacheHit, UINT32 sign, bool sign_flag, bool SHCT_flag);

};

#endif
