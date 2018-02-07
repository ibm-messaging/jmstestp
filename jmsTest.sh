#!/bin/bash
echo "----------------------------------------"
echo "Initialising test environment-----------"
echo "----------------------------------------"
qmname="${MQ_QMGR_NAME:-PERF0}"
host="${MQ_QMGR_HOSTNAME:-localhost}"
port="${MQ_QMGR_PORT:-1420}"
channel="${MQ_QMGR_CHANNEL:-SYSTEM.DEF.SVRCONN}"
nonpersistent="${MQ_NON_PERSISTENT:-0}"

if [ "${nonpersistent}" -eq 1 ]; then
  echo "Running Non Persistent JMS Messaging Tests"
  echo "Running Non Persistent JMS Messaging Tests" > /home/mqperf/jms/results
else
  echo "Running Persistent JMS Messaging Tests"
  echo "Running Persistent JMS Messaging Tests" > /home/mqperf/jms/results
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
echo "----------------------------------------"
echo "Starting JMS tests----------------------"
echo "----------------------------------------"
./jmsresp.sh ${MQ_RESPONDER_THREADS} >> /home/mqperf/jms/output &
#Wait for responders to start
sleep 60
echo "JMS Test Results" >> /home/mqperf/jms/results
echo "2K" >> /home/mqperf/jms/results
export threads=1
echo "$threads thread " >> /home/mqperf/jms/results
./jmsreq.sh $threads | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=8
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=16
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=32
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=64
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=128
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=200
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
echo "" >> /home/mqperf/jms/results
echo "20K" >> /home/mqperf/jms/results
export threads=1
echo "$threads thread " >> /home/mqperf/jms/results
./jmsreq.sh $threads 20480 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=8
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 20480 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=16
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 20480 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=32
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 20480 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=64
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 20480 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=128
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 20480 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=200
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 20480 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
echo "" >> /home/mqperf/jms/results
echo "200K" >> /home/mqperf/jms/results
export threads=1
echo "$threads thread " >> /home/mqperf/jms/results
./jmsreq.sh $threads 204800 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=8
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 204800 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=16
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 204800 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=32
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 204800 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=64
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 204800 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=128
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 204800 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
export threads=200
echo "$threads threads " >> /home/mqperf/jms/results
./jmsreq.sh $threads 204800 | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }' >> /home/mqperf/jms/results
cat /home/mqperf/jms/results
echo "----------------------------------------"
echo "jms testing finished--------------------"
echo "----------------------------------------"
