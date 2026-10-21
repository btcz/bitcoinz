BitcoinZ 2.3.1 is an emergency release. All node operators, mining pools and
exchanges must upgrade.

Notable changes
===============

Block 1,860,181 is invalid
--------------------------

Mainnet block 1,860,181
(`000001963db0caee911c3409e3d1f1b868cb3fbfacc7e7e7422d5d3b80cbb37d`) contains
five transactions without inputs. Each spends a Sprout JoinSplit with
`vpub_new` = 21,000,000,000 BTCZ and an all-zero proof, so together they
create 105,000,000,000 BTCZ that never existed. Earlier releases accepted the
block for two reasons:

- A block received from a peer was first checked with Sprout proof
  verification disabled, and that result was cached in `CBlock::fChecked`.
  The strict check in `ConnectBlock` then returned early and never verified
  the proofs. The same bug let block 1,789,967 through (issue #118).
- The per-block Sprout value sum is a 64-bit integer. Five withdrawals of
  21 billion BTCZ overflow it to a positive number, so the ZIP 209 turnstile
  saw the Sprout pool grow instead of going negative.

2.3.1 treats block 1,860,181 as invalid. The chain continues from block
1,860,180 (`000002a2a96c4cce6bea19220151116c1a7f6a9ce19fbce3c3286370adb3fd1e`).
All transactions confirmed after block 1,860,180 are rolled back. When a node
rewinds, it returns them to its mempool so they can be confirmed again on the
corrected chain. That works for transparent transactions and for shielded
transactions anchored at or before block 1,860,180. Shielded transactions
anchored to a later block refer to a note commitment tree that no longer
exists, so their wallets must send them again. Transactions that spend the
created coins are dropped.

Changes:

- Sprout proofs are verified even after a preliminary check (PR #120).
- JoinSplits are not allowed on mainnet from height 1,860,181. The Sprout
  pool held 0.447078 BTCZ before the incident, and that amount stays locked.
  Closing the pool removes this whole class of attack, including the
  overflow above.
- On first start, a node that has block 1,860,181 in its active chain marks
  it invalid and rewinds to block 1,860,180.
- Block 1,860,181 is rejected without banning the peer that sent it, and
  peers that announce blocks built on it are not banned either. They run an
  older release, and they can reconnect as soon as they upgrade.
- New checkpoint at block 1,860,180. This also lets a new node sync through
  block 1,789,967 from its peers.

Upgrading
---------

- **The first start takes longer than usual.** The rewind is deeper than the
  wallet's 100-block witness cache, so the node clears that cache and rescans
  the wallet from the genesis block, as it does with `-rescan`. On a test copy
  of a production node, the 115-block rewind took 35 seconds and the wallet
  rescan 4.5 minutes. Nodes started with `-disablewallet` skip the rescan.
- **Nodes that are not upgraded do not switch on their own.** When the
  corrected chain gets more work, they stop with "A block chain
  reorganization has been detected that would roll back ... blocks". Upgrade
  them to 2.3.1.
- **Mining pools:** upgrade first, and add the other upgraded pools with
  `addnode` so the corrected chain spreads quickly.
- **Exchanges:** keep deposits and withdrawals suspended until the BitcoinZ
  team announces that the network has switched to the corrected chain. Do
  not credit deposits confirmed on the invalid chain.
- `-reindex` still stops at block 1,789,967 unless it is started with
  `-experimentalaccept1789967=1`. See
  [historical compatibility](../historical-mainnet-compatibility.md).
