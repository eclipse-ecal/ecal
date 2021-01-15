.. include:: /include.txt

.. _applications_sys_cli:

============
eCAL Sys CLI
============

eCAL Sys can be used purely from command line.
The executable is :file:`ecal_sys (.exe)`.
It can execute start / stop / restart commands provided from the command line or as well as from an interactive console.
The interactive mode can also be used to automatize workflows by automatically piping commands to ecal_sys via stdin.

eCAL Sys CLI features two modes:

#. Direct mode (default):

   This mode lets you load a configuration and directly start or stop tasks.

#. Remote control mode:

   This mode will connect to another eCAL Sys application (GUI or CLI).
   Commands given to ecal_sys will then be forwarded to the remote-controlled eCAL Sys instance.
   Use this mode when you want to mainly use eCAL Sys manually (e.g. the GUI) but also automatically send commands to it in certain situation.

.. _applications_sys_cli_usage:

Usage
=====

.. literalinclude:: sys_cli_usage.txt
   :language: txt


-----------------------------------------------------------------

This section is still under construction. But we are working on it! Just be patient.

.. image:: /img/snail.svg
   :alt: Snail
   :align: center

