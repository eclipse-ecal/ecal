.. include:: /include.txt
.. include:: /_include_ecalicons.txt

.. _applications_recorder_gui:

==================================
|ecalrec_APP_ICON_h1| eCAL Rec GUI
==================================

The recorder GUI is the most user friendly application to start recordings and to create :file:`.ecalrec` configuration files.
So let's walk through it to explain all the feature it has.

????????????????? Insert image here ?????????????????

Main Control Panel
------------------

- |ecalicons_POWER_ON| :guilabel:`Activate` / |ecalicons_POWER_OFF| :guilabel:`Deactivate`:
  Clicking this button will Activate and Deactivate eCAL Rec.
  For just starting a recording, it is not mandatory to do that manually.

  However, if you have a pre-buffer configured, activating the recorder actually is important.
  Activating will make the recorder create eCAL Subscribers and start buffering data.
  So if you don't click activate, your pre-buffer will not get filled.

- |ecalicons_RECORD| :guilabel:`Rec` / |ecalicons_STOP| :guilabel:`Stop`:
  Starts or stop a recording.

- |ecalicons_SAVE_TO_DISK| :guilabel:`Save Pre-Buffer`: Save the content of the pre-buffer to its own measurment.

  This button is only enabled, when you have enabled pre-buffering *and* the recorders are activated.
  Basically, this is a shortcut for quickly clicking Rec + Stop.
  However you can even save the pre-buffer while a recording is running.
  It will not interfere with that.7

Topics
------

The topics panel has multiple functions:

- It displays all eCAL that are recorded or can be recorded
- It lets you create a black- or whitelist 
- During a recording it shows which topic is recorded by which recorder