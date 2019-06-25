LitecoinZ Core version 2.0.1 is now available from:

  <https://litecoinz.org/bin/litecoinz-core-2.0.1/>

This is a new minor version release, including new features, various bugfixes
and performance improvements, as well as updated translations.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/litecoinz-project/litecoinz/issues>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over `/Applications/LitecoinZ-Qt` (on Mac)
or `litecoinz`/`litecoinz-qt` (on Linux).

If your node has a txindex, the txindex db will be migrated the first time you
run 2.0.1 or newer, which may take up to a few hours. Your node will not be
functional until this migration completes.

Compatibility
==============

LitecoinZ Core is extensively tested on multiple operating systems using
the Linux kernel, macOS 10.10+, and Windows 7 and newer (Windows XP is not supported).

LitecoinZ Core should also work on most other Unix-like systems but is not
frequently tested on them.

From 2.0.1 onwards macOS <10.10 is no longer supported. 2.0.1 is built using Qt 5.9.x, which doesn't
support versions of macOS older than 10.10.

Notable changes
===============

Sprout note validation bug fixed in wallet
------------------------------------------
LitecoinZ includes a fix for a bug in the wallet which could result in Sprout
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

Cross-compilation
-----------------
LitecoinZ can be compiled using the same source for the following targets:
* Linux
* macOS
* Windows

Gitian building
---------------
Gitian is the deterministic build process that is used to build the LitecoinZ
Core executables. It provides a way to be reasonably sure that the
executables are really built from the git source. It also makes sure that
the same, tested dependencies are used and statically built into the executable.

Multiple developers build the source code by following a specific descriptor
("recipe"), cryptographically sign the result, and upload the resulting signature.
These results are compared and only if they match, the build is accepted and provided
for download.

2.0.1 change log
=================

