.. include:: /include.txt

.. _getting_started_python_preparation:

==================
Python preparation
==================

While eCAL is written in C++, it also offers a great python integration.
In this chapter we show how to setup python for eCAL, so you can start using the samples from :ref:`_getting_started_howto_pubsub_string_hello_world`.

Dependencies
============

First, make sure that you have a Python installation on your system.
We'd recommend making use of Python virtual environments to then install the eCAL Python integration.

* |fa-windows| Windows:

  * Python 3 64-bit (https://www.python.org/downloads/)

* |fa-ubuntu| Ubuntu:

  * Python 3: 

    .. code-block:: bash

       sudo apt install python3 python3-pip


* The eCAL python integration.
  Choose the appropiate version from the :ref:`download_archive` and download the :file:`.whl` file.
  Open a command prompt in your download directory and install the python whl with:

    .. code-block:: batch

       pip install eclipse_ecal-<version>-<python-version>-<architecture>.whl
  
  ``architecture`` is one of the following: ``win_amd64``, ``manylinux_2_17_x86_64`` and ``manylinux_2_17_aarch64``.
  We currently upload wheels for Python 3.8 - 3.13.

  We will soon upload eCAL packages to PyPi for better integration with pip.