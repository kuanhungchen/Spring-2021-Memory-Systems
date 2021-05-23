#include "replacement_state.h"
#include <sched.h>

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

/*
** This file implements the cache replacement state. Users can enhance the code
** below to develop their cache replacement ideas.
**
*/


////////////////////////////////////////////////////////////////////////////////
// The replacement state constructor:                                         //
// Inputs: number of sets, associativity, and replacement policy to use       //
// Outputs: None                                                              //
//                                                                            //
// DO NOT CHANGE THE CONSTRUCTOR PROTOTYPE                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{

    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;

    mytimer    = 0;

    InitReplacementState();
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways
    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];

    // ensure that we were able to create replacement state
    assert(repl);

    // Create the state for the sets
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++)
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];

        for(UINT32 way=0; way<assoc; way++)
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
            // initialize RRPV, signature and outcome (for SRRIP)
            repl[setIndex][way].blockRRPV = 3;
            repl[setIndex][way].blockSignature = 0;
            repl[setIndex][way].blockOutcome = 0;
        }
    }

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE
    InitSHCT(16384);
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the SHCT table by creating storage for the       //
// SHiP.                                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::InitSHCT(UINT32 _num_shct) {
  if (mySHCT)
    delete [] mySHCT;

  numSHCT = _num_shct;
  mySHCT = new UINT32 [numSHCT];
  for (UINT32 i = 0; i < numSHCT; i++)
    mySHCT[i] = 1;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// arguments are the thread id, set index, pointers to ways in current set    //
// and the associativity.  We are also providing the PC, physical address,    //
// and accesstype should you wish to use them at victim selection time.       //
// The return value is the physical way index for the line being replaced.    //
// Return -1 if you wish to bypass LLC.                                       //
//                                                                            //
// vicSet is the current set. You can access the contents of the set by       //
// indexing using the wayID which ranges from 0 to assoc-1 e.g. vicSet[0]     //
// is the first way and vicSet[4] is the 4th physical way of the cache.       //
// Elements of LINE_STATE are defined in crc_cache_defs.h                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc,
                                               Addr_t PC, Addr_t paddr, UINT32 accessType )
{
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU )
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
        return Get_SRRIP_Victim( setIndex );
    }

    // We should never get here
    assert(0);

    return -1; // Returning -1 bypasses the LLC
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// arguments are the set index only.                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 setIndex) {
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU )
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
        return Get_SRRIP_Victim( setIndex );
    }

    // We should never get here
    assert(0);

    return -1; // Returning -1 bypasses the LLC
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The arguments are: the set index, the physical way of the cache,           //
// the pointer to the physical line (should contestants need access           //
// to information of the line filled or hit upon), the thread id              //
// of the request, the PC of the request, the accesstype, and finall          //
// whether the line was a cachehit or not (cacheHit=true implies hit)         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState(
    UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine,
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU )
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
        UpdateSRRIP(setIndex, updateWayID, PC, cacheHit);
    }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The argument are: the set index, the physical way of the cache,            //
// the PC of the request and whether the line was a cache hit or not.         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState(
    UINT32 setIndex, INT32 updateWayID, Addr_t PC, bool cacheHit) {
  // What replacement policy?
  if( replPolicy == CRC_REPL_LRU )
  {
      UpdateLRU( setIndex, updateWayID );
  }
  else if( replPolicy == CRC_REPL_RANDOM )
  {
      // Random replacement requires no replacement state update
  }
  else if( replPolicy == CRC_REPL_CONTESTANT )
  {
      // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
      // Feel free to use any of the input parameters to make
      // updates to your replacement policy
      UpdateSRRIP(setIndex, updateWayID, PC, cacheHit);
  }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//////// HELPER FUNCTIONS FOR REPLACEMENT UPDATE AND VICTIM SELECTION //////////
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the LRU victim in the cache set by returning the       //
// cache block at the bottom of the LRU stack. Top of LRU stack is '0'        //
// while bottom of LRU stack is 'assoc-1'                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
    // Get pointer to replacement state of current set
    LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];

    INT32   lruWay   = 0;

    // Search for victim whose stack position is assoc-1
    for(UINT32 way=0; way<assoc; way++)
    {
        if( replSet[way].LRUstackposition == (assoc-1) )
        {
            lruWay = way;
            break;
        }
    }

    // return lru way
    return lruWay;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);

    return way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the SRRIP victim in the cache set by returning the     //
// cache block whose RRPV is 3.                                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_SRRIP_Victim(UINT32 setIndex) {
  // get pointer to replacement state of current set
  LINE_REPLACEMENT_STATE *replSet = repl[setIndex];

  INT32 victim_way = 0;
  UINT32 maxRRPV = 0;
  for (UINT32 way = 0; way < assoc; way++) {
    if (replSet[way].blockRRPV == 3) {
      maxRRPV = 3;
      victim_way = way;
      break;
    } else if (replSet[way].blockRRPV > maxRRPV) {
      maxRRPV = replSet[way].blockRRPV;
      victim_way = way;
    }
    maxRRPV = max(maxRRPV, replSet[way].blockRRPV);
    if (maxRRPV == 3) {
      victim_way = way;
      break;
    }
  }
  if (maxRRPV != 3) {
    // increment all RRPVs until at least one block reaches 3
    for (UINT32 way = 0; way < assoc; way++) {
      replSet[way].blockRRPV += 3 - maxRRPV;
    }
    // decrement victim block
    replSet[victim_way].blockRRPV -= 1;
  }
  // return victim way
  return victim_way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the LRU update routine for the traditional        //
