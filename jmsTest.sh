#!/bin/bash

#Functions
function runclients {
  threads=$1
  msgsize=$2
  echo "threads=$threads" >> /home/mqperf/jms/results
  echo "Starting test with $threads JMS requesters" >> /home/mqperf/jms/output
  rate=$(./jmsreq.sh $threads $msgsize | tee -a /home/mqperf/jms/output | grep totalRate | awk -F ',' '{ print $3 }')
  rate=$(echo $rate | awk -F '=' '{print $2}')
  echo "avgRate=$rate" >> /home/mqperf/jms/results

  cpu=$(awk '{print $12}' /tmp/mpstat | tail -6 |  awk '{total+=$1} END{printf "%0.2f\n",(NR?100-(total/NR):-1)}')
  echo "CPU=$cpu" >> /home/mqperf/jms/results

  readMB=$(awk -F ',' '{print $6}' /tmp/dstat | tail -n +8 | tail -6 |  awk '{total+=$1} END{printf "%0.2f\n",(NR?((total/NR)/(1024)):-1)}')
  echo "Read=$readMB" >> /home/mqperf/jms/results

  writeMB=$(awk -F ',' '{print $7}' /tmp/dstat | tail -n +8 | tail -6 |  awk '{total+=$1} END{printf "%0.2f\n",(NR?((total/NR)/(1024)):-1)}')
  echo "Write=$writeMB">> /home/mqperf/jms/results

  recvGbs=$(awk -F ',' '{print $8}' /tmp/dstat | tail -n +8 | tail -6 |  awk '{total+=$1} END{printf "%0.2f\n",(NR?((total/NR)/(1024*1024*1024*0.125)):-1)}')
  echo "Recv=$recvGbs" >> /home/mqperf/jms/results

  sendGbs=$(awk -F ',' '{print $9}' /tmp/dstat | tail -n +8 | tail -6 |  awk '{total+=$1} END{printf "%0.2f\n",(NR?((total/NR)/(1024*1024*1024*0.125)):-1)}')
  echo "Send=$sendGbs" >> /home/mqperf/jms/results

  qmcpu=$(tail -6 /tmp/systemerr | awk -F '=' '{print $2}' | awk '{total+=$1} END{printf "%0.2f\n",(NR?(total/NR):-1)}')
  echo "QM_CPU=$qmcpu" >> /home/mqperf/jms/results

  echo "" >> /home/mqperf/jms/results

  if [ -n "${MQ_RESULTS_CSV}" ]; then
    echo "$persistent,$msgsize,$threads,$rate,$cpu,$readMB,$writeMB,$recvGbs,$sendGbs,$qmcpu" >> /home/mqperf/jms/results.csv
  fi
}

function getConcurrentClientsArray {
if  [ -n "${MQ_FIXED_CLIENTS}" ]; then
  clientsArray=(${MQ_FIXED_CLIENTS})
else
  maximumClients=$1
  #maximumClients=`expr ${maximumClients} - 2`
  for (( i=1; i<$maximumClients; i=$i*2 ))
  do
    clientsArray+=($i)
  done
  clientsArray+=($maximumClients)
fi
}

function setupTLS {
  #JMS Clients require keystore in JKS format
  #MQ Clients including runmqsc/dmpmqcfg require keystore in KDB format
  #
  #Therefore we need to both to run automated tests as we use runmqsc for clearing the queues
  #
  #Assuming that key.kdb is already present in the default location /opt/mqm/ssl/key.kdb
  #Assuming currently that kdb is CMS - Not tested yet with PKCS12

  #We will look for JKS keystore at /tmp/keystore.jks
  #There is no password stash file, password is provided to java system property on invocation

  #Override mqclient.ini with SSL Key repository location and reuse count
  cp /opt/mqm/ssl/mqclient.ini /var/mqm/mqclient.ini

  if [[ -n "${MQ_TLS_SNI_HOSTNAME}" ]]; then
    echo "  OutboundSNI=HOSTNAME" >> /var/mqm/mqclient.ini
  fi

  #Create local CCDT; alternatives are to copy it from Server or host it at http location
  mqicipher="${MQ_TLS_CIPHER:-TLS_RSA_WITH_AES_256_GCM_SHA384}"
  echo "DEFINE CHANNEL('$channel') CHLTYPE(CLNTCONN) CONNAME('$host($port)') SSLCIPH($mqicipher) QMNAME($qmname) CERTLABL('${MQ_TLS_CERTLABEL}') REPLACE" | /opt/mqm/bin/runmqsc -n >> /home/mqperf/jms/output 2>&1
}

