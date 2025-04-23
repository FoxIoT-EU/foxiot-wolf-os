FROM --platform=linux/amd64 ubuntu:22.04

ENV HOME=/root

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata && \
    apt-get -y --no-install-recommends install \
    libglib2.0-0 \ 
    u-boot-tools \
    mtd-utils \
    lzma \ 
    device-tree-compiler \
    whois \
    build-essential 

ENV PATH "$PATH:$HOME/.local/bin"

COPY docker/make-inside-docker.sh /
RUN chmod +x make-inside-docker.sh

ENTRYPOINT ["/make-inside-docker.sh"]
CMD ["all"]
