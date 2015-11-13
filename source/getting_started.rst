===============
Getting Started
===============

.. 2015/11/13: initial version

Prerequisites
=============

Review Documentation
--------------------

- All documentation is available `online <https://yahoo.github.io/zeus/>`_


Checkout the Source Config
--------------------------

.. code-block:: shell

   # for example https://github.com/yahoo/zeus/
   mkdir ~/src && cd ~/src
   git clone git@github.com:yahoo/zeus.git
   cd zeus/

Build or Install the Compiler
-----------------------------

.. code-block:: shell
   
   $ make main

.. note:: If you are using a RedHat-based or Debian-based Linux distro we will begin distributing packages soon

Run a Development Server
========================

There is a nodejs-based server to host the compiled configuration on your dev box.
This server runs locally and updates whenever the filesystem changes. Running it
in the background can help validate that the changes look correct.

.. note:: You might want to run this in a new terminal window

.. code-block:: shell

   $ make run
   ./mon conf/* '`find ./conf -type f -name \*.yaml 2>/dev/null | sort -u;`'
   5424: old priority 0, new priority 19
   File is /tmp/tmp.55VPmZmx9F
   1446540458 Restarting server.

Then once you modify the filesystem you might see a message like:

.. code-block:: shell

   1446540538 Files changed. Restarting server.

Make a Change
=============

Now that you are running a server, you are ready to make changes and see updates
in real-time. Modify the configuration in ``conf/master.yaml`` as follows:

.. code-block:: yaml

  # ...
     # my cool integer!
      volume_max: !!int 10
  # ...

Change the "volume_max" value to "11" (because 11 is better).

Verify the Change
=================

Once your change is saved you can run tests like so:

.. code-block:: shell

   $ make test

You can also verify the config on your local dev box by visiting this url to see
the JSON output:

http://localhost:4080

