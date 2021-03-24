.. tabs::

   .. tab:: Rolling Release

      This is the rolling release PPA.
      It will always upgrade your installation and you will always receive the latest-and-greatest eCAL version.
      
      .. code-block:: bash
      
         sudo add-apt-repository ppa:ecal/ecal-latest
         sudo apt-get update
         sudo apt-get install ecal

      Also check out this PPA on `Launchpad <https://launchpad.net/~ecal/+archive/ubuntu/ecal-latest>`_!

@{lastest_version_counter = 0}@
@[for ecal_version in ppa_list]@
@{
ecal_version_string = str(ecal_version.major) + "." + str(ecal_version.minor)
}@
   .. tab:: eCAL @(ecal_version_string)

      This PPA will always stay on eCAL @(ecal_version_string).
@[    if lastest_version_counter <= 1]@
      You will receive patches as long as eCAL @(ecal_version_string) is supported.
@[    end if]@
      If you want to upgrade to a new eCAL Version, you will have to manually add the new PPA.
@[    if lastest_version_counter == 0]@
      
      At the moment, eCAL @(ecal_version_string) is the latest supported version.
@[    end if]@
@[    if lastest_version_counter == 1]@

      At the moment, eCAL @(ecal_version_string) is the supported legacy-version.
      Support will be dropped when the next eCAL Version is released.
@[    end if]@
@[    if lastest_version_counter > 1]@

      eCAL @(ecal_version_string) is not supported any more and will not receive patches.
      You can still use the PPA to install the latest eCAL @(ecal_version_string).
      Please consider upgrading to a newer version.
@[    end if]@

      .. code-block:: bash
      
         sudo add-apt-repository ppa:ecal/ecal-@(ecal_version_string)
         sudo apt-get update
         sudo apt-get install ecal

      Also check out this PPA on `Launchpad <https://launchpad.net/~ecal/+archive/ubuntu/ecal-@(ecal_version_string)>`_!

@{
lastest_version_counter += 1
}@
@[end for]@