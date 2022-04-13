# base image:
FROM ubuntu:bionic

#install eCAL from PPA:
RUN apt-get update && \
	apt-get install -y software-properties-common && \
	rm -rf /var/lib/apt/lists/*
RUN add-apt-repository ppa:ecal/ecal-latest
RUN apt-get install -y ecal
RUN apt install -y cmake g++ libprotobuf-dev protobuf-compiler