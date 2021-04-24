// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2020 The BitcoinZ community
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "key_io.h"
#include "main.h"
#include "crypto/equihash.h"

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
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nSolution = nSolution;
    genesis.nVersion = nVersion;
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
 * >>> 'BitcoinZ' + blake2s(b'BitcoinZ - Your Financial Freedom. Dedicated to The Purest Son of Liberty - Thaddeus Kosciuszko. BTC #484410 - 0000000000000000000c6a5f221ebeb77437cbab649d990facd0e42a24ee6231').hexdigest()
 */
static CBlock CreateGenesisBlock(uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "BitcoinZ2beeec1ef52fd18475953563ebdb287f056453f452200581f958711118e980b2";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
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
        strCurrencyUnits = "BTCZ";
        bip44CoinType = 177; // As registered in https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        consensus.fCoinbaseMustBeProtected = true;
        consensus.nSubsidySlowStartInterval = 0;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = Consensus::POST_BLOSSOM_HALVING_INTERVAL;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 4000;
        consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 13;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 34;
        consensus.nPowMaxAdjustUp = 34;
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = boost::none;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 770006;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight = 328500;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].hashActivationBlock =
            uint256S("000000806b0edc3e39108fa95c35d02ff58975388ca50141d10d7dd52deb13eb");
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 770006;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 328500;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].hashActivationBlock =
            uint256S("000000806b0edc3e39108fa95c35d02ff58975388ca50141d10d7dd52deb13eb");
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 770009;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight = 865600;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("000000000000000000000000000000000000000000000000000092816fe948b0");

        /**
         * The message start string should be awesome! ⓩ❤
         */
        pchMessageStart[0] = 0x24;
        pchMessageStart[1] = 0xe9;
        pchMessageStart[2] = 0x27;
        pchMessageStart[3] = 0x64;
        vAlertPubKey = ParseHex("04d5212ed0303c64db1840e799d31953eb362fd71d8e742dccd9aa78c4713d6d26b44974b44e2ac71aa38b06ef60c020207b85d270e4bdf8c797f3216f969960dc");
        nDefaultPort = 1989;
        nMaxTipAge = 24 * 60 * 60;
        nPruneAfterHeight = 100000;
        eh_epoch_1 = eh200_9;
        eh_epoch_2 = eh144_5;
        eh_epoch_1_endblock = 160010;
        eh_epoch_2_startblock = 160000;

        futureBlockTimeWindows = boost::assign::map_list_of
            ( 0, 2 * 60 ) // originally 2 hours
            ( 159300, 30 ) // 30 minutes
            ( 364400, 5 ); // 5 minutes

        vRollingCheckpointStartHeight = 364400;

        genesis = CreateGenesisBlock(
            1478403829,
            uint256S("0x000000000000000000000000000000000000000000000000000000000000021d"),
            ParseHex(""),
            0x1f07ffff, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0xf499ee3d498b4298ac6a64205b8addb7c43197e2a660229be65db8a4534d75c1")); //incremented by 1 making 2
        assert(genesis.hashMerkleRoot == uint256S("0xf40283d893eb46b35379a404cf06bd58c22ce05b32a4a641adec56e0792789ad"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // use name as: echo -n hostname | sha256sum

        vSeeds.push_back(CDNSSeedData("5051c0f9dfb6e29421647ea34bc3c693c2ba2222af3a867519e4cdd6f1b86c2b.BTCZ", "btzseed2.blockhub.info"));
        vSeeds.push_back(CDNSSeedData("4437c91da6e4c4edca56b57bd52c2e11a3fd7d8b04bd9dec9584fb5220f54b05.BTCZ", "btzseed.blockhub.info"));
        vSeeds.push_back(CDNSSeedData("d3f8adfdab612a8a41329e4d013d3ee0396289c8afb8c3951aa6deabf13f1ccb.BTCZ", "seed.btcz.app"));



        // guarantees the first 2 characters, when base58 encoded, are "t1"
        base58Prefixes[PUBKEY_ADDRESS]     = {0x1C,0xB8};
        // guarantees the first 2 characters, when base58 encoded, are "t3"
        base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0xBD};
        // the first character, when base58 encoded, is "5" or "K" or "L" (as in Bitcoin)
        base58Prefixes[SECRET_KEY]         = {0x80};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x88,0xB2,0x1E};
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x88,0xAD,0xE4};
        // guarantees the first 2 characters, when base58 encoded, are "zc"
        base58Prefixes[ZCPAYMENT_ADDRRESS] = {0x16,0x9A};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVK"
        base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAB,0xD3};
        // guarantees the first 2 characters, when base58 encoded, are "SK"
        base58Prefixes[ZCSPENDING_KEY]     = {0xAB,0x36};

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
            ( 0, uint256S("0xf499ee3d498b4298ac6a64205b8addb7c43197e2a660229be65db8a4534d75c1"))
            ( 2007, uint256S("0x000000215111f83669484439371ced6e3bc48cd7e7d6be8afa18952206304a1b"))
            ( 10000, uint256S("0x00000002ccb858ec2c35fb79ce2079333461efa50f2b59814558b9ae3ce62a40"))
            ( 20675, uint256S("0x00000004804df1618f984fef70c1a210988ade5093b6947c691422fc93013a63")) // Thaddeus Kosciuszko - 200th death anniversary (October 15 2017)
            ( 40000, uint256S("0x00000005a2d9a94e2e16f9c1e578a2eb46cc267ab7a51539d22ff8aa0096140b"))
            ( 56000, uint256S("0x000000026a063927c6746acec6c0957d1f69fa2ab1a59c06ce30d60bbbcea92a"))
            ( 84208, uint256S("0x0000000328e5d0346a78aea2d586154ab3145d51ba3936998253593b0ab2980c"))
            ( 105841, uint256S("0x000000010305387fd72bc70ce5cc5b512fe513016e7208b9ee61d601fe212991"))  //Dr Hawking, Rest in peace.
            ( 140000, uint256S("0x0000000155f89d1ededf519c6445d41c9240ee4daa721c91c19eea0faa2f02c8"))
            ( 153955, uint256S("0x00000006913d3122f32e60c9d64e87edd8e9a05444447df49713c15fbae6484d"))
            ( 160011, uint256S("0x0003a9fbed918bdd83fb5d38016189d5b8fe77495d4a7bd2405d3e8a04a62201"))  //18-06-17  8am UTC Hooray for Zhash!
            ( 166500, uint256S("0x0000002b640d62dd0c2ab68774b05297d2aa72bd63997d3a73ad959963b148d8"))
            ( 352440, uint256S("0x000000188d7e36ac236d2a1b549f14fe6fff287b80b4c68a832b6c80b8810fa2"))
            ( 352540, uint256S("0x00000006838b961606dad5a3da08b595a69cb8fc78684d9a4d3d3727bc96eb2b"))
            ( 352640, uint256S("0x000000c4a4a131d358a4b5419171c627cfb219367a810ca1780ef3119f634b6b"))
            ( 352740, uint256S("0x0000006bcc7d38424a1cf996b3b4ee61c44f941523af16c26c22c2708151a977"))
            ( 357600, uint256S("0x0000003b302a1ecfa6555b64981b1950853f49e022c923e98f94535225c6c54a"))
            ( 454535, uint256S("0x000000b7810d75c2b13b5e72e45712b19658bd68c8d814ef56b4434e54636b0c"))   // 16-NOV-2019 First live meeting in Rome
            ( 681563, uint256S("0x0000039c1dfa07bc3019e67d424ed48d3b7aa19de8a57e29f80cfbd74e72f1b0")),  // Last mined block in 2020 !

            1609455594,     // * UNIX timestamp of last checkpoint block
            1966202,        // * total number of transactions between genesis and last checkpoint
                            //   (the tx=... number in the SetBestChain debug.log lines)
            1521            // * estimated number of transactions per day after checkpoint
                            //   total number of tx / (checkpoint block height / (24 * 24))
        };

        // Hardcoded fallback value for the Sprout shielded value pool balance
        // for nodes that have not reindexed since the introduction of monitoring
        // in #2795.
        nSproutValuePoolCheckpointHeight = 520633;
        nSproutValuePoolCheckpointBalance = 6296192669575187;
        fZIP209Enabled = true;
        hashSproutValuePoolCheckpointBlock = uint256S("000000c04878c0ac973983e3b873ca7b23a0325406ae7d87c6f90957f1264492");

        // Community Fee script expects a vector of 2-of-3 multisig addresses
        vCommunityFeeAddress = {
            "t3eC2B44yVkyj7Q7RMkfBhkDisc4ieYtv5d",
            "t3cwTuGvHTkQc5ym8K39HkQRqgUeovcVXTy",
            "t3TxoqRtAytbfkBP7FrUPbSsLVLJAYXzLT7",
            "t3dghVnkqR8fqKhBipV2ggb4hoHnuWsHA6J",
            "t3LdFm55TvejDv823296TCMaxP1bDDSKQCQ",
            "t3UfK69A7EJCxpDoGFon3LJ5snLP3n1vDKC",
            "t3beERSviug8ardPTZnA2kPSmTQcaJNfL8y",
            "t3QRFq83FBJBJMg6HDgazjUWeStnsT9222x",
            "t3eJppdTuMLyYAKFXR1PEz1caonFW2RmJBB",
            "t3fWX6Tb6oxozvXwikCUV3s6E5uRHom7tEx",
            "t3ZKRdZPBFk3YNPR6ZfDWj82giBqkUqF2hX",
            "t3MkQ4ccb4q1Mz7Jzi8XKuQSxuae7PZzTLh",
            "t3ZyAJzpM8FKiQZZnqzGRB6LyQUYMQyvHMc",
            "t3Ur38PYZer2qHh9S9s5jiqkf7oe5bbtDVg",
            "t3f34ZKtaLZKMeRrPkjMVoGyZRBQGLxXL3t",
            "t3JpszYL1aLDVdhzVGPwSR3DZLGLKrxRLsU",
            "t3XSxsjYsRQG3SqyhURzthbK8KeTAAJdAMc",
            "t3euzVctNvQbqeEpn2xNR82PtgYwQ6qYRjf",
            "t3RG4E22bZfxKc5898VLbaXNHf6ThSJRFib",
            "t3SgMvNMhc8KhHFWN6YYG4de52PnG98HbnY",
            "t3NPGwdKqnixFQKrm9gUi6EezaCmscw1FcQ",
            "t3RzJ8w7pm8N5TiXBwmRu2nhkKfcqrEGCTy",
            "t3ajEu7N81EDAneDBucYNtg6Nc8U1kh9krT",
            "t3fKRecuPaUCJqUa5YbFxN6swETy1wTVqrH",
            "t3cLpLDoAts2Q7s3NsgRBnA2tARDyU6jo32",
            "t3UPnVmdHZjk3ASSzTCihZyvMy9PXGyd6q7",
            "t3KLY8t3HEKx3eKbbMSzKToKBZgVAyCiFhs",
            "t3QsMwSJEkQCgo3sXej5UjQCL3jHmpBHJip",
            "t3ZLJ81cBfUnqJ4s4yG9Ki4TTCq7Bd9eVut",
            "t3UDeoqzJUg9Fr5zwsqGwqhYHKgr36L4RJc",
            "t3SQTn4JtTXu8GurZsCzQx5xxH8MqJQ7iii",
            "t3Rh7iw8Pw4SJZrRwTnoTBv8eV3GwSF2hy9",
            "t3N9p9vVZTpc8reeuAZ9zGx1zoBjH4SjanH",
            "t3gqTdfwB1TiwN2ZCRcf9uEyrZKyXL5ccza",
            "t3gnsKHic8ne96pjx8nrFSJ643whhzcoyeE",
            "t3gsHcGLN3r35yEB59iNhCJw2iHuQKMZRie",
            "t3XxfcJQiy18Ex6jjuUX5k48K94EkqDagQN",
            "t3ZDGsmra5Cqk3bTvXWfsV4vYXXXSNKB9AE",
            "t3Y1YmUwa5LWQ1rTpzePN7EaNJsjb3pqK7J",
            "t3UzzHe2jeZua46RWL9bGuqkKs5STcoqPBJ",
            "t3MUQ1wGzC1977gSGzcoys4wt8d4JCdcuLv",
            "t3TA5fhiZn5AUQwyvL8WMdvySdoeq9wXvCT",
            "t3QKJnR4mrsGN4FyrdCHwoGC5LsEiYzRxKi",
            "t3Zw9p7MymABQDCUAkGbJKbw36Q3yZziwoe",
            "t3e73KWV7uY6rr2WoB1s2MCkkPjRxwGeCpt",
            "t3No3teH29dUJDcvQjLMPMZGoWN7vxU21LN",
            "t3VqnNUwzfrNkyDQoV3eBhcxQAQD3AXFFEZ",
            "t3KwL1ai4HvNaCcvnSYMkow9ywrXpvfz6Rr",
            "t3c72hsWG8SSMmMEwgS4BhVLEbaxS3PuHY4",
            "t3e7m74PF6yzW3zF5zAFYPCK7uVriykHoLu",
            "t3SausNGUC2vU6WkAN63khGL8axYFNYCQDg",
            "t3YPPSp668pSCQRrACgzTPoLuVaRTFFoeus",
            "t3cESR3q2Hh6mJbyC6ZBu4Jz8Dp1t7mbHLY",
            "t3WdLNKb194Ta3JRHxiip5ov83bFdLEwT91",
            "t3RixV5JL3Dr8B3sLZNWKokTWtVgnVMZZqM",
            "t3QKuKTub5vSRmWY6ExZqnUB3q5xLU1Lhp4",
            "t3U2X6AvUMWGWqFc1JxzsmeqDQq7G4Bcw3P",
            "t3Muezje93XcbjcWXKAeiPkACvADqZ4sed5",
            "t3S2fQysABXFxQJHGiE5tonFGRsuJkCYeRh",
            "t3PjeLxNmvbeSra4fURNKJazJDFwYdwSoA4",
            "t3VRFH5L43EbfTdnPwXRPvv3enRiAuvCJyG",
            "t3Tf22bky4tgR1wWKXKaKrtvZuLnuuh1dqM",
            "t3NKkGpiaUAX424KcPmX5UQ5xDx5scmKszz",
            "t3RFL3GARoko4vcPz1kvMpTBBrCUdwUiTM6",
            "t3eWEXExkTwNS4rFAMPKVA4CGVYQcJgbmdR",
            "t3VgtNUJLg1XDva3uMzVs8ZWsfmcneCwBoL",
            "t3Szm7fpJGzHnjxp1oSLciWHvVBNH3JBRg2",
            "t3USbLxCgLD5PzyDEy3bukoMZikiURRSL3S",
            "t3dZSEiB5p6y5WRZtCvz9CXRxnJSGoF9xp7",
            "t3YUjNigA9iD9gcJijy2X7qLvodMQaPwXYv",
            "t3MMfDtoysWMuhSa9wNTyjCvT3PtZ12UbeH",
            "t3KkanhBkRgJWTPckBHJazjasfnNi9DDCKe",
            "t3V3xqLmyjcSi1s6cJshCqP1Cf27x9DE2AB",
            "t3LgvwqUzsBGe9cPLqz2E2SXMxfPqSj6vh8",
            "t3VWw4ZRHHYZPgFMkEjBxdVCrUjJQtEHqah",
            "t3VWzRA9uP7c9zNiuBeB1V45c41ntDYdUQ6",
            "t3UHQzHwBXtb2SL823eVBVVeJiLhS1iL6Jm",
            "t3LhjLqqKKs4yvq1umm9MxpXj7FYuuiiYtm",
            "t3bnF3z7Z2DXc4p2tm5v5wtPQZQh7KFKjAK",
            "t3aLaR9GNpCoFF1HAKUCGzaR1wEEEK8G4vF",
            "t3YvmLwEBtYxLbRNQcMcqmHuSF7MAgRo1Dg",
            "t3ZCuv9FAYFzJBHVXWiGRmdXmE75WfPvi1J",
            "t3gn9cFxcnhuLpbBRX83Vt85EsWWh7t53co",
            "t3UdNuyX5u1ZSp38rsxyWtSYwHkrSd5xcut",
            "t3cotrT3GSzEqyKreNJmmS6kdzpCg6DafWW",
            "t3KBUuKs9LbfaNZRXWVAYcKynXiYR3Ega93",
            "t3duamHU9FHanjbhr2C5PUSUctRP2bujdut",
            "t3KxdJqVTTTVBcjCfcvbHipb4uLRM8WYo8H",
            "t3RzdWNacywKryT31xRvSpv79Viag87cCYG",
            "t3XdEptUkTXQLkiigBzCzzEsNSqHbgo37WT",
            "t3gqDqSuEWbYHxNcsagn44jRySjMHC2z5T2",
            "t3XCxm4jLmqwc4wLBrPhRkoHvp3nCJCqioX",
            "t3b1e9rURGhwAbpKfs9wHJD5qVxZsf44ZTR",
            "t3KP9rhDrCH8V8LzGRx9up281rsPg4tdv1Y",
            "t3XXxYXXnx2PiZSGbzmr9rmEXDvY9yYBvTb",
            "t3LHTCBkpq3b22wjuHT1usGsGSBJ3CdJhSJ",
            "t3PycyM8zzm9zptQ14QV7TT45uGsf3dsEPP",
            "t3fUhKH2G5TYbmuZrkq4a6GJon51D6Qiyss",
            "t3gGLesWeA25QKbb1QFNMw6NN33T6hcQAAE",
            "t3bi7pnM4mQ6RbQZwufGDt9m2uNnxHNBk37"
        };
        vCommunityFeeStartHeight = 328500;
        vCommunityFeeLastHeight = 1400000;
        assert(vCommunityFeeAddress.size() <= GetLastCommunityFeeBlockHeight());
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
        strCurrencyUnits = "TZB";
        bip44CoinType = 1;
        consensus.fCoinbaseMustBeProtected = true;
        consensus.nSubsidySlowStartInterval = 0;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = Consensus::POST_BLOSSOM_HALVING_INTERVAL;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 400;
        consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 13;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 34;
        consensus.nPowMaxAdjustUp = 34;
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = boost::none;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 770006;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight = 1500;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].hashActivationBlock =
            uint256S("0001bd0b788908fe416ac3c2909735bccb8c79b591e76a359ec657a97fb48a6d");
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 770006;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 1500;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].hashActivationBlock =
            uint256S("0001bd0b788908fe416ac3c2909735bccb8c79b591e76a359ec657a97fb48a6d");
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 770009;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight = 32600;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("000000000000000000000000000000000000000000000000000000000470fb4c");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0x1a;
        pchMessageStart[2] = 0xf9;
        pchMessageStart[3] = 0xbf;
        vAlertPubKey = ParseHex("048679fb891b15d0cada9692047fd0ae26ad8bfb83fabddbb50334ee5bc0683294deb410be20513c5af6e7b9cec717ade82b27080ee6ef9a245c36a795ab044bb3");
        nDefaultPort = 11989;
        nMaxTipAge = 24 * 60 * 60;
        nPruneAfterHeight = 1000;
        eh_epoch_1 = eh200_9;
        eh_epoch_2 = eh144_5;
        eh_epoch_1_endblock = 1210;
        eh_epoch_2_startblock = 1200;

        futureBlockTimeWindows = boost::assign::map_list_of
            ( 0, 2 * 60 ) // originally 2 hours
            ( 13999, 30 ) // 30 minutes
            ( 14000, 5 ); // 5 minutes

        vRollingCheckpointStartHeight = 14000;

        genesis = CreateGenesisBlock(
            1479443947,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000013"),
            ParseHex("002b24e10a5d2ab32b053a20ca6ebed779be1d935b1500eeea5c87aec684c6f934196fdfca6539de0cf1141544bffc5c0d1d4bab815fb5d8c2b195ccdf0755599ee492b9d98e3b79a178949f45485ad80dba38ec0461102adaa369b757ebb2bf8d75b5f67a341d666406d862a102c69800f20a7075be360a7eb2d315d78e4ce32c741f3baf7bf3e1e651976f734f367b1f126f62503b34d06d6e99b3659b2a47f5cfcf71c87e24e5023151d4af87454e7638a19b846350dd5fbc53e4ce1cce2597992b36cbcae0c24717e412c8df9ddca3e90c7629bd8c157c66d8906486943cf78e24d55dd4152f45eff49acf9fb9fddef81f2ee55892b38db940c404eaacf819588b83f0f761f1ba5b31a0ea1f8f4c5210638bbb59a2d8ddff9535f546b42a7eac5f3ee87616a075bddc3118b7f2c041f4b1e8dbcd11eea95835403066b5bb50cd23122dcb12166d75aafcfc1ca8f30580b4d48a5aa305657a06b4b650ed4633f2fa496235082feff65f70e19871f41b70632b53e57ddf38c207d631e5a56fa50bb71150f99427f73d82a439a5f70dfc7d8bbfc39d330ca7924527a5deb8950b9fa7020cfde5e07b84546e96764519ef6dd3fdc3a974abd342bdc7e4ee76bc11d5519541015afba1a0517fd347196aa326b0905a5916b83515c16f8f13105479c29f1eff3bc024ddbb07dcc672247cedc0d4ba32332ead0f13c58f50170642e16e076c34f5e75e3e8f5ac7f5238d67564fd385efecf972b0abf939a99bc7ef8f3a21cac21d2168706bbad3f4af66bb01cf61cfbc352a23797b62dcb5480bf2b7b277af233f5ce42a144d47119a89e1d114fa0bec2f13475b6b1df907bc3a429f1771afa3857bf16bfca3f76a5df14da62dc157fff4225bda73c3cfefa989edc24673bf932a024593da4c38b1a4628dd77ad919f4f7b7fb76976e696db69c89016ab30d9aa2d509f78d913d00ca9ac881aa759fc019b8c5e3eac6fddb4e0f044595e10d4997e29c79800f77cf1d97583d534db0f2726cba3739e7371eeffa2aca12b0d290ac45f44973f32f7675a5b49c94c4b608da2926555d16b7eb3670e12345a63f88797e5a5e21252c2c9463d7896001031a81bac0354336b35c5a10c93d9ae3054f6f6e4492f7c1f09a9d75034d5d0b220a9bb231e583659d5b6923a4e879326194de5c9805a02cb648508a8f9b6cd26dc17d322a478c1c599e1ec3adf2da6ce7a7e3a073b55cf30cf6b124f7700409abe14af8c60ab178579623916f165dbfd26f37056bf33c34f3af30939e1277376e4c5cba339f36381a05ef6481db033fb4c07a19e8655f8b12f9ab3c602e127b4ab1ee48e1c6a91382b54ed36ef9bb21b3bfa80a9107864dcb594dcad250e402b312607e648639631a3d1aeb17cfe3370202720ca8a46db15af92e8b46062b5bd035b24c35a592e5620d632faf1bf19a86df179fe52dd4cdbecd3cb7a336ca7489e4d1dc9433f1163c89d88c5eac36fc562496dc7583fe67c559c9a71cf89e9a0a59d5a14764926852d44a88d2ddb361d612ec06f9de874473eaf1d36b3a41911ac072b7826e6acea3d8425dc271833dba2ec17d1a270e49becbf21330ba2f0edc4b05f4df01623f3c82246ae23ea2c022434ef09611aa19ba35c3ecbad965af3ad9bc6c9b0d3b059c239ffbf9272d0150c151b4510d659cbd0e4a9c32945c612681b70ee4dcbeefeacde630b127115fd9af16cef4afefe611c9dfcc63e6833bf4dab79a7e1ae3f70321429557ab9da48bf93647830b5eb5780f23476d3d4d06a39ae532da5b2f30f151587eb5df19ec1acf099e1ac506e071eb52c3c3cc88ccf6622b2913acf07f1b772b5012e39173211e51773f3eb42d667fff1d902c5c87bd507837b3fd993e70ac9706a0"),
            0x2007ffff, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0x198659d06394e6d6b822495cd03dfe154987b48bfb83c137b18a2c62914b55f4"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("6be074a62041bb2bee54f8c48ef41bac55c44b0e1f49aef7c319d992844667c2.TZB", "test.seed.btcz.app"));
        //vSeeds.push_back(CDNSSeedData("rotorproject.org", "test-dnsseed.rotorproject.org")); // Zclassic

        // guarantees the first 2 characters, when base58 encoded, are "tm"
        base58Prefixes[PUBKEY_ADDRESS]     = {0x1D,0x25};
        // guarantees the first 2 characters, when base58 encoded, are "t2"
        base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0xBA};
        // the first character, when base58 encoded, is "9" or "c" (as in Bitcoin)
        base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        // guarantees the first 2 characters, when base58 encoded, are "zt"
        base58Prefixes[ZCPAYMENT_ADDRRESS] = {0x16,0xB6};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVt"
        base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        // guarantees the first 2 characters, when base58 encoded, are "ST"
        base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

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
            ( 0, consensus.hashGenesisBlock),
            genesis.nTime,
            0,
            0
        };

        // Hardcoded fallback value for the Sprout shielded value pool balance
        // for nodes that have not reindexed since the introduction of monitoring
        // in #2795.
        nSproutValuePoolCheckpointHeight = 21000;
        nSproutValuePoolCheckpointBalance = 825099960000;
        fZIP209Enabled = true;
        hashSproutValuePoolCheckpointBlock = uint256S("00360d4e02fbea84aa687722584e24bdc94dd7b768f35b095d453d81c0469d3a");

        // Founders reward script expects a vector of 2-of-3 multisig addresses
        vCommunityFeeAddress = {
            "t2FpKCWt95LAPVRed61YbBny9yz5nqexLGN",
            "t2RqJNenxiDjC5NiVo84xgfHcYuwsPcpCie",
            "t2MsHkAug2oEiqj4L5ZGZH1vHmdogTSb9km",
            "t2EwBFfC96DCiCAcJuEqGUbUes8rTNmaD6Q",
            "t2JqYXRoTsKb9r1rTLLwDs5jMXzsRBV317k",
            "t2RocidGU4ReKPK2uTPYfNFgeZEWDCd3jsj",
            "t2Mu8ToNiVow92PfETBk5Z6HWuAEG7RVXVD",
            "t2MSLT1n4eQ87QC2FAxMvuTZ84zDzEj7FhQ",
            "t2JZNFrWv1c4RqkCmDN9iRkPsG8xAZFdyGS",
            "t2AyjEVUCf5jthGHZjwfbztDBHQbztkJB5v",
            "t2Gs6dTYCzaFdHSeT91zaFLKmYzyqYY3NnP",
            "t2FXfNK7iQhTdMFcGUyrizqXQE5qbmPK6zc",
            "t2UqLwQ85pR1fdFMoUzXadXRB97JxP6vTWY",
            "t2BocGBq7iBXQP8UQiousNVwU8M6AqUtaRx",
            "t2VGGdXhspjF3iQvbWZW2zPNSDRSYauBcM3",
            "t2HTNHicoeEXxsX1wVhsqsX3LgzRq2pYgWH",
            "t2UiVSyM1vuvs6xP3157ytuYMKN6MuqmgJE",
            "t2UmPyNoWSVUgyPzEXzFGN5GS96jMH2kreW",
            "t2MQWZJHxZF5zSw6LbZ3S7jqoLX1y6SWLHQ",
            "t2VUR1c1aFaTUo93uhi7rfFVRRZaT1aQYbv",
            "t2NgLU6QCJhCKgBsR5uX6R4ds82jymzMoMJ",
            "t2RorFwMUEb7NamvXFi3jCXitAdRoQtU1Hs",
            "t2FFtmwePBnYaRVRVg1wsoBPxDzGMLrz3Jv",
            "t2GH3734fKEhPo3NvvAZQazsFf3V51oR4c2",
            "t2Ev3twAmUmono3gM2Q6RsfhRiryy7TnX5E",
            "t2EmhhAjh6cLpyw6Yc9QEXvsjm7qdKpgFQP",
            "t2Gy5N7DYbEZmiHqm3m8Re25a8Bxu7e36ju",
            "t2LVSaxizciFWfc5gr1xccHXT115RSnQ13r",
            "t28zy3Qiq3FtMeB2PCEysF7R5TgW5UfZN1N",
            "t2FcN7o26gRCc8ZuSZcc7X7APPRqWQ5a3W2",
            "t27QTHP9qoi5HkiTqx4JV86MGG37aikK51s",
            "t2CwQ6H9GPT77nqRwkHCuVcyGvtbhxWHfAk",
            "t2HLUDaoimaaSpQhHnvbqpKg6Fi37rAo6cx",
            "t2Ebuq1FX7Qzi3ur1FnwsDMvfNBFjqVqDGX",
            "t2Bca3HbSbwgQp1ZhzheNvGfpwBoU6Syt8G",
            "t2EurfAqyJMsCyx6ujYecQSxrPPY7xxTqcB",
            "t2R1kJGeNhLpKx1dKNCnBUq1BkxBVJjQdcp",
            "t2M3x9koBJWJS1F9bGtWXTsVfr5pesWSTbR",
            "t2La4mEMruVTtBqhndS7zRvmi2WsqWUjPQz",
            "t29GwTHLXxYgF5k7SSj7XFaHB7JsocM9bDU",
            "t2Awpdv7yG2QFeHeq17J1qCSXRw1AM3mfmz",
            "t2BfotpLdNhhewRp9nXpBBYViBaq4y1Lnj5",
            "t2F4CH89prySyGZHUiPYJUjnZk9UPXgLBbf",
            "t2DNx1KzP8a2S3kZgAPngso9ptva5gE7Jbn",
            "t2Eb7orwhjGcu4wYwHBzN5BoXzroPGq3CoM",
            "t2BXYmM21WCdHiC1KiwQVHxaTvLQJpqXTvH",
            "t27Y6774dwAcCFvYrhDKTXgaxtUewAdZdtz",
            "t2JvmRjZnViBZXJJBekDygdvGTCRNWgFEK2",
            "t2PL5W7qy1DKNRPWECbaZ6gV9GEzMn8h97Z",
            "t2S1JaefdSNwaUexdr6ZtNJhqZS8uDGSNFg",
            "t2BTunj4VB44Q22crWpT1ykoBvNGFKMnD7N",
            "t2G7DkSoEUJGaEBH6erKsXemoHFqqTRaSiZ",
            "t2Ldg8Bc6AWDuESqPgUoumWfCYw3zqKF8s9",
            "t2Ft4QMMiJfKXVbhyGBrkwjnfn5ua73VuLo",
            "t26xLxd4Fabbotkc9gfFwpCVHoZG1W9rmN7",
            "t2DyghJMpK6rRKPEAL3DBKmCntUcj8bUiHg",
            "t2RSYhCsgw2AdBiUUyXBCkFf2xE9ddwyESD",
            "t26fv5NLiFYXMmfQnvqcJXcYnt5NY41eqrv",
            "t2Ppht55eXKC1BX7pfusJxZqbHnkp9oWbBW",
            "t2P4AWJ5C4ySU3KzfehAeppH2BV4Y87w34z",
            "t28zjDUH2Gkvt8Ytb8UrW7L6G5U1QMwJFM3",
            "t2JXDd9pumryTAXqDD98vDLS2ZLSQCNQrYZ",
            "t2BNuNGnGq49MZzr7SH8WtEE7sSwZ9n3bsz",
            "t2QumKdHZhkFD6ntrzJ9zJAga2QemEgqc9r",
            "t2UKz2L7V3C6GTeBPDXmQnwMyqKEbgMpuXg",
            "t2CyVugoafiDYpeSNd9DGZEng6Bpr4tqa3d",
            "t2GR9eEen8KUDjhQG1opC1aFt27zxdtufnF",
            "t2JKYuSRNupdHdTR91tqR4xsaU6friVJJgv",
            "t2D2yMZEM3K8ap6iLo3FX2g1Ch9coPSVq2R",
            "t2SeFu34eiE2rCPFpxrN8im6ZvcwMpdKnit",
            "t2KH46EXQy5wnZHDGVDA7Q13FdRkdQ3LUou",
            "t2UsTpuVqP6ZubtN8tQGPnh7Cqjjf1hoefd",
            "t2Dd119xiqDbF9QzWwYfnYWUPfqgnL1CNFu",
            "t29PjecMhv6EygD8W6smcMHAB8MSHQY3YnQ",
            "t2BDZpxgcMRzqgKbDBiXRXrvL3VwD7G8cLc",
            "t2MwiKqfCMdy7o96bXvbZ5aGCrRmVfVWVfA",
            "t2Vhkny4jNjy6ZD53jeQzsdgZiZyejwRsgY",
            "t2K3ouBrLAbYwZv6beoHjzfsE1AbYVa6PuE",
            "t2DskMSpWs8i9vK2PhNpi9Mu2qJSvEDi8UZ",
            "t2JB2Uz3eVWrxFhas1B1cSXLP22JHbRNYtL",
            "t2ArYKW1L8hRoCDK9odNmD4piRwFheErWL1",
            "t2K1zKGHrkibiFoYJ5GtfHe5xJecJPEvFwQ",
            "t2VnABknMprtMk8y5AdDCBr2R9QZnMhfqSm",
            "t2FbjEsP9eeQr5PmP7yC3fopPTuYS9E9VgN",
            "t2Sn2XUPZEnFcggB77jvxBqX6LcjdCzcJUs",
            "t2SEK3Tw5FYYUaeZcF5QemfeG3tiorrxNKp",
            "t2D78THpHVodnhiREjF22A3KRznor5pPnR1",
            "t2GyqFdkf6FoQTShEhLGsNrTxAWqmeq4pui",
            "t2HnNgFLznEqaokYq8PBV44uzRwAmJXQeKd",
            "t2PpHVStdHvWkzXsyuyPYQQq96ZRQu7ALpE",
            "t2FHbHM9rKKHZe74HRBNozwNdRsExug8tCw",
            "t29tM6DkMPSVp9R3g7UjZjvsobKhsbsRqFL",
            "t2K2KixLVJo19phPJMv9ApSiFmxQCSQUvc9",
            "t2AWJcGVUMWFC8A9KC3PL7qoCb1vxSzxbJP",
            "t26p8FyjHmhqZ6duzhRFLCQcExh1TuCD1sC",
            "t27x5n41uRNF3tJkb3Lg1CMomUjTNZwtUfm",
            "t2VhRQJ9xeVkVVk7ic21CtDePKmHnrDyF8Z",
            "t27hL1iAsTHBPWrdc1qYGSSTc3pTyBqohd4",
            "t2RqLYWG8Eo4hopDsn1m8GUoAWtjZQEPE9s",
            "t2V1osVDkcwYFL4PF9qG8t9Ez1XRVMAkAb6"
        };
        vCommunityFeeStartHeight = 1500;
        vCommunityFeeLastHeight = 1400000;
        assert(vCommunityFeeAddress.size() <= GetLastCommunityFeeBlockHeight());
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
        strCurrencyUnits = "REG";
        bip44CoinType = 1;
        consensus.fCoinbaseMustBeProtected = false;
        consensus.nSubsidySlowStartInterval = 0;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_REGTEST_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = Consensus::POST_BLOSSOM_REGTEST_HALVING_INTERVAL;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.powLimit = uint256S("0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f");
        consensus.nPowAveragingWindow = 13;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 0; // Turn off adjustment down
        consensus.nPowMaxAdjustUp = 0; // Turn off adjustment up
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = 0;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 770006;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170006;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170009;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        pchMessageStart[0] = 0xaa;
        pchMessageStart[1] = 0xe8;
        pchMessageStart[2] = 0x3f;
        pchMessageStart[3] = 0x5f;
        nDefaultPort = 11989;
        nMaxTipAge = 24 * 60 * 60;
        //assert(consensus.hashGenesisBlock == uint256S("0x0575f78ee8dc057deee78ef691876e3be29833aaee5e189bb0459c087451305a"));
        nPruneAfterHeight = 1000;
        eh_epoch_1 = eh48_5;
        eh_epoch_2 = eh48_5;
        eh_epoch_1_endblock = 1;
        eh_epoch_2_startblock = 1;

        futureBlockTimeWindows = boost::assign::map_list_of
            ( 0, 2 * 60 ) // originally 2 hours
            ( 159300, 30 ) // 30 minutes
            ( 364400, 5 ); // 5 minutes

        vRollingCheckpointStartHeight = 364400;

        genesis = CreateGenesisBlock(
            1482971059,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000009"),
            ParseHex("05ffd6ad016271ade20cfce093959c3addb2079629f9f123c52ef920caa316531af5af3f"),
            0x200f0f0f, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x029f11d80ef9765602235e1bc9727e3eb6ba20839319f761fee920d63401e327"));
        //assert(genesis.hashMerkleRoot == uint256S("0xc4eaa58879081de3c24a7b117ed2b28300e7ec4c4c1dff1d3f1268b7857a4ddb"));

        vFixedSeeds.clear();  //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();       //!< Regtest mode doesn't have any DNS seeds.
        vSeeds.push_back(CDNSSeedData("6be074a62041bb2bee54f8c48ef41bac55c44b0e1f49aef7c319d992844667c2.TZB", "test.seed.btcz.app"));
        //vSeeds.push_back(CDNSSeedData("rotorproject.org", "test-dnsseed.rotorproject.org")); // Zclassic

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0x0575f78ee8dc057deee78ef691876e3be29833aaee5e189bb0459c087451305a")),
            0,
            0,
            0
        };

                // These prefixes are the same as the testnet prefixes
        base58Prefixes[PUBKEY_ADDRESS]     = {0x1D,0x25};
        base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0xBA};
        base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        base58Prefixes[ZCPAYMENT_ADDRRESS] = {0x16,0xB6};
        base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "zregtestsapling";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviewregtestsapling";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivkregtestsapling";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-regtest";

        // Founders reward script expects a vector of 2-of-3 multisig addresses
        vCommunityFeeAddress = {
            "t2FpKCWt95LAPVRed61YbBny9yz5nqexLGN",
            "t2RqJNenxiDjC5NiVo84xgfHcYuwsPcpCie",
            "t2MsHkAug2oEiqj4L5ZGZH1vHmdogTSb9km",
            "t2EwBFfC96DCiCAcJuEqGUbUes8rTNmaD6Q",
            "t2JqYXRoTsKb9r1rTLLwDs5jMXzsRBV317k",
            "t2RocidGU4ReKPK2uTPYfNFgeZEWDCd3jsj",
            "t2Mu8ToNiVow92PfETBk5Z6HWuAEG7RVXVD",
            "t2MSLT1n4eQ87QC2FAxMvuTZ84zDzEj7FhQ",
            "t2JZNFrWv1c4RqkCmDN9iRkPsG8xAZFdyGS",
            "t2AyjEVUCf5jthGHZjwfbztDBHQbztkJB5v",
            "t2Gs6dTYCzaFdHSeT91zaFLKmYzyqYY3NnP",
            "t2FXfNK7iQhTdMFcGUyrizqXQE5qbmPK6zc",
            "t2UqLwQ85pR1fdFMoUzXadXRB97JxP6vTWY",
            "t2BocGBq7iBXQP8UQiousNVwU8M6AqUtaRx",
            "t2VGGdXhspjF3iQvbWZW2zPNSDRSYauBcM3",
            "t2HTNHicoeEXxsX1wVhsqsX3LgzRq2pYgWH",
            "t2UiVSyM1vuvs6xP3157ytuYMKN6MuqmgJE",
            "t2UmPyNoWSVUgyPzEXzFGN5GS96jMH2kreW",
            "t2MQWZJHxZF5zSw6LbZ3S7jqoLX1y6SWLHQ",
            "t2VUR1c1aFaTUo93uhi7rfFVRRZaT1aQYbv",
            "t2NgLU6QCJhCKgBsR5uX6R4ds82jymzMoMJ",
            "t2RorFwMUEb7NamvXFi3jCXitAdRoQtU1Hs",
            "t2FFtmwePBnYaRVRVg1wsoBPxDzGMLrz3Jv",
            "t2GH3734fKEhPo3NvvAZQazsFf3V51oR4c2",
            "t2Ev3twAmUmono3gM2Q6RsfhRiryy7TnX5E",
            "t2EmhhAjh6cLpyw6Yc9QEXvsjm7qdKpgFQP",
            "t2Gy5N7DYbEZmiHqm3m8Re25a8Bxu7e36ju",
            "t2LVSaxizciFWfc5gr1xccHXT115RSnQ13r",
            "t28zy3Qiq3FtMeB2PCEysF7R5TgW5UfZN1N",
            "t2FcN7o26gRCc8ZuSZcc7X7APPRqWQ5a3W2",
            "t27QTHP9qoi5HkiTqx4JV86MGG37aikK51s",
            "t2CwQ6H9GPT77nqRwkHCuVcyGvtbhxWHfAk",
            "t2HLUDaoimaaSpQhHnvbqpKg6Fi37rAo6cx",
            "t2Ebuq1FX7Qzi3ur1FnwsDMvfNBFjqVqDGX",
            "t2Bca3HbSbwgQp1ZhzheNvGfpwBoU6Syt8G",
            "t2EurfAqyJMsCyx6ujYecQSxrPPY7xxTqcB",
            "t2R1kJGeNhLpKx1dKNCnBUq1BkxBVJjQdcp",
            "t2M3x9koBJWJS1F9bGtWXTsVfr5pesWSTbR",
            "t2La4mEMruVTtBqhndS7zRvmi2WsqWUjPQz",
            "t29GwTHLXxYgF5k7SSj7XFaHB7JsocM9bDU",
            "t2Awpdv7yG2QFeHeq17J1qCSXRw1AM3mfmz",
            "t2BfotpLdNhhewRp9nXpBBYViBaq4y1Lnj5",
            "t2F4CH89prySyGZHUiPYJUjnZk9UPXgLBbf",
            "t2DNx1KzP8a2S3kZgAPngso9ptva5gE7Jbn",
            "t2Eb7orwhjGcu4wYwHBzN5BoXzroPGq3CoM",
            "t2BXYmM21WCdHiC1KiwQVHxaTvLQJpqXTvH",
            "t27Y6774dwAcCFvYrhDKTXgaxtUewAdZdtz",
            "t2JvmRjZnViBZXJJBekDygdvGTCRNWgFEK2",
            "t2PL5W7qy1DKNRPWECbaZ6gV9GEzMn8h97Z",
            "t2S1JaefdSNwaUexdr6ZtNJhqZS8uDGSNFg",
            "t2BTunj4VB44Q22crWpT1ykoBvNGFKMnD7N",
            "t2G7DkSoEUJGaEBH6erKsXemoHFqqTRaSiZ",
            "t2Ldg8Bc6AWDuESqPgUoumWfCYw3zqKF8s9",
            "t2Ft4QMMiJfKXVbhyGBrkwjnfn5ua73VuLo",
            "t26xLxd4Fabbotkc9gfFwpCVHoZG1W9rmN7",
            "t2DyghJMpK6rRKPEAL3DBKmCntUcj8bUiHg",
            "t2RSYhCsgw2AdBiUUyXBCkFf2xE9ddwyESD",
            "t26fv5NLiFYXMmfQnvqcJXcYnt5NY41eqrv",
            "t2Ppht55eXKC1BX7pfusJxZqbHnkp9oWbBW",
            "t2P4AWJ5C4ySU3KzfehAeppH2BV4Y87w34z",
            "t28zjDUH2Gkvt8Ytb8UrW7L6G5U1QMwJFM3",
            "t2JXDd9pumryTAXqDD98vDLS2ZLSQCNQrYZ",
            "t2BNuNGnGq49MZzr7SH8WtEE7sSwZ9n3bsz",
            "t2QumKdHZhkFD6ntrzJ9zJAga2QemEgqc9r",
            "t2UKz2L7V3C6GTeBPDXmQnwMyqKEbgMpuXg",
            "t2CyVugoafiDYpeSNd9DGZEng6Bpr4tqa3d",
            "t2GR9eEen8KUDjhQG1opC1aFt27zxdtufnF",
            "t2JKYuSRNupdHdTR91tqR4xsaU6friVJJgv",
            "t2D2yMZEM3K8ap6iLo3FX2g1Ch9coPSVq2R",
            "t2SeFu34eiE2rCPFpxrN8im6ZvcwMpdKnit",
            "t2KH46EXQy5wnZHDGVDA7Q13FdRkdQ3LUou",
            "t2UsTpuVqP6ZubtN8tQGPnh7Cqjjf1hoefd",
            "t2Dd119xiqDbF9QzWwYfnYWUPfqgnL1CNFu",
            "t29PjecMhv6EygD8W6smcMHAB8MSHQY3YnQ",
            "t2BDZpxgcMRzqgKbDBiXRXrvL3VwD7G8cLc",
            "t2MwiKqfCMdy7o96bXvbZ5aGCrRmVfVWVfA",
            "t2Vhkny4jNjy6ZD53jeQzsdgZiZyejwRsgY",
            "t2K3ouBrLAbYwZv6beoHjzfsE1AbYVa6PuE",
            "t2DskMSpWs8i9vK2PhNpi9Mu2qJSvEDi8UZ",
            "t2JB2Uz3eVWrxFhas1B1cSXLP22JHbRNYtL",
            "t2ArYKW1L8hRoCDK9odNmD4piRwFheErWL1",
            "t2K1zKGHrkibiFoYJ5GtfHe5xJecJPEvFwQ",
            "t2VnABknMprtMk8y5AdDCBr2R9QZnMhfqSm",
            "t2FbjEsP9eeQr5PmP7yC3fopPTuYS9E9VgN",
            "t2Sn2XUPZEnFcggB77jvxBqX6LcjdCzcJUs",
            "t2SEK3Tw5FYYUaeZcF5QemfeG3tiorrxNKp",
            "t2D78THpHVodnhiREjF22A3KRznor5pPnR1",
            "t2GyqFdkf6FoQTShEhLGsNrTxAWqmeq4pui",
            "t2HnNgFLznEqaokYq8PBV44uzRwAmJXQeKd",
            "t2PpHVStdHvWkzXsyuyPYQQq96ZRQu7ALpE",
            "t2FHbHM9rKKHZe74HRBNozwNdRsExug8tCw",
            "t29tM6DkMPSVp9R3g7UjZjvsobKhsbsRqFL",
            "t2K2KixLVJo19phPJMv9ApSiFmxQCSQUvc9",
            "t2AWJcGVUMWFC8A9KC3PL7qoCb1vxSzxbJP",
            "t26p8FyjHmhqZ6duzhRFLCQcExh1TuCD1sC",
            "t27x5n41uRNF3tJkb3Lg1CMomUjTNZwtUfm",
            "t2VhRQJ9xeVkVVk7ic21CtDePKmHnrDyF8Z",
            "t27hL1iAsTHBPWrdc1qYGSSTc3pTyBqohd4",
            "t2RqLYWG8Eo4hopDsn1m8GUoAWtjZQEPE9s",
            "t2V1osVDkcwYFL4PF9qG8t9Ez1XRVMAkAb6"
        };
        vCommunityFeeStartHeight = 200;
        vCommunityFeeLastHeight = 1400000;
        assert(vCommunityFeeAddress.size() <= GetLastCommunityFeeBlockHeight());
    }

    void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
    {
        assert(idx > Consensus::BASE_SPROUT && idx < Consensus::MAX_NETWORK_UPGRADES);
        consensus.vUpgrades[idx].nActivationHeight = nActivationHeight;
    }

    void UpdateRegtestPow(int64_t nPowMaxAdjustDown, int64_t nPowMaxAdjustUp, uint256 powLimit)
    {
        consensus.nPowMaxAdjustDown = nPowMaxAdjustDown;
        consensus.nPowMaxAdjustUp = nPowMaxAdjustUp;
        consensus.powLimit = powLimit;
    }

    void SetRegTestZIP209Enabled() {
        fZIP209Enabled = true;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams &Params(CBaseChainParams::Network network) {
    switch (network) {
        case CBaseChainParams::MAIN:
            return mainParams;
        case CBaseChainParams::TESTNET:
            return testNetParams;
        case CBaseChainParams::REGTEST:
            return regTestParams;
        default:
            assert(false && "Unimplemented network");
            return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network) {
    SelectBaseParams(network);
    pCurrentParams = &Params(network);

    // Some python qa rpc tests need to enforce the coinbase consensus rule
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-regtestprotectcoinbase")) {
        regTestParams.SetRegTestCoinbaseMustBeProtected();
    }

    // When a developer is debugging turnstile violations in regtest mode, enable ZIP209
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-developersetpoolsizezero")) {
        regTestParams.SetRegTestZIP209Enabled();
    }
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}

