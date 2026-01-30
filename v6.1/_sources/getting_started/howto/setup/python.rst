.. include:: /include.txt

.. _getting_started_howto_setup_python:

==================
Python Setup
==================

While eCAL is written in C++, it also offers a great Python integration.
In this chapter we show how to setup Python for eCAL.

First, make sure that you have a Python installation on your system.
We'd recommend making use of Python virtual environments to then install the eCAL Python integration.


* Install Python 3 on your system.

  * |fa-windows| Windows:

    Download and install Python 3 64-bit (https://www.python.org/downloads/)

  * |fa-ubuntu| Ubuntu:
      
    .. code-block:: bash
  
       sudo apt install python3 python3-pip

* Install the eCAL Python integration (eCAL > 6.0.1)

  Install eCAL from PyPi, e.g. via pip
  
  .. code-block:: batch

     pip install eclipse-ecal

* Install the eCAL Python integration (prior to eCAL 6.0.1)

  Choose the appropiate version from the :ref:`download_archive` and download the :file:`.whl` file.
  Open a command prompt in your download directory and install the Python wheel with:

  .. code-block:: batch

     pip install eclipse_ecal-<version>-<python-version>-<architecture>.whl
  
  ``architecture`` is one of the following: ``win_amd64``, ``manylinux_2_17_x86_64`` and ``manylinux_2_17_aarch64``.
  We currently upload wheels for Python 3.8 - 3.13.
