.. include:: /include.txt
.. include:: /_include_ecalicons.txt

.. _applications_sys_client:

=====================================
|ecalsys_APP_ICON_h1| eCAL Sys Client
=====================================

eCAL Sys Client provides the client for the eCAL Sys Host Applications (i.e. :ref:`applications_sys_gui` or :ref:`applications_sys_cli`).
Start this application on all machines, that shall be remote controlled.

eCAL Sys Client is not needed on the machine that runs the eCAL Sys Host Application.

.. important::

   The eCAL Sys Client is a remote-control application with no authentication.
   If you are annoyed by accepting this security implication at startup, you can start eCAL Sys Client with:

   .. code-block:: console

      ecal_sys_client --auto-accept-security-risk


Usage
=====

.. literalinclude:: sys_client_usage.txt
   :language: none
   
