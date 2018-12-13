// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2019 The Dash Core developers
// Copyright (c) 2018-2019 The Absolute Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include "arith_uint256.h"


#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;

    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

static CBlock CreatePoVNETGenesisBlock(const uint256 &prevBlockHash, const std::string& PoVNETName, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    assert(!PoVNETName.empty());

    CMutableTransaction txNew;
    txNew.nVersion = 4;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    // put height (BIP34) and povnet name into coinbase
    txNew.vin[0].scriptSig = CScript() << 1 << std::vector<unsigned char>(PoVNETName.begin(), PoVNETName.end());
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = CScript() << OP_RETURN;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock = prevBlockHash;
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}
/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
 *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
 *   vMerkleTree: e0028e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "2/12/2018 Coindesk - IMF Chief Lagarde: Global Cryptocurrency Regulation Is Inevitable";
    const CScript genesisOutputScript = CScript() << ParseHex("0474f442759ab0f44e4be5d224c95675b477388a830201131ac883bfadb4ab5852aa32373678c5b501f4742f365ffd8494d69d15fd805a4e0e6ffc4706106d99b8") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

static CBlock FindPoVNETGenesisBlock(const Consensus::Params& params, const CBlock &prevBlock, const CAmount& reward)
{
    std::string PoVNETName = GetPoVNETName();
    assert(!PoVNETName.empty());

    CBlock block = CreatePoVNETGenesisBlock(prevBlock.GetHash(), PoVNETName.c_str(), prevBlock.nTime + 1, 0, prevBlock.nBits, prevBlock.nVersion, reward);

    arith_uint256 bnTarget;
    bnTarget.SetCompact(block.nBits);

    for (uint32_t nNonce = 0; nNonce < UINT32_MAX; nNonce++) {
        block.nNonce = nNonce;

        uint256 hash = block.GetHash();
        if (UintToArith256(hash) <= bnTarget)
            return block;
    }

    // This is very unlikely to happen as we start the PoVNET with a very low difficulty. In many cases even the first
    // iteration of the above loop will give a result already
    error("FindPoVNETGenesisBlock: could not find PoVNET genesis block for %s", PoVNETName);
    assert(false);
}
/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */


