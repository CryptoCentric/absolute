# Absolute 0.12.3.2

Absolute will service the AbsoluteX platform designed to help advertising, promotions and events, with smart contracts that will have a global reach of network enablers. Our Proof of View network will allow agents, organisers and promoters the ability to reach a global audience and utilise their participation to achieve their desired media goals.

The network is driven by ultra-low transaction fees, reliable and fast transactions (10x faster than Bitcoin) which are supported by a large core network of Masternodes. 

Absolute Coin gives you the access to the wide range of options created by two-tier blockchain system (Standard and Proof of View nodes). InstaSend payments are near-instant and greatly practical for the network users. PrivateSend is designed and implemented to ensure that sources of payments cannot be tracked and identified. Additionally, MasterNode holders will soon be assigned voting rights to decide about the future of the network.


## Absolute Technical Details

| Area | Absolute Setting |
| ------ | ------ |
| Block Time | 1.5 minutes |
| Block Reward | 30 ABS |
| PoW Algorithm | Lyra2REv2 |
| MN Collateral | 2500 ABS |
| Rewards split | Miners 20% and MasterNodes 80% |
| Maximum supply | 52.5 M - 20.35 M Block Rewards and 32.15 M PoV |
| Difficulty retargeting algo | Dark Gravity Wave v3 | 

| Area | Proof Of View Setting |
| ------ | ------ |
| Platform | On Demand |
| Block Reward | Max 5% Fee of Block Volume |
| PoV Node / Escrow Node Collateral | TBA ABS |

For building please see INSTALL / proper files in the doc subfolder.

Masternode install scripts are supported on Ubuntu 16.04 and 18.04

Development Process
-------------------

The `master` branch is meant to be stable. Development is normally done in separate branches.
[Tags](https://github.com/absolute-community/absolute/tags) are created to indicate new official,
stable release versions of Absolute Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/qa) are installed) with: `qa/pull-tester/rpc-tests.py`

The Travis CI system makes sure that every pull request is built for Windows, Linux, and OS X, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Changes to translations as well as new translations can be submitted to
[Absolute Core's Transifex page](https://www.transifex.com/projects/p/absolute/).

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.

