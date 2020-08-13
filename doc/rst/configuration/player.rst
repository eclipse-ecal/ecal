.. include:: /include.txt

.. _configuration_player:

===================================
|fa-ubuntu| Player - Max open files
===================================

When having an eCAL measurement with many topics on Ubuntu, you may experience crashes.
This is caused by a Linux security feature that limits the number of open file descriptors (to 1024 by default).
As each topic relies on multiple files (used by the shared memory transport layer), that limit may be triggered and cause a crash.

.. note::
   Windows does not have that problem and should replay the measurement just fine.

Please follow the following steps to increase the allowed number of open files.
   
#. Add to :file:`/etc/sysctl.conf`:

   .. code-block:: ini

      fs.file-max = 65535
  
#. Apply this setting:

   .. code-block:: bash
   
      sudo sysctl -p

#. Add to :file:`/etc/security/limits.conf`:

   *Replace YOUR_USERNAME with your actual username!*

   .. code-block::
   
      YOUR_USERNAME     soft     nproc          65535
      YOUR_USERNAME     hard     nproc          65535
      YOUR_USERNAME     soft     nofile         65535
      YOUR_USERNAME     hard     nofile         65535

#. Add to both :file:`/etc/systemd/user.conf` and :file:`/etc/systemd/system.conf`:

   .. code-block:: ini
   
      DefaultLimitNOFILE=65535

#. Reboot

Now the player should play the measurement just fine.