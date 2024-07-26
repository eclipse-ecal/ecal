# Base image:
FROM ubuntu:focal

# Install eCAL from PPA:
RUN apt-get update && \
	apt-get install -y software-properties-common && \
	rm -rf /var/lib/apt/lists/*
RUN add-apt-repository ppa:ecal/ecal-latest
RUN apt-get install -y ecal

# Install dependencies for compiling the hello world examples.
# You can omit this, if you don't want to build applications in the container.
RUN apt-get install -y cmake g++ libprotobuf-dev protobuf-compiler

# Set network_enabled = true in ecal.yaml.
# You can omit this, if you only need local communication.
RUN awk -F"=" '/^network_enabled/{$2="= true"}1' /etc/ecal/ecal.yaml > /etc/ecal/ecal.tmp && \
	rm /etc/ecal/ecal.yaml && \
	mv /etc/ecal/ecal.tmp /etc/ecal/ecal.yaml 

# Print the eCAL config
RUN ecal_config