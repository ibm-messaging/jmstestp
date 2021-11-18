#!/bin/bash
threads="${1:-200}"
msgsize=${2:-2048}
host="${MQ_QMGR_HOSTNAME:-localhost}"
qmname="${MQ_QMGR_NAME:-PERF0}"
port="${MQ_QMGR_PORT:-1420}"
channel="${MQ_QMGR_CHANNEL:-SYSTEM.DEF.SVRCONN}"
requestq="${MQ_QMGR_QREQUEST_PREFIX:-REQUEST}"
replyq="${MQ_QMGR_QREPLY_PREFIX:-REPLY}"
extra="${MQ_JMS_EXTRA}"
userid="${MQ_USERID}"
password="${MQ_PASSWORD}"
nonpersistent="${MQ_NON_PERSISTENT:-0}"
bindings=mqc
cipher="${MQ_JMS_CIPHER}"
keystorepassword="${MQ_JMS_KEYSTOREPASSWORD}"
runlength="${MQ_RUNLENGTH:-90}"

# Setup MQ environment
. /opt/mqm/bin/setmqenv -n Installation1

export MQ_JARS=/opt/mqm/java/lib/*
export PERFHARNESS_JAR=/home/mqperf/jms/perfharness.jar
echo "Classpath: $MQ_JARS:$PERFHARNESS_JAR"

export MQ_LIB_PATH=/opt/mqm/java/lib64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MQ_LIB_PATH
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"

export JVM_OPTS=""
#export JVM_OPTS="$JVM_OPTS -verbose:gc"
#export JVM_OPTS="$JVM_OPTS -Djavax.net.ssl.trustStore=/tmp/jks/keystore.jks -Djavax.net.ssl.keyStore=/tmp/jks/keystore.jks -Djavax.net.ssl.keyStorePassword=passw0rd -Djavax.net.ssl.trustStorePassword=passw0rd"
#export JVM_OPTS="$JVM_OPTS -Djavax.net.debug=ssl"
export JVM_OPTS="$JVM_OPTS -Dcom.ibm.mq.jmqi.defaultMaxMsgSize=8192"
#export JVM_OPTS="$JVM_OPTS -Dcom.ibm.msg.client.commonservices.trace.status=on"
#export JVM_OPTS="$JVM_OPTS -Xhealthcenter:level=headless -Dcom.ibm.java.diagnostics.healthcenter.headless.delay.start=1"

if [ -n "${MQ_JMS_CIPHER}" ]; then
  export JVM_OPTS="$JVM_OPTS -Djavax.net.ssl.trustStore=/tmp/keystore.jks -Djavax.net.ssl.keyStore=/tmp/keystore.jks -Djavax.net.ssl.keyStorePassword=$keystorepassword -Djavax.net.ssl.trustStorePassword=$keystorepassword"
fi


if [ "${nonpersistent}" -eq 1 ]; then
  persistent_flags="-tx false -pp false" 
else
  persistent_flags="-tx true -pp true" 
fi

if [ -n "${MQ_USERID}" ]; then 
  java $JVM_OPTS -cp $MQ_JARS:$PERFHARNESS_JAR -Xms1768M -Xmx1768M -Xmn1600M JMSPerfHarness -ms $msgsize -su -wt 10000 -wi 10 -nt $threads -id 1 -ss 10 -sc BasicStats -rl $runlength -sw 120 -tc jms.r11.Requestor -iq $requestq -oq $replyq -db 1 -dx 10 -to 60 -mt text -jp $port -jc $channel -jb $qmname -jt $bindings -pc WebSphereMQ -jh $host -jq SYSTEM.BROKER.DEFAULT.STREAM -ja 100 $persistent_flags -us $userid -pw $password -jm true -jl $cipher $extra
else
  java $JVM_OPTS -cp $MQ_JARS:$PERFHARNESS_JAR -Xms1768M -Xmx1768M -Xmn1600M JMSPerfHarness -ms $msgsize -su -wt 10000 -wi 10 -nt $threads -id 1 -ss 10 -sc BasicStats -rl $runlength -sw 120 -tc jms.r11.Requestor -iq $requestq -oq $replyq -db 1 -dx 10 -to 60 -mt text -jp $port -jc $channel -jb $qmname -jt $bindings -pc WebSphereMQ -jh $host -jq SYSTEM.BROKER.DEFAULT.STREAM -ja 100 $persistent_flags -jl $cipher $extra 
fi

