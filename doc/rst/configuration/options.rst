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
=================

Registration settings are listed in the section ``registration``
----------------------------------------------------------------

.. option:: registration

  .. option:: registration_refresh 

    ``< registration_timeout/2``, default ``1000``

    topic registration refresh cycle (has to be smaller than registration timeout!)

  .. option:: registration_timeout
    
    ``1000 + (x * 1000)``, default ``60000``

    timeout for topic registration in ms

  .. option:: loopback

      ``true`` / ``false``, default ``true``
    
      enable to receive registration information on the same local machine

  .. option:: host_group_name

      ``<hostname>``, default ``""``

      host group name that enables interprocess mechanisms across (virtual) host borders (e.g. Docker);
      by default equivalent to local host name

  .. option:: network_enabled

    ``true`` / ``false``, default: ``true``
    
    ``true``  = all eCAL components communicate over network boundaries
    
    ``false`` = local host only communication

  .. option:: layer

    .. option:: shm
    
      .. option:: enabled

        ``true`` / ``false``, default: ``false``

        enable shared memory layer

      .. option:: domain

        ``ecal_mon``

        Domain name for shared momory based registration

      .. option:: queue_size

        ``1024``

        Queue size of registration events

    .. option:: udp
      
      .. option:: enabled

        ``true`` / ``false``, default: ``true``

        enable UDP multicast layer

      .. option:: port
        
        ``14000 + x``

        UDP port for registration information if UDP is enabled


Monitoring settings are listed in the section ``monitoring``
----------------------------------------------------------

.. option:: monitoring

  .. option:: timeout          
            
    ``1000 + (x * 1000)`` default ``1000``
    Timeout for topic monitoring in ms
    If no additional registration information for the topic has be received in that time period, topics will no longer be shown in eCAL Monitor.
    

  .. option:: filter_excl

    topics blacklist as regular expression (will not be monitored)
    Per default includes all eCAL internal topics.
    
    default = ``^__.*$`` 
    
  .. option:: filter_incl

    topics whitelist as regular expression (will be monitored only)
    
    default = `` ``


Transportlayer settings are listed in the section ``transport_layer``
---------------------------------------------------------------------

.. option:: transport_layer

  .. option:: udp 

    .. option:: config_version

    ``v1`` / ``v2``, default: ``v2`` 

    UDP configuration version (Since eCAL 5.12.)
    
    ``v1`` = default behavior

    ``v2`` = new behavior, comes with a bit more intuitive handling regarding masking of the groups
    
    .. option:: mode

      ``local`` / ``network``, default: ``local``

      ``local``  = use local network settings - not configurable by the user

      ``network`` = use network settings - configurable by the user
    
    .. option:: port
        
        ``14000 + x``

        UDP multicast port number

    .. option:: mask

      ``v1`` **behavior:** 
      
      ``0.0.0.1``-``0.0.0.255``

      Mask maximum number of dynamic multicast group

      ``v2`` **behavior:** 
      
      ``255.0.0.0``-``255.255.255.255`` 
      
      Mask for the multicast group. Topic traffic may be set on any of the unmasked addresses.
      
      With ``group``: ``239.0.0.1`` and ``mask``: ``255.255.255.0``, topic traffic will be sent on addresses ``239.0.0.0``-``239.0.0.255``.

    .. option:: send_buffer    
    
      ``1024 * x`` default ``5242880``       
      
      UDP send buffer in bytes

    .. option:: receive_buffer
    
      ``1024 * x`` default ``5242880``

      UDP receive buffer in bytes

    .. option:: join_all_interfaces

      ``true`` / ``false`` default ``false``

      Linux specific setting to join all network interfaces independend of their link state.
      Enabling ensures that eCAL processes receive data when they are started before the
      network devices are up and running.
    
    .. option:: npcap_enabled       
    
      ``true`` / ``false`` default ``false``                
    
      Enable to receive UDP traffic with the Npcap based receiver

    .. option:: local

      In local mode the UDP multicast group is set to the local host address ("127.0.0.1") and the multicast TTL is set to 0 by default.
      This is not configurable.

    .. option:: network

      .. option:: group

        ``xxx.xxx.xxx.xxx`` IP address as text, default ``239.0.0.1`` 

        Multicast group base: all registration and logging is sent on this address 

      .. option:: ttl

        ``0 + x`` default ``3``

        TTL (hop limit) is used to determine the amount of routers being traversed towards the destination

    .. option:: tcp

      .. option:: number_executor_reader

        ``1 + x`` default ``1``

        Number of reader threads that shall execute workload

      .. option:: number_executor_writer

        ``1 + x`` default ``1``

        Number of writer threads that shall execute workload

      .. option:: max_reconnections
          
        ``1 + x`` default ``1``

        Reconnection attemps the session will try to reconnect in case of an issue

    .. option:: shm

      .. option:: memfile_min_size_bytes           
    
        ``x * 4096 kB`` default ``4096`` 
        
        Default memory file size for new publisher
              

      .. option:: memfile_reserve_percent           

        ``20 .. x`` default ``50``
        
        Dynamic file size reserve before recreating memory file if topic size changes
 

