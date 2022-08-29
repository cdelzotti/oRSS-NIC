FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    iproute2 \
    iputils-ping \
    tshark \
    iperf3 \