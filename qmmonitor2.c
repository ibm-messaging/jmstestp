 /* @(#) MQMBID sn=p941-lrep-L240917 su=_WdFky3R3Ee-6Cq_6nLjb_A pn=samples/c/amqsruaa.c.pre */
 /********************************************************************/
 /*                                                                  */
 /* Program name: qmmonitor2                                         */
 /*                                                                  */
 /* Description: Sample C program that subscribes to MQ resource     */
 /*              usage topics and formats the resulting published    */
 /*              PCF data.                                           */
 /*              This sample also acts as primary documentation      */
 /*              for customers/vendors wishing to gain a greater     */
 /*              understanding of the (flexible) structure and       */
 /*              nature of MQ's resource usage publications.         */
 /*   <copyright                                                     */
 /*   notice="lm-source-program"                                     */
 /*   pids="5724-H72"                                                */
 /*   years="1994,2024"                                              */
 /*   crc="1728343970" >                                             */
 /*   Licensed Materials - Property of IBM                           */
 /*                                                                  */
 /*   5724-H72                                                       */
 /*                                                                  */
 /*   (C) Copyright IBM Corp. 1994, 2024 All Rights Reserved.        */
 /*                                                                  */
 /*   US Government Users Restricted Rights - Use, duplication or    */
 /*   disclosure restricted by GSA ADP Schedule Contract with        */
 /*   IBM Corp.                                                      */
 /*   </copyright>                                                   */
 /********************************************************************/
 /*                                                                  */
 /* Function:                                                        */
 /*                                                                  */
 /*                                                                  */
 /*   AMQSRUAA is a sample C program to subscribe and get messages   */
 /*   showing how MQ is using resources. It is a basic example of    */
 /*   how to request and consume this type of administrative data.   */
 /*                                                                  */
 /*                                                                  */
 /*   sample                                                         */
 /*      -- subscribes non-durably to the topics identified by the   */
 /*        input parameters.                                         */
 /*                                                                  */
 /*      -- calls MQGET repeatedly to get messages from the topics,  */
 /*         and writes to stdout.                                    */
 /*                                                                  */
 /*      -- writes a message for each MQI reason other than          */
 /*         MQRC_NONE; stops if there is a MQI completion code       */
 /*         of MQCC_FAILED, or when the requested number of          */
 /*         resource usage publications have been consumed.          */
 /*                                                                  */
 /*                                                                  */
 /*                                                                  */
 /*   Program logic:                                                 */
 /*      Determine name of admin topics from the input parameters.   */
 /*      MQSUB topics for INPUT                                      */
 /*      while no MQI failures,                                      */
 /*      .  MQGET next message                                       */
 /*      .  format the results                                       */
 /*      .  (no message available counts as failure, and loop ends)  */
 /*      MQCLOSE the topic                                           */
 /*                                                                  */
 /*                                                                  */
 /* The queue manager publishes resource usage data to topics to be  */
 /* consumed by subscribers to those topics. At queue manager        */
 /* start up the queue manager publishes a set of messages on        */
 /* "meta-topics" describing what resource usage topics are          */
 /* supported by this queue manager, and the content of the          */
 /* messages published to those topics.                              */
 /* Administrative tools subscribe to the meta-data to discover      */
 /* what resource usage information is available, and on what        */
 /* topics, and then subscribe to the advertised topics in order     */
 /* to consume resource usage data.                                  */
 /*                                                                  */
 /* It is worth noting that pretty much the only static information  */
 /* used by amqsruaa.c is the "META_PREFIX" definition which         */
 /* identifies the root topic for the meta-data publications and     */
 /* no other statically defined data needs to be used.               */
 /*                                                                  */
 /* Well written MQ admin programs consuming this type of data       */
 /* should try to follow this model.                                 */
 /*                                                                  */
 /* Much of the published data represents well understood MQ         */
 /* concepts, such as the rate at which messages are being           */
 /* produced and consumed. This type of data is very unlikely to     */
 /* change across MQ configurations.                                 */
 /* A subset of the published data is more reflective of MQ          */
 /* internals, for example data relating to rates of queue           */
 /* avoidance (put to waiting getter), or data relating to lock      */
 /* contention. This type of data is likely to change across         */
 /* MQ configurations. A well written program that subscribes to and */
 /* consumes this type of data should not need changing when new     */
 /* resource usage information is added, nor when the currently      */
 /* published topics and content are changed.                        */
 /*                                                                  */
 /********************************************************************/
 /*                                                                  */
 /*   AMQSRUAA has the following parameters                          */
 /*       optional:                                                  */
 /*                 (1) Queue manager name (-m)                      */
 /*                 (2) The CLASS(s) (-c)                            */
 /*                 (3) The TYPE(s) (-t)                             */
 /*                 (4) The Object name(s) (-o)                      */
 /*                 (5) The number of publications to process (-n)   */
 /*                 (6) The debug level (-d)                         */
 /*                 (7) Metadata prefix (-p)                         */
 /*                 (8) The model queue name (-s)                    */
 /*                                                                  */
 /*   Examples:                                                      */
 /*     amqsrua -m QM1 -c CPU -t QMgrSummary -n 5                    */
 /*     amqsrua -m QM1 -c STATMQI -t PUT -t GET                      */
 /*     amqsrua -m QM1 -c STATQ  -o QUEUE1  -t PUT -t GET -n 10      */
 /*     amqsrua -m QM1 -c DISK -t Log -n 10                          */
 /*     amqsrua -m QM1 -s MODELQUEUE1                                */
 /*                                                                  */
 /********************************************************************/
 /* Notes:                                                           */
 /*                                                                  */
 /* 1. The primary purpose of amqsruaa is to demonstrate how to      */
 /*    interpret the meta-data to make appropriate subscriptions     */
 /*    and how to process the ensuing publications. In order to keep */
 /*    this focus, amqsruaa.c is intentionally very simplistic in    */
 /*    its error handling, and is NOT intended to be a good example  */
 /*    of comprehensive MQ error handling.                           */
 /*    Similarly the usability of amqsruaa as a sample executable    */
 /*    is somewhat constrained by the desire to keep the source      */
 /*    code focused on its primary purpose.                          */
 /*                                                                  */
 /*                                                                  */
 /********************************************************************/
 /*
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/CLASSES/en_GB
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/CLASSES
Register Class: 0000        CPU Platform central processing units
Register Class: 0001       DISK Platform persistent data stores
Register Class: 0002    STATMQI API usage statistics
Register Class: 0003      STATQ API per-queue usage statistics
Register Class: 0004    STATAPP Per-application usage statistics
Register Class: 0005 NHAREPLICA Native HA replica statistics
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/CPU/TYPES
Register Type : 0000 0000 SystemSummary CPU performance - platform wide
TopicLen: 58
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/CPU/SystemSummary
Register Type : 0000 0001 QMgrSummary CPU performance - running queue manager
TopicLen: 56
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/CPU/QMgrSummary
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/DISK/TYPES
Register Type : 0001 0000 SystemSummary Disk usage - platform wide
TopicLen: 59
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/DISK/SystemSummarya
Register Type : 0001 0001 QMgrSummary Disk usage - running queue managers
TopicLen: 57
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/DISK/QMgrSummarySum
Register Type : 0001 0002        Log Disk usage - queue manager recovery log
TopicLen: 49
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/DISK/Log
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/TYPES
Register Type : 0002 0000   CONNDISC MQCONN and MQDISC
TopicLen: 57
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/CONNDISCDIS
Register Type : 0002 0001  OPENCLOSE MQOPEN and MQCLOSE
TopicLen: 58
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/OPENCLOSEni
Register Type : 0002 0002     INQSET MQINQ and MQSET
TopicLen: 55
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/INQSETS
Register Type : 0002 0003        PUT MQPUT
TopicLen: 52
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/PUT
Register Type : 0002 0004        GET MQGET
TopicLen: 52
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/GET
Register Type : 0002 0005  SYNCPOINT Commit and rollback
TopicLen: 58
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/SYNCPOINTAR
Register Type : 0002 0006  SUBSCRIBE Subscribe
TopicLen: 58
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/SUBSCRIBE
Register Type : 0002 0007    PUBLISH Publish
TopicLen: 56
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/PUBLISH
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATQ/TYPES
Register Type : 0003 0000  OPENCLOSE MQOPEN and MQCLOSE
TopicLen: 56
Register Type : 0003 0001     INQSET MQINQ and MQSET
TopicLen: 53
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATQ/INQSETMQI
Register Type : 0003 0002        PUT MQPUT and MQPUT1
TopicLen: 50
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATQ/PUTTA
Register Type : 0003 0003        GET MQGET
TopicLen: 50
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATQ/GETr/
Register Type : 0003 0004    GENERAL General queue metrics
TopicLen: 54
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATQ/GENERAL
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATAPP/TYPES
Register Type : 0004 0000   INSTANCE Application instance statistics
TopicLen: 57
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATAPP/INSTANCE
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/NHAREPLICA/TYPES
Register Type : 0005 0000 REPLICATION Native HA Replication statistics
TopicLen: 63
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/NHAREPLICA/REPLICATION
Register Type : 0005 0001   RECOVERY Native HA recovery group statistics
TopicLen: 60
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/NHAREPLICA/RECOVERY
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/CPU/SystemSummary
Register Elem :0000 0000 0000 10000 User CPU time percentage
Register Elem :0000 0000 0001 10000 System CPU time percentage
Register Elem :0000 0000 0002 0100 CPU load - one minute average
Register Elem :0000 0000 0003 0100 CPU load - five minute average
Register Elem :0000 0000 0004 0100 CPU load - fifteen minute average
Register Elem :0000 0000 0005 10000 RAM free percentage
Register Elem :0000 0000 0006 1048576 RAM total bytes
Update Type   :0000 0000 $SYS/MQ/INFO/QMGR/PERF0/Monitor/CPU/SystemSummary
TopicLen: 49
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/CPU/SystemSummary
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/CPU/QMgrSummary
Register Elem :0000 0001 0000 10000 User CPU time - percentage estimate for queue manager
Register Elem :0000 0001 0001 10000 System CPU time - percentage estimate for queue manager
Register Elem :0000 0001 0002 1048576 RAM total bytes - estimate for queue manager
Update Type   :0000 0001 $SYS/MQ/INFO/QMGR/PERF0/Monitor/CPU/QMgrSummary
TopicLen: 47
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/CPU/QMgrSummary
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/DISK/SystemSummary
Register Elem :0001 0000 0004 1048576 MQ trace file system - bytes in use
Register Elem :0001 0000 0005 10000 MQ trace file system - free space
Register Elem :0001 0000 0006 1048576 MQ errors file system - bytes in use
Register Elem :0001 0000 0007 10000 MQ errors file system - free space
Register Elem :0001 0000 0008 0001 MQ FDC file count
Update Type   :0001 0000 $SYS/MQ/INFO/QMGR/PERF0/Monitor/DISK/SystemSummary
TopicLen: 50
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/DISK/SystemSummaryntageE
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/DISK/QMgrSummary
Register Elem :0001 0001 0000 1048576 Queue Manager file system - bytes in use
Register Elem :0001 0001 0001 10000 Queue Manager file system - free space
Update Type   :0001 0001 $SYS/MQ/INFO/QMGR/PERF0/Monitor/DISK/QMgrSummary
TopicLen: 48
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/DISK/QMgrSummarys file system - bytes in use
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/DISK/Log
0001 0002 0000 0001 Log - bytes in use
0001 0002 0001 0001 Log - bytes max
0001 0002 0002 0001 Log file system - bytes in use
0001 0002 0003 0001 Log file system - bytes max
0001 0002 0004 0002 Log - physical bytes written
0001 0002 0005 0002 Log - logical bytes written
0001 0002 0006 1000000 Log - write latency
0001 0002 0007 10000 Log - current primary space in use
0001 0002 0008 10000 Log - workload primary space utilization
0001 0002 0009 1048576 Log - bytes required for media recovery
0001 0002 0010 1048576 Log - bytes occupied by reusable extents
0001 0002 0011 1048576 Log - bytes occupied by extents waiting to be archived
0001 0002 0012 0001 Log - write size
0001 0002 0013 10000 Log file system - free space
0001 0002 0014 0003 Log - disk written log sequence number
0001 0002 0015 0003 Log - quorum log sequence number
0001 0002 0016 1000000 Log - Slowest write since restart
0001 0002 0017 0001 Log - timestamp of slowest write
Update Type   :0001 0002 $SYS/MQ/INFO/QMGR/PERF0/Monitor/DISK/Log
TopicLen: 40
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/DISK/Log

Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/CONNDISC
Register Elem :0002 0000 0000 0002 MQCONN/MQCONNX count
Register Elem :0002 0000 0001 0002 Failed MQCONN/MQCONNX count
Register Elem :0002 0000 0002 0001 Concurrent connections - high water mark
Register Elem :0002 0000 0003 0002 MQDISC count
Update Type   :0002 0000 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/CONNDISC
TopicLen: 48
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/CONNDISC- physical bytes written
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/OPENCLOSE
Register Elem :0002 0001 0000 0002 MQOPEN count
Register Elem :0002 0001 0001 0002 Failed MQOPEN count
Register Elem :0002 0001 0002 0002 MQCLOSE count
Register Elem :0002 0001 0003 0002 Failed MQCLOSE count
Update Type   :0002 0001 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/OPENCLOSE
TopicLen: 49
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/OPENCLOSE/PERF0/Monitor/STATMQI/CONNDISC- physical bytes written
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/INQSET
Register Elem :0002 0002 0000 0002 MQINQ count
Register Elem :0002 0002 0001 0002 Failed MQINQ count
Register Elem :0002 0002 0002 0002 MQSET count
Register Elem :0002 0002 0003 0002 Failed MQSET count
Update Type   :0002 0002 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/INQSET
TopicLen: 46
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/INQSETENCLOSE/PERF0/Monitor/STATMQI/CONNDISC- physical bytes written
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/PUT
Register Elem :0002 0003 0000 0002 Interval total MQPUT/MQPUT1 count
Register Elem :0002 0003 0001 0002 Interval total MQPUT/MQPUT1 byte count
Register Elem :0002 0003 0002 0002 Non-persistent message MQPUT count
Register Elem :0002 0003 0003 0002 Persistent message MQPUT count
Register Elem :0002 0003 0004 0002 Failed MQPUT count
Register Elem :0002 0003 0005 0002 Non-persistent message MQPUT1 count
Register Elem :0002 0003 0006 0002 Persistent message MQPUT1 count
Register Elem :0002 0003 0007 0002 Failed MQPUT1 count
Register Elem :0002 0003 0008 0002 Put non-persistent messages - byte count
Register Elem :0002 0003 0009 0002 Put persistent messages - byte count
Register Elem :0002 0003 0010 0002 MQSTAT count
Update Type   :0002 0003 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/PUT
TopicLen: 43
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/PUTytes occupied by extents waiting to be archived
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/GET
Register Elem :0002 0004 0000 0002 Interval total destructive get- count
Register Elem :0002 0004 0001 0002 Interval total destructive get - byte count
Register Elem :0002 0004 0002 0002 Non-persistent message destructive get - count
Register Elem :0002 0004 0003 0002 Persistent message destructive get - count
Register Elem :0002 0004 0004 0002 Failed MQGET - count
Register Elem :0002 0004 0005 0002 Got non-persistent messages - byte count
Register Elem :0002 0004 0006 0002 Got persistent messages - byte count
Register Elem :0002 0004 0007 0002 Non-persistent message browse - count
Register Elem :0002 0004 0008 0002 Persistent message browse - count
Register Elem :0002 0004 0009 0002 Failed browse count
Register Elem :0002 0004 0010 0002 Non-persistent message browse - byte count
Register Elem :0002 0004 0011 0002 Persistent message browse - byte count
Register Elem :0002 0004 0012 0002 Expired message count
Register Elem :0002 0004 0013 0002 Purged queue count
Register Elem :0002 0004 0014 0002 MQCB count
Register Elem :0002 0004 0015 0002 Failed MQCB count
Register Elem :0002 0004 0016 0002 MQCTL count
Update Type   :0002 0004 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/GET
TopicLen: 43
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/GET
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/SYNCPOINT
Register Elem :0002 0005 0000 0002 Commit count
Register Elem :0002 0005 0002 0002 Rollback count
Update Type   :0002 0005 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/SYNCPOINT
TopicLen: 49
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/SYNCPOINT
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/SUBSCRIBE
Register Elem :0002 0006 0000 0002 Create durable subscription count
Register Elem :0002 0006 0001 0002 Alter durable subscription count
Register Elem :0002 0006 0002 0002 Resume durable subscription count
Register Elem :0002 0006 0003 0002 Create non-durable subscription count
Register Elem :0002 0006 0006 0002 Failed create/alter/resume subscription count
Register Elem :0002 0006 0007 0002 Delete durable subscription count
Register Elem :0002 0006 0008 0002 Delete non-durable subscription count
Register Elem :0002 0006 0009 0002 Subscription delete failure count
Register Elem :0002 0006 0010 0002 MQSUBRQ count
Register Elem :0002 0006 0011 0002 Failed MQSUBRQ count
Register Elem :0002 0006 0012 0001 Durable subscriber - high water mark
Register Elem :0002 0006 0013 0001 Durable subscriber - low water mark
Register Elem :0002 0006 0014 0001 Non-durable subscriber - high water mark
Register Elem :0002 0006 0015 0001 Non-durable subscriber - low water mark
Update Type   :0002 0006 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/SUBSCRIBE
TopicLen: 49
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/SUBSCRIBE
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATMQI/PUBLISH
Register Elem :0002 0007 0000 0002 Topic MQPUT/MQPUT1 interval total
Register Elem :0002 0007 0001 0002 Interval total topic bytes put
Register Elem :0002 0007 0002 0002 Published to subscribers - message count
Register Elem :0002 0007 0003 0002 Published to subscribers - byte count
Register Elem :0002 0007 0004 0002 Non-persistent - topic MQPUT/MQPUT1 count
Register Elem :0002 0007 0005 0002 Persistent - topic MQPUT/MQPUT1 count
Register Elem :0002 0007 0006 0002 Failed topic MQPUT/MQPUT1 count
Update Type   :0002 0007 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/PUBLISH
TopicLen: 47
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATMQI/PUBLISH
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATQ/OPENCLOSE
Register Elem :0003 0000 0000 0002 MQOPEN count
Register Elem :0003 0000 0001 0002 MQCLOSE count
Update Type   :0003 0000 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATQ/%s/OPENCLOSE
TopicLen: 50
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATQ/%s/OPENCLOSE
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATQ/INQSET
Register Elem :0003 0001 0000 0002 MQINQ count
Register Elem :0003 0001 0001 0002 MQSET count
Update Type   :0003 0001 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATQ/%s/INQSET
TopicLen: 47
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATQ/%s/INQSETENCLOSE
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATQ/PUT
Register Elem :0003 0002 0000 0002 MQPUT/MQPUT1 count
Register Elem :0003 0002 0001 0002 MQPUT byte count
Register Elem :0003 0002 0002 0002 MQPUT non-persistent message count
Register Elem :0003 0002 0003 0002 MQPUT persistent message count
Register Elem :0003 0002 0004 0002 MQPUT1 non-persistent message count
Register Elem :0003 0002 0005 0002 MQPUT1 persistent message count
Register Elem :0003 0002 0006 0002 non-persistent byte count
Register Elem :0003 0002 0007 0002 persistent byte count
Register Elem :0003 0002 0008 10000 queue avoided puts
Register Elem :0003 0002 0009 10000 queue avoided bytes
Register Elem :0003 0002 0010 10000 lock contention
Register Elem :0003 0002 0011 0002 rolled back MQPUT count
Update Type   :0003 0002 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATQ/%s/PUT
TopicLen: 44
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATQ/%s/PUT
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATQ/GET
Register Elem :0003 0003 0000 0002 MQGET count
Register Elem :0003 0003 0001 0002 MQGET byte count
Register Elem :0003 0003 0002 0002 destructive MQGET non-persistent message count
Register Elem :0003 0003 0003 0002 destructive MQGET persistent message count
Register Elem :0003 0003 0004 0002 destructive MQGET non-persistent byte count
Register Elem :0003 0003 0005 0002 destructive MQGET persistent byte count
Register Elem :0003 0003 0006 0002 MQGET browse non-persistent message count
Register Elem :0003 0003 0007 0002 MQGET browse persistent message count
Register Elem :0003 0003 0008 0002 MQGET browse non-persistent byte count
Register Elem :0003 0003 0009 0002 MQGET browse persistent byte count
Register Elem :0003 0003 0010 0002 destructive MQGET fails
Register Elem :0003 0003 0011 0002 destructive MQGET fails with MQRC_NO_MSG_AVAILABLE
Register Elem :0003 0003 0012 0002 destructive MQGET fails with MQRC_TRUNCATED_MSG_FAILED
Register Elem :0003 0003 0013 0002 MQGET browse fails
Register Elem :0003 0003 0014 0002 MQGET browse fails with MQRC_NO_MSG_AVAILABLE
Register Elem :0003 0003 0015 0002 MQGET browse fails with MQRC_TRUNCATED_MSG_FAILED
Register Elem :0003 0003 0016 0002 rolled back MQGET count
Update Type   :0003 0003 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATQ/%s/GET
TopicLen: 44
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATQ/%s/GET
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATQ/GENERAL
Register Elem :0003 0004 0000 0002 messages expired
Register Elem :0003 0004 0001 0002 queue purged count
Register Elem :0003 0004 0002 1000000 average queue time
Register Elem :0003 0004 0003 0001 Queue depth
Update Type   :0003 0004 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATQ/%s/GENERAL
TopicLen: 48
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATQ/%s/GENERAL
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/STATAPP/INSTANCE
Register Elem :0004 0000 0000 0001 Instance count
Register Elem :0004 0000 0001 0001 Movable instance count
Register Elem :0004 0000 0002 0001 Instance shortfall count
Register Elem :0004 0000 0003 0002 Instances started
Register Elem :0004 0000 0004 0002 Initiated outbound instance moves
Register Elem :0004 0000 0005 0002 Completed outbound instance moves
Register Elem :0004 0000 0006 0002 Instances ended during reconnect
Register Elem :0004 0000 0007 0002 Instances ended
Update Type   :0004 0000 $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATAPP/%s/INSTANCE
TopicLen: 51
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/STATAPP/%s/INSTANCEsage count
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/NHAREPLICA/REPLICATION
Register Elem :0005 0000 0000 0002 Synchronous log bytes sent
Register Elem :0005 0000 0001 0002 Catch-up log bytes sent
Register Elem :0005 0000 0002 1000000 Log write average acknowledgement latency
Register Elem :0005 0000 0003 0001 Log write average acknowledgement size
Register Elem :0005 0000 0004 0001 Backlog bytes
Register Elem :0005 0000 0005 0001 Backlog average bytes
Register Elem :0005 0000 0006 0002 Synchronous compressed log bytes sent
Register Elem :0005 0000 0007 0002 Catch-up compressed log bytes sent
Register Elem :0005 0000 0008 0002 Synchronous uncompressed log bytes sent
Register Elem :0005 0000 0009 0002 Catch-up uncompressed log bytes sent
Register Elem :0005 0000 0010 0003 Acknowledged log sequence number
Register Elem :0005 0000 0011 0001 Log file system - bytes in use
Register Elem :0005 0000 0012 10000 Log file system - free space
Register Elem :0005 0000 0013 1048576 Queue Manager file system - bytes in use
Register Elem :0005 0000 0014 10000 Queue Manager file system - free space
Register Elem :0005 0000 0015 0001 MQ FDC file count
Register Elem :0005 0000 0016 1000000 Average network round trip time
Register Elem :0005 0000 0017 1000000 Synchronous log data average compression time
Register Elem :0005 0000 0018 1000000 Catch-up log data average compression time
Register Elem :0005 0000 0019 0001 Synchronous log bytes decompressed
Register Elem :0005 0000 0020 1000000 Synchronous log data average decompression time
Register Elem :0005 0000 0021 0001 Catch-up log bytes decompressed
Register Elem :0005 0000 0022 1000000 Catch-up log data average decompression time
Update Type   :0005 0000 $SYS/MQ/INFO/QMGR/PERF0/Monitor/NHAREPLICA/%s/REPLICATION
TopicLen: 57
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/NHAREPLICA/%s/REPLICATION
Subscribing to topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/METADATA/NHAREPLICA/RECOVERY
Register Elem :0005 0001 0000 0002 Log bytes sent
Register Elem :0005 0001 0001 0001 Backlog bytes
Register Elem :0005 0001 0002 0001 Backlog average bytes
Register Elem :0005 0001 0003 0001 Rebase count
Register Elem :0005 0001 0004 0003 Recovery log sequence number
Register Elem :0005 0001 0005 1000000 Average network round trip time
Register Elem :0005 0001 0006 0001 Compressed log bytes sent
Register Elem :0005 0001 0007 1000000 Log data average compression time
Register Elem :0005 0001 0008 0001 Log bytes decompressed
Register Elem :0005 0001 0009 1000000 Log data average decompression time
Update Type   :0005 0001 $SYS/MQ/INFO/QMGR/PERF0/Monitor/NHAREPLICA/%s/RECOVERY
TopicLen: 54
Topic: $SYS/MQ/INFO/QMGR/PERF0/Monitor/NHAREPLICA/%s/RECOVERYnt*/
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>

 /* includes for MQI */
 #include <cmqc.h>
