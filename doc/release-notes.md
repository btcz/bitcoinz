Notable changes
===============

Fixes
-----

Resolved a critical bug that prevented the program from starting up correctly in some cases.
The issue occurred during the startup process when the program attempted to rescan the latest indexed blocks.
A failure in the LoadBlockIndex() function, caused by a corrupted index file, was identified as the root cause.

Upgrading
=========

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over or bitcoinzd (on Linux).

If you are upgrading from version v2.0.9, due a bug on that version, to fix
a chainstate database corruption, you need to run this release with `-reindex`
option to rebuild the chainstate data structures. This will take anywhere from
30 minutes to several hours, depending on the speed of your machine.

If you are upgrading from a version prior to v2.0.9, then the '-reindex' operation
is not necessary.

On Windows, do not forget to uninstall all earlier versions of the Bitcoin
client first.
