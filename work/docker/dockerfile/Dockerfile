FROM amd64/centos:7
  
MAINTAINER wenxu <wenxu@ucloud.cn>

RUN yum -y update && yum install -y net-tools && yum install -y iproute
RUN yum -y install git autoconf automake libtool make \
  readline-devel net-snmp-devel pkgconfig \
   pam-devel bison flex c-ares-devel \
   systemd-devel  libcap-devel

RUN curl -O https://rpm.frrouting.org/repo/frr-stable-repo-1-0.el7.noarch.rpm
RUN rpm -ivh ./frr-stable-repo-1-0.el7.noarch.rpm
RUN yum -y install frr frr-pythontools vim tcpdump

CMD ["/usr/lib/frr/frrinit.sh","start"]

RUN echo "hahahhha" > tmp_file