#include <cmqxc.h>
 #include <cmqcfc.h>
 /* The maximum number of input classes supported by amqsruaa.c    */
 #define MAX_CLASS_COUNT 10
 /* The maximum size of the string value of a class supported here */
 #define MAX_CLASS_SIZE 32
 /* The maximum number of input types supported by amqsruaa.c      */
 #define MAX_TYPE_COUNT 100
 /* The maximum size of the string value of a type supported here  */
 #define MAX_TYPE_SIZE 32

 /* The root of the topic tree where MQ publishes the meta-data    */
 /* describing what resource usage publications will be published  */
 /* by this queue manager.                                         */
 #define META_PREFIX "$SYS/MQ"

#if MQAT_DEFAULT == MQAT_WINDOWS_NT
  #define Int64 "I64"
#elif defined(MQ_64_BIT)
  #define Int64 "l"
#else
  #define Int64 "ll"
#endif

/* As explained above, amqsruaa error handling is intentionally very */
/* simplistic. The program will return either 0 (expected number of  */
/* pulications received, or 20 (some error occurred).                */
#define ERROR_RETURN 20

#define sruUsageString "%s [-m QMGR-NAME] [-c CLASS-NAME]\n" \
                       "\t [-o object name] [-t TYPE-NAME]\n" \
                       "\t [-p metadata prefix] [-d debug level] \n"\
                       "\t [-n publication count]\n" \
                       "\t [-s model queue name]\n"

/*********************************************************************/
/*                                                                   */
/* amqsruaa reads the meta data to understand what resource usage    */
/* information is supported by the current queue manager             */
/* configuration.                                                    */
/* This information is built into a tree to be used in selecting     */
/* (if not explicitly specified) the data to be consumed and in      */
/* formatting the produced data.                                     */
/* The following typedef's are private to amqsruaa.c and represent   */
/* this programs view of the meta-data.                              */
/*********************************************************************/

/*********************************************************************/
/* Resource usage publications are PCF messages consisting of a      */
/* sequence of PCF elements. The PCF elements published for each     */
/* class/type pair are advertised in the meta-data. We store a leaf  */
/* in the tree describing each element, thus allowing the elements   */
/* to be processed accordingly.                                      */
/*                                                                   */
/*********************************************************************/
typedef struct _sruELEM
{
  struct _sruELEM * next;
  MQLONG            id;
  MQLONG            type;
  MQLONG            descLen;
  char              Buffer[1];
} sruELEM;

/*********************************************************************/
/* MQ resource usage publications are associated with a type         */
/* within a class. Each publication includes the class and type      */
/* which allows the class/type/element definitions to be found       */
/* and the resulting publications to be handled appropriately.       */
/* The class/type/element descriptions are published as meta-data    */
/* at queue manager startup.                                         */
/*********************************************************************/
typedef struct _sruTYPE
{
  struct _sruTYPE * next;
  MQLONG            id;
  MQLONG            typeLen;
  MQLONG            descLen;
  MQLONG            topicLen;
  MQLONG            topicCCSID;
  sruELEM         * elem;
  MQLONG            flags;
  char              Buffer[1];
} sruTYPE;

/*********************************************************************/
/* MQ resource usage publications are associated with a class.       */
/* The classes represent the top level in the tree of meta-data      */
/* decribing the available resource usage information.               */
/*********************************************************************/
typedef struct _sruCLASS
{
  struct _sruCLASS * next;
  MQLONG             id;
  MQLONG             classLen;
  MQLONG             descLen;
  MQLONG             topicLen;
  MQLONG             topicCCSID;
  sruTYPE          * type;
  MQLONG             flags;
  char               Buffer[1];
} sruCLASS;

/*********************************************************************/
/* MQ log sequence numbers are provided as 64-bit integers. The      */
/* sruINT64toLSN macro converts one into a set of four words so it   */
/* can be printed in the standard <nnnn:nnnn:nnnn:nnnn> format.      */
/*********************************************************************/
typedef struct _sruLSN
{
  unsigned short word[4];
} sruLSN;


