
.. _measurement_format:

=======================
eCAL Measurement Format
=======================

eCAL's measurement format is called ``ecalhdf5``.
It is used by eCAL Tools to record and replay measurements. Additional tools have been build around it to read / write / analyze measurement data.
The measurement format has been build on top of hdf5 to fullfill the requirements mentioned in the next section.
It has been build with the requirements in mind, and evolved slightly over the course of the years.

Requirements for a measurement format
=====================================

With eCAL, the user shall be able to record large amounts of data, in a distributed manner.
These distributed measurements shall be merged into one final measurement after a measurement has been finnished.
The measurements need to be self-contained, such that interpretation of the content of a measurement can be done without any additional information about used datatypes.
It shall be possible that eCAL Tooling can replay these measurements and preserve the timing behavior.
It shall be possible to anaylize measurements, with regard to package losses and timing behavior.


Design decisions
================

These toplevel requirements lead to a series of implementation requirements / design decisions

- Each measurement contains data channels and data entries. Each data entry belongs to a channel. 
- Each channel in the measurement is identified by its unique name
- Each channel in the measurement needs to have some meta-data that allows interpretation of binary data of that channel.
  This metadata includes the type of the channel, the protocol of the channel (e.g. protobuf, flatbuffers, ...), and the descriptor.
  The descriptor is a unique string that contains all neccessary information to correctly interpret the raw message content of that channel.
- Each data entry in the measurements consists of a data payload and some meta information.
- Meta information for each data entry include
  
  - Send timestamp (of algorithm)
  - Receive timestamp (in recorder)
  - The clock counter of the sender
  - ID of the sender
  
- Actual measurement files can be split with an adjustable file size
- All hdf5 files inside a given folder are considered to be one big measurement
- Merging of measurements distributed measurements is accomplished by copying measurement folders inside one top-level folder
- Insertion of data entries needs to be quick

Compatibility of file versions
------------------------------

The ecalhdf5 format has evolved over time, and will most likely evolve.
To ensure compatibility, the following needs to be ensured:

- Newer versions of the ecaldhf5 library need to be able to read older measurements.
- It is not neccessary that older versions of the ecalhdf5 library are able to read newer versions of ecalhdf5 measurement files.

Implementation details
======================

As previously mentioned, the ecalhdf5 format is build on top of hdf5.
However it defines its own format on how to structure the data inside the hdf5 file.

Measurement folders
-------------------
The ``ecalhdf5`` API takes a folder path to create a measurement. 
It treats all ``*.hdf5`` files contained in any subfolder as part of the measurement.
E.g. the available channels in a measurement is the union of all channels contained in the individual hdf5 files, as they may have been split according to file size or measurement location (in distributed measurement scenarios)

Top level hdf5 file
-------------------

The top level hdf5 file sets two attributes, ``Channels`` and ``Version``.

``Version``
  This is of Type ``String``. It specifies the version of the ecalhdf5 format. The current version is `5.0`.

``Channel``
  This is of Type ``String``. It is a comma separated list of all Channels present in the measurement.
  
The top level file contains different datasets, it contains datasets that make up the payload for each data entry, and datasets that contain metadata.

Payload datasets
----------------
The name of the dataset is a unique ID that is assigned by the ecalhdf5 library upon insertion.
The payload is saved as a char array.

Channel datasets
----------------
For each channel, there exists a dataset which contains meta information about the channel as attributes and then the meta information for each data payload.

The attributes for each channel store meta info about the serialization format of the channel, the type of the channel and the descriptor. This meta information makes the measurement self described, as it contains all information neccessary to interpret the binary data frames.

``Channel Type``
  This is of Type ``String``. It can be ``proto:pb.People.Person``. The first part is the serialization format, followed by ``:`` and then the type of the channel.
  eCAL supports ``string``, ``proto`` and ``mpack``.

``Channel Descriptor``
  This is of Type ``String``. This field contains information that allow interpretation of the stored message frames without the original datatype description files. It depends highly on the underlying serialization format.

The dataset itself is a table, where each row contains meta information for each data entry.
As of now, the ecalhdf5 format saves the following meta information with each message frame:
- Send timestamp: timestamped by eCAL as the publisher publishes the frame
- Receive timestamp: timestamped by eCAL Recorder as it has received the frame
- Send Clock: eCAL internal counter which is unique to each Publisher per eCAL Process. Counts up by one whenever the `Send` function is called. Useful to detect frame drops.
- Sender ID: ID that can be given to a subscriber to distiguish the data source in scenarios where multiple subscribers publish on the same topic.

Inserting new message frames
-----------------------------

Upon insertion of a data entry, that data entry gets assigned a new, unique data entry ID.
A new dataset, which has that unique ID as a name, is created, and the payload of the entry is stored in the dataset.
Then, a row to the table of the associated channel is appended. On a consecutive insert of another message of the same topic, a new payload dataset is created and the metadata is appended to the channel dataset table.

.. list-table:: Channel entry table after entering two packages for channel person
   :header-rows: 1

   * - person.hdf5
   * - person
   * - 0
   * - 1   


.. list-table:: Channel entry table after entering two packages for channel person
   :header-rows: 1

   * - Send timestamp
     - Unique entry ID
     - Send clock
     - Receive timestamp
     - Sender ID
   * - 1569340952965039
     - 0
     - 203
     - 1569340952964859
     - 0
   * - 1569340953467792
     - 0
     - 204
     - 1569340953467667
     - 0     

