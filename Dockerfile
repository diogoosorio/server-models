FROM ubuntu:latest

RUN apt update && \
    DEBIAN_FRONTEND="noninteractive" \
    apt install -y strace \
        build-essential \
        apt-transport-https \
        wget \
        tzdata \
        psmisc \
        vim \
        iproute2 \
        tmux \
        dnsutils \
        python3

RUN wget https://packages.microsoft.com/config/ubuntu/20.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb && \
    dpkg -i packages-microsoft-prod.deb && \
    apt update && \
    apt install -y dotnet-sdk-3.1

WORKDIR /app
RUN cd /app
