:orphan:

.. include:: /include.txt

.. _npcap_details:

===============================================
About eCAL, UDP, Windows 10, Winsocks and Npcap
===============================================

When migrating from Windows 7 to Windows 10, we experienced a major performance issue in basically all eCAL applications.
Using them on Windows 10 would cause maximum CPU load on one CPU core.
The eCAL applications would experience message drops to a degree that they wouldn't even show up in the eCAL Montior any more.
The CPU load even caused the mouse curser to not move smoothly any more (it's more like it is udated every few seconds), so it is quite hard to even terminate the applications.

**tl;dr**:

* The Windows Defender in Windows 10 has a new security feature that cannot (easily) be deactivated
* This security feature causes a massive CPU load when receiving UDP Multicast packets
* The issue is bigger when the machine is connected to a corporate Windows domain
* The problem is not caused by eCAL, eCAL however solves the problem by implementing its own UDP Stack with the help of NPCAP

If you are interested in further details, please read the internal analysis of the issue:

===================================================================

*From GitHub, written by Florian Reimold (2019-02-06):*

The Problem
===========

The Problem is not eCAL specific, as it can easily be reproduced just by receiving a lot of UDP multicast packets (Sending them doesn't seem to be an issue).
When investigating the System with Process Hacker, one can see that the System process (the Windows Kernel) occupies as much as an entire CPU core.
It is always one thread inside the kernel that that uses up all CPU cycles and it has ``tcpip.sys`` in its call stack:

.. image:: img/tcpip_sys.png
   :alt: tcpip.sys

The reason for that is the built in Windows Defender Firewall (The MpsSvc Service).
The Defender Firewall cannot be deactivated in Windows 10 any more.
The MpsSvc Service will even keep running when temporarily deactivated, as it offers further security features besides the Defender Firewall.

Solutions
=========

In order to fix the issue (besides just not using Win10, which would be the easiest), I propose two solutions:

Solution 1: Disable MpsSvc
--------------------------

When disabling the Windows Defender Firewall Service (MpsSvc), the problem vanishes.
In order to do that, we have to modify ``HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\MpsSvc`` and set ``Start = 4`` (Disabling the Windows Defender is not supported in Windows 10, so it cannot be done using the GUI).
When the Windows is in a Windows Domain, we also have to prevent it connecting to it, as the Windows Group Policy Client would otherwise enable and start MpsSvc again.
Again, we need to set ``Start = 4`` in ``HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\gpsvc``.
As Windows tries to prevent modifying that entry, we need to do it as NT-AUTHORITY/SYSTEM (Normal Administrator rights are not sufficient).
SYSTEM rights can be gained by using `PsExec <https://docs.microsoft.com/en-us/sysinternals/downloads/psexec>`_ and starting regedit with that: ``C:\Windows\System32\PsExec64.exe -i -s regedit.exe``.

* Advantages:

  * It works
  * No software needs to be modified 

* Disadvantages:

  * Microsoft strongly discourages doing that
  * The solution may not work any more eventually, when Microsoft changes something
  * We loose all (?) network related security features built into Windows (including additional stuff like IPSEC)
  * No Group Policies will be applied or updated any more
  * There will be other side effects, as a Domain Windows installation rely on the Group Policy Client setting group policies from the accoring domain
  * The Continental IT will hate us for tampering with the OS
  * No new domain users will be able to login any more *(not sure if this is true)*
  * When playing around I lost my local admin rights.
    As admin rights are required to turn the services back on, it was quite a pain to revert the changes.
    Stuff like that can happen.

All in all this is not an acceptable solution for our dev machines that are administrated by the Continental IT.
It may however be acceptable for offline Car PCs.

Solution 2: Avoid tcpip.sys
---------------------------

On Windows, every (User space initiated) network access uses the Winsocks/2 API.
That API uses the Kernel-Mode filesystem driver Afd.sys, which the uses the Transport Protocol Driver tcpip.sys.
The tcpip.sys driver is where all the Protocol magic happens in Windows.
This is also the point where the Windows Defender Firewall analyzes the traffic.
Microsoft has created an entire API only for that purpose: The Windows Filtering Platform (WFP).
This API is available in Windows Vista and later.
Below tcpip.sys only comes the NDIS.sys (Network Driver Interface Specification) and the actual Ethernet NIC drivers.
If we want to keep MpsSvc, we need to bypass the entire tcpip.sys driver and everything above it.
Refer to the image below (black lines) to get an overview.
There are free and well implemented NDIS drivers available, for instance WinPcap and the (newer) Npcap.
When using one of those drivers, the Windows Defender does not see any open sockets and does not analyze the traffic.
We however have to implement stuff in our user space program that is usually done in the Kernel (e.g. IPv4 and UDP, as we deal with raw ethernet frames, here).
We also loose localhost / loopback traffic support, but NPCAP can work around that by installing a WFP (Windows Filtering Platform) driver which abuses that API to route all local traffic to a virtual adapter.

.. image:: img/winsocks.png
   :alt: Winsocks and NPCAP

* Advantages:

  * We don't need to deactivate the Windows Defender
  * We don't need to forcefully prevent Windows from applying Group Policies
  * The Conti IT will not be mad
  * This is the solution Microsoft recommends
  * This solution should also work in future

* Disadvantages:

  * A lot of implementation effort
  * It may be error-prone to write a custom UDP / IPv4 implementation
  * Each Win10 User has to install the NPCAP driver

I think that if Win10 shall remain an important platform, the implementation effort would be worth it.
In my small examples I can receive UDP MC traffic without any performance issues.
We can implement an NPCAP autodetection (ideas on how to do that are on the NPCAP website), and switch to the high-performance NPCAP mode when NPCAP is installed.

----------------------------------------------------------------------------

*From GitHub, written by Florian Reimold (2019-02-07):*

Some Notes
==========

I use this section to write down some things I discovered. They may not entirely blend in with the section above.

* The Windows Defender seems to be more aggressive when being connected to a domain network, at least in Windows 10 1709.

  * When connecting to my home network (or entirely disconnecting from all networks) I didn't observe the high CPU usage
  * I wasn't able to reproduce the Problem at all with my private Win10 1809-Oct Installation (But I didn't bind it to a domain, either)
  * I definitively have reproduced the Problem with a private Win10 installation some time ago. I think it was Win10 1709, but I am not exactly sure.
  * **Update**: I reproduced the issue with both Win10 1709 and 1809.
    I didn't need to connect to any network and I didn't have to join a Domain.
    It's still not as bad though as on my Conti Notebook, i.e. I need more UDP receiver to trigger the problem.

* I also managed to reproduce the same performance issue with the McAffee Firewall, while it was active. Disabling it fixed the issue.

----------------------------------------------------------------------------

*From GitHub, written by Florian Reimold (2019-02-08):*

Tests with the NPCAP Loopback adapter have shown 2 Problems (both appear to be easily solvable):

1. As the loopback adapter actually shows up as NIC in Windows, Windows will see a non-domain network connection.
   As our beloved Conti IT set a group policy that prevents multiple network connections when connected to a domain network, Windows will sporadically refuse to connect to the Conti network after installing the NPCAP Adapter.
   I am not exactly sure why this happens, as the virtual Adapters installed by VMWare and VirtualBox don't cause that problem.
   However it seems that we can easily work around that issue by disabling all protocols (except NPCAP) in the Adapter settings:
   
   .. image:: img/npcap_adapter.png
      :alt: Npcap loopback adatper

2. As our goal is to not open any receiving socket which joins the according multicast group, Windows optimizes the network flow and doesn't send our multicast packets to localhost.
   Thus we cannot capture them using the NPCAP loopback adapter.
   The solution looks quite similar to the Nvidia Drive PX 2 Socketbug workaround: We create a socket and join the multicast group.
   This will trigger the Kernel to send the Multicast packages to localhost and we can close the socket and continue receiving the raw packages with NPCAP.

----------------------------------------------------------------------------

*From GitHub, written by Florian Reimold (2019-02-13):*

I implemented a primitive Npcap based UDP Receiver and integrated it into eCAL (in a branch, as it is far from being ready to ship, yet).
Currently, only local traffic is supported and I am having some cold start issues (I haven't found a nice way to make the Windows Kernel send multicast traffic to localhost, yet).
My solution from the second point of the post above does not work as smoothly as I wished.

Tests with the eCAL Player (Socket based) and eCAL Recorder (Npcap based) however show, that the Npcap approach does solve the Win10 performance issue while maintaining 100% wire compatibility.

I am in contact with Insecure.Com LLC ("The Nmap Project"), as we may need an Npcap OEM internal license.

----------------------------------------------------------------------------

*From GitHub, written by Florian Reimold (2019-02-14):*

I have made some tests with the Windows 10 included loopback adapter (Which is called Microsoft KM-TEST Loopback Adapter since Windows 8.0).
My hope was to not depend on the special Npcap loopback adapter, which also uses the KM-TEST Loopback driver by Microsoft (but modifies its behaviour).
This would give us the opportunity to ship eCAL with Legacy WinPcap without any restrictions and without any licensing fees.
This is what Wireshark does.
The Loopback adapter however does not work as expected and I wasn't able to get any localhost traffic from it.

**tl;dr**: The Microsoft Loopback Adapter does not work. We need Npcap and cannot use WinPcap.

----------------------------------------------------------------------------

*From GitHub, written by Florian Reimold (2019-02-26):*

I integrated a new NPCAP based UDP Receiver that is now able to both receive loopback traffic and network traffic from arbitrary interfaces.

Current limitations:

* Unicast appears to not work for some reason
* No cold-start (Some program still has to open an actual socket in order to make Windows send multicast traffic to localhost)

Next steps:

* Move the implementation to its own reusable library with an API that may end up being similar to a stripped-down Version of QUdpSocket
* Improve the performance by using the native Win32 handles
* Fix the loopback cold start issue
* Investigate the unicast issue (Not really a priority. I may just solve it by not using NPCAP for unicast traffic)

----------------------------------------------------------------------------

*From GitHub, written by Florian Reimold (2019-03-01):*

Good news for everyone waiting for this. I more or less finished implementing the UdpcapSocket as library. The UdpcapSocket aims to mirror the behavior of a normal Winsocks socket as closely as possible.

Supported features are:

* Binding to an IPv4 address and a port
* Setting the receive buffer size
* Joining and leaving multicast groups
* Enabling and disabling multicast loopback (I managed to fix the cold start issue)
* Receiving unicast and multicast packages (Only one memcpy from kernel to user space memory)

Non supported features:

* Sending data
* Setting bind flags (sockets are always opened shared)
* IPv6

Next steps:

* Integrate it into eCAL
* Document the code
* Document the usage in the eCAL Wiki

----------------------------------------------------------------------------

*From GitHub, written by Florian Reimold (2019-03-06):*

I tested my current Npcap based version on a simulation PC with actual algos and compared it against the native socket based version.
Using Npcap showed massive performance improvements.
I still have some issues detecting npcap, but on systems having npcap installed in Winpcap compatibility mode it is working well.

----------------------------------------------------------------------------

*From GitHub, written by Florian Reimold (2019-03-07):*

I call this issue officially fixed.
If there are any issues with the current solution we can re-open it.