#if (MQENC_NATIVE & MQENC_INTEGER_REVERSED)
#define sruWORD(w) ((unsigned short)(((w) << 8) | ((w) >> 8)))
#define sruINT64toLSN(_i64,_plsn)            \
{                                            \
  union                                      \
  {                                          \
    MQINT64 i64;                             \
    sruLSN  lsn;                             \
  } u;                                       \
  u.i64 = (_i64);                            \
  (_plsn)->word[3] = sruWORD(u.lsn.word[0]); \
  (_plsn)->word[2] = sruWORD(u.lsn.word[1]); \
  (_plsn)->word[1] = sruWORD(u.lsn.word[2]); \
  (_plsn)->word[0] = sruWORD(u.lsn.word[3]); \
}
#else
#define sruWORD(w) (w)
#define sruINT64toLSN(_i64,_plsn)            \
{                                            \
  union                                      \
  {                                          \
    MQINT64 i64;                             \
    sruLSN  lsn;                             \
  } u;                                       \
  u.i64 = (_i64);                            \
  (_plsn)->word[0] = u.lsn.word[0];          \
  (_plsn)->word[1] = u.lsn.word[1];          \
  (_plsn)->word[2] = u.lsn.word[2];          \
  (_plsn)->word[3] = u.lsn.word[3];          \
}
#endif

/*********************************************************************/
/*                                                                   */
/* Data globally addressable within amqsruaa.c                       */
/*                                                                   */
/*********************************************************************/
 static char       QMName[50];             /* queue manager name     */
 static char       ModelQName[50] = {0};   /* model queue name       */
 static char       Class[MAX_CLASS_COUNT][MAX_CLASS_SIZE] = {0};
 static char     * ClassObjName[MAX_TYPE_COUNT] = {0};
 static char       Type[MAX_TYPE_COUNT][MAX_TYPE_SIZE] = {0};
 static char     * TypeObjName[MAX_TYPE_COUNT] = {0};
 static int        ClassTypeCount[MAX_CLASS_COUNT] = {0};
 static int        ClassCount=0;
 static int        TypeCount=0;
 static MQHCONN    Hcon = MQHC_UNUSABLE_HCONN; /* connection handle  */
 static MQHOBJ     Hobj = MQHO_NONE;         /* MQGET object handle  */
 static sruCLASS * ClassChain = NULL;        /* root of tree         */
 static int        sruDebug = 0;
 static char     * meta_prefix = META_PREFIX;
static char runmqsc[] = "runmqsc";
static char mqserver[] = "MQSERVER";
static char collectQueueDepth[] = "display queue(*) where (curdepth gt 0)";

/*********************************************************************/
/*                                                                   */
/* Prototypes for functions local to amqsruaa.c                      */
/*                                                                   */
/*********************************************************************/

 int sruPromptForClass(void);
 int sruPromptForType(void);
 int sruSubscribe(void);
 int sruDiscover(void);
 int sruQueryQMName(void);

 int sruFormatMessage(PMQCFH buffer, MQMD * pMd);

 void sruFormatElem( PMQCFIN   cfin
                   , sruELEM * pElem
                   , MQINT64   interval
                   , char    * pObjName
                   , MQLONG    ObjNameLen);

 int sruRegisterClass( MQLONG   id
                     , char   * class
                     , MQLONG   class_len
                     , char   * desc
                     , MQLONG   desc_len
                     , char   * topic
                     , MQLONG   topic_len
                     , MQLONG   topic_ccsid
                     , MQLONG   flags );

 int sruRegisterType( MQLONG   class_id
                    , MQLONG   type_id
                    , char   * type
                    , MQLONG   type_len
                    , char   * desc
                    , MQLONG   desc_len
                    , char   * topic
                    , MQLONG   topic_len
                    , MQLONG   topic_ccsid
                    , MQLONG   flags);

  int sruUpdateType( MQLONG   class_id
                   , MQLONG   type_id
                   , char   * topic
                   , MQLONG   topic_len
                   , MQLONG   topic_ccsid );

  int sruRegisterElement( MQLONG   class_id
                        , MQLONG   type_id
                        , MQLONG   elem_id
                        , MQLONG   elemType
                        , char   * desc
                        , MQLONG   desc_len );

 sruCLASS * sruIdToClass(MQLONG class);
 sruTYPE * sruIdToType(sruCLASS * pClass, MQLONG type);
 sruELEM * sruIdToElem(sruTYPE * pType, MQLONG elem);

/*********************************************************************/
/*                                                                   */
/* Main:                                                             */
/*                                                                   */
/*********************************************************************/
int main(int argc, char **argv)
{
  /*   Declare MQI structures needed                                */
  MQOD     od = {MQOD_DEFAULT};    /* Object Descriptor             */
  MQMD     md = {MQMD_DEFAULT};    /* Message Descriptor            */
  MQGMO   gmo = {MQGMO_DEFAULT};   /* get message options           */
  /** note, sample uses defaults where it can **/
  MQLONG   CompCode;               /* completion code               */
  MQLONG   Reason;                 /* reason code                   */
  MQBYTE   buffer[4096];           /* message buffer                */
  MQLONG   buflen;                 /* buffer length                 */
  MQLONG   messlen;                /* message length received       */
  MQLONG   MaxGets = -1;
  MQLONG   GetCount = 0;
  int      count;
  int      rc = 0;
  char *   parg;

	MQCNO cno = {MQCNO_DEFAULT};           /* Connection options  */
	MQCD cd = {MQCD_CLIENT_CONN_DEFAULT};  /* Client descriptor   */
	MQCSP csp = {MQCSP_DEFAULT};
	char channelName[MQ_CHANNEL_NAME_LENGTH];  // SVRCONN channel to connect via
	char hostName[MQ_CONN_NAME_LENGTH];        // Name of machine hosting queue manager
	int portNumber = 1420;                     // Port to connect to machine on
	int clientMode = 0;                        // Determine if to connect in client mode
	int autoMode = 0;
	char userid[MQ_USER_ID_LENGTH];
	char password[MQ_CSP_PASSWORD_LENGTH];
    char cipherSpec[MQ_SSL_CIPHER_SPEC_LENGTH];
    MQCHAR certLabel[MQ_CERT_LABEL_LENGTH + 1]; //Name of cert label

    //SSL requires at least Version 4 of MQCNO (Connection Options),
    //            Version 7 of MQCD(Channel Definition),
    //            Version 1 of MQCSP(Security Parameters) and
    //            Version 4 of MQSCO (SSL Configuration Options) - Not currently used, set in mqcd
	cno.Version = MQCNO_VERSION_5;
	
	//Certlabel requires version 11
	cd.Version = MQCD_VERSION_11;
	csp.Version = MQCSP_CURRENT_VERSION;

	strcpy(channelName, "SYSTEM.DEF.SVRCONN"); //Setup default channel name
	setbuf(stdout, NULL);                      //Dont buffer output to stdout
  if (sruDebug > 0)
    printf("qmmonitor2 start\n");

  /******************************************************************/
  /*   Parse arguments                                              */
  /******************************************************************/
  for (count = 1; count < argc; count++)
  {
    if (strncmp("-m", argv[count], 2) == 0)
    {
      if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc))
      {
        parg=argv[++count];
      }
      strncpy(QMName, parg, MQ_Q_MGR_NAME_LENGTH);
    }
    /* Commenting this block out as we use -s to specify channel
    else if (strncmp("-s", argv[count], 2) == 0)
    {
      if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc))
      {
        parg=argv[++count];
      }
      strncpy(ModelQName, parg, MQ_Q_NAME_LENGTH);
    }
    */
    else if (strncmp("-s", argv[count], 2) == 0) {
			if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc)) {
				parg = argv[++count];
			}
			strncpy(channelName, parg, MQ_CHANNEL_NAME_LENGTH);
    }
    else if (strncmp("-c", argv[count], 2) == 0)
    {
      if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc))
      {
        parg=argv[++count];
      }
      if (MAX_CLASS_COUNT > ClassCount)
        strncpy(Class[ClassCount++], parg, MAX_CLASS_SIZE);
    }
    else if (strncmp("-t", argv[count], 2) == 0)
    {
      if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc))
      {
        parg = argv[++count];
      }
      if (MAX_TYPE_COUNT > TypeCount)
        strncpy(Type[TypeCount++], parg, MAX_TYPE_SIZE);
      if (ClassCount)
        ClassTypeCount[ClassCount - 1]++;
    }
    else if (strncmp("-o", argv[count], 2) == 0)
    {
      int i = 0;
      if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc))
      {
        parg=argv[++count];
      }

      for (; parg[i] != '\0'; i++)
      {
        if (parg[i] == '/') parg[i] = '&';
      }

      if (TypeCount)
        TypeObjName[TypeCount - 1] = parg;
      if ( ( ClassCount )
        && ( NULL == ClassObjName[ClassCount - 1] ) )
        ClassObjName[ClassCount - 1] = parg;
    }
    else if (strncmp("-d", argv[count], 2) == 0)
    {
      if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc))
      {
        parg = argv[++count];
      }
      sruDebug = atoi(parg);
    }
    else if (strncmp("-n", argv[count], 2) == 0)
    {
      if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc))
      {
        parg = argv[++count];
      }
      MaxGets = atoi( parg );
    }	
    else if (strncmp("-h", argv[count], 2) == 0) 
	{
		if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc)) 
		{
			parg = argv[++count];
		}
		strncpy(hostName, parg, MQ_CONN_NAME_LENGTH);
		clientMode = 1;
	}	
    else if (strncmp("-p", argv[count], 2) == 0)
    {
      if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc))
      {
        parg = argv[++count];
      }
	  portNumber = atoi(parg);
	  /*Commenting this out as we use the -p option to specify a port*/
      /*meta_prefix = parg;*/ /* must not end with '/' */
      /*if (meta_prefix[strlen(meta_prefix) - 1] == '/')
      {
        printf(sruUsageString, argv[0]);
        rc = ERROR_RETURN;
        goto mod_exit;
      }*/
    }
	else if (strncmp("-s", argv[count], 2) == 0) 
	{
			if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc)) {
				parg = argv[++count];
			}
			strncpy(channelName, parg, MQ_CHANNEL_NAME_LENGTH);
		} else if (strncmp("-?", argv[count], 2) == 0) {
			if ((*(parg = &(argv[count][2])) == '\0')) {
                printf("Usage:   qmmonitor2 [-m QMName] | [-p port] | [-s channelName] | [-h hostname] | [-n NumberStatsMsgsToRecv] | [-u Userid] | [-v Password] | [-l CipherSpec] | [-w certlabel] |\n [-c ClassSelection] | [-t TypeSelection] | [-o ObjectName] | [-d debugLevel]\n");
				printf("Example: qmmonitor2 -m PERF0 -p 1420 -s SYSTEM.DEF.SVRCONN -h remoteHost -n 10 -c CPU -t SystemSummary\n");
				printf("Example: qmmonitor2 -m PERF0 -p 1420 -s SYSTEM.DEF.SVRCONN -h remoteHost -n 10 -c STATQ -t PUT -o REQUEST1\n");
				goto mod_exit;
			}
			strncpy(channelName, parg, MQ_CHANNEL_NAME_LENGTH);
		} else if (strncmp("-a", argv[count], 2) == 0) {
			if ((*(parg = &(argv[count][2])) == '\0')) {
				autoMode = 1;
			}
		} else if (strncmp("-u", argv[count], 2) == 0) {
			if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc)) {
				parg = argv[++count];
			}
			strncpy(userid, parg, MQ_USER_ID_LENGTH);
		} else if (strncmp("-v", argv[count], 2) == 0) {
			if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc)) {
				parg = argv[++count];
			}
			strncpy(password, parg, MQ_CSP_PASSWORD_LENGTH);
		} else if (strncmp("-l", argv[count], 2) == 0) {
			// If you are using TLS, then you will need to create a local CCDT using runmqsc -n with clientconn chan details
			if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc)) {
				parg = argv[++count];
			}
			strncpy(cipherSpec, parg, MQ_SSL_CIPHER_SPEC_LENGTH);
        } else if (strncmp("-w", argv[count], 2) == 0) {
            // Can only be used with '-l' option above
            if ((*(parg = &(argv[count][2])) == '\0') && (count + 1 < argc)) {
                parg = argv[++count];
            }
            strncpy(certLabel, parg, MQ_CERT_LABEL_LENGTH);
        }
    else
    {
      printf(sruUsageString, argv[0]);
      rc = ERROR_RETURN;
      goto mod_exit;
    }
  }
	if (clientMode) {
		if (strcmp(userid, "") != 0) { 
			csp.AuthenticationType = MQCSP_AUTH_USER_ID_AND_PWD;
			csp.CSPUserIdPtr = &userid[0];
			csp.CSPUserIdLength = (MQLONG) strlen(&userid[0]);
			csp.CSPPasswordPtr = &password[0];
			csp.CSPPasswordLength = (MQLONG) strlen(&password[0]);
		
		}
		// At the moment only server bindings are attempted; allow client bindings
		cno.Options |= MQCNO_CLIENT_BINDING;

		// Create connectionName from hostname and port
		char tempName[MQ_CONN_NAME_LENGTH];
		sprintf(tempName, "%s(%d)", hostName, portNumber);
		strncpy(cd.ConnectionName, tempName, MQ_CONN_NAME_LENGTH);
		strncpy(cd.ChannelName, channelName, MQ_CHANNEL_NAME_LENGTH);

		if (strcmp(userid, "") != 0) { 
			printf("Connecting to host: %s using channel: %s and userid: %s\n", cd.ConnectionName,	cd.ChannelName, &userid[0]);
		} else {
			printf("Connecting to host: %s using channel: %s without flowing userid\n", cd.ConnectionName,	cd.ChannelName);
		}

		if (strcmp(cipherSpec, "") != 0) {
	    	strncpy(cd.SSLCipherSpec, cipherSpec, MQ_SSL_CIPHER_SPEC_LENGTH);
			printf("Connecting using SSL Cipher: %s\n", cd.SSLCipherSpec);
            strncpy(cd.CertificateLabel, certLabel, MQ_CERT_LABEL_LENGTH);
            printf("Connecting using Certificate Label: %s\n", cd.CertificateLabel);
	    }

		cno.ClientConnPtr = &cd;
		cno.SecurityParmsPtr = &csp;	
  }

  /******************************************************************/
  /*                                                                */
  /*   Connect to queue manager                                     */
  /*                                                                */
  /******************************************************************/
  MQCONNX(QMName,                  /* queue manager                  */
         &cno,              /* connection options            */
         &Hcon,                   /* connection handle              */
         &CompCode,               /* completion code                */
         &Reason);               /* reason code                    */

  /* report reason and stop if it failed     */
  if (CompCode == MQCC_FAILED)
  {
    printf("MQCONNX ended with reason code %d\n", Reason);
    rc = ERROR_RETURN;
    goto mod_exit;
  }

  /* if there was a warning report the cause and continue */
  if (CompCode == MQCC_WARNING)
  {
    printf("MQCONN generated a warning with reason code %d\n", Reason);
    printf("Continuing...\n");
  }

  /* Find the name of the queue manager that we connected to */
  rc = sruQueryQMName();

  if (rc)
    goto mod_exit;

  /******************************************************************/
  /*                                                                */
  /*  Create a TDQ on which to receive publications.                */
  /*                                                                */
  /******************************************************************/
  if (ModelQName[0] != '\0')
    strncpy( od.ObjectName, ModelQName, MQ_Q_NAME_LENGTH);
  else
    strcpy( od.ObjectName, "SYSTEM.DEFAULT.MODEL.QUEUE");

  MQOPEN( Hcon
        , &od
        , MQOO_INPUT_EXCLUSIVE
        , &Hobj
        , &CompCode
        , &Reason );

  if (MQCC_OK != CompCode)
  {
    printf("MQOPEN ended with reason code %d\n", Reason);
    rc = ERROR_RETURN;
    goto mod_exit;
  }

  /******************************************************************/
  /*                                                                */
  /*   Discover which resource usage information is supported by    */
  /*   the queue manager that we're connected to by subscribing     */
  /*   to the meta-data topics and processing the resulting         */
  /*   publications.                                                */
  /*                                                                */
  /******************************************************************/
  rc = sruDiscover();

  if (rc)
    goto mod_exit;

	if (autoMode) {
		printf("Entering automatic mode\n");
		//First stage of automatic monitoring is to collect any useful static data from the QM
		char commandLine[256];
		sprintf(commandLine, "%s/TCP/%s(%d)", channelName, hostName, portNumber);
		printf("CommandLine: %s", commandLine);

		//Set envvar and overwrite if already exists
		setenv(mqserver, commandLine, 1);

		if (clientMode) {
//			sprintf(commandLine, "echo $MQSERVER > /tmp/out; echo '%s' | %s -c %s >> /tmp/out", collectQueueDepth, runmqsc, QMName);
			sprintf(commandLine, "echo '%s' | %s -c %s", collectQueueDepth, runmqsc, QMName);
		} else {
			sprintf(commandLine, "echo $MQSERVER > /tmp/out; echo '%s' | %s %s >> /tmp/out%s %s", collectQueueDepth, runmqsc, QMName);
		}
		system(commandLine);

		FILE *fp = popen(commandLine, "r");
		char line[256];
		while (fgets(line, 255, fp) != NULL)
			printf("LINE:%s\n", line);

		int status = pclose(fp);

	}
  /******************************************************************/
  /*                                                                */
  /*   If no class/type was explicitly requested then prompt for    */
  /*   appropriate input.                                           */
  /*                                                                */
  /******************************************************************/
  if (0 == ClassCount)
  {
    rc = sruPromptForClass();
    if (rc)
      goto mod_exit;
  }
  if (0 == TypeCount)
  {
    rc = sruPromptForType();
    if (rc)
      goto mod_exit;
  }

  /******************************************************************/
  /*                                                                */
  /*   Subscribe for messages on the appropriate topics.            */
  /*                                                                */
  /******************************************************************/
  rc = sruSubscribe();
  if (rc)
    goto mod_exit;

  /******************************************************************/
  /*                                                                */
  /*   Get messages from the destination queue                      */
  /*   Loop until there is a failure, or the requested number       */
  /*   of publications has been processed.                          */
  /*                                                                */
  /******************************************************************/
  gmo.Options =   MQGMO_WAIT         /* wait for new messages       */
                | MQGMO_NO_SYNCPOINT /* no transaction              */
                | MQGMO_CONVERT      /* convert if necessary        */
                | MQGMO_FAIL_IF_QUIESCING /* Don't hold up shutdown */
                | MQGMO_NO_PROPERTIES;

  gmo.WaitInterval = 30000;        /* 30 second limit for waiting   */
  gmo.Version = MQGMO_CURRENT_VERSION;
  gmo.MatchOptions = MQMO_NONE;

  rc = ERROR_RETURN; /* Default return code */

  while (CompCode != MQCC_FAILED)
  {
    if (MaxGets >= 0)
    {
      if (GetCount >= MaxGets)
      {
        rc = 0;
        break;
      }
    }
    GetCount++;

    /****************************************************************/
    /* Note that we don't (currently) expect any large resource     */
    /* usage messages, and when dealing with small messages it's    */
    /* less important (performance) to use a dynamically sized      */
    /* buffer.                                                      */
    /****************************************************************/
    buflen = sizeof(buffer); /* fixed buffer size available for GET */

    /****************************************************************/
    /*                                                              */
    /*   MQGET sets Encoding and CodedCharSetId to the values in    */
    /*   the message returned, so these fields should be reset to   */
    /*   the default values before every call, as MQGMO_CONVERT is  */
    /*   specified.                                                 */
    /*                                                              */
    /****************************************************************/
    md.Encoding       = MQENC_NATIVE;
    md.CodedCharSetId = MQCCSI_Q_MGR;

    if (sruDebug > 0)
      printf("Calling MQGET : %d seconds wait time\n", gmo.WaitInterval / 1000);

    MQGET(Hcon,                /* connection handle                 */
          Hobj,                /* object handle                     */
          &md,                 /* message descriptor                */
          &gmo,                /* get message options               */
          buflen,              /* buffer length                     */
          buffer,              /* message buffer                    */
          &messlen,            /* message length                    */
          &CompCode,           /* completion code                   */
          &Reason);            /* reason code                       */

    /* report reason, if any     */
    if (Reason != MQRC_NONE)
    {
      if (Reason == MQRC_NO_MSG_AVAILABLE)
      {                         /* special report for normal end    */
        printf("no more messages\n");
      }
      else                      /* general report for other reasons */
      {
        printf("MQGET ended with reason code %d\n", Reason);

        /* treat truncated message as a failure for this sample   */
        /* as none of the current resource usage publications is  */
        /* expected to be more than 4096 bytes long.              */
        if (Reason == MQRC_TRUNCATED_MSG_FAILED)
          CompCode = MQCC_FAILED;
      }
    }

    /****************************************************************/
    /*   Format and display each message received                   */
    /****************************************************************/
    if (CompCode != MQCC_FAILED)
    {
      sruFormatMessage((PMQCFH) buffer, &md);
      printf( "\n");
    }
  }

