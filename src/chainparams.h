// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2022 The BitcoinZ community
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef BITCOIN_CHAINPARAMS_H
#define BITCOIN_CHAINPARAMS_H

#include "chainparamsbase.h"
#include "consensus/params.h"
#include "primitives/block.h"
#include "protocol.h"

#include <vector>

struct CDNSSeedData {
    std::string name, host;
    CDNSSeedData(const std::string &strName, const std::string &strHost) : name(strName), host(strHost) {}
};

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

typedef std::map<int, uint256> MapCheckpoints;
typedef std::map<int, int> MapFutureBlockTimeWindows;

struct CCheckpointData {
    MapCheckpoints mapCheckpoints;
    int64_t nTimeLastCheckpoint;
    int64_t nTransactionsLastCheckpoint;
    double fTransactionsPerDay;
};

struct EHparameters {
    unsigned char n;
    unsigned char k;
    unsigned short int nSolSize;
};

//EH sol size = (pow(2, k) * ((n/(k+1))+1)) / 8;
static const EHparameters eh200_9 = {200,9,1344};
static const EHparameters eh144_5 = {144,5,100};
static const EHparameters eh96_5 = {96,5,68};
static const EHparameters eh48_5 = {48,5,36};
static const unsigned int MAX_EH_PARAM_LIST_LEN = 2;




/**
 * CChainParams defines various tweakable parameters of a given instance of the
 * Bitcoin system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class CChainParams
{
public:
    enum Base58Type {
        PUBKEY_ADDRESS,
        SCRIPT_ADDRESS,
        SECRET_KEY,
        EXT_PUBLIC_KEY,
        EXT_SECRET_KEY,

        ZCPAYMENT_ADDRRESS,
        ZCSPENDING_KEY,
        ZCVIEWING_KEY,

        MAX_BASE58_TYPES
    };

    enum Bech32Type {
        SAPLING_PAYMENT_ADDRESS,
        SAPLING_FULL_VIEWING_KEY,
        SAPLING_INCOMING_VIEWING_KEY,
        SAPLING_EXTENDED_SPEND_KEY,

        MAX_BECH32_TYPES
    };

    const Consensus::Params& GetConsensus() const { return consensus; }
    const CMessageHeader::MessageStartChars& MessageStart() const { return pchMessageStart; }
    const std::vector<unsigned char>& AlertKey() const { return vAlertPubKey; }
    int GetDefaultPort() const { return nDefaultPort; }

    CAmount SproutValuePoolCheckpointHeight() const { return nSproutValuePoolCheckpointHeight; }
    CAmount SproutValuePoolCheckpointBalance() const { return nSproutValuePoolCheckpointBalance; }
    uint256 SproutValuePoolCheckpointBlockHash() const { return hashSproutValuePoolCheckpointBlock; }
    bool ZIP209Enabled() const { return fZIP209Enabled; }

    const CBlock& GenesisBlock() const { return genesis; }
    /** Make miner wait to have peers to avoid wasting work */
    bool MiningRequiresPeers() const { return fMiningRequiresPeers; }
    /** Default value for -checkmempool and -checkblockindex argument */
    bool DefaultConsistencyChecks() const { return fDefaultConsistencyChecks; }
    /** Policy: Filter transactions that do not match well-defined patterns */
    bool RequireStandard() const { return fRequireStandard; }
    int64_t MaxTipAge() const { return nMaxTipAge; }
    int64_t PruneAfterHeight() const { return nPruneAfterHeight; }

    EHparameters eh_epoch_1_params() const { return eh_epoch_1; }
    EHparameters eh_epoch_2_params() const { return eh_epoch_2; }
    unsigned long eh_epoch_1_end() const { return eh_epoch_1_endblock; }
    unsigned long eh_epoch_2_start() const { return eh_epoch_2_startblock; }

    std::string CurrencyUnits() const { return strCurrencyUnits; }
    uint32_t BIP44CoinType() const { return bip44CoinType; }
    /** Make miner stop after a block is found. In RPC, don't return until nGenProcLimit blocks are generated */
    bool MineBlocksOnDemand() const { return fMineBlocksOnDemand; }
    /** In the future use NetworkIDString() for RPC fields */
    bool TestnetToBeDeprecatedFieldRPC() const { return fTestnetToBeDeprecatedFieldRPC; }
    /** Return the BIP70 network string (main, test or regtest) */
    std::string NetworkIDString() const { return strNetworkID; }
    const std::vector<CDNSSeedData>& DNSSeeds() const { return vSeeds; }
    const std::vector<unsigned char>& Base58Prefix(Base58Type type) const { return base58Prefixes[type]; }
    const std::string& Bech32HRP(Bech32Type type) const { return bech32HRPs[type]; }
    const std::vector<SeedSpec6>& FixedSeeds() const { return vFixedSeeds; }
    const CCheckpointData& Checkpoints() const { return checkpointData; }
    /** Return the community fee address and script for a given block height */
    std::string GetCommunityFeeAddressAtHeight(int height) const;
    CScript GetCommunityFeeScriptAtHeight(int height) const;
    std::string GetCommunityFeeAddressAtIndex(int i) const;
    int GetCommunityFeeStartHeight() const { return vCommunityFeeStartHeight; };
    int GetLastCommunityFeeBlockHeight() const { return vCommunityFeeLastHeight; }
    /** Enforce coinbase consensus rule in regtest mode */
    void SetRegTestCoinbaseMustBeProtected() { consensus.fCoinbaseMustBeProtected = true; }
    int GetFutureBlockTimeWindow(int height) const;
    int GetRollingCheckpointStartHeight() const { return vRollingCheckpointStartHeight; }
