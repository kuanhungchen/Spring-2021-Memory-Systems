#include <iostream>
#include <queue>

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
  bool marked;
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
} req_queue;

int form_new_batch(int *total_load, int *bank_load[NUM_BANK]) {
  int batch_size = 0;
  for (int p = 0; p < NUM_PROC; p++) {
    total_load[p] = 0;
    for (int b = 0; b < NUM_BANK; b++) {
      bank_load[b][p] = 0;
    }
  }
  priority_queue<pair<int, int> > pq[NUM_BANK][NUM_PROC];
  for (int i = 0; i < QUEUE_SIZE; i++) {
    if (req_queue.mask[i] == '1') {
      Req r = req_queue.data[i];
      int b = r.bank, p = r.proc, id = r.id;
      pq[b][p].push(make_pair(id, i));
      if (pq[b][p].size() > MARKING_CAP)
        pq[b][p].pop();
    }
  }
  for (int b = 0; b < NUM_BANK; b++) {
    for (int p = 0; p < NUM_PROC; p++) {
      while (!pq[b][p].empty()) {
        int i = pq[b][p].top().second;
        pq[b][p].pop();
        total_load[p]++;
        bank_load[b][p]++;
        req_queue.data[i].marked = true;
        batch_size++;
      }
    }
  }
  return batch_size;
}

void prioritize_reqs(int *bests, int *total_load, int *bank_load[NUM_BANK], Bank *banks) {
  bool is_marked[NUM_BANK], same_row[NUM_BANK];
  for (int i = 0; i < NUM_BANK; bests[i] = -1, is_marked[i] = false,
                                same_row[i] = false, i++);

  for (int i = 0; i < QUEUE_SIZE; i++) {
    if (req_queue.mask[i] == '1') {
      Req r = req_queue.data[i];
      if (!banks[r.bank].occupied) {
        // bank is able to handle a new req
        if (bests[r.bank] == -1)
          bests[r.bank] = i;
        else {
          Req cur_r = req_queue.data[bests[r.bank]];
          bool cur_is_marked = is_marked[cur_r.bank];
          bool cur_same_row = same_row[cur_r.bank];
          int cur_proc = cur_r.proc;
          int cur_id = cur_r.id;
          int cur_total_load = total_load[cur_proc];
          int cur_max_bank_load = 0;
          for (int b = 0; b < NUM_BANK; b++)
            cur_max_bank_load = max(cur_max_bank_load, bank_load[b][cur_proc]);

          bool new_is_marked = r.marked;
          bool new_same_row = r.row == banks[r.bank].current.row;
          int new_proc = r.proc;
          int new_id = r.id;
          int new_total_load = total_load[new_proc];
          int new_max_bank_load = 0;
          for (int b = 0; b < NUM_BANK; b++)
            new_max_bank_load = max(new_max_bank_load, bank_load[b][new_proc]);

          int cur_priority = 2 * cur_is_marked + 1 * cur_same_row;
          int new_priority = 2 * new_is_marked + 1 * new_same_row;
          if (cur_priority < new_priority) {
            bests[r.bank] = i;
          } else if (cur_priority == new_priority) {
            if (cur_max_bank_load > new_max_bank_load) {
              bests[r.bank] = i;
            } else if (cur_max_bank_load == new_max_bank_load) {
              if (cur_total_load > new_total_load) {
                bests[r.bank] = i;
              } else if (cur_total_load == new_total_load) {
                if (cur_proc < new_proc) {
                  bests[r.bank] = i;
                } else if (cur_proc == new_proc) {
                  if (cur_id > new_id) {
                    bests[r.bank] = i;
                  }
                }
              }
            }
          }
        }
        is_marked[r.bank] = req_queue.data[bests[r.bank]].marked;
        same_row[r.bank] = req_queue.data[bests[r.bank]].row == banks[r.bank].current.row;
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

  // initialize req_queue
  req_queue.size = 0;
  req_queue.data = new Req [QUEUE_SIZE];
  req_queue.mask = string(QUEUE_SIZE, '0');
  // initialize banks
  Bank banks[NUM_BANK];
  for (int i = 0; i < NUM_BANK; banks[i].occupied = false,
                                banks[i].current.row = -1, i++);
  // initialize max_bank_load and total_load
  int total_load[NUM_PROC];
  int *bank_load[NUM_BANK];
  for (int i = 0; i < NUM_BANK; bank_load[i] = new int [NUM_PROC], i++);

  // parse input
  Req reqs[NUM_REQ];
  for (int i = 0; i < NUM_REQ; i++)
    cin >> reqs[i].id >> reqs[i].proc >> reqs[i].bank >> reqs[i].row;

  bool req_in;
  Req cur_req;
  int req_idx = 0;
  bool not_done = true;
  int bests[NUM_BANK];
  int cur_batch_size = 0;
  for (int timestamp = 0; not_done; req_in = false, timestamp++) {
    // check if current req finishes for each bank
    for (int i = 0; i < NUM_BANK; i++) {
      if (banks[i].occupied) {
        if (timestamp > banks[i].etime) {
          banks[i].occupied = false;
          if (banks[i].current.marked) {
            Req r = banks[i].current;
            total_load[r.proc]--;
            bank_load[r.bank][r.proc]--;
            cur_batch_size--;
          }
        }
      }
    }

    // mark new batch if previous is done
    bool batch_not_done = false;
    for (int i = 0; i < QUEUE_SIZE; i++)
      batch_not_done = batch_not_done || req_queue.data[i].marked;
    if (!batch_not_done)
      form_new_batch(total_load, bank_load);

    // req prioritization
    prioritize_reqs(bests, total_load, bank_load, banks);

    // handle the reqs found in req_queue
    for (int i = 0; i < NUM_BANK; i++) {
      if (!(banks[i].occupied) && bests[i] != -1) {
        banks[i].occupied = true;
        banks[i].stime = timestamp;
        if (banks[i].current.row == req_queue.data[bests[i]].row)
          banks[i].etime = timestamp + ROW_HIT_LAT - 1;
        else
          banks[i].etime = timestamp + ROW_MISS_LAT - 1;
        banks[i].current = req_queue.data[bests[i]];
        // update information
        req_queue.mask[bests[i]] = '0';
        req_queue.size--;
      }
    }

    // insert a new request into req_queue
    if (req_queue.size != QUEUE_SIZE && req_idx < NUM_REQ) {
      cur_req = reqs[req_idx];
      cur_req.marked = false;
      req_idx++;
      int idx = 0;
      for (; req_queue.mask[idx] != '0'; idx++);
      // update information
      req_queue.data[idx] = cur_req;
      req_queue.mask[idx] = '1';
      req_queue.size++;
      req_in = true;
    }

    // check if all reqs are handled
    not_done = req_queue.size != 0 || req_idx < NUM_REQ;
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
    for (int i = 0; i < NUM_BANK; i++) {
      print_spaces(3);
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
    /* cout << "  ["; */
    /* for (int i = 0; i < QUEUE_SIZE; i++) { */
    /*   if (req_queue.mask[i] == '2') */
    /*     cout << req_queue.data[i].id << " "; */
    /* } */
    /* cout << "]"; */

    cout << endl;
  }

  return 0;
}



/*

cur_marked, cur_row_hit, cur_score, cur_id
new_marked


*/
