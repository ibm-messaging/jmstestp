# jmstestp
Environment for creating a docker image running JMS performance tests for Persistent and Non Persistent messaging.

This repository contains a set of files to help create a Docker image containing the JMSPerfHarness jar, IBM's Java 1.8 and a set of scripts to run an inital set of performance tests.


## Pre-requisites
You will need to separately download the MQ Client (for which license agreement is required) and copy the following files into the root directory before building your docker image:
* /lap/
*  mqlicense.sh
*  ibmmq-client_9.1.0.0_amd64.deb
*  ibmmq-runtime_9.1.0.0_amd64.deb
*  ibmmq-java_9.1.0.0_amd64.deb
*  ibmmq-gskit_9.1.0.0_amd64.deb

The MQ V9 client can be obtained from: http://www-01.ibm.com/support/docview.wss?uid=swg24042176

The MQ V9.1 client can be obtained from: http://www-01.ibm.com/support/docview.wss?uid=swg24044791


### Pre-reqs for enabling TLS communication
If you wish to enable TLS communication between the container and your QM you will need to:
* provide a CMS keystore named key.kdb (and its stash file) containing your QM public certificate in the /ssl directory.
* provide a JKS keystore named keystore.jks containing your QM public certificate in the /ssljks directory.


## Configuring the queue manager
The performance tests use a set of queues with specific names which must exist on your target queue manager.

You can quickly configure those queues using the [createq.mqsc](createq.mqsc) runmqsc script.
If you wish to clear the queues manually then you can also do that using the [clearq.mqsc](clearq.mqsc) script.


## Build and run as a standalone Docker container
You can perform a docker build as normal:

`docker build --tag jmstestp .`

then run in network host mode to connect and run tests against a local QM:

`docker run -it --detach --net="host" jmstestp`

The default configuration looks for a QM located on the localhost called PERF0 with a listener configured on port 1420. The clients will send and receive persistent messages. You can override a number of options by setting environment variables on the docker run command.

`docker run -it --detach --net="host" --env MQ_QMGR_NAME=PERF1 --env MQ_QMGR_HOSTNAME=10.0.0.1 --env MQ_QMGR_PORT=1414 jmstestp`

In addition to the hostname, port and and QM name, the default channel can be overidden using the MQ_QMGR_CHANNEL envvar and the queue prefixes used for the testing can be set using MQ_QMGR_QREQUEST_PREFIX and MQ_QMGR_QREPLY_PREFIX.


## Running inside Red Hat OpenShift Container Platform (OCP)
You can also run this performance harness inside Red Hat OpenShift using the [OpenShift instructions](openshift.md).


## Setting configuration options
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
| MQ_RESULTS              | Log results to stdout at end of tests                | TRUE               |
| MQ_RESULTS_CSV          | Log results to csv file and send to stdout at end    | FALSE              |
| MQ_TLS_CIPHER           | TLS CipherSpec to use for remote configuration       |                    |
| MQ_JMS_CIPHER           | TLS CipherSuite for JMS clients to use               |                    |
| MQ_JMS_KEYSTOREPASSWORD | Keystore password to use with JKS keystore           |                    |
| MQ_ERRORS               | Log MQ error log at end of test                      | FALSE              |


## Retrieving the test results
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


## Configuration for TLS scenarios
One complication is that within the container running the automated JMS test, the scripts use remote runmqsc command to clear the queues that the tests will use. This is straightforward when TLS is not involved as we can configure our remote QM connection details and set them in a MQSERVER envvar before invoking runmqsc –c.
 
To run JMS tests with TLS configured, we will need the CCDT configured locally and also a CMS keystore for runmqsc to communicate with the QM securely; thus you will still need to supply a CMS keystore in the /ssl directory alongside the JKS keystore in the /ssljks directory for the tests to run correctly. You can specify the CipherSuite to use with the JMS clients by setting the MQ_JMS_CIPHER envvar. The JKS keystore password can be supplied by setting MQ_JMS_KEYSTOREPASSWORD. The CipherSpec configured in the CCDT for use by runmqsc will use the MQ_TLS_CIPHER. 


## Version information
The version of the JMSPerfHarness jar contained in this image was taken on 15th February 2018 and compiled with Java 1.8. The base docker image is IBMs Java 1.8 which uses Ubuntu 16.04. 

The current level of Java is Java 8 SR5 FP27. This level of Java contains the full strength cryptography suites without additional modification. If you use an older version of Java you may need to configure it to use the strongest ciphers if supported within your geography. See: https://www.ibm.com/support/knowledgecenter/SSAW57_8.5.5/com.ibm.websphere.nd.multiplatform.doc/ae/tsec_egs.html for more details.

Information on IBM's Java for Docker can be found here:
https://hub.docker.com/_/ibmjava/

The most up to date JMSPerfHarness code can be found here:
https://github.com/ot4i/perf-harness

If you have an older version of this repository or PerfHarness, you may find you need the latest update to be able to work with the IBM MQ on Cloud service (MQoC), as this offering requires the MQ client to use CSP authentication when interacting with the QM. See the newly added -jm option.

