// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2017-2018 The LitecoinZ developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key_io.h"
#include "main.h"
#include "crypto/equihash.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    // To create a genesis block for a new chain which is Overwintered:
    //   txNew.nVersion = OVERWINTER_TX_VERSION
    //   txNew.fOverwintered = true
    //   txNew.nVersionGroupId = OVERWINTER_VERSION_GROUP_ID
    //   txNew.nExpiryHeight = <default value>
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].nValue = 0;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime     = nTime;
    genesis.nBits     = nBits;
    genesis.nNonce    = nNonce;
    genesis.nSolution = nSolution;
    genesis.nVersion  = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = genesis.BuildMerkleTree();
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database (and is in any case of zero value).
 *
 * >>> from pyblake2 import blake2s
 * >>> 'LitecoinZ' + blake2s(b'NY Times 05/Oct/2011 Steve Jobs, Apple’s Visionary, Dies at 56LTC#433272 27659f4c97de825afe9f1d4ab6c2ba4e83751a11f0144905b0fec849c36fb1ce LTC#741825 27659c79fbb898e9c35f55d49126afcc3a63d1501d399651f21980c538cbaa8f DJIA close on 29 Nov 2017: 23,940.68').hexdigest()
 */
static CBlock CreateGenesisBlock(uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "LitecoinZ6f099db24212fc48cc80bcf8d46874cd44a7a196625cc1e08a567bff77da2e79";
    const CScript genesisOutputScript = CScript() << ParseHex("04aaa049f7609d4b17ac733a67dd8abd10c0b5b410f0b3681b36dfb85fdc157fa22257895785bfc0c8741a9ff87d98d8a9a5330a8790aadd2f709576f65b37db89") << OP_CHECKSIG;

    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nSolution, nBits, nVersion, genesisReward);
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

