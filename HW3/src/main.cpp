#include <iostream>
#include <map>

using namespace std;

void print_spaces(int x) {for (int i = 0; i < x; cout << " ", i++);}
int num_digits(int x) {
  if (x == 0) return 1;
  int digits = 0;
  for (; x; x /= 10, digits++);
  return digits;
}

int NUM_PROC, NUM_BANK, QUEUE_SIZE;
int POLICY, ROW_HIT_LAT, ROW_MISS_LAT, MARKING_CAP, NUM_REQ;

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
  string mask;
} queue;

void find_best_reqs(int bests[NUM_BANK], int t, Bank banks[NUM_BANK]) {
  // find a req to handle for each bank if any
  bool same_row = false;
  for (int i = 0; i < NUM_BANK; bests[i] = -1, i++);
  if (POLICY == 0) {
    // first-come-first-serve
    for (int i = 0; i < QUEUE_SIZE; i++) {
      if (queue.mask[i] == '1' && queue.data[i].id != t) {
        Req r = queue.data[i];
        if (!banks[r.bank].occupied) {
          // bank is able to handle a new req
          if (bests[r.bank] == -1) {
            bests[r.bank] = i;
          } else {
            if (r.id < queue.data[bests[r.bank]].id)
              bests[r.bank] = i;
          }
        }
      }
    }
  }
}

int main() {
  cin >> NUM_PROC >> NUM_BANK >> QUEUE_SIZE;
  cin >> POLICY >> ROW_HIT_LAT >> ROW_MISS_LAT >> MARKING_CAP >> NUM_REQ;
  cout << NUM_PROC << endl << NUM_BANK << endl << QUEUE_SIZE << endl;
  cout << POLICY << endl << ROW_HIT_LAT << endl << ROW_MISS_LAT << endl;
  cout << MARKING_CAP << endl << NUM_REQ << endl;

  // initialize queue
  queue.size = 0;
  queue.data = new Req [QUEUE_SIZE];
  queue.mask = string(QUEUE_SIZE, '0');
  // initialize banks
  Bank banks[NUM_BANK];
  for (int i = 0; i < NUM_BANK; banks[i].occupied = false, i++);
  for (int i = 0; i < NUM_BANK; banks[i].current.row = -1, i++);

  // parse input
  Req reqs[NUM_REQ];
  for (int i = 0; i < NUM_REQ; i++)
    cin >> reqs[i].id >> reqs[i].proc >> reqs[i].bank >> reqs[i].row;

  bool req_in;
  Req cur_req;
  int req_idx = 0;
  bool not_done = true;
  int bests[NUM_BANK];
  for (int timestamp = 0; not_done; timestamp++) {
    req_in = false;
    // check if current req finishes for each bank
    for (int i = 0; i < NUM_BANK; i++) {
      if (banks[i].occupied && timestamp > banks[i].etime)
        banks[i].occupied = false;
    }

    // insert a new request into queue
    if (queue.size != QUEUE_SIZE && req_idx < NUM_REQ) {
      cur_req = reqs[req_idx];
      req_idx++;
      int idx = 0;
      for (; queue.mask[idx] != '0'; idx++);
      queue.data[idx] = cur_req;
      queue.mask[idx] = '1';
      queue.size++;
      req_in = true;
    }

    find_best_reqs(bests, timestamp, banks);
    // handle the reqs found in queue
    for (int i = 0; i < NUM_BANK; i++) {
      if (!banks[i].occupied) {
        if (bests[i] != -1) {
          banks[i].occupied = true;
          banks[i].stime = timestamp;
          if (banks[i].etime == timestamp - 1 &&
              banks[i].current.row == queue.data[bests[i]].row)
            banks[i].etime = timestamp + ROW_HIT_LAT - 1;
          else
            banks[i].etime = timestamp + ROW_MISS_LAT - 1;
          banks[i].current = queue.data[bests[i]];
          queue.mask[bests[i]] = '0';
          queue.size--;
        }
      }
    }

    not_done = queue.size != 0 || req_idx < NUM_REQ;
    for (int i = 0; i < NUM_BANK; i++)
      not_done = not_done || banks[i].occupied;
    if (!not_done)
      break;

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
      if (i != NUM_BANK - 1)
        print_spaces(3);
    }
    cout << endl;

  }
  return 0;
}
