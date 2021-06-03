#include <iostream>
#include <sstream>
#include "replacement_state.h"
using namespace std;

int main()
{
  UINT32 num_sets, num_ways, repl, num_ops;
  cin >> num_sets >> num_ways >> repl >> num_ops;
  cout << num_sets << endl << num_ways << endl << repl << endl << num_ops;

  CACHE_REPLACEMENT_STATE *cacheReplState = new CACHE_REPLACEMENT_STATE(num_sets, num_ways, repl);
  cacheReplState->SetOJ(true);

  string s;
  bool _hit;
  Addr_t _pc;
  UINT32 _set;
  INT32 _way;
  for (UINT32 i = 0, j = 0; i < num_ops + 1; i++, j = 0) {
    getline(cin, s);
    stringstream ss(s);
    while (1) {
      int n;
      ss >> n;

      if (j == 0) {
      } else if (j == 1) {
        _set = (UINT32) n;
      } else if (j == 2) {
        if (n == 1) {
          _hit = true;
        } else {
          _hit = false;
        }
      } else {
        if (_hit) _way = (INT32) n;
        else _pc = (Addr_t) n;
      }
      j++;

      if (!ss) break;
    }
    if (i == 0) continue;

    if (_hit) {
      cacheReplState->UpdateReplacementState(_set, _way, (Addr_t) 0, _hit);
    } else {
      _way = cacheReplState->GetVictimInSet(_set);
      cacheReplState->UpdateReplacementState(_set, _way, _pc, _hit);
    }
  }
  return 0;
}
