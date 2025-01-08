# © Copyright IBM Corporation 2015, 2017
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# FROM ibmjava:8-jre
FROM icr.io/appcafe/ibmjava:8-jre

LABEL maintainer "Sam Massey <smassey@uk.ibm.com>"

# Copy MQ install files
COPY *.deb /
COPY mqlicense.sh /
COPY lap /lap

# Install required packages
RUN export DEBIAN_FRONTEND=noninteractive \
  # Install additional packages - do we need/want them all
  && apt-get update -y \
  && apt-get install -y --no-install-recommends \
    bash \
    bc \
    ca-certificates \
    coreutils \
    curl \
    debianutils \
    file \
    findutils \
    gawk \
    grep \
    libc-bin \
    lsb-release \
    mount \
    passwd \
    sed \
    tar \
    util-linux \
    iputils-ping \
    sysstat \
    procps \
    apt-utils \
    pcp \
    vim \
    iproute2

# Create mqm and mqperf users before installing MQ
RUN export DEBIAN_FRONTEND=noninteractive \
  && rm -rf /var/lib/apt/lists/* \
  && groupadd --system --gid 30000 mqm \
  && useradd --system --uid 900 --gid mqm mqm \
  && useradd --uid 30000 --gid mqm mqperf \
  && mkdir -p /home/mqperf/jms \
  # Update the command prompt with the container name, login and cwd
  && echo "export PS1='jmstestp:\u@\h:\w\$ '" >> /home/mqperf/.bashrc \
  && echo "cd ~/jms" >> /home/mqperf/.bashrc \
  && service pmcd start

# MQ must be installed as root
# By running script, you accept the MQ client license, run ./mqlicense.sh -view to view license
ENV MQLICENSE=accept
RUN export DEBIAN_FRONTEND=noninteractive \
  && ./mqlicense.sh -accept \
  && dpkg -i ibmmq-runtime_9.4.0.0_amd64.deb \
  && dpkg -i ibmmq-java_9.4.0.0_amd64.deb \
  && dpkg -i ibmmq-gskit_9.4.0.0_amd64.deb \
  && dpkg -i ibmmq-client_9.4.0.0_amd64.deb

# Copy all TLS, config and script files to the image
USER mqperf
COPY ssl/* /opt/mqm/ssl/
COPY ssljks/* /tmp/
COPY *.jar /home/mqperf/jms/
COPY *.sh /home/mqperf/jms/
COPY *.mqsc /home/mqperf/jms/
COPY qmmonitor2 /home/mqperf/jms/

# Update ownership of files
USER root
RUN export DEBIAN_FRONTEND=noninteractive \
  && chown -R mqperf:mqm /opt/mqm/ssl \
  && chown -R mqperf:mqm /home/mqperf/jms

USER mqperf
WORKDIR /home/mqperf/jms
ENV MQ_QMGR_NAME=PERF0
ENV MQ_QMGR_PORT=1420
ENV MQ_QMGR_CHANNEL=SYSTEM.DEF.SVRCONN
ENV MQ_QMGR_QREQUEST_PREFIX=REQUEST
ENV MQ_QMGR_QREPLY_PREFIX=REPLY
ENV MQ_NON_PERSISTENT=
ENV MQ_JMS_EXTRA=
ENV MQ_USERID=

ENTRYPOINT ["./jmsTest.sh"]
