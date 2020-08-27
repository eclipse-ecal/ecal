
# eCAL - enhanced Communication Abstraction Layer

Copyright (c) 2020, Continental Corporation.

![Build Windows Server 2019](https://github.com/continental/ecal/workflows/Build%20Windows%20Server%202019/badge.svg)

![Build Ubuntu 20.04](https://github.com/continental/ecal/workflows/Build%20Ubuntu%2020.04/badge.svg)

![Build Ubuntu 20.04 (Iceoryx)](https://github.com/continental/ecal/workflows/Build%20Ubuntu%2020.04%20(Iceoryx)/badge.svg)

![Build macOS Catalina 10.15](https://github.com/continental/ecal/workflows/Build%20macOS%20Catalina%2010.15/badge.svg)

[![License](https://img.shields.io/github/license/continental/ecal.svg?style=flat)](LICENSE.txt)

## Preface

The **e**nhanced **C**ommunication **A**bstraction **L**ayer (eCAL) is a middleware that enables scalable, high performance interprocess communication on a single computer node or between different nodes in a computer network.
eCAL uses a **publish / subscribe** pattern to automatically connect different nodes in the network. It's choosing the best available data transport mechanisms - **UDP** for network communication and **High Performance Shared Memory** for local communication.

With eCAL, you can:

- Independently develop all components of your system; they only have to agree on the data format
- Dynamically add and remove publishers and subscribers, use different versions or replace the publisher with a replay
- Spread your publishers and subscribers across multiple machines that may even run different operating systems
- Record, replay and inspect the traffic with powerful tools

## Links

- **eCAL Documentation**: https://continental.github.io/ecal
- **Getting Started Tutorial**: https://continental.github.io/ecal/getting_started/introduction
- **Downloads & Releases**: https://github.com/continental/ecal/releases
