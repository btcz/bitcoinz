Notable changes
===============


This release fixes a security issue described at
https://z.cash/support/security/announcements/security-announcement-2019-09-24/ .

The service period of this release is shorter than normal due to the upcoming
v2.1.0 Blossom release. The End-of-Service of v2.0.7 will occur at block height
775000, expected to be on mid AUG-2020.


Debian Stretch is now a Supported Platform
------------------------------------------

We now provide reproducible builds for Stretch as well as for Jessie.


Fixed a bug in ``z_mergetoaddress``
-----------------------------------

We fixed a bug which prevented users sending from ``ANY_SPROUT`` or ``ANY_SAPLING``
with ``z_mergetoaddress`` when a wallet contained both Sprout and Sapling notes.


Insight Explorer
----------------

We have been incorporating changes to support the Insight explorer directly from
``bitcoinzd``. v2.0.6 includes the first change to an RPC method. If ``bitcoinzd`` is
run with the flag ``--insightexplorer``` (this requires an index rebuild), the
RPC method ``getrawtransaction`` will now return additional information about
spend indices.


Shorter Block Times
-------------------

Shorter block times are coming to BitcoinZ! In the v2.0.7 release we have implemented [ZIP208](https://github.com/zcash/zips/blob/master/zip-0208.rst) which will take effect when Blossom activates. Upon activation, the block times for BitcoinZ will decrease from 150 seconds to 75 seconds, and the block reward will decrease accordingly. This affects at what block height halving events will occur, but should not affect the overall rate at which BitcoinZ is mined. The total Community' reward stays the same, and the total supply of BitcoinZ is decreased only microscopically due to rounding. The BitcoinZ community has not yet made the decision when Blossom will be activated.


Blossom Activation on Testnet
-----------------------------
The v2.0.7 release includes Blossom activation on testnet, bringing shorter block times. The testnet Blossom activation height is not defined yet.


Insight Explorer
----------------
Changes needed for the Insight explorer have been incorporated into BitcoinZ. *This is an experimental feature* and therefore is subject to change. To enable, add the `experimentalfeatures=1`, `txindex=1`, and `insightexplorer=1` flags to `bitcoinz.conf`. This feature adds several RPCs to `bitcoinzd` which allow the user to run an Insight explorer.


Testnet Blossom Rewind
----------------------

Testnet users needed to upgrade to 2.0.7 before Blossom activated. The amount
of notice given to these users was brief, so many were not able to upgrade in
time. These users may now be on the wrong branch. v2.0.7-1 adds an "intended
rewind" to prevent having to manually reindex when reconnecting to the correct
chain.


Insight Explorer Logging Fix
----------------------------

Fixed an issue where `ERROR: spent index not enabled` would be logged unnecessarily.



Pre-Blossom EOS Halt
--------------------

v2.0.7-2 contains a shortened EOS halt so that is in alignment with v2.0.7.



Shrinking of debug.log files is temporarily disabled
----------------------------------------------------

In previous versions, `bitcoinzd` would shrink the `debug.log` file to 200 KB on
startup if it was larger than 10 MB. This behaviour, and the `-shrinkdebugfile`
option that controlled it, has been disabled.


Changelog
=========

Marcelus (1)
      Incorporation of changes into BTCZ

Charlie O'Keefe (1):
      Add stretch to list of suites in gitian linux descriptors

Daira Hopwood (9):
      Closes #3992. Remove obsolete warning message.
      make-release.py: Versioning changes for 2.0.6-rc1.
      make-release.py: Updated manpages for 2.0.6-rc1.
      make-release.py: Updated release notes and changelog for 2.0.6-rc1.
      ld --version doesn't work on macOS.
      Tweak author aliases.
      Add coding declaration to zcutil/release-notes.py
      make-release.py: Versioning changes for 2.0.6.
      make-release.py: Updated manpages for 2.0.6.

Daniel Kraft (1):
      Add some const declarations where they are appropriate.

Eirik Ogilvie-Wigley (1):
      Notable changes for 2.0.6

Eirik Ogilvie-Wigley (7):
      Fix tree depth in comment
      Update author aliases
      Remove old mergetoaddress RPC test
      Replace CSproutNotePlaintextEntry with SproutNoteEntry to match Sapling
      z_getmigrationstatus help message wording change
      Fix z_mergetoaddress sending from ANY_SPROUT/ANY_SAPLING when the wallet contains both note types
      Clarify what combinations of from addresses can be used in z_mergetoaddress

Jack Grigg (10):
      Move Equihash parameters into consensus params
      Globals: Remove Zcash-specific Params() calls from main.cpp
      Globals: Explicitly pass const CChainParams& to IsStandardTx()
      Globals: Explicit const CChainParams& arg for main:
      Globals: Explicitly pass const CChainParams& to ContextualCheckTransaction()
      Globals: Explicit const CChainParams& arg for main:
      Globals: Explicitly pass const CChainParams& to DisconnectBlock()
      Consistently use chainparams and consensusParams
      Globals: Explicitly pass const CChainParams& to IsInitialBlockDownload()
      Globals: Explicitly pass const CChainParams& to ReceivedBlockTransactions()

Jorge Timón (6):
      Globals: Explicit Consensus::Params arg for main:
      Globals: Make AcceptBlockHeader static (Fix #6163)
      Chainparams: Explicit CChainParams arg for main (pre miner):
      Chainparams: Explicit CChainParams arg for miner:
      Globals: Remove a bunch of Params() calls from main.cpp:
      Globals: Explicitly pass const CChainParams& to UpdateTip()

Larry Ruane (1):
      add spentindex to getrawtransaction RPC results

Marco Falke (1):
      [doc] Fix doxygen comments for members

Mary Moore-Simmons (1):
      Fixes issue #3504: Changes to --version and adds a couple other useful commands.

Peter Todd (1):
      Improve block validity/ConnectBlock() comments

Simon Liu (1):
      Fix typo and clean up help message for RPC z_getmigrationstatus.

Wladimir J. van der Laan (1):
      Break circular dependency main ↔ txdb

face (2):
      Pass CChainParams to DisconnectTip()
      Explicitly pass CChainParams to ConnectBlock

Benjamin Winston (1):
      Fixes #4013, added BitcoinABC as a disclosure partner

Daira Hopwood (3):
      Closes #3992. Remove obsolete warning message.


Daniel Kraft (1):
      Add some const declarations where they are appropriate.

Eirik Ogilvie-Wigley (7):
      Fix tree depth in comment
      Update author aliases
      Remove old mergetoaddress RPC test
      Replace CSproutNotePlaintextEntry with SproutNoteEntry to match Sapling
      z_getmigrationstatus help message wording change
      Fix z_mergetoaddress sending from ANY_SPROUT/ANY_SAPLING when the wallet contains both note types
      Clarify what combinations of from addresses can be used in z_mergetoaddress

Jack Grigg (10):
      Move Equihash parameters into consensus params
      Globals: Remove Zcash-specific Params() calls from main.cpp
      Globals: Explicitly pass const CChainParams& to IsStandardTx()
      Globals: Explicit const CChainParams& arg for main:
      Globals: Explicitly pass const CChainParams& to ContextualCheckTransaction()
      Globals: Explicit const CChainParams& arg for main:
      Globals: Explicitly pass const CChainParams& to DisconnectBlock()
      Consistently use chainparams and consensusParams
      Globals: Explicitly pass const CChainParams& to IsInitialBlockDownload()
      Globals: Explicitly pass const CChainParams& to ReceivedBlockTransactions()

Jorge Timón (6):
      Globals: Explicit Consensus::Params arg for main:
      Globals: Make AcceptBlockHeader static (Fix #6163)
      Chainparams: Explicit CChainParams arg for main (pre miner):
      Chainparams: Explicit CChainParams arg for miner:
      Globals: Remove a bunch of Params() calls from main.cpp:
      Globals: Explicitly pass const CChainParams& to UpdateTip()

Larry Ruane (1):
      add spentindex to getrawtransaction RPC results

MarcoFalke (1):
      [doc] Fix doxygen comments for members

Peter Todd (1):
      Improve block validity/ConnectBlock() comments

Simon Liu (1):
      Fix typo and clean up help message for RPC z_getmigrationstatus.

Wladimir J. van der Laan (1):
      Break circular dependency main ↔ txdb

face (2):
      Pass CChainParams to DisconnectTip()
      Explicitly pass CChainParams to ConnectBlock

Benjamin Winston (1):
      Fixes #4013, added BitcoinABC as a disclosure partner

Daira Hopwood (10):
      Add MIT license to Makefiles
      Add MIT license to build-aux/m4/bitcoin_* scripts, and a permissive license to build-aux/m4/l_atomic.m4 .
      Update copyright information for Zcash, leveldb, and libsnark.
      Add license information for Autoconf macros. refs #2827
      Update contrib/debian/copyright for ax_boost_thread.m4
      Replace http with https: in links to the MIT license. Also change MIT/X11 to just MIT, since no distinction was intended.
      qa/zcash/checksec.sh is under a BSD license, with a specialized non-endorsement clause.
      Link to ticket #2827 using URL
      Release process doc: add step to set the gpg key id.
      Release process doc: mention the commit message.

Dimitris Apostolou (5):
      Rename vjoinsplit to vJoinSplit
      Fix naming inconsistency
      Rename joinsplit to shielded
      Rename FindWalletTx to FindWalletTxToZap
      Fix RPC undefined behavior.

Eirik Ogilvie-Wigley (47):
      Make nextHeight required in CalculateNextWorkRequired
      Fix nondeterministic failure in sapling migration test
      Clean up and fix typo
      Apply suggestions from code review
      Shorter block times rpc test
      Update pow_tests for shorter block times
      Update test_pow for shorter block times
      Update block subsidy halving for zip208
      Make NetworkUpgradeAvailable a method of Params
      Temporarily disable test
      Simplify PartitionCheck
      Use static_assert
      Add missing new line at end of file
      pow test cleanup
      Add message to static_assert
      Update expiry height for shorter block times
      Fix zip208 founders reward calculation and update test
      PartitionCheck tests for shorter block times
      Add test for Blossom default tx expiry delta
      Update metrics block height estimation for shorter block times
      Do not create transactions that will expire after the next epoch
      Do not send migration transactions that would expire after a network upgrade
      Fix integer truncation in Blossom halving calculation
      Update main_tests for shorter block times
      Use pre-Blossom max FR height when calculating address change interval
      Make founders reward tests pass before and after Blossom activation height is set
      Extract Halvings method and add tests
      Add comments and fix typos
      Improve EstimateNetHeight calculation
      Fix check transaction tests
      Make sure to deactivate blossom in test case
      Fix parsing txexpirydelta argument
      Do not add expiring soon threshold to expiry height of txs near NU activation
      Fix/update comments
      Make sure that expiry height is not less than height
      Clarify documentation
      Update PoW related assertions
      Remove DefaultExpiryDelta method
      Algebraic improvements related to halving
      Distinguish between height and current header height on metrics screen
      Test clean up and fixes
      Add copyright info
      NPE defense in metrics screen
      Do not estimate height if there is no best header
      Rename method and use int64_t
      make-release.py: Versioning changes for 2.0.7-rc1.
      make-release.py: Updated manpages for 2.0.7-rc1.

Eirik Ogilvie-Wigley (8):
      Use CommitTransaction() rather than sendrawtransaction()
      Move reused async rpc send logic to separate file
      Move reused sign and send logic
      Do not shadow the return value when testmode is true
      Inline sign_send_raw_transaction
      Allow passing optional reserve key as a parameter to SendTransaction
      Use reserve key for transparent change when sending to Sapling
      Fix comment in mergetoaddress RPC test

Jack Grigg (4):
      test: Check for change t-addr reuse in z_sendmany
      Use reserve key for transparent change when sending to Sprout
      test: Fix permissions on wallet_changeaddresses RPC test
      test: Fix pyflakes warnings

Larry Ruane (6):
      add addressindex related RPCs
      add spentindex RPC for bitcore block explorer
      add timestampindex related RPC getblockhashes
      fix getblockdeltas documentation formatting
      insightexplorer minor bug fixes
      insightexplorer fix LogPrintf

Luke Dashjr (2):
      Add MIT license to autogen.sh and share/genbuild.sh
      Trivial: build-aux/m4/l_atomic: Fix typo

Simon Liu (8):
      Redefine PoW functions to accept height parameter.
      Remove use of redundant member nPowTargetSpacing.
      Replace nPoWTargetSpacing -> PoWTargetSpacing()
      Update PoW function calls to pass in height.
      Update GetBlockTimeout() to take height parameter.
      Replace nPoWTargetSpacing -> PoWTargetSpacing() in ProcessMessage()
      Replace nPoWTargetSpacing -> PoWTargetSpacing() in tests
      Modify PartitionCheck to be aware of pre & post Blossom target spacing.

William M Peaster (1):
      Handful of copyedits to README.md

codetriage-readme-bot (1):
      Link to development guidelines in CONTRIBUTING.md

Jack Grigg (1):
      Update README.md

Daira Hopwood (11):
      Add MIT license to Makefiles
      Add MIT license to build-aux/m4/bitcoin_* scripts, and a permissive license to build-aux/m4/l_atomic.m4 .
      Update copyright information for Zcash, leveldb, and libsnark.
      Add license information for Autoconf macros. refs #2827
      Update contrib/debian/copyright for ax_boost_thread.m4
      Replace http with https: in links to the MIT license. Also change MIT/X11 to just MIT, since no distinction was intended.
      qa/zcash/checksec.sh is under a BSD license, with a specialized non-endorsement clause.
      Link to ticket #2827 using URL
      Release process doc: add step to set the gpg key id.
      Release process doc: mention the commit message.
      Add RPC test and test framework constants for Sapling->Blossom activation.

Dimitris Apostolou (5):
      Rename vjoinsplit to vJoinSplit
      Fix naming inconsistency
      Rename joinsplit to shielded
      Rename FindWalletTx to FindWalletTxToZap
      Fix RPC undefined behavior.

Eirik Ogilvie-Wigley (56):
      Make nextHeight required in CalculateNextWorkRequired
      Fix nondeterministic failure in sapling migration test
      Clean up and fix typo
      Apply suggestions from code review
      Shorter block times rpc test
      Update pow_tests for shorter block times
      Update test_pow for shorter block times
      Update block subsidy halving for zip208
      Make NetworkUpgradeAvailable a method of Params
      Temporarily disable test
      Simplify PartitionCheck
      Use static_assert
      Add missing new line at end of file
      pow test cleanup
      Add message to static_assert
      Update expiry height for shorter block times
      Fix zip208 founders reward calculation and update test
      PartitionCheck tests for shorter block times
      Add test for Blossom default tx expiry delta
      Update metrics block height estimation for shorter block times
      Do not create transactions that will expire after the next epoch
      Do not send migration transactions that would expire after a network upgrade
      Fix integer truncation in Blossom halving calculation
      Update main_tests for shorter block times
      Use pre-Blossom max FR height when calculating address change interval
      Make founders reward tests pass before and after Blossom activation height is set
      Extract Halvings method and add tests
      Add comments and fix typos
      Improve EstimateNetHeight calculation
      Fix check transaction tests
      Make sure to deactivate blossom in test case
      Fix parsing txexpirydelta argument
      Do not add expiring soon threshold to expiry height of txs near NU activation
      Fix/update comments
      Make sure that expiry height is not less than height
      Clarify documentation
      Update PoW related assertions
      Remove DefaultExpiryDelta method
      Algebraic improvements related to halving
      Distinguish between height and current header height on metrics screen
      Test clean up and fixes
      Add copyright info
      NPE defense in metrics screen
      Do not estimate height if there is no best header
      Rename method and use int64_t
      make-release.py: Versioning changes for 2.0.7-rc1.
      make-release.py: Updated manpages for 2.0.7-rc1.
      make-release.py: Updated release notes and changelog for 2.0.7-rc1.
      Update download path
      Set testnet Blossom activation height
      Notable changes for v2.0.7
      Enable shorter block times rpc test
      Grammatical fixes and improvements
      Remove constant
      make-release.py: Versioning changes for 2.0.7.
      make-release.py: Updated manpages for 2.0.7.

Eirik Ogilvie-Wigley (8):
      Use CommitTransaction() rather than sendrawtransaction()
      Move reused async rpc send logic to separate file
      Move reused sign and send logic
      Do not shadow the return value when testmode is true
      Inline sign_send_raw_transaction
      Allow passing optional reserve key as a parameter to SendTransaction
      Use reserve key for transparent change when sending to Sapling
      Fix comment in mergetoaddress RPC test

Jack Grigg (5):
      test: Check for change t-addr reuse in z_sendmany
      Use reserve key for transparent change when sending to Sprout
      test: Fix permissions on wallet_changeaddresses RPC test
      test: Fix pyflakes warnings
      test: Fix AuthServiceProxy closed conn detection

Larry Ruane (6):
      add addressindex related RPCs
      add spentindex RPC for bitcore block explorer
      add timestampindex related RPC getblockhashes
      fix getblockdeltas documentation formatting
      insightexplorer minor bug fixes
      insightexplorer fix LogPrintf

Luke Dashjr (2):
      Add MIT license to autogen.sh and share/genbuild.sh
      Trivial: build-aux/m4/l_atomic: Fix typo

Simon Liu (8):
      Redefine PoW functions to accept height parameter.
      Remove use of redundant member nPowTargetSpacing.
      Replace nPoWTargetSpacing -> PoWTargetSpacing()
      Update PoW function calls to pass in height.
      Update GetBlockTimeout() to take height parameter.
      Replace nPoWTargetSpacing -> PoWTargetSpacing() in ProcessMessage()
      Replace nPoWTargetSpacing -> PoWTargetSpacing() in tests
      Modify PartitionCheck to be aware of pre & post Blossom target spacing.

William M Peaster (1):
      Handful of copyedits to README.md

codetriage-readme-bot (1):
      Link to development guidelines in CONTRIBUTING.md

Jack Grigg (1):
      Update README.md

Benjamin Winston (3):
      Updated location to new download server
      Fixes 4097, improves caching on parameter downloads
      Updated location to new download server, fixing #4100

Alex Tsankov (2):
      Update INSTALL
      Typo Fix

Daira Hopwood (1):
      Add intended rewind for Blossom on testnet.

Eirik Ogilvie-Wigley (6):
      Notable changes for v2.0.7-1
      fix typo
      Add note about logging fix
      Better wording in release notes
      make-release.py: Versioning changes for 2.0.7-1.
      make-release.py: Updated manpages for 2.0.7-1.

Larry Ruane (2):
      #4114 don't call error() from GetSpentIndex()
      better fix: make GetSpentIndex() consistent with others...

Eirik Ogilvie-Wigley (3):
      Notable changes for v2.0.7-2
      make-release.py: Versioning changes for 2.0.7-2.
      make-release.py: Updated manpages for 2.0.7-2.

Daira Hopwood (4):
      Add hotfix release notes.
      Make a note of the shorter service period in the release notes.
      make-release.py: Versioning changes for 2.0.7-3.
      make-release.py: Updated manpages for 2.0.7-3.

Jack Grigg (6):
      Disable -shrinkdebugfile command
      SetMerkleBranch: remove unused code, remove cs_main lock requirement
      Remove cs_main lock requirement from CWallet::SyncTransaction
      Ignore exceptions when deserializing note plaintexts
      Move mempool SyncWithWallets call into its own thread
      Enable RPC tests to wait on mempool notifications