// LRU replacement policy. The arguments to the function are the physical     //
// way and set index.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
    // Determine current LRU stack position
    UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;

    // Update the stack position of all lines before the current line
    // Update implies incremeting their stack positions by one
    for(UINT32 way=0; way<assoc; way++)
    {
        if( repl[setIndex][way].LRUstackposition < currLRUstackposition )
        {
            repl[setIndex][way].LRUstackposition++;
        }
    }

    // Set the LRU stack position of new line to be zero
    repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the SRRIP update routine for the SRRIP            //
// replacement policy.                                                        //
// If there is a hit, set RRPV of the block to 0; otherwise, set RRPV based   //
// on SHiP.                                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateSRRIP(UINT32 setIndex, INT32 updateWayID,
                                          Addr_t PC, bool cacheHit)
{
  UINT32 _sign;
  bool sign_flag = false, SHCT_flag = false;
  LINE_REPLACEMENT_STATE *replSet = repl[setIndex];
  if (cacheHit) {
    // reset RRPV as RRIP
    replSet[updateWayID].blockRRPV = 0;

    // set outcome as true
    replSet[updateWayID].blockOutcome = 1;

    // increment corresponding SHCT
    _sign = replSet[updateWayID].blockSignature;
    mySHCT[_sign] = min((int)(mySHCT[_sign] + 1), 3);
    SHCT_flag = true;
  } else {
    // decrement SHCT for the victim
    if (replSet[updateWayID].blockOutcome == 0) {
      _sign = replSet[updateWayID].blockSignature;
      mySHCT[_sign] = max((int)(mySHCT[_sign] - 1), 0);
      SHCT_flag = true;
    }

    // set outcome as zero and a new signature for the new line
    replSet[updateWayID].blockOutcome = 0;
    _sign = replSet[updateWayID].blockSignature;
    replSet[updateWayID].blockSignature = PC % numSHCT;
    sign_flag = true;
    // set RRPV based on signature
    if (mySHCT[replSet[updateWayID].blockSignature] == 0)
      replSet[updateWayID].blockRRPV = 3;
    else
      replSet[updateWayID].blockRRPV = 2;
  }

  if (isOJ) {
    PrintRequest(setIndex, updateWayID, PC, cacheHit);
    PrintCacheReplStatus(replSet, updateWayID, cacheHit, _sign, SHCT_flag, sign_flag);
  }
  IncrementTimer();
}

void CACHE_REPLACEMENT_STATE::PrintCacheReplStatus(
    const LINE_REPLACEMENT_STATE *currLine, INT32 updateWayID, bool cacheHit, UINT32 sign, bool SHCT_flag, bool sign_flag)
{
  // RRPV
  cout << "RRPV:     ";
  for (UINT32 way = 0; way < assoc; way++) {
    if (way == (UINT32)updateWayID)
      cout << " (" << currLine[way].blockRRPV << ")";
    else
      cout << " " << currLine[way].blockRRPV;
  }
  cout << endl;

  // signature
  cout << "Signature:";
  for (UINT32 way = 0; way < assoc; way++) {
    if (sign_flag && way == (UINT32)updateWayID)
      cout << " (" << currLine[way].blockSignature << ")";
    else
      cout << " " << currLine[way].blockSignature;
  }
  cout << endl;

  // outcome
  cout << "outcome:  ";
  for (UINT32 way = 0; way < assoc; way++) {
    if (way == (UINT32)updateWayID)
      cout << " (" << currLine[way].blockOutcome << ")";
    else
      cout << " " << currLine[way].blockOutcome;
  }
  cout << endl;

  // SHCT
  cout << "SHCT:     ";
  for (int i = 0; (UINT32)i < numSHCT; i++) {
    if (SHCT_flag && sign == (UINT32)i)
      cout << " (" << mySHCT[i] << ")";
    else
      cout << " " << mySHCT[i];
  }
  cout << endl;
}

void CACHE_REPLACEMENT_STATE::PrintRequest(UINT32 setIndex, INT32 updateWayID, Addr_t PC, bool cacheHit)
{
  cout << endl;
  cout << mytimer << " " << PC << " " << setIndex << " ";
  if (cacheHit)
    cout << "Hit" << " " << updateWayID;
  else
    cout << "Replace" << " "<< updateWayID;
  cout << endl;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The function prints the statistics for the cache                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{

    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;

    // CONTESTANTS:  Insert your statistics printing here

    return out;

}

