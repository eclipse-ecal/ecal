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

.. rst-class:: bignums-xxl

#. Checkout the eCAL git repository: 

   .. code-block:: console
      
      git clone https://github.com/eclipse-ecal/ecal.git

#. Download and install Visual Studio Code on your PC (it is available for Windows, Linux and macOS).

   https://code.visualstudio.com/

#. Download and install any Python between 3.6 and 3.9 and make sure it is added to the ``PATH``

   .. important::

      Python 3.10 and up will not work at the moment, as we are still using Sphinx 3.5.

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

        pip install wheel esbonio
        pip install -r requirements.txt

   - |fa-ubuntu| Ubuntu:

     .. code-block:: bash.
     
        sudo pip3 install -r requirements.txt
        sudo pip3 install esbonio

#. Install the VS Code Extensions:

   - ``Python`` by Microsoft
   - ``reStructuredText`` by LeXStudio Inc.

     .. image:: img_documentation/vscode_rst_addon.svg
        :alt: VS Code reStructuredText extension

#. Create the :file:`doc/rst/.vscode/settings.json` and paste the following code:

   .. code-block:: json

      {
          "esbonio.sphinx.buildDir" : "${workspaceFolder}/_build/html",
          "esbonio.sphinx.confDir"  : "${workspaceFolder}",
          "esbonio.sphinx.srcDir"   : "${workspaceFolder}"
      }

#. Open the :file:`doc/rst/` directory in VS Code and click on an .rst file of your choice.
   Make sure that the statusbar displays "Sphinx" *(not docutils)*.
   If is doesn't, click on it and select "Sphinx".
   Wait for esbonio to finish building.

   .. image:: img_documentation/vscode_statusbar.svg
      :alt: VS Code Statusbar with Sphinx and Esbonio

   .. important:: 

      The reStructuredText extension may warn you about Syntax highlighting not being available.
      Install the extenstion and discard **any** warning it presents you (also check the bell icon in the statusbar).

      It sometimes happens that the extension will not launch, unless you have discarded all warnings.

#. Click the preview button in the upper right corner.
   The preview will be updated every time you save the .rst file.

   .. image:: img_documentation/vscode_preview_button.svg
      :alt: VS Code Preview button

   .. image:: img_documentation/vscode_preview.png
      :alt: VS Code Preview
