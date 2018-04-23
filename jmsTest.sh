#!/bin/bash

#Functions
function runclients {
threads=$1
msgsize=$2
  echo "threads=$threads" >> /home/mqperf/jms/results
  echo "Starting test with $threads JMS requesters" >> /home/mqperf/jms/output
  ./jmsreq.sh $threads $msgsize | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
  awk '{print $12}' /tmp/mpstat | tail -6 |  awk '{total+=$1} END{printf "CPU=%0.2f\n",(NR?100-(total/NR):-1)}' >> /home/mqperf/jms/results
  awk -F ',' '{print $7}' /tmp/dstat | tail -n +8 | tail -6 |  awk '{total+=$1} END{printf "Read=%0.2f\n",(NR?((total/NR)/(1024*1024)):-1)}' >> /home/mqperf/jms/results
  awk -F ',' '{print $8}' /tmp/dstat | tail -n +8 | tail -6 |  awk '{total+=$1} END{printf "Write=%0.2f\n",(NR?((total/NR)/(1024*1024)):-1)}' >> /home/mqperf/jms/results
  awk -F ',' '{print $9}' /tmp/dstat | tail -n +8 | tail -6 |  awk '{total+=$1} END{printf "Recv=%0.2f\n",(NR?((total/NR)/(1024*1024*1024*0.125)):-1)}' >> /home/mqperf/jms/results
  awk -F ',' '{print $10}' /tmp/dstat | tail -n +8 | tail -6 |  awk '{total+=$1} END{printf "Send=%0.2f\n",(NR?((total/NR)/(1024*1024*1024*0.125)):-1)}' >> /home/mqperf/jms/results
  tail -6 /tmp/systemerr | awk -F '=' '{print $2}' | awk '{total+=$1} END{printf "QM_CPU=%0.2f\n",(NR?(total/NR):-1)}' >> /home/mqperf/cph/results
  echo "" >> /home/mqperf/jms/results
}

echo "----------------------------------------"
echo "Initialising test environment-----------"
echo "----------------------------------------"
qmname="${MQ_QMGR_NAME:-PERF0}"
host="${MQ_QMGR_HOSTNAME:-localhost}"
port="${MQ_QMGR_PORT:-1420}"
channel="${MQ_QMGR_CHANNEL:-SYSTEM.DEF.SVRCONN}"
nonpersistent="${MQ_NON_PERSISTENT:-0}"

echo $(date)
echo $(date) > /home/mqperf/jms/results

if [ "${nonpersistent}" -eq 1 ]; then
  echo "Running Non Persistent JMS Messaging Tests"
  echo "Running Non Persistent JMS Messaging Tests" >> /home/mqperf/jms/results
else
  echo "Running Persistent JMS Messaging Tests"
  echo "Running Persistent JMS Messaging Tests" >> /home/mqperf/jms/results
fi
echo "----------------------------------------"

echo "Testing QM: $qmname on host: $host using port: $port and channel: $channel" 
echo "Testing QM: $qmname on host: $host using port: $port and channel: $channel" >> /home/mqperf/jms/results

if [ -n "${MQ_JMS_EXTRA}" ]; then
  echo "Extra JMS flags: ${MQ_JMS_EXTRA}" 
  echo "Extra JMS flags: ${MQ_JMS_EXTRA}" >> /home/mqperf/jms/results
fi

export MQSERVER="$channel/TCP/$host($port)";
if [ -n "${MQ_USERID}" ]; then
  # Need to flow userid and password to runmqsc
  echo "Using userid: ${MQ_USERID}" 
  echo "Using userid: ${MQ_USERID}" >> /home/mqperf/jms/results
  echo ${MQ_PASSWORD} > /tmp/clearq.mqsc;
  cat /home/mqperf/jms/clearq.mqsc >> /tmp/clearq.mqsc;  
  cat /tmp/clearq.mqsc | /opt/mqm/bin/runmqsc -c -u ${MQ_USERID} -w 60 $qmname > /home/mqperf/jms/output;
  rm -f /tmp/clearq.mqsc;
else
  cat /home/mqperf/jms/clearq.mqsc | /opt/mqm/bin/runmqsc -c $qmname > /home/mqperf/jms/output 2>&1;
fi

#Launch monitoring processes
mpstat 10 > /tmp/mpstat &
dstat --output /tmp/dstat 10 > /dev/null 2>&1 &
if [ -n "${MQ_USERID}" ]; then
  ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c CPU -t SystemSummary -u ${MQ_USERID} -v ${MQ_PASSWORD} >/tmp/system 2>/tmp/systemerr &
  ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c DISK -t Log -u ${MQ_USERID} -v ${MQ_PASSWORD} >/tmp/disklog 2>/tmp/disklogerr &
else
  ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c CPU -t SystemSummary >/tmp/system 2>/tmp/systemerr &
  ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c DISK -t Log >/tmp/disklog 2>/tmp/disklogerr &
fi

echo "----------------------------------------"
echo "Starting JMS tests----------------------"
echo "----------------------------------------"
./jmsresp.sh ${MQ_RESPONDER_THREADS} >> /home/mqperf/jms/output &
#Wait for responders to start
sleep 60
echo "JMS Test Results" >> /home/mqperf/jms/results
echo $(date) >> /home/mqperf/jms/results
echo "2K" >> /home/mqperf/jms/results
runclients 1
runclients 8
runclients 16
runclients 32
runclients 64
runclients 128
runclients 200

echo "----" >> /home/mqperf/jms/results
echo $(date) >> /home/mqperf/jms/results
echo "20K" >> /home/mqperf/jms/results
runclients 1 20480
runclients 8 20480
runclients 16 20480
runclients 32 20480
runclients 64 20480
runclients 128 20480
runclients 200 20480

echo "----" >> /home/mqperf/jms/results
echo $(date) >> /home/mqperf/jms/results
echo "200K" >> /home/mqperf/jms/results
runclients 1 204800
runclients 8 204800
runclients 16 204800
runclients 32 204800
runclients 64 204800
runclients 128 204800
runclients 200 204800
echo "" >> /home/mqperf/jms/results
cat /home/mqperf/jms/results
echo "----------------------------------------"
echo "jms testing finished--------------------"
echo "----------------------------------------"
