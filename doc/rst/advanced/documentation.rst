.. include:: /include.txt

.. _edit_documentation:

======================
Edit the documentation
======================

This documentation is created with `Sphinx <https://www.sphinx-doc.org/>`_\ .
Sphinx uses the reStructuredText (RST) markup format and adds some features like cross-referencing.

In this tutorial we will teach you how to edit the documentation locally on your PC.
For that we will use Visual Studio Code with the reStructuredText extension, that will give you a (somewhat) live preview.

.. hint::
   
   For small changes you can click the :guilabel:`Edit on GitHub` button in the top right corner.
   GitHub however will only render the files as plain RST and miss all features that Sphinx adds.

#. Checkout the eCAL git repository: 

   .. code-block:: console
      
      git clone https://github.com/continental/ecal.git

#. Download and install Visual Studio Code on your PC (it is available for Windows, Linux and macOS).

   https://code.visualstudio.com/

#. Download and install Python 3.5 or newer (Python 3.8 recommended) and make sure it is added to the ``PATH``

   - |fa-windows| Windows:

     - https://www.python.org/downloads/
     - Choose "Add Python to PATH"

   - |fa-ubuntu| Ubuntu:
     
     .. code-block:: bash
        
        sudo apt install python3 python3-pip python3-wheel

#. Navigate to the eCAL directory and install the python requirements with pip:

   .. note::
      
      Python virtual environments are not supported

   - |fa-windows| Windows:

     .. code-block:: batch

        pip install wheel
        pip install -r requirements.txt
        pip install sphinx-autobuild

   - |fa-ubuntu| Ubuntu:

     .. code-block:: bash
     
        sudo pip3 install -r requirements.txt
        sudo pip3 install sphinx-autobuild

#. Install the VS Code Extensions:

   - ``Python`` by Microsoft
   - ``reStructuredText`` by LeXStudio Inc.

     .. image:: img_documentation/vscode_rst_addon.svg
        :alt: VS Code reStructuredText extension

#. Open the :file:`doc/rst/` directory in VS Code.
   Open an .rst file of your choice and click the preview button in the upper right corner.
   If VS Code asks you how to create the preview, select ``Sphinx``

   .. image:: img_documentation/vscode_preview_button.svg
      :alt: VS Code Preview button

After a short time you should have a beautiful preview of the documentation.
The preview will be updated every time you save the .rst file.

.. image:: img_documentation/vscode_preview.png
   :alt: VS Code Preview
