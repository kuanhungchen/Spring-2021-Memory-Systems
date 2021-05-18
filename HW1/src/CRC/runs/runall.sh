# /bin/bash
sim(){
  nthread=$1
  trace=$2
  nset=$3
  icount=$4
  policy=$5
  ../bin/CMPsim.usetrace.64 -threads $nthread -mix ../traces/$trace -o $trace.policy$policy.stats -cache UL3:$nset:64:16 -icount $icount -autorewind 1 -LLCrepl $policy
  duration=$SECONDS
  echo =====================================
  echo simulation time till now : $duration s
  echo =====================================
}

SECONDS=0

for file in trace1 trace2
do
  sim 1 $file 1024 50 0 
  sim 1 $file 1024 50 1 
  sim 1 $file 1024 50 2 
done

for file in trace3 
do
  sim 2 $file 2048 50 0 
  sim 2 $file 2048 50 1 
  sim 2 $file 2048 50 2 
done

for file in trace4 
do
  sim 4 $file 4096 50 0 
  sim 4 $file 4096 50 1 
  sim 4 $file 4096 50 2 
done

