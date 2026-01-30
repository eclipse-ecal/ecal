.. include:: /include.txt
.. include:: /_include_ecalicons.txt

.. _applications_recorder_cli:

==================================
|ecalrec_APP_ICON_h1| eCAL Rec CLI
==================================

.. note::
   
   eCAL Rec CLI is available since eCAL 5.9

eCAL Rec CLI is the Command Line eCAL Rec Host Application (i.e. a replacement for eCAl Rec GUI).
The binary is :file:`ecal_rec /.exe`. eCAL Rec CLI can be used both with *single-shot* parameters, as well as from an interactive console.
When using an interactive console, the application will not exit automatically, but accept commands from STDIN.

.. important::
   Don't confuse the two command line eCAL Rec Applications:
   
   - :file:`ecal_rec /.exe`: The recorder client that is used for distributed recordings.
     *This is the application this topic is about*

   - :file:`ecal_rec_client /.exe`: The recorder host application in command-line flavour.

Just like eCAL rec GUI, the CLI application can:

- Load, create and save :file:`.ecalrec` configuration files
- Create local and distributed recordings

Additional to those features, eCAL Rec CLI has three modes:

- **Direct mode** (default): By default, eCAL Rec CLI will load a config and execute commands by itself.
  It will just act as a normal eCAL Rec Host application.

- **Remote-control mode**: The remote control mode can by activated by passing the ``--remote-control`` parameter.
  When starting eCAL Rec CLI in remote-control mode, it will not execute commands by itself, but relay them to another eCAL Rec Host applicaiton (i.e. an eCAL Rec GUI or an eCAL Rec CLI running in direct + interactive mode).
  This mode is usefull, if you e.g. want to use the GUI, but still want to automatically trigger a recording in certain events.

  Both eCAL Rec GUI and eCAL Rec CLI (in non-remote-control mode) can be remote-controlled.

- **Interactive mode**: The interactive mode can be activated by passing the ``--interactive`` parameter.
  When in interactive mode, eCAL Rec CLI will keep running and start an interactive console, where you can type additional commands.
  
  The interactive mode and remote-control mode can be used simultaneously.

.. image:: img/rec_server_cli.png
   :alt: eCAL Rec CLI

Usage
=====

.. literalinclude:: rec_server_cli_usage.txt
   :language: none


Interactive mode
================

You can activate the interactivate mode by passing the ``--interactive`` parameter on command line.
Instead of exiting eCAL rec will then go to an interactive console that lets you control it interactively by typing in commands.

If you want to remote-control eCAL Rec CLI with another eCAL Rec CLI instance, you also have to set it to interactive mode, just to prevent it from shutting down prematurely.

.. literalinclude:: rec_server_cli_interactive_help.txt
   :language: none

Automatize eCAL Rec CLI
-----------------------

You can use the ecal_rec interactive mode to automatize it via STDIN.
Commands are read line-by-line, i.e. they have to be divided by ``\n``.
Semicolons do not work.
The most concise would probably be to write all your commands in a text file and pipe the content of that to ecal_rec.

.. important::

   Unlike typing commands by hand, piping them into ecal_rec causes them to be executed as fast as possible.
   However, e.g. when using the ``upload`` command, ecal_rec will not check, if it would have to wait for other things to finish (like flushing a recording for example!).

   So make sure you always add at least a ``sleep 1`` before performing operations on measurements!

Let's look at some examples.
The examples will load a config, create a 5 seconds measurement, upload it, print the current status and exit.


- |fa-ubuntu| Ubuntu:

  .. code-block:: bash
     
     # Commands from command line:
     printf "rec 5 \n sleep 1 \n upload \n sleep 1 \n status" | ecal_rec -c ~/tutorial.ecalrec --interactive

     # Commands from a file:
     cat commands.txt | ecal_rec -c ~/tutorial.ecalrec --interactive

- |fa-windows| Windows CMD.exe:

  .. code-block:: bat
     
     rem Commands from command line:
     (echo rec 5 & echo sleep 1 & echo upload & echo sleep 1 & echo status) | ecal_rec -c tutorial.ecalrec --interactive

     rem Commands from a file:
     more commands.txt | ecal_rec -c tutorial.ecalrec --interactive

- |fa-windows| Windows PowerShell:

  .. code-block:: powershell
     
     # Commands from command line:
     echo "rec 5 `n sleep 1 `n upload `n sleep 1 `n status" | ecal_rec -c tutorial.ecalrec --interactive

     # Commands from a file:
     cat commands.txt | ecal_rec -c tutorial.ecalrec --interactive