mod_exit:

  /******************************************************************/
  /*   Subscription handles, and the TDQ handle, are implicitly     */
  /*   closed at MQDISC.                                            */
  /******************************************************************/

  /******************************************************************/
  /*                                                                */
  /*   Disconnect from the queue manager.                           */
  /*                                                                */
  /******************************************************************/
  if (Hcon != MQHC_UNUSABLE_HCONN)
  {
    MQDISC(&Hcon,                     /* connection handle          */
           &CompCode,                 /* completion code            */
           &Reason);                  /* reason code                */

    /* report reason, if any     */
    if (Reason != MQRC_NONE)
    {
      printf("MQDISC ended with reason code %d\n", Reason);
      rc = ERROR_RETURN;
    }
  }


  /******************************************************************/
  /*                                                                */
  /* END OF AMQSRUAA                                                */
  /*                                                                */
  /******************************************************************/
  if (sruDebug > 0)
    printf("qmmonitor end rc=%d\n", rc);

  return rc;
}

/*********************************************************************/
/* sruPromptForClass:                                                */
/* If no class was explicitly requested then list the classes        */
/* supported, prompt for a class, and then validate the supplied     */
/* class.                                                            */
/*                                                                   */
/*********************************************************************/
int sruPromptForClass(void)
{
  size_t         strLen = 0;
  size_t         idx;
  sruCLASS     * pClass;
  char           Buffer[128];
  int            rc = ERROR_RETURN;


  /* List the supported classes */
  for (pClass = ClassChain; pClass; pClass = pClass->next)
  {
    printf("%.*s : %.*s\n"
          , pClass->classLen
          , pClass->Buffer
          , pClass->descLen
          , pClass->Buffer+pClass->classLen );
  }
  /* Prompt for one of those classes */
  printf("Enter Class selection\n");
  printf("==> ");
  fgets(Buffer, sizeof(Buffer), stdin);

  for (idx = 0; idx < sizeof(Buffer); idx++)
  {
    if ( ( 0 == isalpha( (unsigned char)Buffer[idx] ) )
      && ( 0 == isdigit( (unsigned char)Buffer[idx] ) ) )
    {
      Buffer[idx] = 0;
      strLen = idx;
      break;
    }
  }

  /* Validate the supplied class is one of those supported */
  for (pClass = ClassChain; pClass; pClass = pClass->next)
  {
    if ( ((MQLONG)strLen == pClass->classLen )
      && (0 == memcmp(pClass->Buffer, Buffer, strLen) ) )
    {
      strncpy(Class[ClassCount++], Buffer, MAX_CLASS_SIZE);
      rc = 0;
      break;
    }
  }

  if (rc)
    printf("Invalid Class (%s) supplied\n", Buffer);


  return rc;
}


/*********************************************************************/
/* sruPromptForType:                                                 */
/* If no type was explicitly requested then list the types           */
/* supported (withing the 'current' class), prompt for a type, and   */
/* then validate the supplied type.                                  */
/*                                                                   */
/*********************************************************************/
int sruPromptForType(void)
{
  size_t         strLen = 0;
  size_t         idx;
  char           Buffer[128];
  sruCLASS     * pClass;
  sruTYPE      * pType;
  int            rc = ERROR_RETURN;


  strLen = strlen(Class[0]);
  /* Find the relevant class meta-data */
  for (pClass = ClassChain; pClass; pClass = pClass->next)
  {
    if ( (pClass->classLen == (MQLONG)strLen)
      && (0 == memcmp(Class[0], pClass->Buffer, strLen) ) )
    {
      /* List the types supported by this class */
      for (pType = pClass->type; pType; pType = pType->next)
      {
         printf("%.*s : %.*s\n"
              , pType->typeLen
              , pType->Buffer
              , pType->descLen
              , pType->Buffer+pType->typeLen);
      }
      /* Prompt for one of those types */
      printf("Enter Type selection\n" );
      printf("==> ");
      fgets(Buffer, sizeof(Buffer), stdin);

      for (idx = 0; idx < sizeof(Buffer); idx++)
      {
        if ( ( 0 == isalpha( (unsigned char)Buffer[idx] ) )
          && ( 0 == isdigit( (unsigned char)Buffer[idx] ) ) )
        {
          Buffer[idx] = 0;
          strLen = idx;
          break;
        }
      }

      /* Validate the supplied type is one of those supported */
      for (pType = pClass->type; pType; pType = pType->next)
      {
        if ( ((MQLONG)strLen == pType->typeLen )
          && ( 0 == memcmp(pType->Buffer, Buffer, strLen) ) )
        {
          strncpy(Type[ClassCount - 1], Buffer, MAX_TYPE_SIZE);
          ClassTypeCount[ClassCount - 1]++;
          TypeCount++;
          rc = 0;
          goto mod_exit;
        }
      }

      if ( (rc) && (pType == NULL) )
      {
        printf("Invalid Type (%s) supplied\n", Buffer);
        goto mod_exit;
      }
      break;
    }
  }

  if ( (rc) && (pClass == NULL) )
    printf("Invalid Class (%s) supplied\n", Class[0]);

mod_exit:

  return rc;
}

/*********************************************************************/
/* sruSubscribe:                                                     */
/* Subscribe to the required resource usage publications.            */
/*                                                                   */
/* Lookup the topic strings under which resource usage information   */
/* will be published for the requested class/type pairs.             */
/* Subscribe to those topics.                                        */
/*                                                                   */
/*********************************************************************/
int sruSubscribe(void)
{
  MQSD           sd = {MQSD_DEFAULT};    /* Subscription Descriptor  */
  MQHOBJ         Hsub = MQHO_NONE;       /* object handle            */
  char           Buffer[4096];
  char           ObjectName[MQ_OBJECT_NAME_LENGTH+1];
  int            idx, idx1, idx2, idx3;
  size_t         strLen;
  char         * pObjName = 0;
  MQLONG         CompCode, Reason;
  sruCLASS     * pClass;
  sruTYPE      * pType;
  int            rc = 0;


  if (sruDebug > 0)
    printf("ClassCount: %d TypeCount:%d\n", ClassCount, TypeCount);

  for (idx1 = 0, idx3 = 0; ( (rc == 0) && (idx1 < ClassCount) ); idx1++)
  {
    strLen = strlen(Class[idx1]);
    for (pClass = ClassChain; pClass; pClass = pClass->next)
    {
      if ( (pClass->classLen == (MQLONG)strLen )
        && (0 == memcmp(pClass->Buffer, Class[idx1], strLen) ) )
        break;
    }

    if (NULL == pClass)
    {
      printf("Class %s not recognised\n", Class[idx1]);
      rc = ERROR_RETURN;
      break;
    }
    else
    {
      for (idx2 = 0; ( (rc == 0) && (idx2 < ClassTypeCount[idx1]) ); idx2++, idx3++)
      {
        strLen = strlen(Type[idx3]);
        for (pType = pClass->type; pType; pType = pType->next)
        {
          if ( (pType->typeLen == (MQLONG)strLen)
            && (0 == memcmp(pType->Buffer, Type[idx3], strLen) ) )
            break;
        }

        if (NULL == pType)
        {
          printf("Class: %s Type: %s not recognised\n", Class[idx1], Type[idx3]);
          rc = ERROR_RETURN;
          break;
        }
        else
        {
          sd.Options = MQSO_CREATE | MQSO_NON_DURABLE | MQSO_FAIL_IF_QUIESCING;
          /********************************************************************************/
          /* If the meta-data included MQIAMO_MONITOR_FLAGS_OBJNAME then the data         */
          /* is available on a per-object basis. Under these circumstances we must have   */
          /* an object name to qualify the topic.                                         */
          /********************************************************************************/
          if (pType->flags & MQIAMO_MONITOR_FLAGS_OBJNAME)
          {
            if (TypeObjName[idx3])
              pObjName = TypeObjName[idx3];
            else if (ClassObjName[idx1])
              pObjName = ClassObjName[idx1];

            if (!pObjName)
            {
              printf("An object name is required for Class(%.*s) Type(%.*s)\n"
                    , pClass->classLen, pClass->Buffer, pType->typeLen, pType->Buffer );
              printf("Enter object name\n");

              printf("==> ");
              memset(ObjectName, 0, MQ_OBJECT_NAME_LENGTH);
              fgets(ObjectName, MQ_OBJECT_NAME_LENGTH, stdin);
              for (idx = 0; idx < MQ_OBJECT_NAME_LENGTH; idx++)
              {
                if ('\n' == ObjectName[idx])
                {
                  ObjectName[idx] = 0;
                  break;
                }
                else if ('/' == ObjectName[idx])
                {
                  /* Object names may contain '/' characters, but those confuse    */
                  /* the topic hierarchies. The queue manager accepts '&' in place */
                  /* of the slash but we must do that change in the topic string   */
                  ObjectName[idx] = '&';
                }
              }
              ObjectName[MQ_OBJECT_NAME_LENGTH] = 0;
              pObjName = ObjectName;
            }
			if (sruDebug > 0)
            	printf("ObjName: %s\n", pObjName);
            sprintf(Buffer, pType->Buffer + pType->typeLen + pType->descLen, pObjName);
            sd.ObjectString.VSPtr = Buffer;
            sd.ObjectString.VSLength = (MQLONG)(pType->topicLen + strlen(pObjName) - strlen("%s"));
            sd.ObjectString.VSCCSID = pType->topicCCSID;
          }
          else
          {
            sd.ObjectString.VSPtr = pType->Buffer + pType->typeLen + pType->descLen;
            sd.ObjectString.VSLength = pType->topicLen;
            sd.ObjectString.VSCCSID = pType->topicCCSID;
          }

          if (sruDebug > 0)
            printf( "Subscribing to topic: %.*s\n", sd.ObjectString.VSLength, (char*)sd.ObjectString.VSPtr );

          MQSUB(Hcon, &sd, &Hobj, &Hsub, &CompCode, &Reason);

          if (MQCC_OK != CompCode)
          {
            printf("Subscribe to %.*s failed for %d:%d\n"
                  , sd.ObjectString.VSLength, (char*)sd.ObjectString.VSPtr, CompCode, Reason);
            rc = ERROR_RETURN;
            break;
          }
        }
      }
    }
  }


  return rc;
}

