.. include:: /include.txt

.. _edit_documentation:

======================
Edit the documentation
======================

This documentation is created with `Sphinx <https://www.sphinx-doc.org/>`_\ .
Sphinx uses the reStructuredText (RST) markup format and adds features like cross-referencing.

In this tutorial we will teach you how to edit the documentation locally on your PC.
For that we will use Visual Studio Code with Esbonio, which provides Sphinx diagnostics and a live preview.

.. hint::  
   For small changes you can click |fa-github|/|fa-pencil-alt|:guilabel:`suggest edit` button in the top right corner.
   GitHub however will only render the files as plain RST and miss all features that Sphinx adds.

.. rst-class:: bignums-xxl

#. Checkout the eCAL git repository: 

   .. code-block:: console
      
      git clone https://github.com/eclipse-ecal/ecal.git

#. Download and install Visual Studio Code on your PC (it is available for Windows, Linux and macOS).

   https://code.visualstudio.com/

#. Download and install any Python (3.10 and up) and make sure it is added to the ``PATH``.

   - |fa-windows| Windows:

     - https://www.python.org/downloads/
     - Choose "Add Python to PATH"

   - |fa-ubuntu| Ubuntu:
     
     .. code-block:: bash
        
        sudo apt install python3 python3-pip python3-wheel

#. Navigate to the eCAL documentation directory and create a Python virtual environment:

   - |fa-windows| Windows:

     .. code-block:: batch

        cd ecal\doc\rst
        py -m venv .venv
        .venv\Scripts\activate
        python -m pip install --upgrade pip wheel
        python -m pip install -r ..\requirements.txt

   - |fa-ubuntu| Ubuntu:

     .. code-block:: bash
     
        cd ecal/doc/rst
        python3 -m venv .venv
        source .venv/bin/activate
        python -m pip install --upgrade pip wheel
        python -m pip install -r ../requirements.txt

#. Install the VS Code Extensions:

   - ``Python`` by Microsoft
   - ``Esbonio`` by Esbonio Developers

#. Create the :file:`doc/rst/.vscode/settings.json` and paste the following code:

   .. code-block:: json

      {
          "esbonio.sphinx.pythonCommand": [
              "${venv:.venv}"
          ],
          "esbonio.sphinx.buildArguments": [
              "-M",
              "html",
              ".",
              "_build"
          ]
      }

#. Open the :file:`doc/rst/` directory in VS Code and select the Python interpreter from :file:`doc/rst/.venv`.
   Open an ``.rst`` file and wait for Esbonio to start the Sphinx process.

   .. hint::

      If the preview does not start after changing :file:`settings.json`, press :kbd:`Ctrl` + :kbd:`Shift` + :kbd:`P` and run :guilabel:`Esbonio: Restart Language Server` and :guilabel:`Esbonio: Restart Sphinx Process` from the command palette. 

   .. image:: img_documentation/vscode_statusbar.svg
      :alt: VS Code Statusbar with Sphinx and Esbonio

#. Click the preview button in the upper right corner.
   The preview will be updated every time you save the ``.rst`` file.
 
   .. image:: img_documentation/vscode_preview_button.svg
      :alt: VS Code Preview button

   .. image:: img_documentation/vscode_preview.png
      :alt: VS Code Preview