echo "----------------------------------------"
echo "Initialising test environment-----------"
echo "----------------------------------------"
qmname="${MQ_QMGR_NAME:-PERF0}"
host="${MQ_QMGR_HOSTNAME:-localhost}"
port="${MQ_QMGR_PORT:-1420}"
channel="${MQ_QMGR_CHANNEL:-SYSTEM.DEF.SVRCONN}"
msgsizestring="${MQ_MSGSIZE:-2048:20480:204800}"
nonpersistent="${MQ_NON_PERSISTENT:-0}"
mqicipher=""
responders="${MQ_RESPONDER_THREADS:-200}"

# Conversion of MQ_NON_PERSISTENT to persistence for logging purposes
if [ "${nonpersistent}" = "1" ]; then
  persistent=0
else
  persistent=1
fi

echo $(date) | tee /home/mqperf/jms/results

if [ "${nonpersistent}" -eq 1 ]; then
  echo "Running Non Persistent JMS Messaging Tests" | tee -a /home/mqperf/jms/results
else
  echo "Running Persistent JMS Messaging Tests" | tee -a /home/mqperf/jms/results
fi
echo "----------------------------------------"

echo "Testing QM: $qmname on host: $host using port: $port and channel: $channel" | tee -a /home/mqperf/jms/results 

echo -n "Using the following message sizes: " | tee -a /home/mqperf/jms/results
for messageSize in ${msgsizestring}; do
  echo "$messageSize" | tee -a /home/mqperf/jms/results 
done

if [ -n "${MQ_JMS_EXTRA}" ]; then
  echo "Extra JMS flags: ${MQ_JMS_EXTRA}" | tee -a /home/mqperf/jms/results
fi

if [ -n "${MQ_JMS_CIPHER}" ]; then
  echo "JMS TLS Cipher: ${MQ_JMS_CIPHER}" | tee -a /home/mqperf/jms/results
  # Need to complete TLS setup before we try to attach monitors
  setupTLS
  echo "MQI TLS Cipher: ${mqicipher}" | tee -a /home/mqperf/jms/results
fi

#Clear queues
if [ -n "${MQ_JMS_CIPHER}" ]; then
  #Dont configure MQSERVER envvar for TLS scenarios, will use CCDT
  #We do need to specify the Key repository location as runmqsc doesnt use mqclient.ini
  export MQSSLKEYR=/opt/mqm/ssl/key
else
  #Configure MQSERVER envvar
  export MQSERVER="$channel/TCP/$host($port)";
fi

if ! ( [ -n "{MQ_CLEAR_QUEUES}" ] && [ "${MQ_CLEAR_QUEUES}" = "N" ] ); then
  if [ -n "${MQ_USERID}" ]; then
    # Need to flow userid and password to runmqsc
    echo "Using userid: ${MQ_USERID}" | tee -a /home/mqperf/jms/results
    echo ${MQ_PASSWORD} > /tmp/clearq.mqsc;
    cat /home/mqperf/jms/clearq.mqsc >> /tmp/clearq.mqsc;
    cat /tmp/clearq.mqsc | /opt/mqm/bin/runmqsc -c -u ${MQ_USERID} -w 60 $qmname > /home/mqperf/jms/output 2>&1;
    rm -f /tmp/clearq.mqsc;
  else
    cat /home/mqperf/jms/clearq.mqsc | /opt/mqm/bin/runmqsc -c $qmname > /home/mqperf/jms/output 2>&1;
  fi
fi

