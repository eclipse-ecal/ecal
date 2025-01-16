.. include:: /include.txt

.. _configuration_npcap:

==================
|fa-windows| Npcap
==================

When using eCAL on Windows 10, you may experience a slowdown of the entire operating system.
The mouse curser will stutter, your eCAL processes will not run smooth any more and they may even disappear in the eCAL Montior.

The issue is causes by a security feature in the Windows Defender, which causes massive CPU load when encountering UDP Multicast traffic.
Windows 7 did not have that issue.

Although the problem is not caused by eCAL, eCAL can work around it by using Npcap for receiving UDP Multicast packages. 

If you want to learn more details, please read the :ref:`original analysis <npcap_details>` of the problem.

.. note::
   As Ubuntu doesn't have that issue, the NPCAP mode is not available there

How to use Npcap
================

#. Download Npcap: https://nmap.org/npcap/

#. Install Npcap and make sure to check the **Legacy loopback suppport**:

   .. image:: img/npcap_install.svg
      :alt: Npcap install options

   .. note::
      If you did not check the legacy loopback support, eCAL will not use Npcap!
      It will just function as normal (and continue to trigger the Win10 performannce issue).

      Please reinstall Npcap with the required options.

#. Optional:
   If your corporation blocks your Windows to simultaneously connect to domain and non-domain networks, you must remove all elements / protocols except NPCAP and NPF from the Npcap Loopback Adapter

   *(The NPF element is only available if you installed Npcap in WinPcap compatibility mode)*
    
   .. image:: img/npcap_loopback_adapter_settings.png
      :alt: Npcap Loopback adapter settings

4. Edit |ecalini-path-windows|:

   .. code-block:: ini
      
      npcap_enabled = true
    
5. Check eCAL Mon

   .. image:: img/npcap_ecal_mon.svg
      :alt: Npcap in eCAL Monitor System Information

Troubleshooting
===============

If you enable Npcap but didn't install it, installed it with the wrong settings or your installation is broken, eCAL Mon will tell you that the Npcap Initialization has failed.
eCAL will still work, but not use Npcap.

The System Information will display an error. The detailed error can be seen in the eCAL Monitor under ::guilabel:`View / Npcap Status`:

.. image:: img/npcap_error_systeminformation.png
   :alt: eCAL Monitor System Information (Npcap Error)

.. image:: img/npcap_error_npcapstatus.png
   :alt: eCAL Monitor System Information (Npcap Error)

Especially old versions of Npcap seems to break itself if you reinstall it, as the uninstaller does not remove the Loopback Adapter properly.
The new Npcap installation will fail to create a new Loopback Adapter and the installation will be corrupt!

Please do a clean install of Npcap:

1. Uninstall Npcap
2. Manually uninstall all Npcap Loopback Adapters using the Windows device manager.
3. Manually uninstall all Microsoft KM-TEST Loopback adapters with the device manager
4. Install Npcap again with the settings above