class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 350400; // Note: actual number of blocks per calendar year
		/* masternodes */
        consensus.nMasternodePaymentsStartBlock = 500;
        consensus.nMasternodePaymentsIncreaseBlock = 6720; // start increasing after first week
        consensus.nMasternodePaymentsIncreasePeriod = 6720; // weekly increase
        consensus.nMasternodeMinimumConfirmations = 15;

		consensus.nInstantSendKeepLock = 24;

		/* budget */
		consensus.nBudgetPaymentsStartBlock = 98800;
        consensus.nBudgetPaymentsCycleBlocks = 7300; // ~(60*24*30)/1.5 / 4
        consensus.nBudgetPaymentsWindowBlocks = 100;
        consensus.nSuperblockStartBlock = 98800;
        consensus.nSuperblockStartHash = uint256S("00000000000c8a59f1d5e93403f5506bebcd3471d34d1feffe4ecfa2d69182bb");


        consensus.nSuperblockStartBlock = 98800;
        consensus.nSuperblockCycle = 7300; // ~(60*24*30)/1.5 / 4

		/* Governance */
		consensus.nGovernanceMinQuorum = 10;
        consensus.nGovernanceFilterElements = 20000;
        consensus.nMasternodeMinimumConfirmations = 15;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x00000de52875a68d7bf6a5bb5ad1b89fd7df4d67a9603669327949923dc74d7e");
        consensus.BIP65Height = 390000;
        consensus.BIP66Height = 395000;
        consensus.AIP0001Height = 450000;
        consensus.powLimit = uint256S("00000fffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 7.5 * 60; // 5 blocks
        consensus.nPowTargetSpacing = 1.5 * 60; // 1.5 minutes
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nPowDGWHeight = 150;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1486252800; // Feb 5th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517788800; // Feb 5th, 2018

        // Deployment of AIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nStartTime = 1508025600; // Oct 15th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nTimeout = 1539561600; // Oct 15th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nThreshold = 3226; // 80% of 4032

        // Deployment of BIP147
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 1561680000; // Friday, 28-Jun-19
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 1561680000; // Friday, 28-Jun-19
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nThreshold = 3226; // 80% of 4032

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000100000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00"); // always verify

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xcd;
        pchMessageStart[1] = 0xfb;
        pchMessageStart[2] = 0x64;
        pchMessageStart[3] = 0x43;
        vAlertPubKey = ParseHex("04139ff96fe352a5bee201a1b4665c5975c2f949abb7d13fc0ba00bd8d9e30205be35e3bdd41637925a80e8053b8225f30dfacb424b6a711bc6c82d8537143b85f");
        nDefaultPort = 18888;


        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1518598800, 1399438, 0x1e0ffff0, 1, 30 * COIN);

        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000de52875a68d7bf6a5bb5ad1b89fd7df4d67a9603669327949923dc74d7e"));
        assert(genesis.hashMerkleRoot == uint256S("0x12844a9cbf517654e272975506ab56af4d5c8dde0332a0ee48ba159c72daae03"));

        vSeeds.push_back(CDNSSeedData("absolutecoin.net", "seed1.absolutecoin.net"));
        vSeeds.push_back(CDNSSeedData("absolutecoin.net", "seed2.absolutecoin.net"));
        vSeeds.push_back(CDNSSeedData("absolutecoin.net", "seed3.absolutecoin.net"));

        // Absolute addresses start with 'A'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,23);
        // Absolute script addresses start with 's'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,125);
        // Absolute private keys start with 'h'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,110);
        // Absolute BIP32 pubkeys start with 'xpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        // Absolute BIP32 prvkeys start with 'xprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        // Absolute BIP44 coin type is '5'
        nExtCoinType = 5;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fRequireRoutableExternalIP = true;
        fMineBlocksOnDemand = false;
        fAllowMultipleAddressesFromGroup = false;
        fAllowMultiplePorts = false;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60*60; // fulfilled requests expire in 1 hour

        vSporkAddresses = {"AYZX23zmfNQgCZtzq7JsWHRKKyuzzxYHfD"};
        nMinSporkKeys = 1;
        fBIP9CheckMasternodesUpgraded = true;
        consensus.fLLMQAllowDummyCommitments = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (  0, uint256S("0x00000de52875a68d7bf6a5bb5ad1b89fd7df4d67a9603669327949923dc74d7e"))
            (  50000, uint256S("0x000000000124f25449c3561f9bb75baf9afb80e6fa5a7ef075b3887a9c820009"))
            (  150000, uint256S("0x00000000004c2812ed6d8ce5a695c67b64b5df40fcb43858e016f5e27ce244f6"))
            (  250000, uint256S("0x00000000000640d8897addd4200209b6dd754aa3ca1f39857f8f03f36a44cc02"))
            (  300000, uint256S("0x00000000000abec2c4fe6aec36d3f37026df8ed8817789a43278392092fe9d1a"))
            (  350400, uint256S("0x0000000000056195689db2edb979305163b7ed88c1e7ce8875bd2b5feb4c432d"))

	};

        chainTxData = ChainTxData{

            2813478628, // * UNIX timestamp of last known number of transactions
            476506,		// * total number of transactions between genesis and that timestamp
			//   (the tx=... number in the SetBestChain debug.log lines)
            1	//estimated number of transactions per second after that timestamp
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v4)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 500; // Blocks per year
        consensus.nMasternodePaymentsStartBlock = 400; // not true, but it's ok as long as it's less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsIncreaseBlock = 250; // increase after 6 weeks first
        consensus.nMasternodePaymentsIncreasePeriod = 350; // increase every 4 weeks afterwards
        consensus.nInstantSendKeepLock = 6;
        consensus.nBudgetPaymentsStartBlock =450;
        consensus.nBudgetPaymentsCycleBlocks = 150;
        consensus.nBudgetPaymentsWindowBlocks = 100;

        consensus.nSuperblockStartBlock = 450;
        consensus.nSuperblockStartHash = uint256(); // do not check this on testnet
        consensus.nSuperblockCycle = 150; // Superblocks can be issued hourly on testnet
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 500;
        consensus.nMasternodeMinimumConfirmations = 1;

        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x0000e585b5b736b3a33ae8999fa2d63e036fb42e56ea5b6e5eacf3b473dd4e6");
        consensus.BIP65Height = 800; //
        consensus.BIP66Height = 950; //

        consensus.powLimit = uint256S("00000fffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 1 * 60 * 60; // Absolute: 1 day
        consensus.nPowTargetSpacing = 1* 60; // Absolute: 1 minutes
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nPowDGWHeight = 100;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1506556800; // September 28th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1538092800; // September 28th, 2018

        // Deployment of AIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nStartTime = 1505692800; // Sep 18th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nTimeout = 1537228800; // Sep 18th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nThreshold = 50; // 50% of 100

        // Deployment of BIP147
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 1517792400; // Feb 5th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 1549328400; // Feb 5th, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nThreshold = 50; // 50% of 100

        // Deployment of AIP0003
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].nStartTime = 1593458430; // Monday, 29 June 2020 19:20:30
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].nTimeout = 1593458430; // Monday, 29 June 2020 19:20:30
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].nThreshold = 50; // 50% of 100

        // The best chain should have at least this much work.
        // not used currently
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000100000");

        // By default assume that the signatures in ancestors of this block are valid.
        // not used currently
        consensus.defaultAssumeValid = uint256S("0x00"); // always

        pchMessageStart[0] = 0xdb;
        pchMessageStart[1] = 0xfd;
        pchMessageStart[2] = 0x19;
        pchMessageStart[3] = 0x66;
        vAlertPubKey = ParseHex("043b5741a0fa2319111e30c32221a8d0de2eef7fc2a91e2cd230fe9c22c8279b97e6bfaa7f622574ae9171188429332b55730a58839ecdffb2ce2736aa859b0459");
        nDefaultPort = 17777;
        nPruneAfterHeight = 1500000;