Publisher settings are listed in the section ``publisher``
----------------------------------------------------------

.. option:: publisher

  .. option:: layer

    .. option:: shm

      .. option:: enable

        ``true`` / ``false`` default ``true``

        Enable shared memory layer

      .. option:: zero_copy_mode

        ``true`` / ``false`` default ``false``

        Enable zero copy mode for shared memory transport mode

      .. option:: acknowledge_timeout_ms

        ``0 + x`` default ``0``

        Force connected subscribers to send acknowledge event after processing the message.
        The publisher send call is blocked on this event with this timeout (0 == no handshake).

      .. option:: memfile_buffer_count

        ``1 + x`` default ``1``

        Maximum number of used buffers (needs to be greater than 0, default = 1)

    .. option:: udp
      
      .. option:: enable

        ``true`` / ``false`` default ``true``

        Enable UDP multicast layer

    .. option:: tcp

      .. option:: enable

        ``true`` / ``false`` default ``false``

        Enable TCP layer

  .. option:: share_topic_type

      ``true`` / ``false`` default ``true``

      Share topic type via registration layer

    

  .. option:: share_topic_description

    ``true`` / ``false`` default ``true``

    Share topic description via registration layer.
    If set to false, reflection is completely disabled. It is not possible then to monitor the content of messages in the eCAL Monitor.

  .. option:: priority_local

    ``["shm", "udp", "tcp"]`` default ``["shm", "udp", "tcp"]``

    A list of transport layers as text that specifies the prioritized layer for local communication.

  .. option:: priority_remote

    ``["udp", "tcp"]`` default ``["udp", "tcp"]``

    A list of transport layers as text that specifies the prioritized layer for remote communication. 


Subscriber settings are listed in the section ``subscriber``
------------------------------------------------------------

.. option:: subscriber

  .. option:: layer

    .. option:: shm
        
      .. option:: enable

        ``true`` / ``false`` default ``true``

        Enable shared memory layer
        
    .. option:: udp
    
      .. option:: enable
      
        ``true`` / ``false`` default ``true``
      
        Enable UDP multicast layer

    .. option:: tcp

      .. option:: enable

        ``true`` / ``false`` default ``false``

        Enable TCP layer

  .. option:: drop_out_of_order_messages

      ``true`` / ``false`` default ``false``
    
      Enable dropping of payload messages that arrive out of order


Time settings are listed in the section ``time``
------------------------------------------------

.. option:: time

  .. option:: rt        
    
    default: ``"ecaltime-localtime"``
    
    Module (dll / so) name time sync interface. The name will be extended with debug suffix (d) and platform extension (.dll|.so)

    Available modules are:
    
    - ecaltime-localtime    local system time without synchronization        
    - ecaltime-linuxptp     For PTP / gPTP synchronization over ethernet on Linux (device configuration in ecaltime.ini)
    - ecaltime-simtime      Simulation time as published by the eCAL Player.

  .. option:: replay

    default ``""``

    Module name of time plugin for replaying


Service settings are listed in the section ``service``
------------------------------------------------------

.. option:: service

  .. option:: protocol_v0
    
    ``true`` / ``false`` default ``true``

    Support service protocol v0, eCAL 5.11 and older

  .. option:: protocol_v1
    
    ``true`` / ``false`` default ``false``

    Support service protocol v1, eCAL 5.12 and newer


Application settings are listed in the section ``application``
--------------------------------------------------------------

.. option::application

  .. option:: sys

    .. option:: filter_excl      

      Text as regex, default ``"^eCALSysClient$|^eCALSysGUI$|^eCALSys$*"``

      Apps blacklist to be excluded when importing tasks from cloud.

  .. option:: terminal

    .. option:: emulator

      External terminal emulator as text, default ``""``

      Linux only command for starting applications with an external terminal emulator. 
      e.g. 
      /usr/bin/x-terminal-emulator -e
      /usr/bin/gnome-terminal -x
      /usr/bin/xterm -e
      
      If empty, the command will be ignored.


Logging settings are listed in the section ``logging``
------------------------------------------------------

.. option:: logging

  .. option:: sinks

    Log levels are: "info", "warning", "error", "fatal", "debug1", "debug2", "debug3", "debug4"

    .. option:: console

      .. option:: enable
          
          ``true`` / ``false`` default ``false``
    
          Enable console logging

      .. option:: level

        List of log levels as text, default ``["info", "warning", "error", "fatal"]``

    .. option:: file

      .. option:: enable
          
          ``true`` / ``false`` default ``false``
    
          Enable file logging

      .. option:: level

        List of log levels as text, default ``[]``

      .. option:: path

        Path as text, default ``ecal.log``

    .. option:: udp

      .. option:: enable
          
          ``true`` / ``false`` default ``false``
    
          Enable udp logging

      .. option:: level

        List of log levels as text, default ``["info", "warning", "error", "fatal"]``

      .. option:: port

        ``14000 + x`` default ``14001``

        UDP port for logging information
