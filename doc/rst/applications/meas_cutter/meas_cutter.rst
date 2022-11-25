.. include:: /include.txt
.. include:: /_include_ecalicons.txt

.. _applications_meas_cutter:

========================
 eCAL Measurement Cutter
========================

.. seealso::
   To learn more about the measurement format used by eCAL please check out the :ref:`measurement_format` chapter.

eCAL Measurement Cutter is a Command Line Application used to manipulate eCAL measurements. 
The executable is :file:`ecal_meas_cutter /.exe`.
It can trim measurements by time, exclude / include channels, modify base file names and split the measurement files by size.

.. note::
   eCAL Measurement Cutter is available since eCAL 5.11.
   
Usage
=====

.. literalinclude:: meas_cutter_usage.txt
   :language: none

Now let's take them one by one and explain.

1. Input path (``-i, --input``)
--------------------------------

The path to the measurement that we want to process. It can also be a path pointing to an :file:`.ecalmeas` file.

.. important::
   Since the application can alter multiple measurements simultaneously, this argument can be given multiple times. But it also needs a matching number of output paths.

   Example: :file:`-i measurement_1 -o output_measurement_1 -i measurement_2 -o output_measurement_2`

2. Output path (``-o, --output``)
---------------------------------

The path where the converted measurement will be saved.

.. important::
   The application will create a folder called :file:`MEASUREMENT_CONVERTER` in the output path.

   If the folder already exists, it is important for it to be empty, otherwise the application will not continue.

3. Configuration file (``-c, --config``)
----------------------------------------

| The application uses a YAML configuration file that describes the changes to be applied on the given measurement.
| 
| Let's see how this configuration file can look like.
|

.. literalinclude:: config.yaml
   :language: yaml
   :linenos:

.. note::
   **What will be the result by using this configuration file?**

   We will have a measurement with base file name :file:`measurement` with a maximum size of 1024 MB per file.
   It will have a duration of 10 seconds, from second 10 of the original measurement to second 20, excluding at the same time any channel containg the characters sequence :file:`Image` and :file:`V2X`, and the channel :file:`TrafficParticipantListPb`, if they exist in the measurement.

* **basename** : the base name of the :file:`.hdf5` files
* **splitsize** : maximum size of the :file:`.hdf5` files in Megabytes
* **trim** : specifies the fact that we want to trim the measurement, either at the start, the end or both
  
  - **start** \/ **end**  : specifies how the start/end will be calculated
  
    + **time** : the value that we want to use to trim the start/end
    + **base** : the reference point from where the new start/end is calculated
    + **scale** : the scale that we want to use
  
.. important::
   Possible values for **base**: start, end, absolute.
   The absolute value means that the timestamp from the measurement will be used.

   Possible values for **scale**: s, second, seconds, us, microsecond, microseconds, ms, millisecond, milliseconds.

* **include**\/**exclude** (mutually exclusive) : a YAML list describing the channel names that we want to include or exclude from a measurement.

.. note::
   When trying to include/ exclude channels using regex, use :file:`- re:`.
   Use one list item for one expression.

4. Save log (``-s, --save_log``)
--------------------------------

When this flag is enabled, the application will also write its output in a log file.
This file can be found in the folder :file:`log`, next to the executable.

5. Disable verbose (``-q, --quiet``)
------------------------------------

When this flag is enabled, console output is disabled except for error messages.

6. Enable one file per topic (``--enable-one-file-per-topic``)
--------------------------------------------------------------

When this flag is enabled, each topic will be written in its own HDF5 file.

