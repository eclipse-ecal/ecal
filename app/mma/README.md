
# MMA - Machine Monitoring Agent

Copyright (c) 2018, Continental Corporation.

The Machine Monitoring Application is an application which collects usage information about the hardware it runs on. 
It can show: 
- The CPU usage.
- Available and total RAM memory.
- The logical partitions on the device displaying the name, capacity, available space and I/O information(read/write bytes per second).
- The network interface cards displaying the name, IP address and I/O information (send/receive bytes per second).


## Windows

### Installation

You need Visual Studio 2015 to build the windows version of MachineMonitoringAgent.
  
Load the Visual Studio 2015 solution file and build all configurations. (Build / Batch Build / Select All / Build)

The MachinemonitoringAgent will be installed along the eCAL installer created by the InnoSetup, as an addon.

The MachineMonitoringAgent application on Windows needs the performance counters (from performance monitor) in order to work correctly. In case the performance counters are not working, the user has 2 possibilities:

  I:
      He can run in administrator mode the batch file "perfmon_error_rebuild.bat" and this will repair the performance monitor counters.

   II:
      1. Run the cmd as administrator.
      The path to where cmd opens will be: C:\windows\system32\

      2. Run the command "lodctr /r". Now you should see a message like this: Info: Successfully rebuilt performance counter setting from system backup store
      In case you haven't received this message and have received a message like: "Error: Unable to rebuild performance counter setting from system backup store, error code is 2" you should go one folder up with "cd..", then change the folder to "cd syswow64", and now you can rerun the command "lodctr /r"


## Linux

The Machine Monitoring Application on Linux collects information via:
  - /proc virtual file system
  - command df
  - file /etc/os-release

These should be available on all standard Linux distributions
and even on embedded Linux distributions.

### Installation

  In order to install the MachineMonitoringAgent along with eCAL, the cmake "-DECAL_BUILD_APPS=ON" flag needs to be set ON.

  MachineMonitoringAgent stand-alone installation:

    Go to the MachineMonitoringAgent root folder and run the following commands:
    mkdir build
    cd build
    cmake ..
    make
    sudo make install

    The installation folder on Linux will be "usr/local/bin".

    To make the MMA application run at startup on Linux:
      1. Open the Dash by pressing the windows key.
      2. Open Startup Applications.
      3. Click Add and enter the following:
                  Name: Machine Monitoring Agent
                            Command: /usr/local/bin/mma
                            Comment: Machine monitoring task.