Notable changes
===============

Private viewing keys (PVK)
--------------------------

Implement Sapling viewing keys zcash/zcash#3060
Add support for importing and exporting sapling ivk #3060 zcash/zcash#3822

Changelog
=========

Marcelus (9):
      Incorporation of ZEC changes into BTCZ
      - Add z_gettreestate RPC
      - Fix the use of 0 as expiryheight in createrawtransaction, to disable expiry
      - Bitcoin 0.12 wallet PRs 1
      - Fix GHSA-xwpp-3gx9-68rr
      - Wallet interface refactor
      - Update z_viewtransaction
      - insightexplorer: LOCK(cs_main) during rpcs
      - Return address and type of imported key in z_importkey