- f7ca3e9 Display download percentage while downloading params files
- a7fe542 [doc] Add 2.0.1 release note
- 27d2f3f [doc] Add zcash release notes
- cd529d7 Version 2.0.1
- 9851916 Issue#1643: Coinselection prunes extraneous inputs from ApproximateBestSubset
- a12500a Checks for null data transaction before issuing error to debug.log
- 223fa7e [walletdb] Fix syntax error in key parser
- b20c0b8 Update the wallet best block marker when pruning
- f526fc7 Correctly report high-S violations
- 96db9a1 Update secp256k1 subtree to latest upstream
- 2a55b33 GCC-7 and glibc-2.27 compat code
- 381b5c3 [net] Fix sent reject messages for blocks and transactions
- a4e07bd [P2P] Ensure headers count is correct
- fd4caef Mark blocks with too many sigops as failed
- 8cbe47d Add missing help for args
- 78941d4 Update checkpoints
- 093ada3 [gitian] Disable gtest dependency build
- 2682ca5 [depends] Add NO_GTEST option
- 74cd1f9 Cleanup
- b3890a4 Update ISSUE_TEMPLATE.md
- 3a24c9e Update COPYING
- b6fa551 Update CONTRIBUTING.md
- f353b51 Update INSTALL.md
- 175e19c Update share
- 2df18d9 Update README.md
- 90f6e1b Update LitecoinZ URLs
- 1be19f1 [build] Add univalue missing files
- 796c767 [build] Fix macOS libsnark build
- e9fa208 Add estimatedheight to getblockchaininfo
- f5f6dd4 Add confirmations to z_listreceivedbyaddress
- dba07d9 [depends] Add missing qt patches
- 38c0239 Fix enable-debug build DB_COINS undefined
- 1476070 Fix rpc-tests permissions
- e39c242 Fix z_mergetoaddress sending from ANY_SPROUT/ANY_SAPLING when the wallet contains both note types
- e7ca47f Fix help message of RPC getwalletinfo
- c6c729b Fix new HD seed generation for previously-encrypted wallets
- 324bcdd Fix unprotected coinbase transfer
- 5a9d90c Add -addressindex changes for bitcore insight block explorer
- aa2e97a Use a custom error type if creating joinsplit descriptions fails
- 5eb1df0 Add Sprout support to TransactionBuilder
- 9ab2a97 [build] Build improvement
- 8d19ac5 Update univalue
- 282620c Update leveldb
- 1b2f8a4 Update secp256k1
- 002fb74 [contrib] Update gitian building
- 151284f [doc] Updates
- 603d65e [depends] Update packages.mk
- e418500 [build] Update build-aux
- 2add770 [depends] Update qt to 5.9.7
- 4a1ad47 Update .gitignore
- 808a2e4 [contrib] Update macdeploy
- ce40c18 [contrib] Update init
- b9d9bf6 [contrib] Update devtools
- 182816a [contrib] Update 'gen_base58_test_vectors.py'
- 1bb2d8d [depends] Support additional cross-compilation targets in Rust
- 3d524b7 [depends] Update rust to 1.32.0
- 92318ad [depends] Switch to python3
- 06fd7c8 [depends] Add cross compiling support for native_protobuf
- 4f3ee4c [depends] Update native_mac_alias to 2.0.7
- 4f3451d [depends] Update native_biplist to 1.0.3
- d6825d7 [depends] Use clang 8.0.0
- 506cd0d [depends] Add cross compiling support for bdb
- 95fde93 [depends] Update boost to 1.69.0
- f35b758 [depends] Remove native_ccache
- dd9645f [depends] Use libxcb 1.10
- 7111429 [depends] Update libsodium to 1.0.17
- 13e0049 [depends] Use libXext 1.3.2
- c5c7e78 [depends] Add cross compiling support for libXau
- 67141c6 [depends] Use libX11 1.6.2
- 645d9fe [depends] Use freetype 2.7.1
- 6e76c13 [depends] Update expat to 2.2.6
- 218dd81 [depends] Use dbus 1.10.18
- 083f4ca [depends] Add cross compiling support for curl
- a583df4 [depends] Add cross compiling support for openssl
- 4c1e986 [depends] Add cross compiling support for protobuf
- 05e7be8 [depends] Use qrencode 3.4.4
- 7d999e7 [depends] Use xcb_proto 1.10
- 8625b73 [depends] Add cross compiling support for xextproto
- 407ee45 [depends] Use xproto 7.0.26
- f2658dc [depends] Use xtrans 1.3.4
- 07f61bf [depends] Update zmq to 4.3.1
- 69631d2 [depends] Update build dependencies engine
- 3af7e17 Update logo and icons
- 91f5b8c Add block delay penalty
- ab84211 Add missing header in 'src/Makefile.am'
- ed60113 Merge from zcash 2.0.4
- f63da05 Remove 'addrgen' utility
- 61a8f6a Fix wrong EXT_SECRET_KEY on Testnet and RegTest
- 801cd13 Merge pull request #27 from LitecoinZ/master
- d18f48b Fix wrong EXT_SECRET_KEY
- d77ff04 Remove i686 building from gitian
- c093ceb Remove ci-workers folder
- aa82310 Gitian Build
- e5570db Merge branch 'master' into gitian
- fd17c2a Update gitian-linux.yml
- e549a6b Remove non-ascii characters from help text
- 388903a Add release notes for 2.0.1-rc1
- 9c7b7af Update version to 2.0.1-rc1
- ea03595 Merge from LitecoinZ master
- bad12ea Update contrib folder
- 7275644 Gitian build
- 4481bd5 RPC sendrawtransaction returns error if tx expiring soon
- 25cf136 Update qa tests broken by expiring soon threshold
- e66ee49 RPC createrawtransaction returns error if tx expiring soon
- d1860f7 Don't propagate txs which are expiring soon in p2p messages
- 66c0208 Mitigate potential tx expiry height related DoS vector
- 3e2feac Add support for 'notfound' message to mininode
- 2db52f3 Adjust z_mergetoaddress assertions in Sapling rpc test
- e305d4d Add additional z_mergetoaddress parameter tests
- f2fdc46 Better error messages when sending between Sprout and Sapling
- e2e3551 Add locking for Sapling notes
- f0e9b68 Add Sapling support to z_mergetoaddress
- bb0d42e Extend Sprout mergetoaddress rpc test to also work for Sapling
- 2943c42 Add fail method to rpc test utils
- 1bb8a18 Fix z_mergetoaddress parameter tests
- 1263ef7 Make sapling the default for z_getnewaddress RPC
- 792dc75 Update README.md

Credits
=======

Thanks to everyone who directly or indirectly contributed to this release.
