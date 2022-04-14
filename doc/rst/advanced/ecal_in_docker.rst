.. include:: /include.txt

.. _ecal_in_docker:

==============
eCAL in Docker
==============

Here we will show how to deploy eCAL into a docker container, and how to use its image from other containers.


.. important::
   
   This will work with eCAL 5.10 and up.
   Older eCAL versions will lack Shared Memory communication when being run in a docker container.

Prerequisite
============

* Install Docker as described in the `Docker installation documentation <https://docs.docker.com/engine/install/ubuntu/#install-using-the-repository>`_ 

  .. seealso::

     If you have problems regarding to the installation, check these links:

     * `Docker package not found <https://stackoverflow.com/questions/61401626/docker-installation-failed-on-ubuntu-20-04-ltsvmware>`_      
     * `Docker installation failed <https://forums.docker.com/t/cant-install-docker-on-ubuntu-20-04/93058>`_

* Optional: If you want your docker container to talk to eCAL Nodes on other machines:
  Set up you :ref:`multicast routes <getting_started_cloud_ubuntu_routes>` on the host.

* Optional: If you want to use eCAL on your host to subscribe to data from your docker containers: :ref:`Install eCAL <getting_started_setup_ubuntu>` on the host.


Getting Started
===============

In this tutorial we are going to create:

- A general purpose eCAL Docker container
- A publisher container with a custom Hello World Publisher
- A subscriber container receiving the Hello World data.

The file hierarchy that we are going to follow:
   
.. parsed-literal::

   |fa-folder-open| ecal_in_docker
   ├─ |fa-file-alt| :download:`docker-compose.yaml <src/ecal_in_docker/docker-compose.yaml>`
   |
   ├─ |fa-folder-open| ecal_runtime_container
   |  └─ |fa-file-alt| :download:`Dockerfile <src/ecal_in_docker/ecal_runtime_container/Dockerfile>`
   |
   ├─ |fa-folder-open| pub_container
   |  ├─ |fa-file-alt| :download:`Dockerfile <src/ecal_in_docker/pub_container/Dockerfile>`
   |  ├─ |fa-file-alt| :download:`CMakeLists.txt </getting_started/src/hello_world/hello_world_snd/CMakeLists.txt>`
   |  └─ |fa-file-alt| :download:`main.cpp </getting_started/src/hello_world/hello_world_snd/main.cpp>`
   |
   └─ |fa-folder-open| sub_container
      ├─ |fa-file-alt| :download:`Dockerfile <src/ecal_in_docker/sub_container/Dockerfile>`
      ├─ |fa-file-alt| :download:`CMakeLists.txt </getting_started/src/hello_world/hello_world_rec/CMakeLists.txt>`
      └─ |fa-file-alt| :download:`main.cpp </getting_started/src/hello_world/hello_world_rec/main.cpp>`

eCAL runtime container
----------------------

#. Create the file |fa-file-alt| :file:`ecal_runtime_container/Dockerfile` and paste the following installation commands:

   .. literalinclude:: src/ecal_in_docker/ecal_runtime_container/Dockerfile
      :language: Docker
      :linenos:

#. Build the image:

   .. code-block:: console

      cd ecal_in_docker
      sudo docker build . --rm -t ecal-runtime

#. Test the image

   .. code-block:: bash

      sudo docker run --rm -it --ipc=host --pid=host --network=host ecal-runtime

   At this point you are in the docker container.
   You can exit it with ``exit``.
   If you run ``ecal_sample_person_snd`` in the docker container and have an eCAL installation on your host, you can subscribe to the data via the eCAL Monitor or ``ecal_sample_person_rec``.

   .. note::
      
      * ``--ipc=host`` will enable Shared Memory communication with your host system and other docker containers that are started with the same parameter.
        This is important for local communication.

      * ``--network=host`` will share the host's network.
        This is important for network communcation with other machines.
        It is also important for local shared memory communication, as it affects the hostname of the container.
        The hostname is used to determine whether an eCAL topic is avaialble via shared memory.

      * ``--pid=host`` will share the Process-ID range with the host.
        Otherwise processes from different containers may get the same Process ID, which will prevent communication between those two processes.



Publisher container
-------------------

The publisher container will be built on top of the ``ecal-runtime`` container.
It will contain the Hello World Sample from the :ref:`Getting Started Section <getting_started_hello_world_publisher>`.

#. Create a file |fa-file-alt| :file:`pub_container/Dockerfile` and paste the following content:

   .. literalinclude:: src/ecal_in_docker/pub_container/Dockerfile
      :language: Docker
      :linenos:

#. Create publisher source code: |fa-file-alt| :file:`pub_container/main.cpp`

   .. literalinclude:: /getting_started/src/hello_world/hello_world_snd/main.cpp
      :language: c++
      :linenos:

#. Create file  |fa-file-alt| :file:`pub_container/CMakeLists.txt`
 
   .. literalinclude:: /getting_started/src/hello_world/hello_world_snd/CMakeLists.txt
      :language: cmake
      :linenos:

#. Build the image:

   .. code-block:: bash

      cd pub_container
      sudo docker build . --rm -t ecal-publisher:1.0.0

Subscriber container
--------------------

The subscriber container will also be based on the ``ecal-runtime`` container and contain the Hello World Sample from the :ref:`Getting Started Section <getting_started_hello_world_subscriber>`.

#. Create a file: |fa-file-alt| :file:`sub_container/Dockerfile`

   .. literalinclude:: src/ecal_in_docker/sub_container/Dockerfile
      :language: Docker
      :linenos:

#. Create subscriber source code: |fa-file-alt| :file:`sub_container/main.cpp`

   .. literalinclude:: /getting_started/src/hello_world/hello_world_rec/main.cpp
      :language: c++
      :linenos:

#. Create file |fa-file-alt| :file:`sub_container/CMakeLists.txt`
   
   .. literalinclude:: /getting_started/src/hello_world/hello_world_rec/CMakeLists.txt
      :language: cmake
      :linenos:
  
#. Build the image:

   .. code-block:: bash

      cd sub_container
      sudo docker build . --rm -t ecal-subscriber:1.0.0

Run the docker containers
-------------------------

* You can run the publisher and subscriber images manually with ``docker run``.

  .. code-block:: bash

     sudo docker run --rm -it --ipc=host --network=host --pid=host ecal-subscriber:1.0.0
     sudo docker run --rm -it --ipc=host --network=host --pid=host ecal-publisher:1.0.0

* You can also use the docker-compose file to manage multiple containers.

  #. In the parent folder create file |fa-file-alt| :file:`docker-compose.yaml` and paste the following content:
     
     .. literalinclude:: src/ecal_in_docker/docker-compose.yaml
        :language: yaml
        :linenos:

  #. You can now use that docker-compose to build/run the publisher and subscriber containers:
    
     .. code-block:: bash

        sudo docker-compose build
        sudo docker-compose up