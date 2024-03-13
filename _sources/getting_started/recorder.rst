.. include:: /include.txt
.. include:: /_include_ecalicons.txt

.. _getting_started_recorder:

==============================
|ecalrec_APP_ICON_h1| Recorder
==============================

.. hint::
   Don't be afraid to play around with the GUI! If you closed or moved something and can't get it back, just click :guilabel:`View / Reset Layout`!

   .. image:: img/recorder_reset_layout.png
      :alt: eCAL Recorder Reset Layout

The eCAL Recorder can record any data that is sent through your eCAL network and save it to disk.
The output of the recorder (-> the recording) is called an **eCAL Measurement**.
You can use that measurement later to analyze it or to stimulate other applications that need that data as input.

.. seealso::
   This chapter will focus on the very basics of eCAL Rec to get you started.
   To learn more about all features of eCAL Rec, both the GUI and the command line application, please check out the :ref:`Advanced eCAL Rec <applications_recorder>` chapter. 

Features
========

The eCAL Recorder is quite powerful, so we will only look at a limited setup in this tutorial.
The eCAL Recorder has two essential modes:

#. **Centralized recording**: Record all topics on your main machine (-> over network).
   This is the most trivial mode and we will use it in this tutorial

#. **Distributed recording**: Each machine records its own topics.
   This mode is used to avoid network congestion.

.. seealso::

   To learn more about centralized and distributed recordings, please refer to the advanced :ref:`applications_recorder_centralized_distributed_recording` chapter.

GUI Overview
============

Let's start with a short GUI overview.

* |fa-windows| On Windows you can start the eCAL recorder from the Start Menu: |ecal_rec-start-menu-path-windows| 
* |fa-ubuntu| On Ubuntu start :file:`ecal_rec_gui` from a terminal

For the screenshot, the Windows machine is running the eCAL Recorder, while the Linux machine is running the `person_snd` sample.

.. image:: img/recorder_overview_annotated.svg
   :alt: eCAL Recorder
   :target: ../_images/recorder_overview_blank.png

.. rst-class:: bignums-xxl

   #. **Topics panel**

      Here you can see all available topics.
      You can select whether to blacklist or whitelist some of them.
      After activating the recorder, you can also see which recorder is recording which topic, which is mainly interesting when using distributed recording.

      As you can see in the screenshot, the recorder has already picked up the *person* topic published by the sample running on *florian-ubuntu*

   #. **Configuration**

      In the configuration panel you can configure the measurement:

      * *Meas dir*: The path where to save your measurements to
      * *Meas name*: How the measurement should be named (This will create a subfolder in your measurement directory).

      You can also enter a description and select the maximum file size.
      If a measurement exceeds the maximum size, it will be splitted to multiple files.

      .. note::
         
         Don't be confused by the strange syntax. You can use plain paths (e.g. ``D:\meas\root\dir``) if you like.
         Using the eCAL replacement syntax however solves the problem of having different paths for Windows and Linux machines and timestamped subfolders for each measurement.

   #. **Active recorders**

      Here you can see all recorder instances and their reported statuses.
      Currently there is only one recorder (florian-windows).
      In a distributed recording configuration however, this panel would display all recorder clients.
      
      At the bottom of the panel you can activate pre-buffering and set the number of seconds that shall be kept in the buffer.

   #. **Controls**

      In this area are the three main buttons for controlling your recording:

      * |ecalicons_POWER_ON| :guilabel:`Activate`: Clicking this button will cause the recorder to create subscribe to the eCAL topics it is supposed to record.
        This will also cause the pre-buffer getting filled.

      * |ecalicons_RECORD| :guilabel:`REC`: This button starts and stops the recording.
        If you haven't clicked *Activate*, this step is done automatically.
        So, unless you care about the pre-buffer function, you don't need to manually activate the recorder.

      * |ecalicons_SAVE_TO_DISK| :guilabel:`Save Pre-Buffer`: This button only becomes active, when you enabled Pre-buffering.
        Clicking it will save the content of the current pre-buffer as a new measurement, without having to click REC and STOP immediately afterwards. 

   #. **Measurement history**

      Whenever you start a recording (either by clicking *REC* or *Save Pre-Buffer*), the measurement is displayed here.
      From this panel you can delete it, add comments to it or upload it to an FTP Server for merging distributed measurements.

Let's record some data!
=======================

As mentioned above, we have a *person_snd* sample running on a remote machine. Now let's record it!

#. Click |ecalicons_RECORD| :guilabel:`REC` 
#. Wait some seconds
#. Click |ecalicons_STOP| :guilabel:`STOP`

That's it. You have just created your first eCAL measurement.
Also try adding a comment to your measurement by clicking the |ecalicons_ADD_FILE| :guilabel:`Comment...` button in the measurement list!
If you haven't modified the measurement path, it is saved to:

* |fa-windows| Windows: :file:`C:\\ecal_meas\\`
* |fa-ubuntu| Linux: :file:`~/ecal_meas/`

.. image:: img/recorder_recording.png
   :alt: eCAL Recorder recording

eCAL Measurements
=================

Before continuing to the next section and replaying the measurement, let's quickly look at what the Recorder has just created:

.. image:: img/recorder_measurement_filesystem.png
   :alt: eCAL Measurement

* |fa-file-alt| :file:`doc/description.txt`: Your description and your comment is saved in this file
* |fa-folder-open| :file:`florian-windows`: Each recorder creates a directory with its hostname for its files. This is important in a distributed measurement scenario, where multiple recorders exist. You will see a directory with your own hostname here.

  * |fa-file-alt| :file:`ecal.ini`: The configuration file at the time when you started the recorder. Useful for finding configuration issues later.
  * |fa-file| :file:`florian-windows.hdf5`: The actual recorded data. eCAL records data in the standardized `HDF5 format <https://www.hdfgroup.org/solutions/hdf5/>`_
  * |fa-file-alt| :file:`system-information.txt`: The same system information that we have seen in the eCAL Monitor. Useful for finding issues later.

* |fa-file| :file:`2020-08-06_02-12-12.596_measurement.ecalmeas`: "Index-File" that can be opened with the eCAL Player. Does not contain any data by itself.

Now that you know how to record data and have created your first eCAL Measurement continue to the next section and try to replay it with the eCAL Player!