/* old
        genesis = CreateGenesisBlock(1518597800, 519328, 0x1e0ffff0, 1, 30 * COIN);
        assert(consensus.hashGenesisBlock == uint256S("0x0000e585b5b736b3a33ae8999fa2d63e036fb42e56ea5b6e5eacf3b473dd4e6"));
        assert(genesis.hashMerkleRoot == uint256S("0x12844a9cbf517654e272975506ab56af4d5c8dde0332a0ee48ba159c72daae03"));
*/
        genesis = CreateGenesisBlock(1518598800, 1399438, 0x1e0ffff0, 1, 30 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0x00000de52875a68d7bf6a5bb5ad1b89fd7df4d67a9603669327949923dc74d7e"));
        assert(genesis.hashMerkleRoot == uint256S("0x12844a9cbf517654e272975506ab56af4d5c8dde0332a0ee48ba159c72daae03"));

        vFixedSeeds.clear();
        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        vSeeds.clear();
	// nodes with support for servicebits filtering should be at the top
		    vSeeds.push_back(CDNSSeedData("absolutecoin.net", "tseed1.absolutecoin.net"));
        vSeeds.push_back(CDNSSeedData("absolutecoin.net", "tseed2.absolutecoin.net"));
        vSeeds.push_back(CDNSSeedData("absolutecoin.net", "tseed3.absolutecoin.net"));


        // Testnet Absolute addresses start with 'y'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Testnet Absolute script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Testnet Absolute BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Testnet Absolute BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Testnet Absolute BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fRequireRoutableExternalIP = true;
        fMineBlocksOnDemand = true;
        fAllowMultipleAddressesFromGroup = false;
        fAllowMultiplePorts = false;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        vSporkAddresses = {"yQzniXSTqRwaEujSkvZicJsYZkt7wpa4NJ"};
        nMinSporkKeys = 1;
        fBIP9CheckMasternodesUpgraded = true;
        consensus.fLLMQAllowDummyCommitments = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (      0, uint256S("0x00000e585b5b736b3a33ae8999fa2d63e036fb42e56ea5b6e5eacf3b473dd4e6"))
        };

        chainTxData = ChainTxData{
            1518597800, // * UNIX timestamp of last checkpoint block
            0,		// * total number of transactions between genesis and last checkpoint
			//   (the tx=... number in the SetBestChain debug.log lines)
            100		// * estimated number of transactions per day after checkpoint
        };

    }
};
static CTestNetParams testNetParams;

/**
 * PoVNET
 */
