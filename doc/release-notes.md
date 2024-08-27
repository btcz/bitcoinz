Notable changes
===============

Reduce upload traffic
---------------------

A major part of the outbound traffic is caused by serving historic blocks to
other nodes in initial block download state.

It is now possible to reduce the total upload traffic via the `-maxuploadtarget`
parameter. This is *not* a hard limit but a threshold to minimize the outbound
traffic. When the limit is about to be reached, the uploaded data is cut by not
serving historic blocks (blocks older than one week).
Moreover, any SPV peer is disconnected when they request a filtered block.

This option can be specified in MiB per day and is turned off by default
(`-maxuploadtarget=0`).
The recommended minimum is 1152 * MAX_BLOCK_SIZE (currently 2304MB) per day.

Whitelisted peers will never be disconnected, although their traffic counts for
calculating the target.

A more detailed documentation about keeping traffic low can be found in
[/doc/reducetraffic.md](/doc/reducetraffic.md).

Fixes
-----

Resolved a critical bug that prevented the program from starting up correctly in some cases.
The issue occurred during the startup process when the program attempted to rescan the latest indexed blocks.
A failure in the LoadBlockIndex() function, caused by a corrupted index file, was identified as the root cause.

Changed command-line options
-----------------------------
- `-debuglogfile=<file>` can be used to specify an alternative debug logging file.

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
