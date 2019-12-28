Notable changes
===============

Sprout note validation bug fixed in wallet
------------------------------------------
We include a fix for a bug in the LitecoinZ wallet which could result in Sprout
z-addresses displaying an incorrect balance. Sapling z-addresses are not
impacted by this issue. This would occur if someone sending funds to a Sprout
z-address intentionally sent a different amount in the note commitment of a
Sprout output than the value provided in the ciphertext (the encrypted message
from the sender).

Users should install this update and then rescan the blockchain by invoking
“litecoinzd -rescan”. Sprout address balances shown by the litecoinzd wallet should
then be correct.

Miner address selection behaviour fixed
---------------------------------------
LitecoinZ inherited a bug from upstream Bitcoin Core where both the internal miner
and RPC call `getblocktemplate` would use a fixed transparent address, until RPC
`getnewaddress` was called, instead of using a new transparent address for each
mined block.  This was fixed in Bitcoin 0.12 and we have now merged the change.

Miners who wish to use the same address for every mined block, should use the
`-mineraddress` option. 

Bitcoin 0.12 Performance Improvements
-------------------------------------
This change makes sigcache faster, more efficient, and larger. It also reduces
the number of database lookups when processing new transactions.

Sprout to Sapling Migration Tool
--------------------------------
This release includes the addition of a tool that will enable users to migrate
shielded funds from the Sprout pool to the Sapling pool while minimizing
information leakage. 

The migration can be enabled using the RPC `z_setmigration` or by including
`-migration` in the `litecoinz.conf` file. Unless otherwise specified funds will be
migrated to the wallet's default Sapling address; it is also possible to set the 
receiving Sapling address using the `-migrationdestaddress` option in `litecoinz.conf`.

New consensus rule: Reject blocks that violate turnstile
--------------------------------------------------------
In this release the consensus rules were changed to enforce a consensus rule
which marks blocks as invalid if they would lead to a turnstile violation in
the Sprout or Shielded value pools.

Developers can use a new experimental feature `-developersetpoolsizezero` to test
Sprout and Sapling turnstile violations.

64-bit ARMv8 support
--------------------
Added ARMv8 (AArch64) support. This enables users to build litecoinz on even more
devices.

Fixed a bug in ``z_mergetoaddress``
-----------------------------------

We fixed a bug which prevented users sending from ``ANY_SPROUT`` or ``ANY_SAPLING``
with ``z_mergetoaddress`` when a wallet contained both Sprout and Sapling notes.

Insight Explorer
----------------

We have been incorporating changes to support the Insight explorer directly from
``litecoinzd``. v2.0.2 includes the first change to an RPC method. If ``litecoinzd`` is
run with the flag ``--insightexplorer``` (this requires an index rebuild), the
RPC method ``getrawtransaction`` will now return additional information about
spend indices.
