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

Removal of time adjustment and the -maxtimeadjustment= option
-------------------------------------------------------------

Prior to v2.1.0, `bitcoinzd` would adjust the local time that it used by up
to 70 minutes, according to a median of the times sent by the first 200 peers
to connect to it. This mechanism was inherently insecure, since an adversary
making multiple connections to the node could effectively control its time
within that +/- 70 minute window (this is called a "timejacking attack").

In the v2.1.0 security release, in addition to other mitigations for
timejacking attacks, the maximum time adjustment was set to zero by default.
This effectively disabled time adjustment; however, a `-maxtimeadjustment=`
option was provided to override this default.

As a simplification the time adjustment code has now been completely removed,
together with `-maxtimeadjustment=`. Node operators should instead simply
ensure that local time is set reasonably accurately.

If it appears that the node has a significantly different time than its peers,
a warning will still be logged and indicated on the metrics screen if enabled.

Option handling
---------------

- The `-reindex` and `-reindex-chainstate` options now imply `-rescan`
  (provided that the wallet is enabled and pruning is disabled, and unless
  `-rescan=0` is specified explicitly).

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


### RPC and REST

RPC Changes
-----------

- The `estimatepriority` RPC call has been removed.
- The `priority_delta` argument to the `prioritisetransaction` RPC call now has
  no effect and must be set to a dummy value (0 or null).

Changes to Transaction Fee Selection
------------------------------------

- The `-sendfreetransactions` option has been removed. This option used to
  instruct the wallet's legacy transaction creation APIs (`sendtoaddress`,
  `sendmany`, and `fundrawtransaction`) to use a zero fee for "small" transactions
  that spend "old" inputs. It will now cause a warning on node startup if used.

Changes to Block Template Construction
--------------------------------------

- The block template construction algorithm no longer favours transactions that
  were previously considered "high priority" because they spent older inputs. The
  `-blockprioritysize` config option, which configured the portion of the block
  reserved for these transactions, has been removed and will now cause a warning
  if used.

Removal of Priority Estimation
------------------------------

- Estimation of "priority" needed for a transaction to be included within a target
  number of blocks, and the associated `estimatepriority` RPC call, have been
  removed. The format for `fee_estimates.dat` has also changed to no longer save
  these priority estimates. It will automatically be converted to the new format
  which is not readable by prior versions of the software.

### Configuration and command-line options

### Block and transaction handling

### P2P protocol and network code

The p2p alert system has been removed in #7692 and the 'alert' message is no longer supported.

Direct headers announcement (BIP 130)
-------------------------------------

Between compatible peers, BIP 130 direct headers announcement is used. This
means that blocks are advertized by announcing their headers directly, instead
of just announcing the hash. In a reorganization, all new headers are sent,
instead of just the new tip. This can often prevent an extra roundtrip before
the actual block is downloaded.


Fee filtering of invs (BIP 133)
------------------------------------

The optional new p2p message "feefilter" is implemented and the protocol
version is bumped to 70013. Upon receiving a feefilter message from a peer,
a node will not send invs for any transactions which do not meet the filter
feerate. [BIP 133](https://github.com/bitcoin/bips/blob/master/bip-0133.mediawiki)

### Validation

### Build system

### Wallet

### GUI

### Tests

### Miscellaneous

