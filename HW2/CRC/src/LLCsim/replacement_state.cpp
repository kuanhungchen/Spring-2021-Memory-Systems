#include "replacement_state.h"

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
    isOJ = false;

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

        for(INT32 way=0; way < (INT32) assoc; way++)
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
        }
    }

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE

    // indicate the addrs of cached lines in each set
    cachedAddrs = new Addr_t* [numsets];
    for (UINT32 setIndex = 0; setIndex < numsets; setIndex++) {
      cachedAddrs[setIndex] = new Addr_t[assoc];
      for (INT32 way = 0; way < (INT32) assoc; way++)
        cachedAddrs[setIndex][way] = (Addr_t) -1;
    }
    // map addr to distance distribution, set-specific
    addr2dist = new unordered_map<Addr_t, string> [numsets];
    // map addr to age, set-specific
    addr2age = new unordered_map<Addr_t, UINT32> [numsets];
    // map addr to last access time, set-specific
    addr2access = new unordered_map<Addr_t, UINT32> [numsets];
    // count access times for each set
    setTimers = new UINT32 [numsets];
    for (UINT32 set = 0; set < numsets; set++)
      setTimers[set] = 0;
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
        return Get_PRP_Victim(setIndex);
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
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet(UINT32 setIndex)
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
        return Get_PRP_Victim(setIndex);
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
        UpdatePRP(setIndex, updateWayID, PC, cacheHit);
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
      UpdatePRP(setIndex, updateWayID, PC, cacheHit);
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
// This function finds the PRP victim in the cache set by returning the       //
// cache block with lowest weighted importance.                               //
// If a line has not been accessed, its score is considered -1.               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_PRP_Victim(UINT32 setIndex) {
  INT32 victim_way = 0;
  INT32 minImportance = 20;
  for (INT32 way = 0; way < (INT32) assoc; way++) {
    Addr_t addr = cachedAddrs[setIndex][way];
    INT32 imp = -1;
    if (addr != (Addr_t) -1) {
      string dist = addr2dist[setIndex][addr];
      UINT32 age = setTimers[setIndex] - addr2access[setIndex][addr];
      imp = Compute_Importance(dist, age);
    }
    if (imp < minImportance) {
      minImportance = imp;
      victim_way = way;
    }
  }
  return victim_way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Given distance distribution and age of target cache line, return the       //
// weighted importance.                                                       //
// If the distribution vector is (0, 0, 0, 0, 0, 0), i.e. the line is used    //
// first time, then return its score as 0.                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Compute_Importance(string dist, UINT32 age) {
  UINT32 idx = 1, age_range = 16 * assoc;
  UINT32 importance[6] = {15, 14, 12, 10, 9, 1};
  UINT32 numer = 0, denom = 0;
  for (; age_range >= assoc && age < age_range; idx++, age_range /= 2);
  for (UINT32 i = 0; i < idx; i++) {
    denom += (dist[6 - i - 1] - '0');
    numer += (dist[6 - i - 1] - '0') * importance[6 - i - 1];
  }
  return (denom == 0) ? 0 : numer / denom;
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

void CACHE_REPLACEMENT_STATE::UpdatePRP(UINT32 setIndex, INT32 updateWayID,
                                        Addr_t PC, bool cacheHit)
{
  Addr_t addr;

  // decide addr
  if (cacheHit)
    addr = cachedAddrs[setIndex][updateWayID];
  else
    addr = cachedAddrs[setIndex][updateWayID] = PC & 0xffffffc0;

  // increment set-specific timer
  setTimers[setIndex]++;

  // update age
  if (addr2access[setIndex].find(addr) == addr2access[setIndex].end())
    addr2age[setIndex][addr] = 0;
  else
    addr2age[setIndex][addr] = setTimers[setIndex] - addr2access[setIndex][addr] - 1;

  // update distance distribution
  if (addr2dist[setIndex].find(addr) == addr2dist[setIndex].end()) {
    addr2dist[setIndex][addr] = "000000";
  } else {
    UINT32 age = addr2age[setIndex][addr];
    UINT32 idx = 5, age_range = 16 * assoc;
    for (; age_range >= assoc && age < age_range; idx--, age_range /= 2);
    addr2dist[setIndex][addr][idx]++;
    if ((addr2dist[setIndex][addr][idx] - '0') > 15) {
      for (UINT32 i = 0; i < 6; i++)
        addr2dist[setIndex][addr][i] = '0' + (addr2dist[setIndex][addr][i] - '0') / 2;
    }
  }

  // update access time
  addr2access[setIndex][addr] = setTimers[setIndex];

  if (isOJ)
    PrintResult(setIndex, updateWayID, cacheHit);

  // increment access times of current set
  IncrementTimer();
}

void CACHE_REPLACEMENT_STATE::PrintResult(UINT32 setIndex, INT32 updateWayID, bool cacheHit) {
  Addr_t addr = cachedAddrs[setIndex][updateWayID];

  cout << endl;
  cout << mytimer << " " << setIndex << " " << (cacheHit ? "Hit " : "Miss Replace ") << updateWayID << endl;
  cout << "Line address: " << addr << endl;

  cout << "Line reuse:  ";
  for (UINT32 i = 0; i < 6; i++)
    cout << " " << addr2dist[setIndex][addr][i] - '0';
  cout << endl;

  cout << "Way scores:  ";
  for (INT32 way = 0; way < (INT32) assoc; way++) {
    Addr_t addr = cachedAddrs[setIndex][way];
    if (addr == (Addr_t) -1) {
      cout << " -1";
    } else {
      string dist = addr2dist[setIndex][addr];
      UINT32 age = setTimers[setIndex] - addr2access[setIndex][addr];
      cout << " " << Compute_Importance(dist, age);
    }
  }
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

