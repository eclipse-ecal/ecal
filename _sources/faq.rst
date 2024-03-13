.. _faq:

===
FAQ
===

Something doesn't work? Read through this list of hotfixes.

What happened to eCAL 1-4?
==========================

eCAL 1-4 have been developed as internal tool and never got published. The first public version of eCAL is eCAL 5.

eCAL is not functioning while being connected to a VPN
======================================================

VPN Software may block or route the eCAL Multicast traffic.
Try to :ref:`configure eCAL in local mode <configuration_local>` or disconnect from the VPN.

The eCAL Player is crashing on Ubuntu
=====================================

The eCAL Player creates a lot of shared memory files; multiple files for each topic from the measurement.
When opening a large measurement, this triggers a limit set by the operating system.

:ref:`Increase the limit <configuration_player>` and the Player should work just fine, again!

eCALRec cannot control the clients on other hosts
=================================================

Usually, this happens when the OS cannot resolve the hostname to an IPv4 address.

1. Make sure you actually started the recorder client
2. Check the :file:`hosts` file on all PCs and try to ping them **by name**! You can follow the :ref:`getting started tutorial <getting_started_services>`!

.. _faq_ubuntu_22_04_graphical_issues:

eCAL GUI Applications are having graphical issues on Ubuntu 22.04
=================================================================

Ubunt 22.04 finally switched to Wayland and (kind of) deprecated X11.
Well, maybe they should have waited a little bit longer, because *all* Qt5 applications (eCAL Rec/Sys/Mon/Play, Qt Creator etc.) show severe graphical issues on Ubuntu 22.04.
You can fix it however, by selecting "Ubuntu on XOrg" on your login screen, so Ubuntu will boot with X11.

.. image:: img/ubuntu_22_04_on_xorg_cut.svg
   :alt: Ubuntu 22.04 on Xorg