class CPoVNETParams : public CChainParams {
public:
    CPoVNETParams() {
        strNetworkID = "PoV";
        consensus.nSubsidyHalvingInterval = 350400;
        consensus.nMasternodePaymentsStartBlock = 4010; // not true, but its ok as long as its less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsIncreaseBlock = 4030;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nInstantSendConfirmationsRequired = 2;
        consensus.nInstantSendKeepLock = 6;
        consensus.nBudgetPaymentsStartBlock = 4100;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nSuperblockStartBlock = 4200; // NOTE: Should satisfy nSuperblockStartBlock > nBudgetPeymentsStartBlock
        consensus.nSuperblockStartHash = uint256(); // do not check this on devnet
        consensus.nSuperblockCycle = 24; // Superblocks can be issued hourly on devnet
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 500;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.BIP34Height = 1; // BIP34 activated immediately on povnet
        consensus.BIP65Height = 1; // BIP65 activated immediately on povnet
        consensus.BIP66Height = 1; // BIP66 activated immediately on povnet
        consensus.AIP0001Height = 2; // AIP0001 activated immediately on povnet
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~uint256(0) >> 1
        consensus.nPowTargetTimespan = 24 * 60 * 60; // ABS: 1 day
        consensus.nPowTargetSpacing = 2 * 60; // ABS: 2 minutes
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;

        consensus.nPowDGWHeight = 100;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1506556800; // September 28th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1538092800; // September 28th, 2018

        // Deployment of AIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nStartTime = 1505692800; // Sep 18th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nTimeout = 1537228800; // Sep 18th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nThreshold = 50; // 50% of 100

        // Deployment of BIP147
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 1517792400; // Feb 5th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 1549328400; // Feb 5th, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nThreshold = 50; // 50% of 100

        // Deployment of AIP0003
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].nStartTime = 1556150400; // Thursday, 25-Apr-19 00:00:00 UTC
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].nTimeout = 1587772800; // Saturday, 25-Apr-20 00:00:00 UTC
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].nThreshold = 50; // 50% of 100

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x000000000000000000000000000000000000000000000000000000000000000");

        pchMessageStart[0] = 0xce;
        pchMessageStart[1] = 0xe2;
        pchMessageStart[2] = 0xca;
        pchMessageStart[3] = 0xff;
        vAlertPubKey = ParseHex("04517d8a699cb43d3938d7b24faaff7cda448ca4ea267723ba614784de661949bf632d6304316b244646dea079735b9a6fc4af804efb4752075b9fe2245e14e412");
        nDefaultPort = 19999;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1417713337, 1096447, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000008ca1832a4baf228eb1553c03d3a2c8e02399550dd6ea8d65cec3ef23d2e"));
        assert(genesis.hashMerkleRoot == uint256S("0xe0028eb9648db56b1ac77cf090b99048a8007e2bb64b68f092c03c7f56a662c7"));

        PoVNETGenesis = FindPoVNETGenesisBlock(consensus, genesis, 50 * COIN);
        consensus.hashPoVNETGenesisBlock = PoVNETGenesis.GetHash();

        vFixedSeeds.clear();
        vSeeds.clear();
        //vSeeds.push_back(CDNSSeedData("absolutecoin.net",  "pov.absolutecoin.net"));

        // Testnet Absolute addresses start with 'y'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Testnet Absolute script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Testnet Absolute BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Testnet Absolute BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Testnet Absolute BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fAllowMultipleAddressesFromGroup = true;
        fAllowMultiplePorts = true;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        vSporkAddresses = {"yQzniXSTqRwaEujSkvZicJsYZkt7wpa4NJ"};
        nMinSporkKeys = 1;
        // devnets are started with no blocks and no MN, so we can't check for upgraded MN (as there are none)
        fBIP9CheckMasternodesUpgraded = false;
        consensus.fLLMQAllowDummyCommitments = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (      0, uint256S("0x000008ca1832a4baf228eb1553c03d3a2c8e02399550dd6ea8d65cec3ef23d2e"))
            (      1, PoVNETGenesis.GetHash())
        };

        chainTxData = ChainTxData{
            PoVNETGenesis.GetBlockTime(), // * UNIX timestamp of povnet genesis block
            2,                            // * we only have 2 coinbase transactions when a povnet is started up
            0.01                          // * estimated number of transactions per second
        };
    }

    void UpdateSubsidyAndDiffParams(int nMinimumDifficultyBlocks, int nHighSubsidyBlocks, int nHighSubsidyFactor)
    {
        consensus.nMinimumDifficultyBlocks = nMinimumDifficultyBlocks;
        consensus.nHighSubsidyBlocks = nHighSubsidyBlocks;
        consensus.nHighSubsidyFactor = nHighSubsidyFactor;
    }
};
static CPoVNETParams *PoVNETParams;


