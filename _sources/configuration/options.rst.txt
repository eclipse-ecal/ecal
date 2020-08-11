.. include:: /include.txt

.. _configuration_options:

========
ecal.ini
========

eCAL has many options that can be configured via an .ini file which is located at:

* |fa-windows| Windows: |ecalini-path-windows|
* |fa-ubuntu| Ubuntu: |ecalini-path-ubuntu|


[network]
=========

The network setting drive how and which ...

.. option:: network_enabled

   ``true`` / ``false``, default: ``true``
   
   ``true``  = all eCAL components communicate over network boundaries
   
   ``false`` = local host only communication
   
.. option:: multicast_group

   IPV4 Adress, default ``239.0.0.1``

   UDP multicast group base. 

.. option:: multicast_mask

   ``0.0.0.1``-``0.0.0.255``

   Mask maximum number of dynamic multicast group

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

.. option:: inproc_rec_enabled  
   
   ``true``

   Enable to receive on eCAL inner process layer

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
========

.. option:: registration_timeout
  
   ``1000 + (x * 1000)``, default ``60000``

   timeout for topic registration in ms

.. option:: registration_refresh 

   ``< registration_timeout/2``, default ``1000``

   topic registration refresh cylce (has to be smaller then registration timeout !)


[time]
======

.. option:: timesync_module_rt        
   
   default: ``"ecaltime-localtime"``
   
   module (dll / so) name time sync interface. The name will be extended with debug suffix (d) and platform extension (.dll|.so)

   Available modules are:
   
   - ecaltime-localtime    local system time without synchronization        
   - ecaltime-linuxptp     For PTP / gPTP synchronization over ethernet on Linux (device configuration in ecaltime.ini)
   - ecaltime-simtime      Simulation time as published by the eCAL Player.

[iceoryx]
=========

.. option:: service  
   
   ``eCAL``

   Default Iceoryx service name

.. option:: instance       
   
   ``""``                
   
   Default Iceoryx service instance name

[publisher]
===========

.. option:: use_inproc
   
   use inner process transport layer 
   
   - 0 = off
   - 1 = on
   - 2 = auto
   
   default = 0
   
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
============

Monitor settings are listed in the section ``monitoring``

.. option:: timeout          
          
   timeout for topic monitoring in ms (``1000 + (x * 1000)``)
   If no additional registration information for the topic has be received in that time period, topics will no longer be shown in eCAL Monitor.
   
   default = 5000

.. option:: filter_excl

   topics blacklist as regular expression (will not be monitored)
   Per default includes all eCAL internal topics.
   
   default = ``__.*`` 
   
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