/*********************************************************************/
/* sruDiscover:                                                      */
/* subscribe to the meta data topics and read the meta data retained */
/* publications to determine what resource usage data will be        */
/* published by the currently connected queue manager.               */
/*                                                                   */
/* the queue manager publishes a set of PCF messages, as retained    */
/* messages, describing what resource usage information can be       */
/* subscribed to in the current queue manager environment.           */
/*                                                                   */
/* Note that this information is relatively dynamic, and might       */
/* change with the queue manager configuration, or service level.    */
/*                                                                   */
/* This data is ideally re-processed each time the queue manager     */
/* is restarted, and 'applications' don't depend upon hard coded     */
/* values other than the location of the root of the meta data.      */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
int sruDiscover(void)
{
  MQHOBJ     Hsub = MQHO_NONE;       /* object handle               */
  MQSD       sd = {MQSD_DEFAULT};    /* Subscription Descriptor     */
  MQMD       md = {MQMD_DEFAULT};    /* Message Descriptor          */
  MQGMO      gmo = {MQGMO_DEFAULT};  /* get message options         */
  MQLONG     CompCode, Reason;
  char       Buffer[4096];
  PMQCFH     cfh;
  PMQCFIN    cfin;
  PMQCFST    cfst;
  PMQCFGR    cfgr;
  char     * topic = NULL;
  char     * type;
  char     * class;
  char     * desc;
  MQLONG     topic_len = 0;
  MQLONG     topic_ccsid = MQCCSI_UNDEFINED;
  MQLONG     type_len;
  MQLONG     class_len;
  MQLONG     desc_len;
  MQLONG     msg_len;
  MQLONG     class_id = 0, type_id = 0, elem_id = 0, elem_type = 0, flags = 0;
  int        idx;
  int        idx1;
  sruCLASS * pClass;
  sruTYPE  * pType;
  char       NLS_suffix[8] = {0};
  char     * lang;
  int        rc = 0;


  /******************************************************************/
  /* The meta-data is published in a set of languages. Use the      */
  /* first five bytes of the LANG environment variable as the       */
  /* suffix.                                                        */
  /* By restricting presentation of 'constant' data to the strings  */
  /* in the meta-data we achieve a level of NLS independance.       */
  /******************************************************************/
  lang = getenv("LANG");
  if ( (lang) && (strcmp(lang, "C")) && (strlen(lang) >= 5) &&
       (isalpha((unsigned char)lang[0])) && (isalpha((unsigned char)lang[1])) && (lang[2] == '_') &&
       (isalpha((unsigned char)lang[3])) && (isalpha((unsigned char)lang[4])) )
  {
    sprintf(NLS_suffix, "/%.5s", lang);
  }

  /******************************************************************/
  /*                                                                */
  /* Subscribe using our previously created TDQ destination queue   */
  /*                                                                */
  /******************************************************************/
  sprintf(Buffer, "%s/INFO/QMGR/%s/Monitor/METADATA/CLASSES%s", meta_prefix, QMName, NLS_suffix);
  sd.ObjectString.VSPtr = Buffer;
  sd.ObjectString.VSLength = (MQLONG)strlen(Buffer);
  sd.Options = MQSO_CREATE | MQSO_NON_DURABLE | MQSO_FAIL_IF_QUIESCING;

  if (sruDebug > 0)
    printf("Subscribing to topic: %.*s\n", sd.ObjectString.VSLength, (char*)sd.ObjectString.VSPtr);

  MQSUB(Hcon, &sd, &Hobj, &Hsub, &CompCode, &Reason);

  /* report reason, if any; stop if failed      */
  if (Reason != MQRC_NONE)
  {
    printf("MQSUB ended with reason code %d\n", Reason);
    rc = ERROR_RETURN;
    goto mod_exit;
  }

  gmo.Version = MQGMO_CURRENT_VERSION;
  gmo.Options =   MQGMO_NO_SYNCPOINT
                | MQGMO_CONVERT
                | MQGMO_NO_PROPERTIES
                | MQGMO_FAIL_IF_QUIESCING
                | MQGMO_WAIT;
  gmo.MatchOptions = MQMO_NONE;
  /* We don't assume that the meta-data is published 'immediately'     */
  /* Give the queue manager a few seconds to produce this data.        */
  gmo.WaitInterval = 10000;
  md.Encoding       = MQENC_NATIVE;
  md.CodedCharSetId = MQCCSI_Q_MGR;

  MQGET(Hcon, Hobj, &md, &gmo, sizeof(Buffer), Buffer, &msg_len, &CompCode, &Reason);

  if (MQCC_OK != CompCode)
  {
    if ( (MQRC_NO_MSG_AVAILABLE == Reason) && (NLS_suffix[0] != 0) )
    {
      MQCLOSE(Hcon, &Hsub, MQCO_NONE, &CompCode, &Reason);

      /* It looks like the requested language is not available, use the default */
      sprintf(Buffer, "%s/INFO/QMGR/%s/Monitor/METADATA/CLASSES", meta_prefix, QMName);
      sd.ObjectString.VSPtr = Buffer;
      sd.ObjectString.VSLength = (MQLONG)strlen(Buffer);
      sd.Options = MQSO_CREATE | MQSO_NON_DURABLE | MQSO_FAIL_IF_QUIESCING;
      if (sruDebug > 0)
        printf("Subscribing to topic: %.*s\n", sd.ObjectString.VSLength, (char*)sd.ObjectString.VSPtr);

      MQSUB(Hcon, &sd, &Hobj, &Hsub, &CompCode, &Reason);

      /* report reason, if any; stop if failed      */
      if (Reason != MQRC_NONE)
      {
        printf("MQSUB ended with reason code %d\n", Reason);
        rc = ERROR_RETURN;
        goto mod_exit;
      }

      MQGET(Hcon, Hobj, &md, &gmo, sizeof(Buffer), Buffer, &msg_len, &CompCode, &Reason);
    }

    if (MQCC_OK != CompCode)
    {
      printf("MQGET ended with reason code %d\n", Reason);
      rc = ERROR_RETURN;
      goto mod_exit;
    }
  }

  MQCLOSE(Hcon, &Hsub, MQCO_NONE, &CompCode, &Reason);

  if (MQCC_OK != CompCode)
  {
    printf("MQCLOSE ended with reason code %d\n", Reason);
    rc = ERROR_RETURN;
    goto mod_exit;
  }

  /****************************************************************************/
  /* Parse the resulting publication and populate the first level of the      */
  /* class/type/element tree based upon the content.                          */
  /*                                                                          */
  /* Note that it is good practice not to depend upon the order of elements   */
  /* in a PCF message.                                                        */
  /****************************************************************************/
  cfh = (PMQCFH)Buffer;
  cfin = (PMQCFIN)(Buffer+cfh->StrucLength);

  for (idx = 0; idx < cfh->ParameterCount; idx++)
  {
    if ( (cfin->Type == MQCFT_GROUP )
      && (cfin->Parameter == MQGACF_MONITOR_CLASS ) )
    {
      cfgr = (PMQCFGR)cfin;
      cfin = (PMQCFIN)(((char *)cfgr) + cfgr->StrucLength);
      topic = class = desc = "";
      topic_len = class_len = desc_len = 0;

      for (idx1 = 0; idx1 < cfgr->ParameterCount; idx1++)
      {
        cfst = (PMQCFST)cfin;
        if ( (cfin->Type == MQCFT_INTEGER)
          && (cfin->Parameter == MQIAMO_MONITOR_CLASS) )
        {
          class_id = cfin->Value;
          cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
        }
        else if ( (cfin->Type == MQCFT_STRING)
               && (cfin->Parameter == MQCAMO_MONITOR_CLASS) )
        {
          class_len = cfst->StringLength;
          class = cfst->String;;
          cfin = (PMQCFIN)(((char *)cfst) + cfst->StrucLength);
        }
        else if ( (cfin->Type == MQCFT_STRING)
               && (cfin->Parameter == MQCAMO_MONITOR_DESC) )
        {
          desc_len = cfst->StringLength;
          desc = cfst->String;;
          cfin = (PMQCFIN)(((char *)cfst) + cfst->StrucLength);
        }
        else if ( (cfin->Type == MQCFT_STRING)
               && (cfin->Parameter == MQCA_TOPIC_STRING) )
        {
          topic_len = cfst->StringLength;
          topic = cfst->String;;
          topic_ccsid = cfst->CodedCharSetId;
          cfin = (PMQCFIN)(((char *)cfst) + cfst->StrucLength);
        }
        else if ( (cfin->Type == MQCFT_INTEGER )
               && (cfin->Parameter == MQIAMO_MONITOR_FLAGS ) )
        {
          flags = cfin->Value;
          cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
        }
        else
          cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
      }

      rc = sruRegisterClass(class_id, class, class_len, desc, desc_len, topic, topic_len, topic_ccsid, flags);
      if (rc)
        goto mod_exit;
    }
    else
      cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
  }

  /****************************************************************************/
  /* For each of the discovered classes, discover what types are supported    */
  /* by those classes and populate the second level of the class/type/        */
  /* element tree.                                                            */
  /****************************************************************************/
  for (pClass = ClassChain; pClass; pClass = pClass->next)
  {
    sprintf(Buffer, "%.*s", pClass->topicLen, pClass->Buffer + pClass->classLen + pClass->descLen );
    sd.ObjectString.VSPtr = Buffer;
    sd.ObjectString.VSLength = pClass->topicLen;
    sd.ObjectString.VSCCSID = pClass->topicCCSID;
    sd.Options = MQSO_CREATE | MQSO_NON_DURABLE | MQSO_FAIL_IF_QUIESCING;

    if (sruDebug > 0)
      printf("Subscribing to topic: %.*s\n", sd.ObjectString.VSLength, (char*)sd.ObjectString.VSPtr);

    MQSUB(Hcon, &sd, &Hobj, &Hsub, &CompCode, &Reason);

    /* report reason, if any; stop if failed      */
    if (Reason != MQRC_NONE)
    {
      printf("MQSUB ended with reason code %d\n", Reason);
      rc = ERROR_RETURN;
      goto mod_exit;
    }

    gmo.Version = MQGMO_CURRENT_VERSION;
    gmo.Options =   MQGMO_NO_SYNCPOINT
                  | MQGMO_CONVERT
                  | MQGMO_NO_PROPERTIES
                  | MQGMO_FAIL_IF_QUIESCING
                  | MQGMO_WAIT;
    gmo.MatchOptions = MQMO_NONE;
    /* We don't assume that the meta-data is published 'immediately'     */
    /* Give the queue manager a few seconds to produce this data.        */
    gmo.WaitInterval = 10000;
    md.Encoding       = MQENC_NATIVE;
    md.CodedCharSetId = MQCCSI_Q_MGR;

    MQGET(Hcon, Hobj, &md, &gmo, sizeof(Buffer), Buffer, &msg_len, &CompCode, &Reason);

    if (MQCC_OK != CompCode)
    {
      printf("MQGET ended with reason code %d\n", Reason);
      rc = ERROR_RETURN;
      goto mod_exit;
    }

    MQCLOSE(Hcon, &Hsub, MQCO_NONE, &CompCode, &Reason);

    if (MQCC_OK != CompCode)
    {
      printf("MQCLOSE ended with reason code %d\n", Reason);
      rc = ERROR_RETURN;
      goto mod_exit;
    }

    cfh = (PMQCFH)Buffer;
    cfin = (PMQCFIN)(Buffer + cfh->StrucLength);
    for (idx = 0; idx < cfh->ParameterCount; idx++)
    {
      if ( (cfin->Type == MQCFT_INTEGER )
        && (cfin->Parameter == MQIAMO_MONITOR_CLASS ) )
      {
        class_id = cfin->Value;
        cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
      }
      else if ( (cfin->Type == MQCFT_INTEGER )
             && (cfin->Parameter == MQIAMO_MONITOR_FLAGS ) )
      {
        flags = cfin->Value;
        cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
      }
      else if ( (cfin->Type == MQCFT_GROUP )
             && (cfin->Parameter == MQGACF_MONITOR_TYPE ) )
      {
        cfgr = (PMQCFGR)cfin;
        cfin = (PMQCFIN)(((char *)cfgr) + cfgr->StrucLength);
        topic = type = desc = "";
        topic_len = type_len = desc_len = 0;
        for (idx1=0; idx1 < cfgr->ParameterCount; idx1++)
        {
          cfst = (PMQCFST)cfin;
          if ( (cfin->Type == MQCFT_INTEGER)
            && (cfin->Parameter == MQIAMO_MONITOR_TYPE) )
          {
            type_id = cfin->Value;
            cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
          }
          else if ( (cfin->Type == MQCFT_STRING)
                 && (cfin->Parameter == MQCAMO_MONITOR_TYPE) )
          {
            type_len = cfst->StringLength;
            type = cfst->String;
            cfin = (PMQCFIN)(((char *)cfst) + cfst->StrucLength);
          }
          else if ( (cfin->Type == MQCFT_STRING)
                 && (cfin->Parameter == MQCAMO_MONITOR_DESC) )
          {
            desc_len = cfst->StringLength;
            desc = cfst->String;
            cfin = (PMQCFIN)(((char *)cfst) + cfst->StrucLength);
          }
          else if ( (cfin->Type == MQCFT_STRING)
                 && (cfin->Parameter == MQCA_TOPIC_STRING) )
          {
            topic_len = cfst->StringLength;
            topic = cfst->String;
            topic_ccsid = cfst->CodedCharSetId;
            cfin = (PMQCFIN)(((char  *)cfst) + cfst->StrucLength);
          }
          else
            cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
        }

        rc = sruRegisterType(class_id, type_id, type, type_len, desc, desc_len, topic, topic_len, topic_ccsid, flags);
        if (rc)
          goto mod_exit;
      }
      else
        cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
    }
  }

  /****************************************************************************/
  /* For each type, within each class, discover what elements are published   */
  /* in those publications, and add the element descriptions to the           */
  /* class/type/element tree.                                                 */
  /****************************************************************************/
  for (pClass = ClassChain; pClass; pClass = pClass->next)
  {
    for (pType = pClass->type; pType; pType = pType->next)
    {
      sprintf(Buffer, "%.*s", pType->topicLen, pType->Buffer + pType->typeLen + pType->descLen);
      sd.ObjectString.VSPtr = Buffer;
      sd.ObjectString.VSLength = pType->topicLen;
      sd.ObjectString.VSCCSID = pType->topicCCSID;
      sd.Options = MQSO_CREATE | MQSO_NON_DURABLE | MQSO_FAIL_IF_QUIESCING;

      if (sruDebug > 0)
        printf( "Subscribing to topic: %.*s\n", sd.ObjectString.VSLength, (char*)sd.ObjectString.VSPtr );

      MQSUB(Hcon, &sd, &Hobj, &Hsub, &CompCode, &Reason);

      /* report reason, if any; stop if failed      */
      if (Reason != MQRC_NONE)
      {
        printf("MQSUB ended with reason code %d\n", Reason);
        rc = ERROR_RETURN;
        goto mod_exit;
      }

      gmo.Version = MQGMO_CURRENT_VERSION;
      gmo.Options =   MQGMO_NO_SYNCPOINT
                    | MQGMO_CONVERT
                    | MQGMO_NO_PROPERTIES
                    | MQGMO_FAIL_IF_QUIESCING
                    | MQGMO_WAIT;
      gmo.MatchOptions = MQMO_NONE;
      /* We don't assume that the meta-data is published 'immediately'     */
      /* Give the queue manager a few seconds to produce this data.        */
      gmo.WaitInterval = 10000;
      md.Encoding       = MQENC_NATIVE;
      md.CodedCharSetId = MQCCSI_Q_MGR;

      MQGET(Hcon, Hobj, &md, &gmo, sizeof(Buffer), Buffer, &msg_len, &CompCode, &Reason);

      if (MQCC_OK != CompCode)
      {
        printf("MQGET ended with reason code %d\n", Reason);
        rc = ERROR_RETURN;
        goto mod_exit;
      }

      MQCLOSE(Hcon, &Hsub, MQCO_NONE, &CompCode, &Reason);

      if (MQCC_OK != CompCode)
      {
        printf("MQCLOSE ended with reason code %d\n", Reason);
        rc = ERROR_RETURN;
        goto mod_exit;
      }

      cfh = (PMQCFH)Buffer;
      cfin = (PMQCFIN)(Buffer+cfh->StrucLength);
      for (idx = 0; idx < cfh->ParameterCount; idx++)
      {
        if ( (cfin->Type == MQCFT_INTEGER )
          && (cfin->Parameter == MQIAMO_MONITOR_CLASS ) )
        {
          class_id = cfin->Value;
          cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
        }
        else if ( (cfin->Type == MQCFT_INTEGER )
               && (cfin->Parameter == MQIAMO_MONITOR_TYPE ) )
        {
          type_id = cfin->Value;
          cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
        }
        else if ( (cfin->Type == MQCFT_STRING)
               && (cfin->Parameter == MQCA_TOPIC_STRING) )
        {
          cfst = (PMQCFST)cfin;
          topic_len = cfst->StringLength;
          topic = cfst->String;;
          topic_ccsid = cfst->CodedCharSetId;
          cfin = (PMQCFIN)(((char *)cfst) + cfst->StrucLength);
        }
        else if ( (cfin->Type == MQCFT_GROUP )
               && (cfin->Parameter == MQGACF_MONITOR_ELEMENT ) )
        {
          cfgr = (PMQCFGR)cfin;
          cfin = (PMQCFIN)(((char *)cfgr) + cfgr->StrucLength);
          topic = type = desc = "";
          topic_len = type_len = desc_len = 0;
          for (idx1=0; idx1 < cfgr->ParameterCount; idx1++)
          {
            cfst = (PMQCFST)cfin;
            if ( (cfin->Type == MQCFT_INTEGER)
              && (cfin->Parameter == MQIAMO_MONITOR_ELEMENT) )
            {
              elem_id = cfin->Value;
              cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
            }
            else if ( (cfin->Type == MQCFT_INTEGER)
                   && (cfin->Parameter == MQIAMO_MONITOR_DATATYPE) )
            {
              elem_type = cfin->Value;
              cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
            }
            else if ( (cfin->Type == MQCFT_STRING)
                   && (cfin->Parameter == MQCAMO_MONITOR_DESC) )
            {
              desc_len = cfst->StringLength;
              desc = cfst->String;;
              cfin = (PMQCFIN)(((char *)cfst) + cfst->StrucLength);
            }
            else
              cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
          }

          rc = sruRegisterElement(class_id, type_id, elem_id, elem_type, desc, desc_len);
          if (rc)
            goto mod_exit;
        }
        else
          cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
      }

      rc = sruUpdateType(class_id, type_id, topic, topic_len, topic_ccsid);
      if (rc)
        goto mod_exit;
    }
  }

mod_exit:


  return rc;
}

