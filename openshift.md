# Running the performance framework inside Red Hat OpenShift Container Platform (OCP)
The following instructions describe how to run the IBM MQ JMS performance harness inside an
OpenShift Container Platform (OCP) cluster. Note that the steps here are based on
the instructions for running a Docker image inside OpenShift as [described here](https://www.openshift.com/blog/getting-any-docker-image-running-in-your-own-openshift-cluster), 
but customized for the specific scenario of the performance harness.


Firstly, perform a Docker build of the performance harness as normal:
```bash
docker build --tag jmstestp .
```

## Authenticate to the OpenShift and Docker CLIs
```bash
# Authenticate to the OpenShift API endpoint using the OC CLI
# For example your hostname might be "mycluster.mycompany.com"
oc login https://api.<hostname>:6443 -u kubeadmin -p <password>

# Switch to the project representing the namespace into which you wish to deploy the
# performance harness container, which could be the same as that in which your target
# queue manager is deployed, for example "mq".
oc project mq

# Authenticate to the OpenShift image registry using the Docker CLI
docker login -u <username> -p $(oc whoami -t) https://default-route-openshift-image-registry.apps.<hostname>
```

**Note:** If your OpenShift registry is using an unencrypted HTTP connection or a self-signed certificate
then you may see an error as follows when attempting to push the image;
```bash
Error response from daemon: Get https://default-route-openshift-image-registry.apps.<hostname>/v2/: x509: certificate signed by unknown authority
```
In the self-signed certificate case the most secure way to resolve this is to
[configure the Docker daemon to trust that certificate](https://docs.docker.com/registry/insecure/#use-self-signed-certificates), however a quick (and insecure) option is to disable the security
checking by [marking the registry as insecure](https://docs.docker.com/registry/insecure/#deploy-a-plain-http-registry) by
setting `insecure-registries` to include `default-route-openshift-image-registry.apps.<hostname>`.


## Push your Docker image to the OpenShift image registry
```bash
# Firstly, tag your image with the hostname (and port if necessary) of the OpenShift image registry.
# As above, your hostname might be "mycluster.mycompany.com" and your namespace might be "mq"
docker tag jmstestp:latest default-route-openshift-image-registry.apps.<hostname>/<namespace>/jmstestp:1.0

# Then use the Docker push command to send the image to the OpenShift image registry.
docker push default-route-openshift-image-registry.apps.<hostname>/<namespace>/jmstestp:1.0
```

## Publish the Docker container as an OpenShift application
```bash
# The following command will create template objects to enable the Docker image
# to be deployed as an OpenShift application, and publish those objects to the cluster.
oc new-app --docker-image jmstestp
```

Now we'll check on our newly published app in the OpenShift console:
- Log in to the OpenShift console (e.g. `https://console-openshift-console.apps.<hostname>`, kubeadmin + password)
- Navigate to Workloads, Pods and select your project namespace (e.g. "mq")
- You will see two pods listed, similar to `jmstestp-N-deploy` and `jmstestp-N-XXXXX`
- The `jmstestp-N-XXXXX` pod will likely be in ImagePullBackOff state
- Click on the name of the pod, then select the Events tab
- You will see an error message like the following, because OpenShift is trying to load the image from the public docker.io repository;
```
Failed to pull image "jmstestp:1.0": rpc error: code = Unknown desc = Error reading manifest 1.0 in docker.io/library/jmstestp: errors: denied: requested access to the resource is denied unauthorized: authentication required
```

To fix the image pull backoff error;
- Navigate to Workloads > Deployment Configs, and select "jmstestp"
- Select the YAML tab and search for "image"
- Update the image attribute value from the default `'jmstestp:1.0'` to `'image-registry.openshift-image-registry.svc:5000/<namespace>/jmstestp:1.0'` (e.g. where namespace="mq") to tell OCP to load the image from its own local registry
- Click Save on the YAML
- Navigate back to Pods and you will see you `jmstestp-N-XXXXX` pod go into Running state


## Checking the jmstestp pod log for permission errors
In the OpenShift console, navigate to Workloads > Pod, make sure you have selected the correct project
namespace select the (e.g. "mq"), and click on the name of your running pod, for example `jmstestp-N-XXXXX`,
then select Logs to see the pod log.

Depending on the configuration of your cluster you are likely to see "Permission denied" errors as
illustrated below, which are because the Docker container expects to run as the `mqperf`
user, but OpenShift is configured by default to prevent containers from running as a fixed user.
```
----------------------------------------
Initialising test environment-----------
----------------------------------------
Fri May 22 10:59:08 UTC 2020
./jmsTest.sh: line 88: /home/mqperf/jms/results: Permission denied
...
```

Since this pod is only used for testing purposes the easiest way to resolve this issue is to grant the
OpenShift permission for the pod to run with its preferred user as follows;
```bash
oc project mq
oc adm policy add-scc-to-user anyuid -z default
```

From the Pod Details page (or the list of Pods), select Actions > Delete Pod to force the pod to be
restarted with the new permissions.

Once the pod has restarted click on the name of the pod, then Logs to see that the `Permission denied`
errors no longer appear.

## Configure the environment properties
At this point the pod is running successfully with no errors displayed, and the Logs output will
look something like the following;
```
----------------------------------------
Initialising test environment-----------
----------------------------------------
Mon May 25 14:38:51 UTC 2020
Running Persistent JMS Messaging Tests
----------------------------------------
Testing QM: PERF0 on host: localhost using port: 1420 and channel: SYSTEM.DEF.SVRCONN
...
```

The performance tests are now attempting to run, but will fail because they cannot connect to
the queue manager using the default values specified. 

To set the necessary environment variables, navigate in the OpenShift console to
Workloads > Deployment Configs > jmstestp > Environment, and set the following variables;
- `MQ_QMGR_HOSTNAME`
  - Find the internal service hostname of the queue manager by navigating to Networking > Services
  - Set the hostname to the name of the Kubernetes service for the queue manager (not the "-metrics" one)
- `MQ_QMGR_PORT`
  - Following on from the preceding step, click into the Kubernetes service for the queue manager
  - Set the port number to the service port mapping of the "qmgr" (typically 1414)
- `MQ_QMGR_NAME`
  - If you don't already know the name of your queue manager you can discover it from the OpenShift console
  - Click on Workloads > Stateful Sets, select the StatefulSet for your queue manager
  - Select the YAML tab for the StatefulSet and search for "MQ_QMGR_NAME", then take the value of the property such as "QM1"

Optionally you may also wish to set any other properties as described in the [README](README.md#setting-configuration-options)
such as `MQ_RESPONDER_THREADS` (to restrict the number of threads used), `MQ_QMGR_CHANNEL` to set the channel name,
or `MQ_USERID` and `MQ_PASSWORD` if you need to authenticate the client connections.

Once you are finished click Save on the Deployment Config YAML and the pod will be restarted
automatically to pick up the new values.


## Congratulations!
You have successfully configured the IBM MQ JMS performance test harness to run in a Red Hat
OpenShift cluster!

You can now observe the harness running from the OpenShift console by navigating to the pod and
selecting either the Logs tab or going to Terminal tab and type `tail -f output`.

For further details on reading the output please see the [README.md](README.md).