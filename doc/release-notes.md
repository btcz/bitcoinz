(note: this is a temporary file, to be added-to by anybody, and moved to
release-notes at release time)

Notable changes
===============

Sprout validation cache and historical mainnet compatibility
----------------------------------------------------------

Block validation no longer caches a skipped Sprout proof check as a completed
strict check. Blocks without Sprout JoinSplits retain the existing fast path.

An experimental, mainnet-only option, `experimentalaccept1789967=1`, permits
the known invalid Sprout proof in one exact historical block and transaction.
It is disabled by default. Without this option, strict validation continues to
reject that block and cannot synchronize through it on the existing mainnet
chain. This option acknowledges existing history; it does not make the proof
valid or establish that the withdrawal was authorized. Do not deploy the cache
fix as a mainnet recovery release without deciding the historical policy.

See [historical compatibility and its release test plan](historical-mainnet-compatibility.md)
for the exact scope, configuration, limitations and required release checks.
