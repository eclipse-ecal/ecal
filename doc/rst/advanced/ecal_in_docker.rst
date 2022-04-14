.. include:: /include.txt

.. _ecal_in_docker:

==============
eCAL In Docker
==============

Here we will show how to deploy eCAL into a docker container, and how to use its image from other containers.


.. important::
   
   This will work with eCAL 5.10 and up.
   Older eCAL versions will lack Shared Memory communication when being run in a docker container.

Prerequisite
============

* Installed Docker

  * Install docker from repository: `Docker Instalation <https://docs.docker.com/engine/install/ubuntu/#install-using-the-repository>`_ 

  .. seealso::

     * If you have problems regarding to the installation, check these links:

       * `Docker package not found <https://stackoverflow.com/questions/61401626/docker-installation-failed-on-ubuntu-20-04-ltsvmware>`_      
       * `Docker installation failed <https://forums.docker.com/t/cant-install-docker-on-ubuntu-20-04/93058>`_



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
   |  ├─ |fa-file-alt| :download:`CMakeLists.txt <src/ecal_in_docker/pub_container/CMakeLists.txt>`
   |  └─ |fa-file-alt| :download:`main.cpp <src/ecal_in_docker/pub_container/main.cpp>`
   |
   └─ |fa-folder-open| sub_container
      ├─ |fa-file-alt| :download:`Dockerfile <src/ecal_in_docker/sub_container/Dockerfile>`
      ├─ |fa-file-alt| :download:`CMakeLists.txt <src/ecal_in_docker/sub_container/CMakeLists.txt>`
      └─ |fa-file-alt| :download:`main.cpp <src/ecal_in_docker/sub_container/main.cpp>`

#. Create a container for the eCAL runtime and build it:

   * Create the file |fa-file-alt| :file:`ecal_runtime_container/Dockerfile` and paste the following installation steps:

     .. literalinclude:: src/ecal_in_docker/ecal_runtime_container/Dockerfile
        :language: Docker
        :linenos:
 
   * Build the image:
 
     .. code-block:: bash

        cd ecal_in_docker
        sudo docker build . --rm -t ecal-runtime

   The eCAL image is ready now, so you can depend on it in your images.
   So, let's create 2 containers that act as a publisher and a subscriber,
   both of them will depend on the eCAL runtime image.

#. Create the **Publisher container**:

   * Create a file |fa-file-alt| :file:`pub_container/Dockerfile` and paste the following content:

     .. literalinclude:: src/ecal_in_docker/pub_container/Dockerfile
        :language: Docker
        :linenos:

   * Create publisher source code: |fa-file-alt| :file:`pub_container/main.cpp`

     .. literalinclude:: src/ecal_in_docker/pub_container/main.cpp
        :language: c++
        :linenos:

   * Create file  |fa-file-alt| :file:`pub_container/CMakeLists.txt`
   
     .. literalinclude:: src/ecal_in_docker/pub_container/CMakeLists.txt
        :language: cmake
        :linenos:

   * Build the image:

     .. code-block:: bash

        cd pub_container
        sudo docker build . --rm -t ecal-publisher:1.0.0

#. Create the **Subscriber container**:

   * Create a file: |fa-file-alt| :file:`sub_container/Dockerfile`

     .. literalinclude:: src/ecal_in_docker/sub_container/Dockerfile
        :language: Docker
        :linenos:

   * Create subscriber source code: |fa-file-alt| :file:`sub_container/main.cpp`

     .. literalinclude:: src/ecal_in_docker/sub_container/main.cpp
        :language: c++
        :linenos:

   * Create file |fa-file-alt| :file:`sub_container/CMakeLists.txt`
     
     .. literalinclude:: src/ecal_in_docker/sub_container/CMakeLists.txt
        :language: cmake
        :linenos:
    
   * Build the image:
 
     .. code-block:: bash

        cd sub_container
        sudo docker build . --rm -t ecal-subscriber:1.0.0

#. You have built the publisher and subscriber images, then run them:

   .. code-block:: bash

      sudo docker run --rm -it --ipc=host --network host ecal-subscriber:1.0.0
      sudo docker run --rm -it --ipc=host --network host ecal-publisher:1.0.0

#. You can use the docker-compose file to manage multiple containers.

   * In the parent folder create file:  |fa-file-alt| :file:`docker-compose.yaml`
     
     .. literalinclude:: src/ecal_in_docker/docker-compose.yaml
        :language: yaml
        :linenos:

   * You can now use that docker-compose to build/run the publisher and subscriber containers:
    
     .. code-block:: bash

        sudo docker-compose build
        sudo docker-compose up

That was guidelines about how to use eCAL inside a container, and how your containers can make use of it.