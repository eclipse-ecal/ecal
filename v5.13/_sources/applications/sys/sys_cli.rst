.. include:: /include.txt
.. include:: /_include_ecalicons.txt

.. _applications_sys_cli:

==================================
|ecalsys_APP_ICON_h1| eCAL Sys CLI
==================================

eCAL Sys can be used purely from command line.
The executable is :file:`ecal_sys /.exe`.
It can execute start / stop / restart commands provided from the command line or as well as from an interactive console.
The interactive mode can also be used to automatize workflows by automatically piping commands to ecal_sys via stdin.

.. seealso::
   eCAL Sys CLI cannot create .ecalsys files.
   Please use :ref:`eCAL Sys GUI<applications_sys_gui>` for that purpose.

eCAL Sys CLI features two modes:

#. **Direct mode** (default):

   This mode lets you load a configuration and directly start or stop tasks.

#. **Remote control mode**:

   This mode will connect to another eCAL Sys host application (GUI or CLI).
   Commands given to ecal_sys will then be forwarded to the remote-controlled eCAL Sys instance.
   Use this mode when you want to mainly use eCAL Sys manually (e.g. the GUI) but also automatically send commands to it in certain situation.


.. image:: img/sys_cli.png
   :alt: eCAL Sys CLI

.. _applications_sys_cli_usage:

Usage
=====

.. literalinclude:: sys_cli_usage.txt
   :language: none
   
Typical use-cases
=================

The following examples are meant as an orientation how to use eCAL Sys.

Single-shot commands
--------------------

Loading an .ecalsys file and starting / Stopping all tasks from it:

.. code-block:: bash

   # Start:
   ecal_sys -c ~/tutorial.ecalsys --start

   # Stop:
   ecal_sys -c ~/tutorial.ecalsys --stop

   # Restart:
   ecal_sys -c ~/tutorial.ecalsys --restart

.. note::

   ecal_sys will terminate once the command has been executed.
   Thus, terminating non-eCAL tasks will not work, as the new instance of ecal_sys that is responsible for stopping does not know the Process IDs.

.. _applications_sys_cli_interactive_mode:

Interactive mode
----------------

Launch the interactive mode to keep eCAL Sys running, while you can directly type commands.
Type ``help`` to view a list of all commands available in interactive mode.

If you combine a single-shot command with the interactive mode, eCAL Sys will continue with the interactive mode after it has executed the command.
eCAL Sys will not terminate on its own.

.. code-block:: bash

   ecal_sys -c ~/tutorial.ecalsys --interactive

Available commands are:

.. literalinclude:: sys_cli_interactive_help.txt
   :language: none

.. note::

   When starting terminal applications from the interactive mode, their STDOUT and STDERR will be printed in the ecal_sys terminal.
   This makes it *very* hard to enter another command or view information.

   You may want to use another ecal_sys instance in remote-control mode to work around that issue.

Remote-control eCAL Sys
-----------------------

First you need something that can be remote controlled:

.. code-block:: bash

   # GUI
   ecal_sys_gui ~/tutorial.ecalsys

   # CLI (--interactive Flag will prevent the CLI from exiting)
   ecal_sys -c ~/tutorial.ecalsys --interactive

Now you can use another instance of ecal_sys to remote-control that application.
You can use single-shot commands, the interactive mode or both.
In remote-control mode you cannot load an .ecalsys file, as this is done by the main application.

The following command will send the *start* command to your main application and then continue in interactive mode.

.. code-block:: bash

   ecal_sys --remote-control --remote-host YOUR_HOSTNAME --start --interactive

Automatize eCAL Sys CLI
-----------------------

You can use the ecal_sys interactive mode to automatize it via STDIN.
Commands are read line-by-line, i.e. they have to be divided by ``\n``.
Semicolons do not work.
The most concise would probably be to write all your commands in a text file and pipe the content of that to ecal_sys.

- |fa-ubuntu| Ubuntu:

  .. code-block:: bash
     
     # Commands from command line:
     printf "Start \n Sleep 10 \n List \n Stop \n Sleep 5" | ecal_sys -c ~/tutorial.ecalsys --interactive

     # Commands from a file:
     cat commands.txt | ecal_sys -c ~/tutorial.ecalsys --interactive

- |fa-windows| Windows CMD.exe:

  .. code-block:: bat
     
     rem Commands from command line:
     (echo Start & echo Sleep 10 & echo List & echo Stop & echo Sleep 5) | ecal_sys -c tutorial.ecalsys --interactive

     rem Commands from a file:
     more commands.txt | ecal_sys -c tutorial.ecalsys --interactive

- |fa-windows| Windows PowerShell:

  .. code-block:: powershell
     
     # Commands from command line:
     echo "Start `n Sleep 10 `n List `n Stop `n Sleep 5" | ecal_sys -c tutorial.ecalsys --interactive

     # Commands from a file:
     cat commands.txt | ecal_sys -c tutorial.ecalsys --interactive
