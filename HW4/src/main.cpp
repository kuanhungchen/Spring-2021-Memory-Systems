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

struct Proc {
  // special struct for the process ranking
  int tot, mx, id;
  bool operator < (const Proc &p) const {
    if (mx != p.mx) return mx > p.mx;
    else if (tot != p.tot) return tot > p.tot;
    else return id < p.id;
  }
} proc;

int form_new_batch(int *proc_tot_load) {
  // mark requests to form a new batch
  int batch_size = 0;
  priority_queue<pair<int, int> > pq[NUM_BANK][NUM_PROC];
  for (int i = 0; i < QUEUE_SIZE; i++) {
    if (req_queue.mask[i] == '1') {
      Req r = req_queue.data[i];
      int b = r.bank, p = r.proc, id = r.id;
      pq[b][p].push(make_pair(id, i));
      if (pq[b][p].size() > MARKING_CAP)
        // pop out element if exceeds the capacity
        pq[b][p].pop();
    }
  }
  for (int b = 0; b < NUM_BANK; b++) {
    for (int p = 0; p < NUM_PROC; p++) {
      while (!pq[b][p].empty()) {
        int i = pq[b][p].top().second;
        pq[b][p].pop();
        req_queue.data[i].marked = true;
        batch_size++;
        proc_tot_load[p]++;
      }
    }
  }
  return batch_size;
}

void rank_procs(int *proc_tot_load, int *proc_ranking) {
  // compute the ranks of process after forming a batch
  priority_queue<Proc> pq;
  int proc_num_in_batch[NUM_BANK][NUM_PROC];
  for (int b = 0; b < NUM_BANK; b++)
    for (int p = 0; p < NUM_PROC; proc_num_in_batch[b][p] = 0, p++);
  for (int i = 0; i < QUEUE_SIZE; i++) {
    if (req_queue.mask[i] == '1' && req_queue.data[i].marked) {
      Req r = req_queue.data[i];
      proc_num_in_batch[r.bank][r.proc]++;
    }
  }
  for (int p = 0; p < NUM_PROC; p++) {
    proc.id = p;
    proc.tot = proc_tot_load[p];
    proc.mx = -1;
    for (int b = 0; b < NUM_BANK; b++)
      proc.mx = max(proc.mx, proc_num_in_batch[b][p]);
    pq.push(proc);
  }
  int rank = 0;
  while (!pq.empty()) {
    int p = pq.top().id;
    proc_ranking[p] = rank;
    pq.pop();
    rank++;
  }
}

void prioritize_reqs(int *bests, int *proc_ranking, Bank *banks) {
  // prioritize the requests in queue by defined prioritization rules
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
          int cur_proc_ranking = proc_ranking[cur_r.proc];

          bool new_is_marked = r.marked;
          bool new_same_row = r.row == banks[r.bank].current.row;
          int new_proc_ranking = proc_ranking[r.proc];

          int cur_priority = 2 * cur_is_marked + 1 * cur_same_row;
          int new_priority = 2 * new_is_marked + 1 * new_same_row;

          if (cur_priority < new_priority) {
            // rule BS and rule RH
            bests[r.bank] = i;
          } else if (cur_priority == new_priority) {
            if (cur_proc_ranking > new_proc_ranking) {
              // rule RANK
              bests[r.bank] = i;
            } else if (cur_proc_ranking == new_proc_ranking) {
              if (cur_r.id > r.id)
                // rule FCFS
                bests[r.bank] = i;
            }
          }
        }
        // update current best option
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
  // initialize process load and ranking
  int proc_tot_load[NUM_PROC];
  int proc_ranking[NUM_PROC];
  for (int p = 0; p < NUM_PROC; proc_tot_load[p] = 0,
                                proc_ranking[p] = -1, p++);

  // parse input
  Req reqs[NUM_REQ];
  for (int i = 0; i < NUM_REQ; i++)
    cin >> reqs[i].id >> reqs[i].proc >> reqs[i].bank >> reqs[i].row;

  bool req_in;
  Req cur_req;
  int req_idx = 0;
  bool not_done = true;
  int bests[NUM_BANK];
  int batch_size = 0;
  for (int timestamp = 0; not_done; req_in = false, timestamp++) {
    // check if current req finishes for each bank
    for (int b = 0; b < NUM_BANK; b++) {
      if (banks[b].occupied) {
        if (timestamp > banks[b].etime)
          banks[b].occupied = false;
      }
    }

    if (batch_size == 0) {
      // if previous batch is done, form a new one and re-rank the processes
      batch_size = form_new_batch(proc_tot_load);
      rank_procs(proc_tot_load, proc_ranking);
    }

    // prioritize requests
    prioritize_reqs(bests, proc_ranking, banks);

    // insert a new request into req_queue
    if (req_queue.size != QUEUE_SIZE && req_idx < NUM_REQ) {
      cur_req = reqs[req_idx];
      cur_req.marked = false;
      req_idx++;
      int idx = 0;
      for (; req_queue.mask[idx] != '0'; idx++);  // find a position to insert
      req_queue.data[idx] = cur_req;
      req_queue.mask[idx] = '1';
      req_queue.size++;
      req_in = true;
    }

    // handle the requests found in queue
    for (int b = 0; b < NUM_BANK; b++) {
      if (bests[b] != -1) {
        banks[b].occupied = true;
        banks[b].stime = timestamp;
        if (banks[b].current.row == req_queue.data[bests[b]].row)
          banks[b].etime = timestamp + ROW_HIT_LAT - 1;
        else
          banks[b].etime = timestamp + ROW_MISS_LAT - 1;
        banks[b].current = req_queue.data[bests[b]];
        if (banks[b].current.marked)
          // decrement current batch size
          batch_size--;
        req_queue.mask[bests[b]] = '0';
        req_queue.size--;
      }
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
    cout << endl;
  }

  return 0;
}
