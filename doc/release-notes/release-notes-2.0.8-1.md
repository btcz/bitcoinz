Notable changes
===============

Debug.log file Flooding (low impact) Update
---------------------------------------------

By the node version 2.0.7-10 we added the possibility to add an OP code with an Hex encoded string in the transactions that also give an unexpected output format if it is used. This unexpected output format is `OP_RETURN _<HEX-STRING>_` .

Now, when the Insight option is activated, the node try to find the spent index information about this `OP_RETURN` code that are not existing as the spent amount is zero (0). And this trigger an error in the debug.log file for each Raw DATA.

Error in the log file for Insight : `ERROR: Unable to get spent index information`
Check https://github.com/btcz/bitcoinz/issues/71 for more details.

Changelog
=========

Marcelus (3)
    In the wallet class make a test to know if it's an TX_NULL_DATA.
    In the main class make a test to know if outputValue is zero (0) so no spent info possible.
    Update version to 2.0.8-1
