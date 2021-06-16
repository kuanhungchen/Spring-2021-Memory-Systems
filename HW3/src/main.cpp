#include <iostream>
#include <sstream>

using namespace std;

int NUM_PROC, NUM_BANK, QUEUE_SIZE;
int POLICY, ROW_HIT_LAT, ROW_MISS_LAT, MARKING_CAP, NUM_REQ;

int num_digits(int x) {
  int digits = 0;
  while (x) {
    x /= 10;
    digits++;
  }
  return digits;
}
void print_spaces(int x) {
  for (int i = 0; i < x; i++, cout << " ");
}

struct Req {
  int id;
  int proc;
  int bank;
  int row;
};

struct Bank {
  bool occupied;
  int stime;
  int etime;
  Req current;
};

struct Queue {
  int size;
  Req *data;
} queue;

int main() {
  cin >> NUM_PROC >> NUM_BANK >> QUEUE_SIZE;
  cin >> POLICY >> ROW_HIT_LAT >> ROW_MISS_LAT >> MARKING_CAP >> NUM_REQ;

  // allocate memory space for queue
  queue.data = new Req [QUEUE_SIZE];
  // initialize banks
  Bank banks[NUM_BANK];

  // parse input
  Req reqs[NUM_REQ];
  string s;
  for (int i = 0, j = 0; i < NUM_REQ + 1; i++, j = 0) {
    getline(cin, s);
    stringstream ss(s);
    while (1) {
      int n;
      ss >> n;
      switch (j) {
        case 0:
          reqs[i].id = n;
          break;
        case 1:
          reqs[i].proc = n;
          break;
        case 2:
          reqs[i].bank = n;
          break;
        case 3:
          reqs[i].row = n;
          break;
        default:
          break;
      }
      j++;
      if (!ss) break;
    }
  }

  // start handling requests
  int timestamp = 0;
  int req_idx = 0;
  bool req_in;
  Req cur_req;
  while ((queue.size != 0) || (req_idx < NUM_REQ)) {
    req_in = false;
    // insert a new request into queue
    if (queue.size != QUEUE_SIZE) {
      cur_req = reqs[req_idx];
      req_idx++;
      queue.data[queue.size] = cur_req;
      queue.size++;
      req_in = true;
    }
    for (int i = 0; i < NUM_BANK; i++) {
      // check if finish current request
      if (banks[i].occupied && timestamp > banks[i].etime)
        banks[i].occupied = false;
      // check if possible to handle a new request
      if (!banks[i].occupied) {
        // TODO: select a best req to handle if any
        // TODO: decide latency
        banks[i].occupied;
        banks[i].stime = timestamp;
        // TODO: setup etime
      }
    }
    // print output for this timestamp
    cout << timestamp;
    print_spaces(7 - num_digits(timestamp));
    if (req_in) {
      cout << "t" << cur_req.id;
      print_spaces(5 - num_digits(cur_req.id));
      cout << "P" << cur_req.proc;
      print_spaces(2 - num_digits(cur_req.proc));
      cout << "B" << cur_req.bank;
      print_spaces(2 - num_digits(cur_req.bank));
      cout << "(" << cur_req.row;
      print_spaces(2 - num_digits(cur_req.row));
      cout << ")";
    } else {
      print_spaces(16);
    }
    print_spaces(3);
    for (int i = 0; i < NUM_BANK; i++) {
      if (banks[i].occupied) {
        if (timestamp == banks[i].stime) {
          Req r = banks[i].current;
          cout << "t" << r.id;
          print_spaces(5 - num_digits(r.id));
          cout << "P" << r.proc;
          print_spaces(2 - num_digits(r.proc));
          cout << "B" << r.bank;
          print_spaces(2 - num_digits(r.bank));
          cout << "(" << r.row;
          print_spaces(2 - num_digits(r.row));
          cout << ")";
        } else if (timestamp == banks[i].etime) {
          cout << " -------------- ";
        } else {
          cout << "|              |";
        }
      } else {
        print_spaces(16);
      }
    }


    timestamp++;
  }

  return 0;
}

/*

   Bank records remaining time of current req(process, bank, row)
   Queue contains `sz_q` elements, each with (entry time, process, bank, row)
   Each timestamp:
   - update remaining time of req in each bank
   - if queue is not full, insert a new req into queue
   - check each bank, if available then put req into it
   - print for each bank

   struct Bank {
      bool occupied;
      int entering_time;
      req current;
   };

   reqs = [req1, req2, ...];

   struct Queue {
      int size;
      req **data;
      // queue.data[i][j]. i: idx of bank; j: idx of req
   };

   struct Req {
      int entry;
      int proc;
      int bank;
      int row;
   };

   while queue.size != 0 and req_idx != total_reqs:
      // each iteration
      for i in num_banks:
          update_bank(i);
      // at most insert one req
      if queue.size != size_queue:
          req = reqs[req_idx];
          req_idx += 1;
          queue.insert(req);
          queue.size += 1;
      for i in num_banks:
          if not bank[i].occupied:

              insert

*/
