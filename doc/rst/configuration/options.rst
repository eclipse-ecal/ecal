.. include:: /include.txt

.. _configuration_options:

========
ecal.yaml
========

eCAL has many options that can be configured via an .ini file which is located at:

* |fa-windows| Windows: |ecalini-path-windows|
* |fa-ubuntu| Ubuntu: |ecalini-path-ubuntu|

Loading strategy (Priority)
===========================

The eCAL configuration file is loaded based on the following priorities, whereever it is found first.
If you want a specific eCAL Node to run with another ``ecal.yaml`` than the others, you can set the ``ECAL_DATA`` variable before starting the process, e.g. from a batch or shell skript.
In addition, some eCAL applications support providing a path from the command line option ``--ecal-config-file``.

.. important::
   This loading strategy is valid for eCAL 5.10 and up.

- |fa-windows| Windows:

  1. ``%ECAL_DATA%/ecal.yaml``
  2. ``%ProgramData%/ecal/ecal.yaml``

- |fa-ubuntu| Ubuntu:

  1. ``$ECAL_DATA/ecal.yaml``
  2. ``/etc/ecal/ecal.yaml`` (from ``CMAKE_INSTALL_SYSCONFDIR``)
  3. ``/etc/ecal/ecal.yaml`` (fallback)
      
     .. note::

        This second path is set from CMake to ``CMAKE_INSTALL_SYSCONFDIR/ecal/ecal.yaml``.
        Official builds are configured to use ``/etc``.
        If you are compiling eCAL yourself and don't provide the SYSCONFDIR, CMake will usually use ``/usr/local/etc/ecal/ecal.yaml``.

ecal.yaml options
================

[network]
---------

The network setting drive how and which ...

.. option:: network_enabled

   ``true`` / ``false``, default: ``true``
   
   ``true``  = all eCAL components communicate over network boundaries
   
   ``false`` = local host only communication
   
.. option:: multicast_config_version

   ``v1`` / ``v2``, default: ``v1`` 

   UDP configuration version (Since eCAL 5.12.)
   
   ``v1`` = default behavior

   ``v2`` = new behavior, comes with a bit more intuitive handling regarding masking of the groups

.. option:: multicast_group

   IPV4 Adress, default ``239.0.0.1``

   UDP multicast group base. All registration and logging information is sent on this address.

.. option:: multicast_mask

   ``v1`` **behavior:** 
   
   ``0.0.0.1``-``0.0.0.255``

   Mask maximum number of dynamic multicast group

   ``v2`` **behavior:** 
   
   ``255.0.0.0``-``255.255.255.255`` 
   
   Mask for the multicast group. Topic traffic may be set on any of the unmasked addresses.
   
   With ``multicast_group``: ``239.0.0.1`` and ``multicast_mask``: ``255.255.255.0``, topic traffic will be sent on addresses ``239.0.0.0``-``239.0.0.255``.
   
.. option:: multicast_port     
   
   ``14000 + x``

   UDP multicast port number (eCAL will use at least the 2 following port numbers too, so please modify in steps of 10 (e.g. 1010, 1020 ...)

.. option:: multicast_ttl
   
   ``0 + x``              
   
   UDP ttl value, also known as hop limit, is used in determining the intermediate routers being traversed towards the destination

.. option:: multicast_sndbuf    
   
   ``1024 * x``           
   
   UDP send buffer in bytes

.. option:: multicast_rcvbuf
   
   ``1024 * x``

   UDP receive buffer in bytes

.. option:: bandwidth_max_udp   
   
   ``1048576``              
   
   UDP bandwidth limit for eCAL udp layer (-1 == unlimited)

.. option:: shm_rec_enabled     

   ``true``

   Enable to receive on eCAL shared memory layer

.. option:: udp_mc_rec_enabled  
   
   ``true``

   Enable to receive on eCAL udp multicast layer

.. option:: npcap_enabled       
   
   ``false``                
   
   Enable to receive UDP traffic with the Npcap based receiver


[common]
--------

.. option:: registration_timeout
  
   ``1000 + (x * 1000)``, default ``60000``

   timeout for topic registration in ms

.. option:: registration_refresh 

   ``< registration_timeout/2``, default ``1000``

   topic registration refresh cylce (has to be smaller then registration timeout !)


[time]
------

.. option:: timesync_module_rt        
   
   default: ``"ecaltime-localtime"``
   
   module (dll / so) name time sync interface. The name will be extended with debug suffix (d) and platform extension (.dll|.so)

   Available modules are:
   
   - ecaltime-localtime    local system time without synchronization        
   - ecaltime-linuxptp     For PTP / gPTP synchronization over ethernet on Linux (device configuration in ecaltime.ini)
   - ecaltime-simtime      Simulation time as published by the eCAL Player.

[process]
---------

.. _ecalini_option_terminal_emulator:

.. option:: terminal_emulator        
   
   default: ``""``

   command for starting applications with an external terminal emulator.
   If empty, the command will be ignored.
   Ignored on Windows.
   
   e.g:

   - ``/usr/bin/x-terminal-emulator -e``
   - ``/usr/bin/gnome-terminal -x``
   - ``/usr/bin/xterm -e``

[publisher]
-----------
   
.. option:: use_shm
   
   use shared memory transport layer 
   
   - 0 = off
   - 1 = on
   - 2 = auto
   
   default = 2
   
.. option:: use_udp_mc
   
   use udp multicast transport layer 
   
   - 0 = off
   - 1 = on
   - 2 = auto
   
   default = 2
   


.. option:: memfile_minsize           = x * 4096 kB            
   
   default memory file size for new publisher (``x * 4096 kB``)
   
   default = 4096

.. option:: memfile_reserve           

   dynamic file size reserve before recreating memory file if topic size changes (``20 .. x``)

   default = 50   

.. option:: memfile_ack_timeout

   Publisher timeout for ack event from subscriber that memory file content is processed
   
   default = 0

.. option:: share_ttype

   share topic type via registration layer ( ``0, 1``)

   default = 1

.. option:: share_tdesc

   share topic description via registration layer ( ``0, 1``)
   If set to 0, reflection is completely disabled. It is not possible then to monitor the content of messages in the eCAL Monitor.
   
   default = 1

[monitoring]
------------

Monitor settings are listed in the section ``monitoring``

.. option:: timeout          
          
   timeout for topic monitoring in ms (``1000 + (x * 1000)``)
   If no additional registration information for the topic has be received in that time period, topics will no longer be shown in eCAL Monitor.
   
   default = 5000

.. option:: filter_excl

   topics blacklist as regular expression (will not be monitored)
   Per default includes all eCAL internal topics.
   
   default = ``^__.*$`` 
   
.. option:: filter_incl

   topics whitelist as regular expression (will be monitored only)
   
   default = `` ``
   
.. option:: filter_log_con      

   log messages to console (all, info, warning, error, fatal, debug1, debug2, debug3, debug4)
   
   default = ``error, fatal``
   
.. option:: filter_log_file               

   log messages to log file
   
   default = ``error, fatal``
   
.. option:: filter_log_udp      

   log messages to udp bus
   
   default = ``info, warning, error, fatal``

[sys]
-----

.. option:: filter_excl      

   Apps blacklist to be excluded when importing tasks from cloud.