/*********************************************************************/
/* sruFormatMessage:                                                 */
/* Format a resource usage publication.                              */
/*                                                                   */
/*********************************************************************/
int sruFormatMessage(PMQCFH cfh, MQMD * pMd)
{
  MQLONG     class = -1, type = -1;
  MQLONG     Count;
  PMQCFIN    cfin;
  PMQCFST    cfst;
  PMQCFIN64  cfin64;
  MQINT64    interval = -1;
  MQINT64    int64;
  char     * pObjName = 0;
  MQLONG     ObjNameLen = 0;
  MQUINT64   days = 0, hours = 0, minutes = 0, seconds = 0, millisecs = 0;
  sruCLASS * pClass;
  sruTYPE  * pType;
  sruELEM  * pElem;
  int        rc = ERROR_RETURN;


  printf("Publication received PutDate:%8.8s PutTime:%8.8s"
        , pMd->PutDate, pMd->PutTime);
  cfin = (PMQCFIN)(((char *)cfh) + cfh->StrucLength);

  /* Once again, it is good practise not to depend upon   */
  /* the order of elements in a PCF message.              */
  /* First pass to establish the Class, Type and interval */
  for (Count = 0; Count < cfh->ParameterCount; Count++)
  {
    if ( (cfin->Type == MQCFT_INTEGER)
      && (cfin->Parameter == MQIAMO_MONITOR_CLASS))
    {
      class = cfin->Value;
    }
    else if ( (cfin->Type == MQCFT_INTEGER)
            &&(cfin->Parameter == MQIAMO_MONITOR_TYPE))
    {
      type = cfin->Value;
    }
    else if ( (cfin->Type == MQCFT_INTEGER64)
           && (cfin->Parameter == MQIAMO64_MONITOR_INTERVAL))
    {
      printf(" Interval:");
      cfin64 = (PMQCFIN64) cfin;
      memcpy(&interval, &cfin64->Value, sizeof(interval));
      millisecs = (interval % 1000000) / 1000;
      seconds = interval / 1000000;
      if (seconds)
      {
        minutes = seconds / 60;
        seconds = seconds % 60;
        if (minutes)
        {
          hours = minutes / 60;
          minutes = minutes % 60;
          if (hours)
          {
            days = hours / 24;
            hours = hours % 24;
            if (days)
              printf("%" Int64 "u days,", days);
            printf("%" Int64 "u hours,", hours);
          }
          printf("%" Int64 "u minutes,", minutes);
        }
      }
      printf("%" Int64 "u.%3.3" Int64 "u seconds", seconds, millisecs);
    }
    else if ( (cfin->Type == MQCFT_STRING)
              && ( (cfin->Parameter == MQCA_Q_NAME)
                  || (cfin->Parameter == MQCACF_APPL_NAME)
#ifdef MQCACF_NHA_INSTANCE_NAME
                  || (cfin->Parameter == MQCACF_NHA_INSTANCE_NAME)
#endif
#ifdef MQCACF_NHA_GROUP_NAME
                  || (cfin->Parameter == MQCACF_NHA_GROUP_NAME)
#endif
                  ) )
    {
      cfst = (PMQCFST)cfin;
      pObjName = cfst->String;
      ObjNameLen = cfst->StringLength;
    }
    cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
  }
  printf( "\n");

  pClass = sruIdToClass(class);
  if (NULL == pClass)
  {
    printf( "Unrecognised resource usage message\n");
  }
  else
  {
    pType = sruIdToType(pClass, type);
    if (NULL == pType)
    {
      printf( "Unrecognised resource usage message for class %.*s %d\n",
               pClass->classLen, pClass->Buffer, type);
    }
    else
    {
      cfin = (PMQCFIN)(((char *)cfh) + cfh->StrucLength);
      for (Count = 0; Count < cfh->ParameterCount; Count ++)
      {
        if ( (cfin->Type == MQCFT_INTEGER)
          && ( (cfin->Parameter == MQIAMO_MONITOR_CLASS)
            || (cfin->Parameter == MQIAMO_MONITOR_TYPE)
            || (cfin->Parameter == MQIACF_OBJECT_TYPE) ) )
        {
        }
        else if ( (cfin->Type == MQCFT_INTEGER64)
               && (cfin->Parameter == MQIAMO64_MONITOR_INTERVAL) )
        {
        }
        else
        {
          pElem = sruIdToElem(pType, cfin->Parameter);
          if (pElem)
          {
            sruFormatElem(cfin, pElem, interval, pObjName, ObjNameLen);
          }
          else
          {
            if (cfin->Type == MQCFT_INTEGER)
            {
              printf("Parameter(%d) Value(%d) not recognised\n", cfin->Parameter, cfin->Value);
            }
            else if (cfin->Type == MQCFT_INTEGER64)
            {
              cfin64 = (PMQCFIN64)cfin;
              memcpy(&int64, &cfin64->Value, sizeof(int64));
              printf("Parameter(%d) Value(%" Int64 "d) not recognised\n", cfin64->Parameter, int64);
            }
          }
        }
        cfin = (PMQCFIN)(((char *)cfin) + cfin->StrucLength);
      }

      rc = 0;
      /* Make sure the formatted output isn't hidden by the C runtime buffering */
      fflush(stdout);
    }
  }


  return rc;
}

