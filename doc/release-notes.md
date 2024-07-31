Notable changes
===============

DoS Mitigation: Mempool Size Limit and Random Drop
--------------------------------------------------

This release adds a mechanism for preventing nodes from running out of memory
in the situation where an attacker is trying to overwhelm the network with
transactions. This is achieved by keeping track of and limiting the total
`cost` and `evictionWeight` of all transactions in the mempool. The `cost` of a
transaction is determined by its size in bytes, and its `evictionWeight` is a
function of the transaction's `cost` and its fee. The maximum total cost is
configurable via the parameter `mempooltxcostlimit` which defaults to
80,000,000 (up to 20,000 txs). If a node's total mempool `cost` exceeds this
limit the node will evict a random transaction, preferentially picking larger
transactions and ones with below the standard fee. To prevent a node from
re-accepting evicted transactions, it keeps track of ones that it has evicted
recently. By default, a transaction will be considered recently evicted for 60
minutes, but this can be configured with the parameter
`mempoolevictionmemoryminutes`.

For full details see ZIP 401.

Asynchronous Operations Incorrectly Reporting Success
-----------------------------------------------------
We fixed an issue where asynchronous operations were sometimes reporting sucess
when they had actually failed. One way this could occur was when trying to use
`z_sendmany` to create a transaction spending coinbase funds in a way where
change would be generated (not a valid use of `z_sendmany`). In this case the
operation would erroneously report success, and the only way to see that the
transaction had actually failed was to look in the `debug.log` file. Such
operations will now correctly report that they have failed.

Fake chain detection during initial block download
--------------------------------------------------

One of the mechanisms that `bitcoinzd` uses to detect whether it is in "initial
block download" (IBD) mode is to compare the active chain's cumulative work
against a hard-coded "minimum chain work" value. This mechanism (inherited from
Bitcoin Core) means that once a node exits IBD mode, it is either on the main
chain, or a fake alternate chain with similar amounts of work. In the latter
case, the node has most likely become the victim of a 50% + 1 adversary.

Starting from this release, `bitcoinzd` additionally hard-codes the block hashes
for the activation blocks of each past network upgrade (NU). During initial
chain synchronization, and after the active chain has reached "minimum chain
work", the node checks the blocks at each NU activation height against the
hard-coded hashes. If any of them do not match, the node will immediately alert
the user and **shut down for safety**.

Disabling old Sprout proofs
---------------------------

As part of our ongoing work to clean up the codebase and minimise the security
surface of `bitcoinzd`, we are removing `libsnark` from the codebase, and dropping
support for creating and verifying old Sprout proofs. Funds stored in Sprout
addresses are not affected, as they are spent using the hybrid Sprout circuit
(built using `bellman`) that was deployed during the Sapling network upgrade.

This change has several implications:

- `bitcoinzd` no longer verifies old Sprout proofs, and will instead assume they
  are valid. This has a minor implication for nodes: during initial block
  download, an adversary could feed the node fake blocks containing invalid old
  Sprout proofs, and the node would accept the fake chain as valid. However,
  as soon as the active chain contains at least as much work as the hard-coded
  "minimum chain work" value, the node will detect this situation and shut down.

- Shielded transactions can no longer be created before Sapling has activated.
  This does not affect BitcoinZ itself, but will affect downstream codebases that
  have not yet activated Sapling (or that start a new chain after this point and
  do not activate Sapling from launch). Note that the old Sprout circuit is
  [vulnerable to counterfeiting](https://z.cash/support/security/announcements/security-announcement-2019-02-05-cve-2019-7167/)
  and should not be used in current deployments.

- Starting from this release, the circuit parameters from the original Sprout
  MPC are no longer required to start `bitcoinzd`, and will not be downloaded by
  `fetch-params.sh`. They are not being automatically deleted at this time.

Option parsing behavior
-----------------------

Command line options are now parsed strictly in the order in which they are
specified. It used to be the case that `-X -noX` ends up, unintuitively, with X
set, as `-X` had precedence over `-noX`. This is no longer the case. Like for
other software, the last specified value for an option will hold.

Low-level RPC changes
---------------------

- Bare multisig outputs to our keys are no longer automatically treated as
  incoming payments. As this feature was only available for multisig outputs for
  which you had all private keys in your wallet, there was generally no use for
  them compared to single-key schemes. Furthermore, no address format for such
  outputs is defined, and wallet software can't easily send to it. These outputs
  will no longer show up in `listtransactions`, `listunspent`, or contribute to
  your balance, unless they are explicitly watched (using `importaddress` or
  `importmulti` with hex script argument). `signrawtransaction*` also still
  works for them.

View shielded information in wallet transactions
------------------------------------------------

In previous `bitcoinzd` versions, to obtain information about shielded transactions
you would use either the `z_listreceivedbyaddress` RPC method (which returns all
notes received by an address) or `z_listunspent` (which returns unspent notes,
optionally filtered by addresses). There were no RPC methods that directly
returned details about spends, or anything equivalent to the `gettransaction`
method (which returns transparent information about in-wallet transactions).

This release introduces a new RPC method `z_viewtransaction` to fill that gap.
Given the ID of a transaction in the wallet, it decrypts the transaction and
returns detailed shielded information for all decryptable new and spent notes,
including:

- The address that each note belongs to.
- Values in both decimal ZEC and zatoshis.
- The ID of the transaction that each spent note was received in.
- An `outgoing` flag on each new note, which will be `true` if the output is not
  for an address in the wallet.
- A `memoStr` field for each new note, containing its text memo (if its memo
  field contains a valid UTF-8 string).

Information will be shown for any address that appears in `z_listaddresses`;
this includes watch-only addresses linked to viewing keys imported with
`z_importviewingkey`, as well as addresses with spending keys (both generated
with `z_getnewaddress` and imported with `z_importkey`).

Build system
------------

- The `--enable-lcov`, `--disable-tests`, and `--disable-mining` flags for
  `zcutil/build.sh` have been removed. You can pass these flags instead by using
  the `CONFIGURE_FLAGS` environment variable. For example, to enable coverage
  instrumentation (thus enabling "make cov" to work), call:

  ```
  CONFIGURE_FLAGS="--enable-lcov --disable-hardening" ./zcutil/build.sh
  ```

- The build system no longer defaults to verbose output. You can re-enable
  verbose output with `./zcutil/build.sh V=1`
