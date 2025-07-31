.. eCAL documentation master file, created by
   sphinx-quickstart on Wed Apr 25 18:37:11 2018.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. include:: /include.txt

========================
Welcome to Eclipse eCAL™
========================

eCAL (**e**\ nhanced **C**\ ommunication **A**\ bstraction **L**\ ayer) is a fast publish-subscribe middleware that can manage inter-process data exchange, as well as inter-host communication.

Getting Started
===============

To start learning about eCAL, please refer to the :ref:`Getting Started <getting_started_introduction>` chapter.

This chapter will walk you through 

* :ref:`Setting up <getting_started_setup>` and :ref:`configuring eCAL <getting_started_network>`
* The :ref:`applications <getting_started_applications>` that come with eCAL
* Writing your first :ref:`Hello World <_getting_started_howto_pubsub_string_hello_world>` program 

Learn about how to use and develop with eCAL in our Webinar:

.. youtube:: 8AAxlu1WbdU

.. include:: _download_main_page/_main_page_download_section.rst.txt


Download
========

- |fa-windows| Windows / |fa-ubuntu| Ubuntu:

  Go to the Release Page to get the latest version of eCAL:
  
  .. raw:: html
  
     <p>
        <a class="btn btn-sm btn-primary" href="https://eclipse-ecal.github.io/ecal/releases/" target="_blank" style="color: #fff;">
           <span class="btn-icon"><span class="fa fa-download"></span></span>
           <span class="btn-text">Download eCAL</span>
        </a>
     </p>

- |fa-ubuntu| Ubuntu (from PPA):

  Install eCAL from our PPA:
  
  .. code-block:: bash

     sudo add-apt-repository ppa:ecal/ecal-latest
     sudo apt-get update
     sudo apt-get install ecal
  
  This PPA will always upgrade you to the latest eCAL Release (-> Rolling Release PPA). If you intend to stay on an specific release, check out other PPAs :ref:`here <getting_started_setup_ubuntu_ppa>`.

  
License
=======

eCAL is licensed under `Apache License 2.0 <http://www.apache.org/licenses/LICENSE-2.0>`_. You are free to

* Use eCAL commercially
* Modify eCAL
* Distribute eCAL

eCAL is provided on an "as is" basis without warranties or conditions of any kind.

Trademark
=========

Eclipse eCAL™ is a trademark of the Eclipse Foundation, Inc. and is used with permission.

.. toctree::
   :hidden:
   :caption: Getting started
   :numbered: 

   getting_started/introduction
   getting_started/setup
   getting_started/samples
   getting_started/network
   getting_started/services
   getting_started/applications
   getting_started/howto

.. toctree::
   :hidden:
   :caption: Development

   development/build_ecal
   ecal_api

.. toctree::
   :hidden:
   :caption: Using eCAL

   configuration
   applications
   faq

.. toctree::
   :hidden:
   :caption: Advanced
   
   versions/ecal_versions
   Downloads <https://eclipse-ecal.github.io/ecal/releases/>
   versions/6.0/migration_guide
   advanced/tutorials
   advanced/ecal_internals
   advanced/ros_comparison
   license