/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.nMasternodePaymentsStartBlock = 240;
        consensus.nMasternodePaymentsIncreaseBlock = 350;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nInstantSendConfirmationsRequired = 2;
        consensus.nInstantSendKeepLock = 6;
        consensus.nBudgetPaymentsStartBlock = 1000;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nSuperblockStartBlock = 1500;
        consensus.nSuperblockStartHash = uint256(); // do not check this on regtest
        consensus.nSuperblockCycle = 10;
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 100;
        consensus.nMasternodeMinimumConfirmations = 1;

        consensus.BIP34Height = -1; // BIP34 is not required
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.AIP0001Height = 2000;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Absolute: 1 day
        consensus.nPowTargetSpacing = 2.5 * 60; // Absolute: 2.5 minutes
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;

        consensus.nPowDGWHeight = 25;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0001].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_AIP0003].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00"); // always verify

        pchMessageStart[0] = 0xf5;
        pchMessageStart[1] = 0x9b;
        pchMessageStart[2] = 0xc5;
        pchMessageStart[3] = 0x43;


        nDefaultPort = 16666;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1518596800, 2542998, 0x1e0ffff0, 1, 30 * COIN);


        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000d8cfc345deda15f4897bb9ed59878fb4c84f02c478dc1165ee8fbaede56"));
        assert(genesis.hashMerkleRoot == uint256S("0x12844a9cbf517654e272975506ab56af4d5c8dde0332a0ee48ba159c72daae03"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fRequireRoutableExternalIP = false;
        fMineBlocksOnDemand = true;
        fAllowMultipleAddressesFromGroup = true;
        fAllowMultiplePorts = true;

        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        // privKey: cP4EKFyJsHT39LDqgdcB43Y3YXjNyjb5Fuas1GQSeAtjnZWmZEQK
        vSporkAddresses = {"ydUCk8yb3HJ9BsUjfi6snQxHKw3cEDCiNW"};
        nMinSporkKeys = 1;
        // regtest usually has no masternodes in most tests, so don't check for upgraged MNs
        fBIP9CheckMasternodesUpgraded = false;
        consensus.fLLMQAllowDummyCommitments = true;

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0x00000d8cfc345deda15f4897bb9ed59878fb4c84f02c478dc1165ee8fbaede56"))
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };
        // Regtest Absolute addresses start with 'y'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Regtest Absolute script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Regtest private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Regtest Absolute BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Regtest Absolute BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Regtest Absolute BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;
   }

    void UpdateBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::POVNET) {
            assert(PoVNETParams);
            return *PoVNETParams;
    } else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    if (network == CBaseChainParams::POVNET) {
        povNetParams = (CPoVNetParams*)new uint8_t[sizeof(CPoVNetParams)];
        memset(povNetParams, 0, sizeof(PoVNetParams));
        new (povNetParams) CPoVNetParams();
    }
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    regTestParams.UpdateBIP9Parameters(d, nStartTime, nTimeout);
}

void UpdateRegtestBudgetParameters(int nMasternodePaymentsStartBlock, int nBudgetPaymentsStartBlock, int nSuperblockStartBlock)
{
    regTestParams.UpdateBudgetParameters(nMasternodePaymentsStartBlock, nBudgetPaymentsStartBlock, nSuperblockStartBlock);
}

void UpdatePovnetSubsidyAndDiffParams(int nMinimumDifficultyBlocks, int nHighSubsidyBlocks, int nHighSubsidyFactor)
{
    assert(povNetParams);
    PoVNETParams->UpdateSubsidyAndDiffParams(nMinimumDifficultyBlocks, nHighSubsidyBlocks, nHighSubsidyFactor);
}