protected:
    CChainParams() {}

    Consensus::Params consensus;
    CMessageHeader::MessageStartChars pchMessageStart;
    //! Raw pub key bytes for the broadcast alert signing key.
    std::vector<unsigned char> vAlertPubKey;
    int nDefaultPort = 0;
    long nMaxTipAge = 0;
    uint64_t nPruneAfterHeight = 0;
    EHparameters eh_epoch_1 = eh200_9;
    EHparameters eh_epoch_2 = eh144_5;
    unsigned long eh_epoch_1_endblock = 150000;
    unsigned long eh_epoch_2_startblock = 140000;
    std::vector<CDNSSeedData> vSeeds;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
    std::string bech32HRPs[MAX_BECH32_TYPES];
    std::string strNetworkID;
    std::string strCurrencyUnits;
    uint32_t bip44CoinType;
    CBlock genesis;
    std::vector<SeedSpec6> vFixedSeeds;
    bool fMiningRequiresPeers = false;
    bool fDefaultConsistencyChecks = false;
    bool fRequireStandard = false;
    bool fMineBlocksOnDemand = false;
    bool fTestnetToBeDeprecatedFieldRPC = false;
    CCheckpointData checkpointData;
    std::vector<std::string> vCommunityFeeAddress;
    int vCommunityFeeStartHeight;
    int vCommunityFeeLastHeight;
    MapFutureBlockTimeWindows futureBlockTimeWindows;
    CAmount nSproutValuePoolCheckpointHeight = 0;
    CAmount nSproutValuePoolCheckpointBalance = 0;
    uint256 hashSproutValuePoolCheckpointBlock;
    bool fZIP209Enabled = false;
    int vRollingCheckpointStartHeight;
};

/**
 * Return the currently selected parameters. This won't change after app
 * startup, except for unit tests.
 */
const CChainParams &Params();

/** Return parameters for the given network. */
CChainParams &Params(CBaseChainParams::Network network);

/** Sets the params returned by Params() to those for the given network. */
void SelectParams(CBaseChainParams::Network network);

/**
 * Looks for -regtest or -testnet and then calls SelectParams as appropriate.
 * Returns false if an invalid combination is given.
 */
bool SelectParamsFromCommandLine();

/**
 * Allows modifying the network upgrade regtest parameters.
 */
void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight);

void UpdateRegtestPow(int64_t nPowMaxAdjustDown, int64_t nPowMaxAdjustUp, uint256 powLimit);

int validEHparameterList(EHparameters *ehparams, unsigned long blockheight, const CChainParams& params);

bool checkEHParamaters(int solSize, int height, const CChainParams& params);

#endif // BITCOIN_CHAINPARAMS_H
