Absolute Core version 0.12.2.5
==========================

Release is now available from:

  <hhttps://github.com/absolute-community/absolute/releases>

Please report bugs using the issue tracker at github:

  <https://github.com/absolute-community/absolute/issues>


Upgrading and downgrading
=========================

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over /Applications/absolute-Qt (on Mac) or
absoluted/absolute-qt (on Linux).

Downgrade warning
-----------------

### Downgrade to a version < 0.12.3.1

Downgrading to a version smaller than 0.12.3.1 is only supported as long as AIP2/AIP3
has not been activated. Activation will happen when enough miners signal compatibility
through a BIP9 (bit 3) deployment.

Notable changes
===============

AIP0002 - Special Transactions
------------------------------
Currently, new features and consensus mechanisms have to be implemented on top of the restrictions
imposed by the simple nature of transactions. Since classical transactions can only carry inputs
and outputs, they are most useful for financial transactions (i.e. transfers of quantities of Absolute
between addresses on the distributed ledger). These inputs and outputs carry scripts and signatures
which are used to authorize and validate the transaction.

To implement new on-chain features and consensus mechanisms which do not fit into this concept of
financial transactions, it is often necessary to misuse the inputs/outputs and their corresponding
scripts to add additional data and meaning to a transaction. For example, new opcodes would have
to be introduced to mark a transaction as something special and add a payload. In other cases,
OP_RETURN has been misused to store data on-chain.

The introduction of special transactions will require the whole Absolute ecosystem to perform a one-time
mandatory update of all the software and libraries involved. Software and libraries will have to be
changed so that they can differentiate between classical transactions and special transactions.
Deserialization of a classical transaction remains unchanged. Deserialization of a special transaction
requires the software/library to at least implement skipping and ignoring the extra_payload field.
Validation and processing of the inputs and outputs of a special transaction must remain identical to
classical transactions.

AIP0003 - Deterministic Masternode Lists
----------------------------------------
This AIP provides on-chain consensus for masternode lists that in turn allow for deterministic quorum
derivation and service scoring of masternode rewards.

In the previous system, each node maintained its own individual masternode list. Masternodes gained
entry to that masternode list after the owner created a 1000 Absolute UTXO and the masternode broadcast
a "masternode broadcast/announcement" P2P message. This in turn set the masternode to a PRE_ENABLED
state in the list maintained by each node. Masternodes then regularly broadcasted ping messages to
keep the masternode in ENABLED state.

The previous system was maintained with consensus mechanisms that predated Satoshi Nakamoto’s solution
to the Byzantine Generals Problem. This meant that each node needed to maintain their own individual
masternode list with P2P messages and not a blockchain based solution. Due to the nature of the P2P
system, there was no guarantee that nodes would come to the same conclusion on what the masternode
list ought to look like. Discrepancies might, for example, occur due to a different order of message
reception or if messages had not been received at all. This posed some risks in regard to consensus
and limited the possible uses of quorums by the system.

Additionally, the previous system required a complicated and failure prone "masternode sync" after
the initial startup of the node. After the blockchain was synced, the node would request the current
masternode list, the reward payment votes, and then verify the received list. This process tended to
take an unnecessarily long amount of time and sometimes resulted in failure.

In the new system, the masternode list is derived entirely from information found on-chain. New
masternodes are added by new special transactions called Provider Registration Transactions
(abbreviated as ProRegTx). They are only removed by spending the collateral. A ProRegTx is a special
transaction which includes either a 1000-Absolute collateral payment or a reference to it, along with
other payload information (AIP0002).

The new system is going to be activated via combination of a BIP9-like deployment (bit 3) and new spork
(`SPORK_15_DETERMINISTIC_MNS_ENABLED`).

AIP0004 - Simplified Verification of Deterministic Masternode Lists
-------------------------------------------------------------------
A verifiable and correct masternode list is foundational to many Absolute features, including verification
of an InstantSend transaction, mixing in PrivateSend and many features of Evolution. The deterministic
masternode lists introduced by AIP0003 enable full derivation and verification of a masternode list via
on-chain data. This, however, requires the full chain to be available to construct or verify this list.
A SPV client does not have the full chain and thus would have to rely on a list provided by one or more
nodes in the network. This provided list must be verifiable by the SPV client without needing the full
chain. This AIP proposes additions to the block’s coinbase transaction and new P2P messages to get and
update a masternode list with additional proof data.

Read more: https://github.com/Absolutepay/AIPs/blob/master/AIP-0004.md

Mining
------
Please note that masternode payments in `getblocktemplate` rpc are now returned as an array and not as
a single object anymore. Make sure to apply corresponding changes to your pool software.

Also, deterministic masternodes can now set their payout address to a P2SH address. The most common use
case for P2SH is multisig but script can be pretty much anything. If your pool software doesn't recognize
P2SH addresses, the simplest way to fix it is to use `script` field which shows scriptPubKey for each
entry of masternode payments array in `getblocktemplate`.

And finally, after AIP0003 activation your pool software must be able to produce Coinbase Special
Use `coinbase_payload` from `getblocktemplate` to get extra payload needed to construct this transaction.

PrivateSend
-----------
With further refactoring of PrivateSend code it became possible to implement mixing in few parallel
mixing sessions at once from one single wallet. You can set number of mixing sessions via
`privatesendsessions` cmd-line option or Absolute.conf. You can pick any number of sessions between 1 and 10,
default is 4 which should be good enough for most users. For this feature to work you should also make
sure that `privatesendmultisession` is set to `1` via cmd-line or `Enable PrivateSend multi-session` is
enabled in GUI.

