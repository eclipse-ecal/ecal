.. include:: /include.txt
.. include:: /_include_ecalicons.txt

.. _getting_started_player:

=============================
|ecalplay_APP_ICON_h1| Player
=============================

.. hint::
   Don't be afraid to play around with the GUI! If you closed or moved something and can't get it back, just click :guilabel:`View / Reset Layout`!

   .. image:: img/player_reset_layout.png
      :alt: eCAL Player Reset Layout

After you have created an eCAL measurement in the previous section, you now learn how to replay the data. 

So, start the eCAL Player!

* |fa-windows| On Windows you can start it from the Start Menu: |ecal_play-start-menu-path-windows|
* |fa-ubuntu| On Ubuntu start :file:`ecal_play_gui` from a terminal

Also load a measurement, before we get to the GUI overview:

#. Click :guilabel:`File / Open measurement...`
#. Select the :file:`.ecalmeas` file from your eCAL measurement

Overview
========

.. image:: img/player_overview_annotated.svg
   :alt: eCAL Player
   :target: ../_images/player_overview_blank.png

.. rst-class:: bignums-xxl

   #. **Channels**
   
      Here you find a list of all topics in the measurement.
      At the moment we only have one topic.

      You can de-select topics here you don't want to be replayed.
      You can also rename topics (from the context menu or by pressing :kbd:`F2`).

   #. **Labels**
      
      If your measurement contains some parts that are of special interest, you can add a label here.
      You can then easily jump to that point later.

   #. **Description**
      
      The measurement description. You should notice the description and the comment from the previous section.

   #. **Main controls**

      Here you can start / stop the playback, step through the measurement and enable repeat-mode.

Replaying the measurement
=========================

Click |ecalicons_START| :guilabel:`Play` and close any `person_snd` you may still have running.
Open a `person_rec` or an eCAL Monitor and watch the data incoming from the Player!

.. tip::

   Activate the repeat-option, so your replay starts over again

.. image:: img/player_monitor_person_rec.png
   :alt: eCAL Player, Monitor and person_rec sample
