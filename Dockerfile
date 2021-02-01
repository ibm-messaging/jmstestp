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

FROM ibmjava:8-jre

LABEL maintainer "Sam Massey <smassey@uk.ibm.com>"

COPY *.deb /
COPY mqlicense.sh /
COPY lap /lap

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
    procps \
    sed \
    tar \
    util-linux \
    iputils-ping \
    sysstat \
    procps \
    apt-utils \
    dstat \
    vim \
    iproute2 \
  # Apply any bug fixes not included in base Ubuntu or MQ image.
  # Don't upgrade everything based on Docker best practices https://docs.docker.com/engine/userguide/eng-image/dockerfile_best-practices/#run
  && apt-get upgrade -y libkrb5-26-heimdal \
  && apt-get upgrade -y libexpat1 \
  # End of bug fixes
  && rm -rf /var/lib/apt/lists/* \
  # Optional: Update the command prompt with the MQ version
  && echo "jms" > /etc/debian_chroot \
  && sed -i 's/password\t\[success=1 default=ignore\]\tpam_unix\.so obscure sha512/password\t[success=1 default=ignore]\tpam_unix.so obscure sha512 minlen=8/' /etc/pam.d/common-password \
  && groupadd --system --gid 999 mqm \
  && useradd --system --uid 999 --gid mqm mqperf \
  && echo mqperf:orland02 | chpasswd \
  && mkdir -p /home/mqperf/jms \
  && chown -R mqperf /home/mqperf/jms \
  && echo "cd ~/jms" >> /home/mqperf/.bashrc

RUN export DEBIAN_FRONTEND=noninteractive \
  && ./mqlicense.sh -accept \
  && dpkg -i ibmmq-runtime_9.1.0.0_amd64.deb \
  && dpkg -i ibmmq-client_9.1.0.0_amd64.deb \
  && dpkg -i ibmmq-java_9.1.0.0_amd64.deb \
  && dpkg -i ibmmq-gskit_9.1.0.0_amd64.deb


WORKDIR /home/mqperf/jms
COPY ssl/* /opt/mqm/ssl/
COPY ssljks/* /tmp/
COPY *.jar /home/mqperf/jms/
COPY *.sh /home/mqperf/jms/
COPY *.mqsc /home/mqperf/jms/
COPY qmmonitor2 /home/mqperf/jms/
RUN export DEBIAN_FRONTEND=noninteractive \
  && chown -R mqperf:mqm /opt/mqm/ssl \
  && chown -R mqperf:mqm /tmp
USER mqperf

ENV MQ_QMGR_NAME=PERF0
ENV MQ_QMGR_PORT=1420
ENV MQ_QMGR_CHANNEL=SYSTEM.DEF.SVRCONN
ENV MQ_QMGR_QREQUEST_PREFIX=REQUEST
ENV MQ_QMGR_QREPLY_PREFIX=REPLY
ENV MQ_NON_PERSISTENT=
ENV MQ_JMS_EXTRA=
ENV MQ_USERID=

ENTRYPOINT ["./jmsTest.sh"]