#Launch monitoring processes
mpstat 10 > /tmp/mpstat &
pcp dstat --output /tmp/dstat 10 > /dev/null 2>&1 &
if [ -n "${MQ_USERID}" ]; then
  if [ -n "${MQ_TLS_CIPHER}" ]; then
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c CPU -t SystemSummary -u ${MQ_USERID} -v ${MQ_PASSWORD} -l ${MQ_TLS_CIPHER} -w ${MQ_TLS_CERTLABEL} >/tmp/system 2>/tmp/systemerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c DISK -t Log -u ${MQ_USERID} -v ${MQ_PASSWORD} -l ${MQ_TLS_CIPHER} -w ${MQ_TLS_CERTLABEL} >/tmp/disklog 2>/tmp/disklogerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c NHAREPLICA -t REPLICATION -o + -u ${MQ_USERID} -v ${MQ_PASSWORD} -l ${MQ_TLS_CIPHER} -w ${MQ_TLS_CERTLABEL} >/tmp/nhalog 2>/tmp/nhalogerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c NHAREPLICA -t RECOVERY -o + -u ${MQ_USERID} -v ${MQ_PASSWORD} -l ${MQ_TLS_CIPHER} -w ${MQ_TLS_CERTLABEL} >/tmp/nhacrrlog 2>/tmp/nhacrrlogerr &
  else
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c CPU -t SystemSummary -u ${MQ_USERID} -v ${MQ_PASSWORD} >/tmp/system 2>/tmp/systemerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c DISK -t Log -u ${MQ_USERID} -v ${MQ_PASSWORD} >/tmp/disklog 2>/tmp/disklogerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c NHAREPLICA -t REPLICATION -o + -u ${MQ_USERID} -v ${MQ_PASSWORD} >/tmp/nhalog 2>/tmp/nhalogerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c NHAREPLICA -t RECOVERY -o + -u ${MQ_USERID} -v ${MQ_PASSWORD} >/tmp/nhacrrlog 2>/tmp/nhacrrlogerr &
  fi
else
  if [ -n "${MQ_TLS_CIPHER}" ]; then
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c CPU -t SystemSummary -l ${MQ_TLS_CIPHER} -w ${MQ_TLS_CERTLABEL} >/tmp/system 2>/tmp/systemerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c DISK -t Log -l ${MQ_TLS_CIPHER} -w ${MQ_TLS_CERTLABEL} >/tmp/disklog 2>/tmp/disklogerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c NHAREPLICA -t REPLICATION -o + -l ${MQ_TLS_CIPHER} -w ${MQ_TLS_CERTLABEL} >/tmp/nhalog 2>/tmp/nhalogerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c NHAREPLICA -t RECOVERY -o + -l ${MQ_TLS_CIPHER} -w ${MQ_TLS_CERTLABEL} >/tmp/nhacrrlog 2>/tmp/nhacrrlogerr &
  else
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c CPU -t SystemSummary >/tmp/system 2>/tmp/systemerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c DISK -t Log >/tmp/disklog 2>/tmp/disklogerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c NHAREPLICA -t REPLICATION -o + >/tmp/nhalog 2>/tmp/nhalogerr &
    ./qmmonitor2 -m $qmname -p $port -s $channel -h $host -c NHAREPLICA -t RECOVERY -o + >/tmp/nhacrrlog 2>/tmp/nhacrrlogerr &
  fi
fi

#Write CSV header if required
if [ -n "${MQ_RESULTS_CSV}" ]; then
  echo "# CSV Results" > /home/mqperf/jms/results.csv
  echo "# TLS Cipher: ${MQ_JMS_CIPHER}" >> /home/mqperf/jms/results.csv
  printf "# " >> /home/mqperf/jms/results.csv
  echo $(date) >> /home/mqperf/jms/results.csv
  echo "# Persistence, Msg Size, Threads, Rate (RT/s), Client CPU, IO Read (MB/s), IO Write (MB/s), Net Recv (Gb/s), Net Send (Gb/s), QM CPU" >> /home/mqperf/jms/results.csv
fi