/*********************************************************************/
/* sruFormatElem:                                                    */
/* The element data in the meta-data includes some information on    */
/* the 'type' of the element, and therefore how that element might   */
/* be displayed.                                                     */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
void sruFormatElem( PMQCFIN   cfin
                  , sruELEM * pElem
                  , MQINT64       interval
                  , char        * pObjName
                  , MQLONG        ObjNameLen
                  )
{
  PMQCFIN64 cfin64;
	float f, cpu_util;
	static float cpu_user = -1, cpu_system = -1;
  MQINT64   rate;
  MQINT64   int64;
  char      buffer[64];

  if (sruDebug > 0) {
    printf("pElem type: %u ",pElem->type);
  }

  if (pObjName)
  {
    printf("%-48.*s", ObjNameLen, pObjName);
  }
  
  MQLONG            id;
  MQLONG            type;
  MQLONG            descLen;
  char              Buffer[1];

  cfin64 = (PMQCFIN64) cfin;

  switch (pElem->type)
  {
    case MQIAMO_MONITOR_PERCENT:    //type: 10000
      if (cfin->Type == MQCFT_INTEGER)
        f = (float)cfin->Value;
      else
      {
        memcpy(&int64, &cfin64->Value, sizeof(int64));
        f = (float)int64;
      }
      printf("%.*s %.2f%%\n", pElem->descLen, pElem->Buffer, f/100);
		  // Search for the User and System CPU, and when both present add them up, print total to stderr and reset
      if (strncmp(pElem->Buffer, "User CPU", 8) == 0) {
        cpu_user = f;
			  //printf("Setting User: %f\n", f);
      }
      if (strncmp(pElem->Buffer, "System CPU", 10) == 0) {
        cpu_system = f;
			  //printf("Setting Sys: %f\n", f);
      }
      if ((cpu_user >= 0) && (cpu_system >= 0)) {
        cpu_util = cpu_user + cpu_system;
        fprintf(stderr, "cpu_util=%.2f\n", cpu_util / 100);
        cpu_user = -1;
        cpu_system = -1;
      }
      if (strncmp(pElem->Buffer, "lock contention", 15) == 0) {
        fprintf(stderr, "q_lock=%.2f\n", f/100);
      }
      if (strncmp(pElem->Buffer, "queue avoided puts", 18) == 0) {
        fprintf(stderr, "q_avoid=%.2f\n", f/100);
      }
      if (strncmp(pElem->Buffer, "Log - current primary", 21) == 0) {
        fprintf(stderr, "log_curr=%.2f\n", f/100);
      }
      if (strncmp(pElem->Buffer, "Log - workload primary", 22) == 0) {
        fprintf(stderr, "log_work=%.2f\n", f/100);
      }
      //NHAREPLICA (REPLICATION & RECOVERY)
      if (strncmp(pElem->Buffer, "Log file system - free space", 28) == 0) {
        if (pObjName) {
          fprintf(stderr, "%.*s_nha_log_filesystem_free_space=%.2f\n", ObjNameLen, pObjName, f/100);
        } else {
          fprintf(stderr, "log_filesystem_free_space=%.2f\n", f/100);
        }
      }
      if (strncmp(pElem->Buffer, "Queue Manager file system - free space", 38) == 0) {
        if (pObjName) {
          fprintf(stderr, "%.*s_nha_qm_filesystem_free_space=%.2f\n", ObjNameLen, pObjName, f/100);
        } else {
          fprintf(stderr, "qm_filesystem_free_space=%.2f\n", f/100);
        }
      }
      break;

    case MQIAMO_MONITOR_HUNDREDTHS:   //type: 100
      if (cfin->Type == MQCFT_INTEGER)
        f = (float)cfin->Value;
      else
      {
        memcpy(&int64, &cfin64->Value, sizeof(int64));
        f = (float)int64;
      }
      printf("%.*s %.2f\n", pElem->descLen, pElem->Buffer, f/100);
      break;

    case MQIAMO_MONITOR_KB:   //type: 1024
      if (cfin->Type == MQCFT_INTEGER)
        printf("%.*s %dKB\n", pElem->descLen, pElem->Buffer, cfin->Value);
      else
      {
        memcpy(&int64, &cfin64->Value, sizeof(int64));
        printf("%.*s %" Int64 "dKB\n", pElem->descLen, pElem->Buffer, int64);
      }
      break;

    case MQIAMO_MONITOR_MB:   //type: 1048576
      if (cfin->Type == MQCFT_INTEGER)
        printf("%.*s %dMB\n", pElem->descLen, pElem->Buffer, cfin->Value);
      else
      {
        memcpy(&int64, &cfin64->Value, sizeof(int64));
        printf("%.*s %" Int64 "dMB\n", pElem->descLen, pElem->Buffer, int64);
      }
      //NHAREPLICA (REPLICATION & RECOVERY)
      if (strncmp(pElem->Buffer, "Queue Manager file system - bytes in use", 40) == 0) {
        if (cfin->Type == MQCFT_INTEGER)
          if (pObjName) {
            fprintf(stderr, "%.*s_nha_qm_filesystem_mbytes=%d\n", ObjNameLen, pObjName, cfin->Value);
          } else {
            fprintf(stderr, "qm_filesystem_mbytes=%d\n", cfin->Value);
          }
        else
        {
          memcpy(&int64, &cfin64->Value, sizeof(int64));
          if (pObjName) {
            fprintf(stderr, "%.*s_nha_qm_filesystem_mbytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
          } else {
            fprintf(stderr, "qm_filesystem_mbytes=%" Int64 "d\n", int64);
          }
        }
      }
      //Log
      if (strncmp(pElem->Buffer, "Log - bytes required for media recovery", 39) == 0) {
        if (cfin->Type == MQCFT_INTEGER)
          fprintf(stderr, "log_media_recovery_mbytes=%d\n", cfin->Value);
        else
        {
          memcpy(&int64, &cfin64->Value, sizeof(int64));
          fprintf(stderr, "log_media_recovery_mbytes=%" Int64 "d\n", int64);
        }
      }

      if (strncmp(pElem->Buffer, "Log - bytes occupied by reusable extents", 40) == 0) {
        if (cfin->Type == MQCFT_INTEGER)
          fprintf(stderr, "log_reusable_extents_mbytes=%d\n", cfin->Value);
        else
        {
          memcpy(&int64, &cfin64->Value, sizeof(int64));
          fprintf(stderr, "log_reusable_extents_mbytes=%" Int64 "d\n", int64);
        }
      }

      if (strncmp(pElem->Buffer, "Log - bytes occupied by extents waiting to be archived", 54) == 0) {
        if (cfin->Type == MQCFT_INTEGER)
          fprintf(stderr, "log_extents_waiting_archive_mbytes=%d\n", cfin->Value);
        else
        {
          memcpy(&int64, &cfin64->Value, sizeof(int64));
          fprintf(stderr, "log_extents_waiting_archive_mbytes=%" Int64 "d\n", int64);
        }
      }
      break;

    case MQIAMO_MONITOR_GB:   //type: 100000000
      if (cfin->Type == MQCFT_INTEGER)
        printf("%.*s %dGB\n", pElem->descLen, pElem->Buffer, cfin->Value);
      else
      {
        memcpy(&int64, &cfin64->Value, sizeof(int64));
        printf("%.*s %" Int64 "dGB\n", pElem->descLen, pElem->Buffer, int64);
      }

      break;

	  case MQIAMO_MONITOR_MICROSEC:   //type: 1000000
      if (cfin->Type == MQCFT_INTEGER) {
        printf("%.*s %d uSec\n", pElem->descLen, pElem->Buffer,	cfin->Value);
        if (strncmp(pElem->Buffer, "Log - write latency", 19) == 0) {
          fprintf(stderr, "log_lat=%d\n", cfin->Value);
        }
        if (strncmp(pElem->Buffer, "average queue time", 18) == 0) {
          fprintf(stderr, "q_time=%d\n", cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Log - slowest write since restart", 33) == 0) {
          fprintf(stderr, "log_slowest_write=%d\n", cfin->Value);
        }
        //NHAREPLICA (REPLICATION & RECOVERY)
        if (strncmp(pElem->Buffer, "Log write average acknowledgement latency", 41) == 0) {
          fprintf(stderr, "%.*s_nha_log_ack_lat=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Synchronous log data average compression time", 45) == 0) {
          fprintf(stderr, "%.*s_nha_sync_compress_time=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, " Catch-up log data average compression time", 42) == 0) {
          fprintf(stderr, "%.*s_nha_catchup_compress_time=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Average network round trip time", 31) == 0) {
          fprintf(stderr, "%.*s_nha_net_round_trip=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Synchronous log data average decompression time", 47) == 0) {
          fprintf(stderr, "%.*s_nha_sync_decompress_time=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Catch-up log data average decompression time", 44) == 0) {
          fprintf(stderr,"%.*s_nha_catchup_decompress_time=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Log data average compression time", 33) == 0) {
          fprintf(stderr,"%.*s_nha_log_compress_time=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Log data average decompression time", 35) == 0) {
          fprintf(stderr,"%.*s_nha_log_decompress_time=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
      }
      else
      {
        memcpy(&int64, &cfin64->Value, sizeof(int64));
        printf("%.*s %" Int64 "d uSec\n", pElem->descLen, pElem->Buffer, int64);
        if (strncmp(pElem->Buffer, "Log - write latency", 19) == 0) {
          fprintf(stderr, "log_lat=%" Int64 "d\n", int64);
        }
        if (strncmp(pElem->Buffer, "average queue time", 18) == 0) {
          fprintf(stderr, "q_time=%" Int64 "d\n", int64);
        }
        if (strncmp(pElem->Buffer, "Log - slowest write since restart", 33) == 0) {
          fprintf(stderr, "log_slowest_write=%" Int64 "d\n", int64);
        }
        //NHAREPLICA (REPLICATION & RECOVERY)
        if (strncmp(pElem->Buffer, "Log write average acknowledgement latency", 41) == 0) {
          fprintf(stderr, "%.*s_nha_log_ack_lat=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Synchronous log data average compression time", 45) == 0) {
          fprintf(stderr, "%.*s_nha_sync_compress_time=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Catch-up log data average compression time", 42) == 0) {
          fprintf(stderr, "%.*s_nha_catchup_compress_time=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Average network round trip time", 31) == 0) {
          fprintf(stderr, "%.*s_nha_net_round_trip=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Synchronous log data average decompression time", 47) == 0) {
          fprintf(stderr, "%.*s_nha_sync_decompress_time=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Catch-up log data average decompression time", 44) == 0) {
          fprintf(stderr,"%.*s_nha_catchup_decompress_time=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Log data average compression time", 33) == 0) {
          fprintf(stderr, "%.*s_nha_log_compress_time=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Log data average decompression time", 35) == 0) {
          fprintf(stderr,"%.*s_nha_log_decompress_time=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
      }
      break;

    case MQIAMO_MONITOR_DELTA:    //type: 2
      /* We've been given a delta value, the duration over which the delta */
      /* occurred is 'interval' micro-seconds.                             */
      if (cfin->Type == MQCFT_INTEGER)
        rate = cfin->Value;
      else
      {
        memcpy(&int64, &cfin64->Value, sizeof(int64));
        rate = int64;
      }
      if (interval < 10000)
      {
        /* Very short intervals can result in misleading rates, we therefore */
        /* simply ignore rates for intervals of less than 10 milliseconds.   */
        buffer[0] = 0;
      }
      else
      {
        rate = rate * 1000000;
        rate = rate + (interval/2);
        rate = rate / interval;
        if (rate == 0)
          buffer[0] = 0;
        else
          sprintf( buffer, "%" Int64 "d/sec", rate );
      }
      //DISK LOG
      if (strncmp(pElem->Buffer, "Log - physical", 14) == 0) {
        fprintf(stderr, "log_phy=%" Int64 "d\n", rate);
      }
      if (strncmp(pElem->Buffer, "Log - logical", 13) == 0) {
        fprintf(stderr, "log_log=%" Int64 "d\n", rate);
      }

      //STATMQI PUT
      if (strncmp(pElem->Buffer, "Non-persistent message MQPUT count", 34) == 0) {
          fprintf(stderr, "mqi_np_put_rate=%" Int64 "d\n", rate);
      }
      if (strncmp(pElem->Buffer, "Persistent message MQPUT count", 30) == 0) {
          fprintf(stderr, "mqi_p_put_rate=%" Int64 "d\n", rate);
      }
      if (strncmp(pElem->Buffer, "Failed MQPUT count", 18) == 0) {
          if( cfin->Type == MQCFT_INTEGER )
              fprintf(stderr, "mqi_failed_puts=%d\n", cfin->Value);
          else
              fprintf(stderr, "mqi_failed_puts=%" Int64 "d\n", int64);
      }
      if (strncmp(pElem->Buffer, "Non-persistent message MQPUT1 count", 35) == 0) {
          fprintf(stderr, "mqi_np_put1_rate=%" Int64 "d\n", rate);
      }
      if (strncmp(pElem->Buffer, "Persistent message MQPUT1 count", 31) == 0) {
          fprintf(stderr, "mqi_p_put1_rate=%" Int64 "d\n", rate);
      }
      if (strncmp(pElem->Buffer, "Failed MQPUT1 count", 19) == 0) {
          if( cfin->Type == MQCFT_INTEGER )
              fprintf(stderr, "mqi_failed_put1s=%d\n", cfin->Value);
          else
              fprintf(stderr, "mqi_failed_put1s=%" Int64 "d\n", int64);
      }
      //STATMQI GET
      if (strncmp(pElem->Buffer, "Non-persistent message destructive get - count", 46) == 0) {
          fprintf(stderr, "mqi_np_get_rate=%" Int64 "d\n", rate);
      }
      if (strncmp(pElem->Buffer, "Persistent message destructive get - count", 42) == 0) {
          fprintf(stderr, "mqi_p_get_rate=%" Int64 "d\n", rate);
      }
      if (strncmp(pElem->Buffer, "Failed MQGET - count", 20) == 0) {
          if( cfin->Type == MQCFT_INTEGER )
              fprintf(stderr, "mqi_failed_gets=%d\n", cfin->Value);
          else
              fprintf(stderr, "mqi_failed_gets=%" Int64 "d\n", int64);
      }
      //NHAREPLICA (REPLICATION & RECOVERY)
      if (strncmp(pElem->Buffer, "Synchronous log bytes sent", 26) == 0) {
          if( cfin->Type == MQCFT_INTEGER )
              fprintf(stderr, "%.*s_nha_sync_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
          else
              fprintf(stderr, "%.*s_nha_sync_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
      }
      if (strncmp(pElem->Buffer, "Synchronous compressed log bytes sent", 37) == 0) {
          if( cfin->Type == MQCFT_INTEGER )
              fprintf(stderr, "%.*s_nha_sync_compressed_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
          else
              fprintf(stderr, "%.*s_nha_sync_compressed_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
      } 
      if (strncmp(pElem->Buffer, "Synchronous uncompressed log bytes sent", 39) == 0) {
          if( cfin->Type == MQCFT_INTEGER )
              fprintf(stderr, "%.*s_nha_sync_uncompressed_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
          else
              fprintf(stderr, "%.*s_nha_sync_uncompressed_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
      }
      if (strncmp(pElem->Buffer, "Catch-up log bytes sent", 23) == 0) {
          if( cfin->Type == MQCFT_INTEGER )
              fprintf(stderr, "%.*s_nha_catchup_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
          else
              fprintf(stderr, "%.*s_nha_catchup_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
      }
      if (strncmp(pElem->Buffer, "Catch-up compressed log bytes sent", 34) == 0) {
          if( cfin->Type == MQCFT_INTEGER )
              fprintf(stderr, "%.*s_nha_catchup_compressed_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
          else
              fprintf(stderr, "%.*s_nha_catchup_compressed_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
      }
      if (strncmp(pElem->Buffer, "Catch-up uncompressed log bytes sent", 36) == 0) {
          if( cfin->Type == MQCFT_INTEGER )
              fprintf(stderr, "%.*s_nha_catchup_uncompressed_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
          else
              fprintf(stderr, "%.*s_nha_catchup_uncompressed_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
      }
      if (strncmp(pElem->Buffer, "Log bytes sent", 14) == 0) {
          if( cfin->Type == MQCFT_INTEGER )
              fprintf(stderr, "%.*s_nha_log_bytess=%d\n", ObjNameLen, pObjName, cfin->Value);
          else
              fprintf(stderr, "%.*s_nha_log_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
      }

      if (cfin->Type == MQCFT_INTEGER)
        printf("%.*s %d %s\n", pElem->descLen, pElem->Buffer, cfin->Value, buffer);
      else
      {
        memcpy(&int64, &cfin64->Value, sizeof(int64));
        printf("%.*s %" Int64 "d %s\n", pElem->descLen, pElem->Buffer, int64, buffer);
      }
      break;

#ifdef MQIAMO_MONITOR_LSN
    case MQIAMO_MONITOR_LSN:  //type: 3
      if (cfin->Type == MQCFT_INTEGER)
        printf("%.*s %d\n", pElem->descLen, pElem->Buffer, cfin->Value);
      else
      {
        sruLSN lsn;
        memcpy(&int64, &cfin64->Value, sizeof(int64));
        sruINT64toLSN(int64, &lsn);
        printf("%.*s <%hu:%hu:%hu:%hu> 0x%16.16"Int64"x\n", pElem->descLen, pElem->Buffer,
               sruWORD(lsn.word[0]), sruWORD(lsn.word[1]), sruWORD(lsn.word[2]), sruWORD(lsn.word[3]), int64);
      }

      //NHAREPLICA (REPLICATION & RECOVERY)
      if (strncmp(pElem->Buffer, "Acknowledged log sequence number", 32) == 0) {
        if (cfin->Type == MQCFT_INTEGER)
          fprintf(stderr, "%.*s_nha_ack_log_seq_num=%d\n", ObjNameLen, pObjName, cfin->Value);
        else
        {
          sruLSN lsn;
          memcpy(&int64, &cfin64->Value, sizeof(int64));
          sruINT64toLSN(int64, &lsn);
          fprintf(stderr, "%.*s_nha_ack_log_seq_num=<%hu:%hu:%hu:%hu> 0x%16.16"Int64"x\n", ObjNameLen, pObjName,
                  sruWORD(lsn.word[0]), sruWORD(lsn.word[1]), sruWORD(lsn.word[2]), sruWORD(lsn.word[3]), int64);
        }
      }
      if (strncmp(pElem->Buffer, "Recovery log sequence number", 28) == 0) {
        if (cfin->Type == MQCFT_INTEGER)
          fprintf(stderr, "%.*s_nha_recovery_log_sequence_num=%d\n", ObjNameLen, pObjName, cfin->Value);
        else
        {
          sruLSN lsn;
          memcpy(&int64, &cfin64->Value, sizeof(int64));
          sruINT64toLSN(int64, &lsn);
          fprintf(stderr, "%.*s_nha_recovery_log_sequence_num=<%hu:%hu:%hu:%hu> 0x%16.16"Int64"x\n", ObjNameLen, pObjName,
                  sruWORD(lsn.word[0]), sruWORD(lsn.word[1]), sruWORD(lsn.word[2]), sruWORD(lsn.word[3]), int64);
        }
      }
#endif

      break;

    default:    //type: 1 - MQIAMO_MONITOR_UNIT
      if (cfin->Type == MQCFT_INTEGER) {
        printf("%.*s %d\n", pElem->descLen, pElem->Buffer, cfin->Value);
        if (strncmp(pElem->Buffer, "Log - write size", 16) == 0) {
          fprintf(stderr, "log_siz=%d\n", cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Queue depth", 11) == 0) {
          fprintf(stderr, "q_depth=%d\n", cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Log write average acknowledgement size", 38) == 0) {
          fprintf(stderr, "%.*s_nha_log_ack_size=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Backlog bytes", 13) == 0) {
          fprintf(stderr, "%.*s_nha_backlog_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Backlog average bytes", 11) == 0) {
          fprintf(stderr, "%.*s_nha_backlog_avg_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Synchronous log bytes decompressed", 34) == 0) {
          fprintf(stderr, "%.*s_nha_sync_decompressed_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Catch-up log bytes decompressed", 31) == 0) {
          fprintf(stderr, "%.*s_nha_catchup_decompressed_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Log file system - bytes in use", 30) == 0) {
          if (pObjName) {
            fprintf(stderr, "%.*s_nha_log_filesystem_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
          } else {
            fprintf(stderr, "log_filesystem_bytes=%d\n", cfin->Value);
          }
        }
        if (strncmp(pElem->Buffer, "MQ FDC file count", 17) == 0) {
          if (pObjName) {
            fprintf(stderr, "%.*s_nha_fdc_file_count=%d\n", ObjNameLen, pObjName, cfin->Value);
          } else {
            fprintf(stderr, "fdc_file_count=%d\n", cfin->Value);
          }
        }
        if (strncmp(pElem->Buffer, "Compressed log bytes sent", 25) == 0) {
          fprintf(stderr, "%.*s_nha_log_compressed_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Log bytes decompressed", 22) == 0) {
          fprintf(stderr, "%.*s_nha_log_decompressed_bytes=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Rebase count", 12) == 0) {
          fprintf(stderr, "%.*s_nha_rebase_count=%d\n", ObjNameLen, pObjName, cfin->Value);
        }
        if (strncmp(pElem->Buffer, "Log - timestamp of slowest write", 32) == 0) {
          if (pObjName) {
            fprintf(stderr, "%.*s_log_timestamp_slowest_write=%d\n", ObjNameLen, pObjName, cfin->Value);
          } else {
            fprintf(stderr, "log_timestamp_slowest_write=%d\n", cfin->Value);
          }
        }
      } else {
        memcpy(&int64, &cfin64->Value, sizeof(int64)); 
        printf("%.*s %" Int64 "d\n", pElem->descLen, pElem->Buffer,	int64);
        if (strncmp(pElem->Buffer, "Log - write size", 16) == 0) {
          fprintf(stderr, "log_siz=%" Int64 "d\n", int64);
        }
        if (strncmp(pElem->Buffer, "Queue depth", 11) == 0) {
          fprintf(stderr, "q_depth=%" Int64 "d\n", int64);
        }
        if (strncmp(pElem->Buffer, "Log write average acknowledgement size", 38) == 0) {
          fprintf(stderr, "%.*s_nha_log_ack_siz=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Backlog bytes", 13) == 0) {
          fprintf(stderr, "%.*s_nha_backlog_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Backlog average bytes", 11) == 0) {
          fprintf(stderr, "%.*s_nha_backlog_avg_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Synchronous log bytes decompressed", 34) == 0) {
          fprintf(stderr, "%.*s_nha_sync_decompressed_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Catch-up log bytes decompressed", 31) == 0) {
          fprintf(stderr, "%.*s_nha_catchup_decompressed_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Log file system - bytes in use", 30) == 0) {
          if (pObjName) {
            fprintf(stderr, "%.*s_nha_log_filesystem_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
          } else {
            fprintf(stderr, "log_filesystem_bytes=%" Int64 "d\n", int64);
          }
        }
        if (strncmp(pElem->Buffer, "MQ FDC file count", 17) == 0) {
          if (pObjName) {
            fprintf(stderr, "%.*s_nha_fdc_file_count=%" Int64 "d\n", ObjNameLen, pObjName, int64);
          } else {
            fprintf(stderr, "fdc_file_count=%" Int64 "d\n", int64);
          }
        }
        if (strncmp(pElem->Buffer, "Compressed log bytes sent", 25) == 0) {
          fprintf(stderr, "%.*s_nha_log_compressed_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Log bytes decompressed", 22) == 0) {
          fprintf(stderr, "%.*s_nha_log_decompressed_bytes=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Rebase count", 12) == 0) {
          fprintf(stderr, "%.*s_nha_rebase_count=%" Int64 "d\n", ObjNameLen, pObjName, int64);
        }
        if (strncmp(pElem->Buffer, "Log - timestamp of slowest write", 32) == 0) {
          if (pObjName) {
            fprintf(stderr, "%.*s_log_timestamp_slowest_write=%" Int64 "d\n", ObjNameLen, pObjName, int64);
          } else {
            fprintf(stderr, "log_timestamp_slowest_write=%" Int64 "d\n", int64);
          }
        }
      }
      break;
	}
	return;
}

/*********************************************************************/
/* sruRegisterClass:                                                 */
/* remember what classes are supported.                              */
/*                                                                   */
/*********************************************************************/
int sruRegisterClass( MQLONG   id
                    , char   * class
                    , MQLONG   class_len
                    , char   * desc
                    , MQLONG   desc_len
                    , char   * topic
                    , MQLONG   topic_len
                    , MQLONG   topic_ccsid
                    , MQLONG   flags )
{
  sruCLASS *  pClass;
  sruCLASS ** ppClass;
  int rc = ERROR_RETURN;


  if (sruDebug > 0)
		fprintf(stdout, "Register Class: %4.4u %10.*s %.*s\n", id, class_len, class, desc_len, desc);

  pClass = (sruCLASS *)malloc(sizeof(sruCLASS) + class_len + desc_len + topic_len + 1);

  if (NULL == pClass)
  {
    printf("malloc failed");
  }
  else
  {
    pClass->id = id;
    pClass->type = NULL;
    pClass->classLen = class_len;
    pClass->next = NULL;
    pClass->descLen = desc_len;
    pClass->topicLen = topic_len;
    pClass->topicCCSID = topic_ccsid;
    pClass->flags = flags;
    memcpy(pClass->Buffer, class, class_len);
    memcpy(pClass->Buffer + class_len, desc, desc_len);
    memcpy(pClass->Buffer + class_len + desc_len, topic, topic_len);
    pClass->Buffer[class_len + desc_len + topic_len] = 0;

    for (ppClass = &ClassChain; ; ppClass = &(*ppClass)->next)
    {
      if (NULL == *ppClass)
      {
        *ppClass = pClass;
        break;
      }
    }
    rc = 0;
  }


  return rc;
}

/*********************************************************************/
/* sruRegisterType:                                                  */
/* remember what types are supported.                                */
/*                                                                   */
/*********************************************************************/
int sruRegisterType( MQLONG   class_id
                   , MQLONG   type_id
                   , char   * type
                   , MQLONG   type_len
                   , char   * desc
                   , MQLONG   desc_len
                   , char   * topic
                   , MQLONG   topic_len
                   , MQLONG   topic_ccsid
                   , MQLONG   flags )
{
  sruCLASS  * pClass;
  sruTYPE   * pType;
  sruTYPE  ** ppType;
  int         rc = ERROR_RETURN;


  if (sruDebug > 0)
 		fprintf(stdout, "Register Type : %4.4u %4.4u %10.*s %.*s\n", class_id, type_id, type_len, type, desc_len, desc);

    if (sruDebug > 0) {
        fprintf(stdout, "TopicLen: %u\n", topic_len);
        fprintf(stdout, "Topic: %s\n", topic);
    }
  pClass = sruIdToClass( class_id );
  if (pClass)
  {
    pType = (sruTYPE *)malloc(sizeof(sruTYPE) + type_len + desc_len + topic_len + 1);

    if (NULL == pType)
    {
      printf("malloc failed");
    }
    else
    {
      pType->id = type_id;
      pType->typeLen = type_len;
      pType->descLen = desc_len;
      pType->topicLen = topic_len;
      pType->topicCCSID = topic_ccsid;
      pType->flags = flags;
      pType->elem = NULL;
      memcpy(pType->Buffer, type, type_len);
      memcpy(pType->Buffer + type_len, desc, desc_len);
      memcpy(pType->Buffer + type_len + desc_len, topic, topic_len);
      pType->Buffer[type_len + desc_len + topic_len] = 0;
      pType->next = NULL;

      for (ppType = &pClass->type; ; ppType = &(*ppType)->next)
      {
        if (NULL == *ppType)
        {
          *ppType = pType;
          break;
        }
      }

      rc = 0;
    }
  }
  else
    printf("unknown class: %d", class_id);


  return rc;
}

/*********************************************************************/
/* sruUpdateType:                                                    */
/* Store the topic string for resource usage publications, rather    */
/* than that for metadata (discovery).                               */
/*********************************************************************/
int sruUpdateType( MQLONG   class_id
                 , MQLONG   type_id
                 , char   * topic
                 , MQLONG   topic_len
                 , MQLONG   topic_ccsid )
{
  sruCLASS * pClass;
  sruTYPE  * pType;
  int        rc = ERROR_RETURN;


  if (sruDebug > 0)
    fprintf(stdout, "Update Type   :%4.4d %4.4d %.*s\n", class_id, type_id, topic_len, topic);

    if (sruDebug > 0) {
        fprintf(stdout, "TopicLen: %u\n", topic_len);
        fprintf(stdout, "Topic: %s\n", topic);
    }
  pClass = sruIdToClass(class_id);
  if (pClass)
  {
    pType = sruIdToType(pClass, type_id);
    if (NULL == pType)
    {
      printf("topic: %d.%d (%.*s) unexpectedly not found", class_id, type_id, topic_len, topic);
    }
    /* The 'real' string should always be shorter than the metadata string */
    else if (pType->topicLen < topic_len)
    {
      printf("topic metadata: %d.%d (%.*s) unexpectedly short", class_id, type_id, topic_len, topic);
    }
    else
    {
      memcpy(pType->Buffer + pType->typeLen + pType->descLen, topic, topic_len);
      pType->topicLen = topic_len;
      pType->topicCCSID = topic_ccsid;
      rc = 0;
    }
  }
  else
    printf("unknown class: %d", class_id);


  return rc;
}

/*********************************************************************/
/* sruRegisterElement:                                               */
/* remember what elements are supported.                             */
/*                                                                   */
/*********************************************************************/
int sruRegisterElement( MQLONG   class_id
                      , MQLONG   type_id
                      , MQLONG   elem_id
                      , MQLONG   elemType
                      , char   * desc
                      , MQLONG   desc_len )
{
  sruCLASS  * pClass;
  sruTYPE   * pType;
  sruELEM   * pElem;
  sruELEM  ** ppElem;
  int         rc = ERROR_RETURN;


  if (sruDebug > 0)
    fprintf(stdout, "Register Elem :%4.4d %4.4d %4.4d %4.4d %.*s\n", class_id, type_id, elem_id, elemType, desc_len, desc);

  pClass = sruIdToClass(class_id);
  if (pClass)
  {
    pType = sruIdToType(pClass, type_id);
    if (pType)
    {
      pElem = (sruELEM *)malloc(sizeof(sruELEM) + desc_len + 1);

      if (NULL == pElem)
      {
        printf("malloc failed");
      }
      else
      {
        pElem->id = elem_id;
        pElem->type = elemType;
        pElem->descLen = desc_len;
        memcpy(pElem->Buffer, desc, desc_len);
        pElem->Buffer[desc_len] = 0;
        pElem->next = NULL;

        for (ppElem = &pType->elem; ; ppElem = &(*ppElem)->next)
        {
          if (NULL == *ppElem)
          {
            *ppElem = pElem;
            break;
          }
        }
        rc = 0;
      }
    }
  }
  else
    printf("unknown class: %d", class_id);


  return rc;
}

/*********************************************************************/
/* sruIdToClass:                                                     */
/* translate a class identifier to an sruCLASS pointer.              */
/*                                                                   */
/*********************************************************************/
sruCLASS * sruIdToClass(MQLONG class)
{
  sruCLASS * pClass;


  for (pClass = ClassChain; pClass; pClass = pClass->next)
  {
    if (pClass->id == class)
      break;
  }


  return pClass;
}

/*********************************************************************/
/* sruIdToType:                                                      */
/* translate a type identifier to an sruTYPE pointer.                */
/*                                                                   */
/*********************************************************************/
sruTYPE * sruIdToType(sruCLASS *pClass, MQLONG type)
{
  sruTYPE * pType;


  for (pType = pClass->type; pType; pType=pType->next)
  {
    if (pType->id == type)
      break;
  }


  return pType;
}

/*********************************************************************/
/* sruIdToElem:                                                      */
/* translate an element identifier to an sruELEM pointer.            */
/*                                                                   */
/*********************************************************************/
sruELEM * sruIdToElem(sruTYPE * pType, MQLONG elem)
{
  sruELEM * pElem;


  for (pElem = pType->elem; pElem; pElem=pElem->next)
  {
    if (pElem->id == elem)
      break;
  }


  return pElem;
}

/*********************************************************************/
/* sruQueryQMName:                                                   */
/* query the currently connected queue manager name.                 */
/*                                                                   */
/*********************************************************************/
int sruQueryQMName(void)
{
  int    rc = 0;
  int    index;
  MQOD   od = {MQOD_DEFAULT};    /* Object Descriptor             */
  MQLONG CompCode, Reason;
  MQLONG Selector;
  MQHOBJ LocalHobj;


  /******************************************************************/
  /*                                                                */
  /*   Open the queue manager object to find out its name           */
  /*                                                                */
  /******************************************************************/
  od.ObjectType = MQOT_Q_MGR;       /* open the queue manager object*/
  MQOPEN(Hcon,                      /* connection handle            */
         &od,                       /* object descriptor for queue  */
         (MQOO_INQUIRE |            /* open it for inquire          */
           MQOO_FAIL_IF_QUIESCING), /* but not if MQM stopping      */
         &LocalHobj,                /* object handle                */
         &CompCode,                 /* MQOPEN completion code       */
         &Reason);                  /* reason code                  */

  /* report error, if any */
  if (CompCode)
  {
    printf("MQOPEN ended with reason code %d\n", Reason);
    printf("Unable to open queue manager for inquire\n");
    rc = ERROR_RETURN;
  }
  else
  {
    /****************************************************************/
    /*                                                              */
    /*   Inquire the name of the queue manager                      */
    /*                                                              */
    /****************************************************************/
    Selector = MQCA_Q_MGR_NAME;
    MQINQ(Hcon,                     /* connection handle            */
          LocalHobj,                /* object handle for q manager  */
          1,                        /* inquire only one selector    */
          &Selector,                /* the selector to inquire      */
          0,                        /* no integer attributes needed */
          NULL,                     /* so no buffer supplied        */
          MQ_Q_MGR_NAME_LENGTH,     /* inquiring a q manager name   */
          QMName,                   /* the buffer for the name      */
          &CompCode,                /* MQINQ completion code        */
          &Reason);                 /* reason code                  */
    if (CompCode)
    {
      /* report error, if any */
      printf("MQINQ ended with reason code %d\n", Reason);
      rc = ERROR_RETURN;
    }
    else
    {
      /* Remove and blank padding from the name */
      for (index = 0; index < MQ_Q_MGR_NAME_LENGTH; index++)
      {
        if (0 == QMName[index])
          break;
        else if (' ' == QMName[index])
        {
          QMName[index] = 0;
          break;
        }
      }
    }

    /******************************************************************/
    /*                                                                */
    /*   Close the queue manager object (if it was opened)            */
    /*                                                                */
    /******************************************************************/
    MQCLOSE(Hcon,                    /* connection handle           */
            &LocalHobj,              /* object handle               */
            MQCO_NONE,               /* no close options            */
            &CompCode,               /* completion code             */
            &Reason);                /* reason code                 */

    /* report error, if any     */
    if (CompCode)
    {
      printf("MQCLOSE ended with reason code %d\n", Reason);
      rc = ERROR_RETURN;
    }
  }


  return rc;
}

