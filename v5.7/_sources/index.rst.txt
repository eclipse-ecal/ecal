.. eCAL documentation master file, created by
   sphinx-quickstart on Wed Apr 25 18:37:11 2018.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. include:: include.txt

================================
Welcome to eCAL's documentation!
================================

eCAL (**e**\ nhanced **C**\ ommunication **A**\ bstraction **L**\ ayer) is a fast publish-subscribe middleware that can manage inter-process data exchange, as well as inter-host communication.

Getting Started
===============

To start learning about eCAL, please refer to the :ref:`Getting Started <getting_started_introduction>` chapter.

This chapter will walk you through 

* :ref:`Setting up <getting_started_setup>` and :ref:`configuring eCAL <getting_started_cloud>`
* The :ref:`applications <getting_started_applications>` that come with eCAL
* Writing your first :ref:`Hello World <getting_started_hello_world>` program 

Links
=====

If you just want to download eCAL for |fa-windows| Windows or |fa-ubuntu| Linux, click the following button:

.. raw:: html

   <p>
      <a class="btn btn-sm btn-primary" href="https://github.com/continental/ecal/releases" target="_blank">
         <span class="btn-icon"><span class="fas fa-download"></span></span>
         <span class="btn-text">Download eCAL</span>
      </a>
   </p>

To contribute, please visit our github repository:

.. raw:: html

   <p>
      <a class="btn btn-sm btn-primary" href="https://github.com/continental/ecal" target="_blank">
         <span class="btn-icon"><span class="fab fa-github"></span></span>
         <span class="btn-text">eCAL on Github</span>
      </a>
   </p>
   
License
=======

eCAL is licensed under `Apache License 2.0 <http://www.apache.org/licenses/LICENSE-2.0>`_. You are free to

* Use eCAL commercially
* Modify eCAL
* Distribute eCAL

eCAL is provided on an "as is" basis without warranties or conditions of any kind.

.. toctree::
   :hidden:
   :caption: Getting started
   :numbered: 

   getting_started/introduction
   getting_started/setup
   getting_started/samples
   getting_started/cloud
   getting_started/applications
   getting_started/services
   getting_started/development

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
   
   advanced/documentation
   advanced/performance
   advanced/transport_layers

