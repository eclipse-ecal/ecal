.. include:: /include.txt

.. _getting_started_python_preparation:

==================
Python preparation
==================

While eCAL is written in C++, it also offers a great python integration.
In this chapter we show how to setup python for eCAL, so you can start using the samples from :ref:`_getting_started_howto_pubsub_string_hello_world`.

.. important::

   The eCAL Python integration is called ``python3-ecal5``.
   With the ``5`` at the end.
   The other name has already been taken by another package.

Dependencies
============

First, you have to install some more development dependencies:

* |fa-windows| Windows:

  * Python 3 64-bit (https://www.python.org/downloads/)

  * The eCAL python integration.
    Choose the appropiate version from the :ref:`download_archive` and download the :file:`.whl` file.
    Open a command prompt in your donwload directory and install the python whl with:

    .. code-block:: batch

       pip install ecal5-****-win_amd64.whl

* |fa-ubuntu| Ubuntu:

  * Python 3: 

    .. code-block:: bash

       sudo apt install python3 python3-pip

  * The eCAL Python integration.

    * If you have installed eCAL from a :ref:`PPA<getting_started_setup_ubuntu_ppa>`:
      
      .. code-block:: bash

         sudo apt install python3-ecal5

    * If you are not using a PPA, choose the appropiate version from the :ref:`download_archive` and download the :file:`.whl` file.
      Open a terminal in your donwload directory and install the python whl with:

  
      .. code-block:: bash

         sudo pip3 install ecal5-*-linux_x86_64.whl