echo "----------------------------------------"
echo "Starting JMS tests----------------------"
echo "----------------------------------------"
./jmsresp.sh ${responders} >> /home/mqperf/jms/output & disown
#Wait for responders to start
sleep 60
#Determine sequence of requester clients to use from the number of responders
getConcurrentClientsArray ${responders}
echo "Using the following progression of concurrent connections: ${clientsArray[@]}" | tee -a /home/mqperf/jms/results
echo "Using ${responders} responder threads" | tee -a /home/mqperf/jms/results

IFS=:
echo "JMS Test Results" >> /home/mqperf/jms/results
for messageSize in ${msgsizestring}; do
  unset IFS
  echo $(date) >> /home/mqperf/jms/results
  IFS=:
  echo "$messageSize" >> /home/mqperf/jms/results
  for concurrentConnections in ${clientsArray[@]}
  do
    runclients $concurrentConnections $messageSize
  done
done
unset IFS

#Always dump logger stats
echo "Logger Summary" >> /home/mqperf/jms/results
slowest_write=$(cat /tmp/disklog | grep "slowest write since restart" | awk '{print $7}' | tail -1)
echo "Slowest write since QM restart: $slowest_write microsecs" >> /home/mqperf/jms/results
slowest_write_timestamp=$(cat /tmp/disklog | grep "timestamp of slowest write" | awk '{print $7}' | tail -1)
echo "Slowest write timestamp: $slowest_write_timestamp microsecs since epoch" >> /home/mqperf/jms/results
max_latency=$(cat /tmp/disklog | grep "write latency" | awk '{print $5}' | sort -n | tail -1)
echo "Highest log write latency: $max_latency microsecs" >> /home/mqperf/jms/results
max_write_size=$(cat /tmp/disklog | grep "write size" | awk '{print $5}' | sort -n | tail -1)
echo "Highest log write size: $max_write_size bytes" >> /home/mqperf/jms/results
max_phys_write=$(cat /tmp/disklog | grep "physical bytes" | awk '{print $7}' | sort -n | tail -1)
echo "Max physical write throughput (bytes): $max_phys_write" >> /home/mqperf/jms/results
min_log_free_space=$(cat /tmp/disklog | grep "Log file system - free space" | awk '{print $7}' | sort -n | head -1)
echo "Min Log free space: $min_log_free_space" >> /home/mqperf/jms/results
max_log_bytes_in_use=$(cat /tmp/disklog | grep "Log file system - bytes in use" | awk '{print $8}' | sort -n | tail -1)
echo "Max log bytes in use: $max_log_bytes_in_use" >> /home/mqperf/jms/results
echo "" >> /home/mqperf/jms/results

#Check if we have an active NHA log file
nha_log_lines=$(cat /tmp/nhalog | wc -l)
if [[ $nha_log_lines -gt 2 ]]; then
  echo "NHA Summary (across all replicas)" >> /home/mqperf/jms/results
  max_sync_log_bytes=$(cat /tmp/nhalog | grep "Synchronous log bytes sent" | awk '{print $7}' | sort -n | tail -1)
  echo "Max sync log bytes: $max_sync_log_bytes" >> /home/mqperf/jms/results
  max_sync_comp_log_bytes=$(cat /tmp/nhalog | grep "Synchronous compressed log bytes sent" | awk '{print $8}' | sort -n | tail -1)
  echo "Max sync compressed log bytes: $max_sync_comp_log_bytes" >> /home/mqperf/jms/results
  max_catch_up_log_bytes=$(cat /tmp/nhalog | grep "Catch-up log bytes sent" | awk '{print $6}' | sort -n | tail -1)
  echo "Max catch up log bytes: $max_catch_up_log_bytes" >> /home/mqperf/jms/results
  max_backlog_bytes=$(cat /tmp/nhalog | grep "Backlog bytes" | awk '{print $4}' | sort -n | tail -1)
  echo "NHA Max backlog bytes: $max_backlog_bytes" >> /home/mqperf/jms/results
  max_nha_rtt=$(cat /tmp/nhalog | grep "Average network round trip time" | awk '{print $7}' | tail -1)
  echo "Max (of average) network round trip time: $max_nha_rtt microsecs" >> /home/mqperf/jms/results
  max_log_write_ack_lat=$(cat /tmp/nhalog | grep "Log write average acknowledgement latency" | awk '{print $7}' | sort -n | tail -1)
  echo "Max (of average) log write ack latency: $max_log_write_ack_lat microsecs" >> /home/mqperf/jms/results
  min_qmgr_free_space=$(cat /tmp/nhalog | grep "Queue Manager file system - free space" | awk '{print $9}' | sort -n | head -1)
  echo "Min QMgr free space: $min_qmgr_free_space" >> /home/mqperf/jms/results
  max_qmgr_bytes_in_use=$(cat /tmp/nhalog | grep "Queue Manager file system - bytes in use" | awk '{print $10}' | sort -n | tail -1)
  echo "Max QMgr bytes in use: $max_qmgr_bytes_in_use" >> /home/mqperf/jms/results
  min_log_free_space=$(cat /tmp/nhalog | grep "Log file system - free space" | awk '{print $8}' | sort -n | head -1)
  echo "Min Log free space: $min_log_free_space" >> /home/mqperf/jms/results
  max_log_bytes_in_use=$(cat /tmp/nhalog | grep "Log file system - bytes in use" | awk '{print $9}' | sort -n | tail -1)
  echo "Max log bytes in use: $max_log_bytes_in_use" >> /home/mqperf/jms/results
  echo "" >> /home/mqperf/jms/results