const arith_uint256 maxUint = UintToArith256(uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        strCurrencyUnits = "LTZ";
        bip44CoinType = 221; // As registered in https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        consensus.fCoinbaseMustBeProtected = true;
        consensus.nSubsidyHalvingInterval = 840000;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 4000;
        const size_t N_1 = 200, K_1 = 9;
        const size_t N_2 = 144, K_2 = 5;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N_1, K_1));
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N_2, K_2));
        consensus.nEquihashN_1 = N_1;
        consensus.nEquihashK_1 = K_1;
        consensus.nEquihashN_2 = N_2;
        consensus.nEquihashK_2 = K_2;
        consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = boost::none;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170004;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170004;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170005;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight = 190000;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170007;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 190000;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170009;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.nEquihashForkHeight = 95000;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000000003cbab61c14c");

        pchMessageStart[0] = 0xd8;
        pchMessageStart[1] = 0xcf;
        pchMessageStart[2] = 0xcd;
        pchMessageStart[3] = 0x93;
        vAlertPubKey = ParseHex("048c64efd9e320f8dc6ab14f3e2c674877fe1b027f62afaffdd46ee4a191b3be90dd5a3bf8c7f6b2205dbde788793e97a645a16c2d8bede3dca0182dfc3cc15137");

        nDefaultPort = 29333;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(
            1512832667,
            uint256S("0x00000000000000000000000000000000000000000000000000000000000002e6"),
            ParseHex("00070c35670ed414e88f629ba7dae8dcadcbb16419221ad948059cbd018917e2b8bec09405ad14327e810ca560c67568235f72a9116a980b83f6c132ddaee132b7dceaccd2cd473d94a492b14d50caea2c998962008d96dbe4d023f9f43583b85552a401883b1075190c19808ebe4b06d5a3ede1535dd4068587e1fb1c961a04f70fc2c8c282fe5986bf5bc1ec1e55143e1fa12812444e3c2915b1810193d9a2f0408d474f59237f074a4e773808162efe813380f2a4acb20607b40e133385c1f815cd533abc3ab921f6e26cf2724bb95aaf1f563ae2149cf3fbe12cf27e5b21b7baf70aff163d35588534c4693d87e775c9728cd9e9079a9e7e21412626238eda92f7ad62372a910fd82fff185cfbd3d42a23e73c04dc42cb192204196231b222331016e16c2a20cb3842509a99370a02c7efdb9bc11b78f511b548b22cce5199abe1c849f994fe69a903bbfdbe5dd0025168cd45c96c3d4d458132819dc515b41c3a8544089e2554c568fa0fe802c123b63447adbc8ad49f9d208b62ad588b3f2ab0141210e0efe94b37b49b667568f4859767a1cc3397b5dae222e3ad1ebd8bfe6d27104bdbad5a4d6bd6b0c473a2b9e1efbf4cad9ef27b1734bc0cb56515bf586cc2b74ca65b71bf02361fd2210ce67fe25547bace2fe2fef9e5ec21854c1e6fb52f7a5ddb45999c3390a843bcfaafdbc55bf46b0472045e2fee4e94e98d1ebc01ffa94518ee6dff7a161115e8c5e688c6eab316a651db392055fafef65a5ed7069a772e2bcbfd8e9e7195cb187f2d837b769f6b7726960720b1a77fefacd2b33dc61f9739d78636b8a1078f6859950449d3f422e1fbc45d5030972ab06df45f5d23fdff1c4a256135774d133cbbae07771c27360ee54cc5fe54d393c17a74963a68e8ae5e017f480f2550b4d9c4720cb7b3dd47f810d0e12aaf047a2b3d003f78ee1bc94910640ed9f96875c2be854b95ccc31bd751f06a9281631fc87406d2bb3a6d31d45f376a22bbb325dbfe3a13f93d356dc2c41ebdbf8458a66735222d49a823c3cf9493aa0188deecd6f9a3dd2f033146045d9930ae09e2b0137dc877dbbb015f5d965e496f766c7a94de9fa7e55b8e2fea1aaf81615c94f94af9ebf5ebab5e99929d6947b6ffd983b6789f222551ebe6dfa899e35ba19ac8d0e6500e2fb3d71fd572077055d7db468f84589851aee7de7643854c9df7ec544ccdcdd72be08572a7b7048d57b859f19ed9b1b907f763c6cf60b4bfbc3e780f62ee8e66f5cf1c2b963928f20ddf20ccbb277f95cebee326a99080f9a2d60b4e805d444826fe6c0ac8d1c8f14496d73abd50131a65360057a90b51676de2316718f1c26aa47426280d6f8ba0cda4775db73346e6ae6ffcb68351bca27b48ee96751b2463df6bb2881a2ed8138e615478363601a0cb32dc0d4bed1d7e416f13baf2bad22099716423afd2cd79ca13aac929c26cd3147fa9de8cff0cb20ec3ec4f80d2e531d82e727ff1a5436d601f4c2ad2880ed50992b1fad1deadf8a659f154e25edbb5d891091ba473471070ae867ad59692f340fa0d5010d7d914d3b0abe7a1557df47d719b355a0984c6db898b36125781999b3a39d7e838f4efe2fb30df49bd5c22af43ea1656a363bc9fafe9f4622035b77e224cfc51230869d77d6e86ac09f34670c3f634aec89a759376e70f3dfbe572b174958e62613a11bcc6599c29360f640bb93691f0dfc6d1ecd4b148767c1c00f13dbc62f4209f1934b55bca72e5ff92897920bd6d1f6f0f0601093cd0fe1b02d7c24b1816855aede3da2a277ab12d13c5491d4b1b4c59607ea8f027ee5012c159dbc9551340ef820e0babe096dec30b8838d89dc0ad10eb4d1a5672d67abca3e7e84111c61ce0280c8df8392513"),
            0x1f07ffff, 4, 0);

        consensus.hashGenesisBlock = genesis.GetHash();
        assert(genesis.hashMerkleRoot == uint256S("0x87800ace8ce2299bbf33ac3f76186ee66be9d7de9a9e22cdba040d3c4776870e"));
        assert(consensus.hashGenesisBlock == uint256S("0x0003f52e78c166a7f44506c152f5e1df1577f21cb7062f863dd8f9a47e125ff9"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("litecoinz.org", "dnsseed.litecoinz.org")); // LitecoinZ

        // guarantees the first 2 characters, when base58 encoded, are "L1"
        base58Prefixes[PUBKEY_ADDRESS]     = {0x0A,0xB3};
        // guarantees the first 2 characters, when base58 encoded, are "L3"
        base58Prefixes[SCRIPT_ADDRESS]     = {0x0A,0xB8};
        // the first character, when base58 encoded, is "5" or "K" or "L" (as in Bitcoin)
        base58Prefixes[SECRET_KEY]         = {0x80};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x88,0xB2,0x1E};	// xpub
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x88,0xAD,0xE4};	// xprv
        // guarantees the first 2 characters, when base58 encoded, are "zm"
        base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0xAA};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVK"
        base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAB,0xD3};
        // guarantees the first 2 characters, when base58 encoded, are "MK"
        base58Prefixes[ZCSPENDING_KEY]     = {0x89,0x64};

        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "zs";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviews";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivks";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-main";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (      0, consensus.hashGenesisBlock)
            (    293, uint256S("000094343dc23483c26239f71603980a5c56062c061b81a6b6f30a77e6717d41"))
            (    586, uint256S("000015256f276b0bb1d8e3b601ac37644e76cf860d8bc565aa1ff82fc0a6ef3f"))
            (    879, uint256S("000083828428d8d2f5245d9d4ada17df9418ae1c320268f477a853f800df3365"))
            (   1330, uint256S("0000c242f621ac3a22e6cd230b25fc27800082072724cab678f5e32c12f8d1f8"))
            (   2659, uint256S("00004c165c02803abd5cbb066a7b70328efa8e0345fa7658701af96c53458da8"))
            (   5317, uint256S("0000486d6d3e2587fe0c3f48fd74ed21676202bf1867648dbf4c1a52f3659d4e"))
            (  10633, uint256S("0000005f809e79671f57d4dbb0ad8777d1e4a2f07d72e146316271567e6987b0"))
            (  21266, uint256S("000000126cfdf6cb5279df2a53e79ab30ea0f7336c794120c3197b9373908dd6"))
            (  31901, uint256S("00000088282df4c26a47bea22cb313a29ffe9ac8f30aa7de0129ca2c3a770f9a"))
            (  50000, uint256S("0000003f6762c60d9699a212e819a7d6630c6ea602a475908bfefe962f0803bb"))
            (  60000, uint256S("0000001334ddb7af2a2c17fd6dc0f7d0c0f6ba077403a0aed763b304001b7402"))
            (  70000, uint256S("000000094161a482b4d2f6dd2a261bcd6962a2c00a5e6ba94002d0633ce8912e"))
            (  80000, uint256S("000000157847bab44d199ad34954da98910c726575716270771ee7e32ceba1d6"))
            (  90000, uint256S("0000002564e3694cd1240d570fdcf9cf36791b2e2c462040fb78af34959dd02e"))
            (  93096, uint256S("00000038101895ae9add3b5d288db258b053c4bdc39642aeb6be44f7f53bc929"))
            (  95001, uint256S("0000000169dcc8f2b842beefd00bb7bf0069eba2acdbbfc70c9fa8edcbe326e1"))
            ( 120000, uint256S("000004c863a6f98cccecab76eab12b4a349f13f77ba6adab932197e2fb122f31"))
            ( 200000, uint256S("0000030578048515037b4fdcf86a0a5a100c1bff8df73a96992688d64dcd3aa1"))
            ( 262840, uint256S("000014ceea9e80561902ab4cde1d137e7a9925fe8388f72fa614e0314bd7f328")),
            1554904170,     // * UNIX timestamp of last checkpoint block
            262840,         // * total number of transactions between genesis and last checkpoint
                            //   (the tx=... number in the SetBestChain debug.log lines)
            1600            // * estimated number of transactions per day after checkpoint
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        strCurrencyUnits = "TLZ";
        bip44CoinType = 1;
        consensus.fCoinbaseMustBeProtected = true;
        consensus.nSubsidyHalvingInterval = 840000;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 400;
        const size_t N_1 = 200, K_1 = 9;
        const size_t N_2 = 144, K_2 = 5;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N_1, K_1));
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N_2, K_2));
        consensus.nEquihashN_1 = N_1;
        consensus.nEquihashK_1 = K_1;
        consensus.nEquihashN_2 = N_2;
        consensus.nEquihashK_2 = K_2;
        consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = boost::none;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170004;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170004;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170005;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight = 4000;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170007;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 4000;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170008;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.nEquihashForkHeight = 435;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000f1eb9b");

        pchMessageStart[0] = 0xfe;
        pchMessageStart[1] = 0x90;
        pchMessageStart[2] = 0x86;
        pchMessageStart[3] = 0x5d;
        vAlertPubKey = ParseHex("04a229174e5e5d0680151333b36a2ebebfb5fc01460724fa14050d0263678ee51996f7611774bc93377289dde6b05036c3db28ef312341309bf9737e6b80aaf66c");

        nDefaultPort = 39333;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(
            1511954736,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000f65"),
            ParseHex("00e7097053c67d09d457920796dcebadfcc6f469240eb5ef9bbb198c3aff994217d6afa7ea0bf415aaae194344a0c848f5f93d6d853cc1ad4aba4195940af83183265a331ce64efc9f63e6d7fd02ca27ad1c0a59059b8b245c0b5b5769698239cf9bcb80db0d5988dd2bfc2ce6e95313c711bea49f316976cd866d2d833b07a5aa27989c55ddba9371d8f9d92242c44cf7604f1b0b85675e4c18c9b9c82cc65568fd9344637ae9790315c65faf0e10c798c2c1179aeb8080ad665c0c503d74cc0f978fbf77abfb06a53ac142eac15ab91c1e087791bc9c538515cf8f021298e8eb7267839b0c2f0b3c52889cc624b83ac6b20876f3abd1c406518bf40c875614648e4c12f9c781590e996e25919057f9f617eacb4bab9d02ddeeb27c8ef67393fb46be5e794d1073853aee2773db821ed13f72f9421cbf0a3932ff1b1405ddb7d844af1328161d7cda7906a85c9c5d73021f1e36fdd2042bd949d3564bf95e05731bd7056b0a703df6248dd373249ba51600dba051a7b95d56c72313a9fe7164d57b49ee83e8f869509212cdf5ee06470887559fb1b6f7eaac39ecf2cf7b77517bff99120a24840088532c7baa077239324841c7523d9ff7e042b64334a5b334efe53ac47480ec86dae65d9c3fcc0e0905a707db00250da931319fdf5354e54c6ee9311ae4af4007174b2372774aefb7e292d7223dfbcb220371de952656c860f3e38252959fb0296c696ec5d915d5ed810e457705cc7b76cd6861b3e680c89a8098277046d706ba8047fcde429e2861fa7dca00ff1a5e5e02eb12adb37abfdfb256d4767091c9c1b077c7b60b646e358a1ed1171264c0cdfcd3d11f55fdde24fe1f44b9cb7b6936f1cd42b26178dd426498ebb91f8a150732ed1af34ae5e89f15437bdb2e5aa4147af79b5c742f190624b19747cd593e986058a678a3d7546201b8893fa0e890c7e5a7c3a66221a5eda1a5b7d0c7171d6e3e464ec8a16ded599c875900df5dd5fc45560a19e6c9314dab78795ef348544f7e917da5d4c15e49474b487b3461c3b9601546417987657c3c16f5ed14a58752200de1717863e3199efb49bf97967fc3867e051f9c73a16071e21d19685dccb5c70c173f1c8130353b6f16dbf95b393d73c8a03afe019ab9f8698272807dc0331f07618ca8c8226ef01e1b2ee9599e260495ce17e5c279ae5584e1cbe1da50b08ccddf6b230878f97e82ca143f4a4e41b1dc7e650f1b103e73da13eb2e93184e04fd35fb915ef67e6cdae06effdb4e39fe0b79889c8fd18556d3e45fa0f995358536c15d09a8b8b7ef585dcb229891def2d9eef8bfd72b5aa43531acee7f2a5fb3cc98e5b3a1ecb5c58c43f66eb01c79e6785d8cb799f53432c69d7e0e42172d132cf93d0f9b2c6390b1b31705b6b7bae8216e895df5ad0402c29be10db92389cc3380e07cf9d76a4d45b95e391898e679cfd64096f267d2dea03af9a5e3cef1b6a50950729406505c2518be51dd84715944a0b67299116d9707e4b3626dfd62f7bb0d5ee810ef89b4dcb3c40357f4f2ed70dcc1f907d11199bd15c754f93d04630c71fa683325f605d5937130e91ab081571cbf64150ab6caf3ce0843292c4bb0eef8fa816b0c5f9b01ce69275f82fd5c1fd71439f9787b500a277842fd68df059e944a5fae619bdfbde696d37ef9a663ee9a3fcf06a17db961821ec36eea695e17df77df45cffd658f06a9858187207a8bc988071d1c6f78623254793d9e1f6e6de23616488d9605f4d853eb069af068da4eb6097252ed9160642d499130d86996a6711821ed393911ee39cbe8863e676155d25b2fbd3b1133151c8c2a1a5d96dc271b5e7791e4822900c413b11b8eb31fa33654b1fddb5ae4e94984944a964b7005c45d6fed16"),
            0x1f07ffff, 4, 0);

        consensus.hashGenesisBlock = genesis.GetHash();
        assert(genesis.hashMerkleRoot == uint256S("0x87800ace8ce2299bbf33ac3f76186ee66be9d7de9a9e22cdba040d3c4776870e"));
        assert(consensus.hashGenesisBlock == uint256S("0x000777e63f7c2efb3c554405a07a4f3dd1def8ea5cef4fda65b2c57247171141"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("litecoinz.org", "dnsseed.litecoinz.org")); // LitecoinZ

        // guarantees the first 2 characters, when base58 encoded, are "T1"
        base58Prefixes[PUBKEY_ADDRESS]     = {0x0E,0xA4};
        // guarantees the first 2 characters, when base58 encoded, are "T3"
        base58Prefixes[SCRIPT_ADDRESS]     = {0x0E,0xA9};
        // the first character, when base58 encoded, is "9" or "c" (as in Bitcoin)
        base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};	// tpub
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};	// tprv
        // guarantees the first 2 characters, when base58 encoded, are "zt"
        base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0xB6};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVt"
        base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        // guarantees the first 2 characters, when base58 encoded, are "TK"
        base58Prefixes[ZCSPENDING_KEY]     = {0xB1,0xF8};

        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "ztestsapling";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviewtestsapling";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivktestsapling";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-test";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, consensus.hashGenesisBlock),
            1511954736,  // * UNIX timestamp of last checkpoint block
            1,           // * total number of transactions between genesis and last checkpoint
                         //   (the tx=... number in the SetBestChain debug.log lines)
            715          //   total number of tx / (checkpoint block height / (24 * 24))
        };
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        strCurrencyUnits = "RLZ";
        bip44CoinType = 1;
        consensus.fCoinbaseMustBeProtected = false;
        consensus.nSubsidyHalvingInterval = 150;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        const size_t N_1 = 48, K_1 = 5;
        const size_t N_2 = 96, K_2 = 5;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N_1, K_1));
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N_2, K_2));
        consensus.nEquihashN_1 = N_1;
        consensus.nEquihashK_1 = K_1;
        consensus.nEquihashN_2 = N_2;
        consensus.nEquihashK_2 = K_2;
        consensus.powLimit = uint256S("0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 0; // Turn off adjustment down
        consensus.nPowMaxAdjustUp = 0; // Turn off adjustment up
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = 0;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170004;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170004;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170005;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170006;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170008;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.nEquihashForkHeight = 100;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        pchMessageStart[0] = 0xea;
        pchMessageStart[1] = 0x8c;
        pchMessageStart[2] = 0x71;
        pchMessageStart[3] = 0x19;

        nDefaultPort = 49444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(
            1511954736,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000007"),
            ParseHex("0d728a7a610f130fdf24bf911ff28730b51c1e748dfd4646564b4e30dce57bf5a6b6233e"),
            0x200f0f0f, 4, 0);

        consensus.hashGenesisBlock = genesis.GetHash();

        assert(genesis.hashMerkleRoot == uint256S("0x87800ace8ce2299bbf33ac3f76186ee66be9d7de9a9e22cdba040d3c4776870e"));
        assert(consensus.hashGenesisBlock == uint256S("0x010539fc03180019d4de063a7fc0089e5e7d500ed5e943936ea7ea0e7aacd54a"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        // These prefixes are the same as the testnet prefixes
        base58Prefixes[PUBKEY_ADDRESS]     = {0x0E,0xA4};
        base58Prefixes[SCRIPT_ADDRESS]     = {0x0E,0xA9};
        base58Prefixes[SECRET_KEY]         = {0xEF};
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0xB6};
        base58Prefixes[ZCSPENDING_KEY]     = {0xB1,0xF8};

        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "zregtestsapling";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviewregtestsapling";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivkregtestsapling";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-regtest";

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, consensus.hashGenesisBlock),
            1511954736,
            1,
            0
        };
    }

    void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
    {
        assert(idx > Consensus::BASE_SPROUT && idx < Consensus::MAX_NETWORK_UPGRADES);
        consensus.vUpgrades[idx].nActivationHeight = nActivationHeight;
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
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

unsigned int CChainParams::EquihashSolutionWidth(int height) const
{
    return EhSolutionWidth(consensus.EquihashN(height), consensus.EquihashK(height));
}

void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    regTestParams.UpdateNetworkUpgradeParameters(idx, nActivationHeight);
}
