.. include:: /include.txt

.. _getting_started_hello_world_multilanguage:

===========
Hello World
===========

In this section you will learn how to create a simple Hello World application using eCAL.
You will see how it is done in the major programming languages we support: C++, C, C# and Python.

=====================
Hello World Publisher
=====================

Let's begin with the publisher side of our Hello World application. 

`Some more text to make it nice to read. Goethe shall be proud`

.. tabs::

    .. tab:: C++

        .. literalinclude:: /source_code_samples/cpp/hello_world/hello_send/src/hello_send.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. tab:: C
        
        .. literalinclude:: /source_code_samples/c/hello_world/hello_send/src/hello_send.c
            :language: cpp
            :linenos:
            :lines: 20-

    .. tab:: C#

        .. literalinclude:: /source_code_samples/csharp/hello_world/string_snd_csharp.cs
            :language: cpp
            :linenos:
            :lines: 31-

    .. tab:: Python

        .. literalinclude:: /source_code_samples/python/hello_world/string_snd.py
            :language: cpp
            :linenos:
            :lines: 19-

======================
Hello World Subscriber
======================

Now let's have a look at the subscriber side:

.. tabs::

    .. tab:: C++

        .. literalinclude:: /source_code_samples/cpp/hello_world/hello_receive/src/hello_receive.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. tab:: C
        
        .. literalinclude:: /source_code_samples/c/hello_world/hello_receive/src/hello_receive.c
            :language: cpp
            :linenos:
            :lines: 20-

    .. tab:: C#

        .. literalinclude:: /source_code_samples/csharp/hello_world/string_rec_csharp.cs
            :language: cpp
            :linenos:
            :lines: 31-

    .. tab:: Python

        .. literalinclude:: /source_code_samples/python/hello_world/string_rec.py
            :language: cpp
            :linenos:
            :lines: 19-