fi

#Check if we have an active NHA CRR log file
nha_log_lines=$(cat /tmp/nhacrrlog | wc -l)
if [[ $nha_log_lines -gt 2 ]]; then
  echo "NHA:CRR Summary" >> /home/mqperf/jms/results
  max_async_log_bytes=$(cat /tmp/nhacrrlog | grep "Log bytes sent" | awk '{print $6}' | sort -n | tail -1)
  echo "Max async log bytes: $max_async_log_bytes" >> /home/mqperf/jms/results
  max_backlog_bytes=$(cat /tmp/nhacrrlog | grep "Backlog bytes" | awk '{print $4}' | sort -n | tail -1)
  echo "NHA:CRR Max backlog bytes: "$max_backlog_bytes >> /home/mqperf/jms/results
  max_avg_backlog_bytes=$(cat /tmp/nhacrrlog | grep "Backlog average bytes" | awk '{print $5}' | sort -n | tail -1)
  echo "NHA:CRR Max (of average) backlog bytes: "$max_avg_backlog_bytes >> /home/mqperf/jms/results
  lowest_rebase_count=$(cat /tmp/nhacrrlog | grep "Rebase count" | awk '{print $4}' | head -1)
  echo "Rebase count start: "$lowest_rebase_count >> /home/mqperf/jms/results
  rebase_count=$(cat /tmp/nhacrrlog | grep "Rebase count" | awk '{print $4}' | tail -1)
  echo "Rebase count end: "$rebase_count >> /home/mqperf/jms/results
  max_nhacrr_rtt=$(cat /tmp/nhacrrlog | grep "Average network round trip" | awk '{print $7}' | sort -n | tail -1)
  echo "Max (of average) network round trip time: $max_nhacrr_rtt microsec" >> /home/mqperf/jms/results
  echo "" >> /home/mqperf/jms/results
fi


if ! [ "${MQ_RESULTS}" = "FALSE" ]; then
  cat /home/mqperf/jms/results
fi

if [ -n "${MQ_DATA}" ] && [ ${MQ_DATA} -eq 1 ]; then
  cat /tmp/system
  cat /tmp/disklog
  cat /tmp/nhalog
  cat /tmp/nhacrrlog
  env | sort
fi

if [ -n "${MQ_ERRORS}" ]; then
  cat /home/mqperf/jms/output
  cat /var/mqm/errors/AMQERR01.LOG
  env | sort
fi

if [ -n "${MQ_RESULTS_CSV}" ]; then
  cat /home/mqperf/jms/results.csv
fi
echo "----------------------------------------"
echo "jms testing finished--------------------"
echo "----------------------------------------"