Introducing parallel mixing sessions should speed mixing up which makes it reasonable to add a new
mixing denom (0.00100001 Absolute) now while keeping all the old ones too. It also makes sense to allow more
mixing rounds now, so the new default number of rounds is 4 and the maximum number of rounds is 16 now.

You can also adjust rounds and amount via `setprivatesendrounds` and `setprivatesendamount` RPC commands
which override corresponding cmd-line params (`privatesendrounds` and `privatesendamount` respectively).

NOTE: Introducing the new denom and a couple of other changes made it incompatible with mixing on
masternodes running on pre-0.13 software. Please keep using 0.12.3 local wallet to mix your coins until
there is some significant number of masternodes running on version 0.13 to make sure you have enough
masternodes to choose from when the wallet picks one to mix funds on.

InstantSend
-----------
With further improvements of networking code it's now possible to handle more load, so we are changing
InstantSend to be always-on for so called "simple txes" - transactions with 4 or less inputs. Such
transactions will be automatically locked even if they only pay minimal fee. According to stats, this
means that up to 90% of currently observed transactions will became automatically locked via InstantSend
with no additional cost to end users or any additional effort from wallet developers or other service
providers.

This feature is going to be activated via combination of a BIP9-like deployment (we are reusing bit 3)
and new spork (`SPORK_16_INSTANTSEND_AUTOLOCKS`).

Historically, InstantSend transactions were shown in GUI and RPC with more confirmations than regular ones,
which caused quite a bit of confusion. This will no longer be the case, instead we are going to show real
blockchain confirmations only and a separate indicator to show if transaction was locked via InstantSend
or not. For GUI it's color highlight and a new column, for RPC commands - `instantlock` field and `addlocked`
param.

One of the issues with InstantSend adoption by SPV wallets (besides lack of Deterministic Masternode List)
was inability to filter all InstantSend messages the same way transactions are filtered. This should be
fixed now and SPV wallets should only get lock votes for transactions they are interested in.

Another popular request was to preserve information about InstantSend locks between wallet restarts, which
is now implemented. This data is stored in a new cache file `instantsend.dat`. You can safely remove it,
if you don't need information about recent transaction locks for some reason (NOTE: make sure it's not one
of your wallets!).

We also added new ZMQ notifications for double-spend attempts which try to override transactions locked
via InstantSend - `zmqpubrawinstantsenddoublespend` and `zmqpubhashinstantsenddoublespend`.

Sporks
------
There are a couple of new sporks introduced in this version `SPORK_15_DETERMINISTIC_MNS_ENABLED` (block
based) and `SPORK_16_INSTANTSEND_AUTOLOCKS` (timestamp based). There is aslo `SPORK_17_QUORUM_DKG_ENABLED`
(timestamp based) which is going to be used on testnet only for now.

Spork data is stored in a new cache file (`sporks.dat`) now.

Governance
----------
Introduction of Deterministic Masternodes requires replacing of the old masternode private key which was used
both for operating a MN and for voting on proposals with a set of separate keys, preferably fresh new ones.
This means that votes casted for proposals by Masternode Owners via the old system will no longer be valid
after AIP0003 activation and must be re-casted using the new voting key.

Also, you can now get notifications about governance objects or votes via new ZMQ notifications:
`zmqpubhashgovernancevote`, `zmqpubhashgovernanceobject`, `zmqpubrawgovernancevote` and
`zmqpubhashgovernanceobject`.

GUI changes
-----------
Masternodes tab has a new section dedicated to AIP0003 registered masternodes now. After AIP0003 activation
this will be the only section shown here, the two old sections for non-deterministic masternodes will no
longer be available.

There are changes in the way InstantSend transactions are displayed, see `InstantSend` section above.


RPC changes
-----------
There are a few changes in existing RPC interfaces in this release:
- `gobject prepare` allows to send proposal transaction as an InstantSend one and also accepts an UTXO reference to spend;
- `masternode status` and `masternode list` show some AIP0003 related info now;
- `previousbits` and `coinbase_payload` fields were added in `getblocktemplate`;
- `getblocktemplate` now returns an array for masternode payments instead of a single object (miners and mining pools have to upgrade their software to support multiple masternode payees);
- masternode and superblock payments in `getblocktemplate` show payee scriptPubKey in `script` field in addition to payee address in `payee`;
- `getblockchaininfo` shows BIP9 deployment progress;
- `help command subCommand` should give detailed help for subcommands e.g. `help protx list`;
- `compressed` option in `masternode genkey`;
- `dumpwallet` shows info about dumped wallet and warns user about security issues;
- `instantlock` field added in output of `getrawmempool`, `getmempoolancestors`, `getmempooldescendants`, `getmempoolentry`,
`getrawtransaction`, `decoderawtransaction`, `gettransaction`, `listtransactions`, `listsinceblock`;
- `addlocked` param added to `getreceivedbyaddress`, `getreceivedbyaccount`, `getbalance`, `sendfrom`, `sendmany`,
`listreceivedbyaddress`, `listreceivedbyaccount`, `listaccounts`.

### Downgrade to 0.12.2.5

Downgrading to 0.12.2.2 does not require any additional actions, should be
fully compatible.

Notable changes
===============

Initial release.