// Block height must be >0 and <=last founders reward block height	// Block height must be >0 and <=last founders reward block height
// Index variable i ranges from 0 - (vCommunityFeeAddress.size()-1)
std::string CChainParams::GetCommunityFeeAddressAtHeight(int nHeight) const {
  int preBlossomMaxHeight = GetLastCommunityFeeBlockHeight();
  // zip208

  // FounderAddressAdjustedHeight(height) :=
  // height, if not IsBlossomActivated(height)
  // BlossomActivationHeight + floor((height - BlossomActivationHeight) / BlossomPoWTargetSpacingRatio), otherwise
  bool blossomActive = consensus.NetworkUpgradeActive(nHeight, Consensus::UPGRADE_BLOSSOM);
  if (blossomActive) {
      int blossomActivationHeight = consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight;
      nHeight = blossomActivationHeight + ((nHeight - blossomActivationHeight) / Consensus::BLOSSOM_POW_TARGET_SPACING_RATIO);
  }
  assert(nHeight > 0 && nHeight <= preBlossomMaxHeight);
  size_t addressChangeInterval = (preBlossomMaxHeight + vCommunityFeeAddress.size()) / vCommunityFeeAddress.size();
  size_t i = nHeight / addressChangeInterval;
  return vCommunityFeeAddress[i];
}

