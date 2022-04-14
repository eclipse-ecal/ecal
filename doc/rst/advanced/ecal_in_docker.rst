.. include:: /include.txt

.. _ecal_in_docker:

eCAL In Docker
===============

Here we will show how to deploy eCAL into a docker container, and how to use its image from other containers.

Prerequisite
-------------
* Installed Docker
   * install docker from repository: `Docker Instalation <https://docs.docker.com/engine/install/ubuntu/#install-using-the-repository>`_ 
   * If you have problems regarding to the installation, check these links:
      * `docker package not found <https://stackoverflow.com/questions/61401626/docker-installation-failed-on-ubuntu-20-04-ltsvmware>`_
      * `docker installation failed <https://forums.docker.com/t/cant-install-docker-on-ubuntu-20-04/93058>`_



Getting Started
----------------
#. File Hierarchy that we are going to follow:
   
   .. parsed-literal::

     |fa-folder-open|
     ├─ |fa-folder-open| ecal_in_docker
     │  ├─ |fa-file-alt| :download:`docker-compose.yaml <src/ecal_in_docker/docker-compose.yaml>`
     │  └─ |fa-folder-open| ecal_runtime_container
     |  |  └─ |fa-file-alt| :download:`Dockerfile <src/ecal_in_docker/ecal_runtime_container/Dockerfile>`
     |  └─ |fa-folder-open| pub_container
     |  |  └─ |fa-file-alt| :download:`Dockerfile <src/ecal_in_docker/pub_container/Dockerfile>`
     |  |  └─ |fa-file-alt| :download:`CMakeLists.txt <src/ecal_in_docker/pub_container/CMakeLists.txt>`
     |  |  └─ |fa-file-alt| :download:`main.cpp <src/ecal_in_docker/pub_container/main.cpp>`
     └─ └─ |fa-folder-open| sub_container
           └─ |fa-file-alt| :download:`Dockerfile <src/ecal_in_docker/sub_container/Dockerfile>`
           └─ |fa-file-alt| :download:`CMakeLists.txt <src/ecal_in_docker/sub_container/CMakeLists.txt>`
           └─ |fa-file-alt| :download:`main.cpp <src/ecal_in_docker/sub_container/main.cpp>`


  

#. Create a container for the eCAL runtime:
    * Create a folder: ecal_runtime_container
    * Create a file: Dockerfile
    * Add installation steps of ecal to the   |fa-file-alt| :file:`Dockerfile`:

      .. literalinclude:: src/ecal_in_docker/ecal_runtime_container/Dockerfile
         :language: Docker
         :linenos:
 
    * Build the image:
 
      .. code-block:: batch

         sudo docker build . --rm -t ecal-runtime

#. The eCAL image is ready now, so you can depend on it in your images.
   So, let's create 2 containers that act as a publisher and a subscriber,
   both of them will depend on the eCAL runtime image.

#. Publisher container:
    * Create a folder: pub_container
    * Create a file:  |fa-file-alt| :file:`Dockerfile`

      .. literalinclude:: src/ecal_in_docker/pub_container/Dockerfile
         :language: Docker
         :linenos:

    * Create publisher source code: |fa-file-alt| :file:`main.cpp`

      .. literalinclude:: src/ecal_in_docker/pub_container/main.cpp
         :language: c++
         :linenos:
 
    * Create file  |fa-file-alt| :file:`CMakeLists.txt`
     
      .. literalinclude:: src/ecal_in_docker/pub_container/CMakeLists.txt
         :language: cmake
         :linenos:

    * Build the image:
 
      .. code-block:: batch

         sudo docker build . --rm -t ecal-publisher:1.0.0

#. Subscriber container:
    * Create a folder: sub_container
    * Create a file:  |fa-file-alt| :file:`Dockerfile`

      .. literalinclude:: src/ecal_in_docker/sub_container/Dockerfile
         :language: Docker
         :linenos:

    * Create subscriber source code: |fa-file-alt| :file:`main.cpp`

      .. literalinclude:: src/ecal_in_docker/sub_container/main.cpp
         :language: c++
         :linenos:

    * Create file  |fa-file-alt| :file:`CMakeLists.txt`
     
      .. literalinclude:: src/ecal_in_docker/sub_container/CMakeLists.txt
         :language: cmake
         :linenos:
    
    * Build the image:
 
      .. code-block:: batch

         sudo docker build . --rm -t ecal-subscriber:1.0.0

#. You have built the publisher and subscriber images, then run them:
    .. code-block:: batch

         sudo docker run --rm -it --ipc=host --network host ecal-subscriber:1.0.0
         sudo docker run --rm -it --ipc=host --network host ecal-publisher:1.0.0

#. You can use the docker-compose file to manage multiple containers.
    * In the parent folder create file:  |fa-file-alt| :file:`docker-compose.yaml`
     
      .. literalinclude:: src/ecal_in_docker/docker-compose.yaml
         :language: yaml
         :linenos:

    * You can now use that docker-compose to build/run the publisher and subscriber containers:
    
      .. code-block:: batch

        sudo docker-compose build
        sudo docker-compose up

That was guidelines about how to use eCAL inside a container, and how your containers can make use of it.