# jmstestp
Environment for creating a docker image running jms performance tests for Persistent and Non Persistent messaging.

This repository contains a set of files to help create a Docker image containing the JMSPerfHarness jar, IBM's Java 1.8 and a set of scripts to run an inital set of performance tests.

You will need to seperately download the MQ Client (for which license agreement is required) and copy the following files into the root directory before building your docker image:
* /lap/
*  mqlicense.sh
*  ibmmq-client_9.0.5.0_amd64.deb
*  ibmmq-runtime_9.0.5.0_amd64.deb

The MQ V9 client can be obtained from:
http://www-01.ibm.com/support/docview.wss?uid=swg24042176

then perform a docker build as normal:

`docker build --tag jmstestp .`

then run in network host mode to connect and run tests against a local QM:

`docker run -it --detach --net="host" jmstestp`

The default configuration looks for a QM located on the localhost called PERF0 with a listener configured on port 1420. The clients will send and receive persistent messages. You can override a number of options by setting environment variables on the docker run command.

`docker run -it --detach --net="host" --env MQ_QMGR_NAME=PERF1 --env MQ_QMGR_HOSTNAME=10.0.0.1 --env MQ_QMGR_PORT=1414 --env MQ_QMGR_CHANNEL=SYSTEM.DEF.SVRCONN --env MQ_QMGR_QREQUEST_PREFIX=REQUEST --env MQ_QMGR_QREPLY_PREFIX=REPLY jmstestp`

In addition to the hostname, port and and QM name, the default channel can be overidden using the MQ_QMGR_CHANNEL envvar and the queue prefixes used for the testing can be set using MQ_QMGR_QREQUEST_PREFIX and MQ_QMGR_QREPLY_PREFIX.

In the latest release further configuration options have been added. The table below provides the full set:

| Envvar                  | Description                                          | Default if not set |
|-------------------------|------------------------------------------------------|--------------------|
| MQ_QMGR_NAME            | Queue Manager Name                                   | PERF0              |
| MQ_QMGR_HOSTNAME        | Hostname where QM is running                         | localhost          |
| MQ_QMGR_PORT            | Port where QM listener is running                    | 1420               |
| MQ_QMGR_CHANNEL         | Channel name to use to connect to QM                 | SYSTEM.DEF.SVRCONN |
| MQ_RESPONDER_THREADS    | Number of responder threads to run                   | 200                |
| MQ_QMGR_QREQUEST_PREFIX | Prefix of request queues to use.                     | REQUEST            |
| MQ_QMGR_QREPLY_PREFIX   | Prefix of reply queues to use.                       | REPLY              |
| MQ_NON_PERSISTENT       | QOS to be used by connecting clients                 | 0 (Persistent)     |
| MQ_USERID               | Userid to use when authenticating                    |                    |
| MQ_PASSWORD             | Password to use when authenticating                  |                    |
| MQ_JMS_EXTRA            | Additional string field to propogate to jms client   |                    |



The container will run a number of tests using different numbers of threads with messages of 2K, 20K and 200K. The scenario is a Request/Responder scenario as featured in the latest xLinux and Appliance performance reports available here:
https://ibm-messaging.github.io/mqperf/

When the testing is complete the final results will be posted to the docker logs and can be viewed in the normal way:

`docker logs <containerID>`

You can also obtain the available results by:

`docker cp <containerID>:/home/mqperf/jms/results .`

The output from the running responder and requester processes can be viewed by:

`docker cp <containerID>:/home/mqperf/jms/output .`

An interactive session with the running container can be access by:

`docker -ti <containerID> /bin/bash`

The version of the JMSPerfHarness jar contained in this image was taken on 15th February 2018 and compiled with Java 1.8. The base docker image is IBMs Java 1.8 which uses Ubuntu 16.04. The most up to date JMSPerfHarness code can be found here:
https://github.com/ot4i/perf-harness

Information on IBM's Java for Docker can be found here:
https://hub.docker.com/_/ibmjava/

If you have an older version of this repository or PerfHarness, you may find you need the latest update to be able to work with the MQoC service, as this offering requires the MQ client to use CSP authentication when interacting with the QM. See the newly added -jm option.