// Block height must be >0 and <=last founders reward block height
// The founders reward address is expected to be a multisig (P2SH) address
CScript CChainParams::GetCommunityFeeScriptAtHeight(int nHeight) const {
    assert(nHeight > 0 && nHeight <= GetLastCommunityFeeBlockHeight());

    CTxDestination address = DecodeDestination(GetCommunityFeeAddressAtHeight(nHeight).c_str());
    assert(IsValidDestination(address));
    assert(boost::get<CScriptID>(&address) != nullptr);
    CScriptID scriptID = boost::get<CScriptID>(address); // address is a boost variant
    CScript script = CScript() << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
    return script;
}

std::string CChainParams::GetCommunityFeeAddressAtIndex(int i) const {
    assert(i >= 0 && i < vCommunityFeeAddress.size());
    return vCommunityFeeAddress[i];
}

void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    regTestParams.UpdateNetworkUpgradeParameters(idx, nActivationHeight);
}

void UpdateRegtestPow(int64_t nPowMaxAdjustDown, int64_t nPowMaxAdjustUp, uint256 powLimit) {
    regTestParams.UpdateRegtestPow(nPowMaxAdjustDown, nPowMaxAdjustUp, powLimit);
}

int validEHparameterList(EHparameters *ehparams, unsigned long blockheight, const CChainParams& params){
    //if in overlap period, there will be two valid solutions, else 1.
    //The upcoming version of EH is preferred so will always be first element
    //returns number of elements in list
    if(blockheight >= params.eh_epoch_2_start() && blockheight > params.eh_epoch_1_end()){
        ehparams[0] = params.eh_epoch_2_params();
        return 1;
    }
    if(blockheight < params.eh_epoch_2_start()){
        ehparams[0] = params.eh_epoch_1_params();
        return 1;
    }
    ehparams[0] = params.eh_epoch_2_params();
    ehparams[1] = params.eh_epoch_1_params();
    return 2;
}

bool checkEHParamaters(int solSize, int height, const CChainParams& params) {
    // Block will be validated prior to mining, and will have a zero length
    // equihash solution. These need to be let through.
    if (height == 0) {
        return true;
    }

    //allocate on-stack space for parameters list
    EHparameters ehparams[MAX_EH_PARAM_LIST_LEN];
    int listlength = validEHparameterList(ehparams, height, params);
    for(int i = 0; i < listlength; i++){
        LogPrint("pow", "checkEHParamaters height: %d n:%d k:%d solsize: %d \n",
            height, ehparams[i].n, ehparams[i].k, ehparams[i].nSolSize);
        if (ehparams[i].nSolSize == solSize)
            return true;
    }

    return false;
}

int CChainParams::GetFutureBlockTimeWindow(int height) const {
    BOOST_REVERSE_FOREACH(const MapFutureBlockTimeWindows::value_type& i, futureBlockTimeWindows)
    {
        if (i.first <= height) {
            return i.second * 60;
        }
    }
    return 2 * 60 * 60;
}
