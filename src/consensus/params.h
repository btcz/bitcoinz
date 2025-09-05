// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include "script/script.h"
#include "key_constants.h"
#include "uint256.h"

#include <optional>
#include <variant>

namespace Consensus {

// Early declaration to ensure it is accessible.
struct Params;

/**
 * Index into Params.vUpgrades and NetworkUpgradeInfo
 *
 * Being array indices, these MUST be numbered consecutively.
 *
 * The order of these indices MUST match the order of the upgrades on-chain, as
 * several functions depend on the enum being sorted.
 */
enum UpgradeIndex : uint32_t {
    // Sprout must be first
    BASE_SPROUT,
    UPGRADE_TESTDUMMY,
    UPGRADE_OVERWINTER,
    UPGRADE_SAPLING,
    UPGRADE_CANOPY,
    // NOTE: Also add new upgrades to NetworkUpgradeInfo in upgrades.cpp
    MAX_NETWORK_UPGRADES
};

struct NetworkUpgrade {
    /**
     * The first protocol version which will understand the new consensus rules
     */
    int nProtocolVersion;

    /**
     * Height of the first block for which the new consensus rules will be active
     */
    int nActivationHeight;

    /**
     * Special value for nActivationHeight indicating that the upgrade is always active.
     * This is useful for testing, as it means tests don't need to deal with the activation
     * process (namely, faking a chain of somewhat-arbitrary length).
     *
     * New blockchains that want to enable upgrade rules from the beginning can also use
     * this value. However, additional care must be taken to ensure the genesis block
     * satisfies the enabled rules.
     */
    static constexpr int ALWAYS_ACTIVE = 0;

    /**
     * Special value for nActivationHeight indicating that the upgrade will never activate.
     * This is useful when adding upgrade code that has a testnet activation height, but
     * should remain disabled on mainnet.
     */
    static constexpr int NO_ACTIVATION_HEIGHT = -1;

    /**
     * The hash of the block at height nActivationHeight, if known. This is set manually
     * after a network upgrade activates.
     *
     * We use this in IsInitialBlockDownload to detect whether we are potentially being
     * fed a fake alternate chain. We use NU activation blocks for this purpose instead of
     * the checkpoint blocks, because network upgrades (should) have significantly more
     * scrutiny than regular releases. nMinimumChainWork MUST be set to at least the chain
     * work of this block, otherwise this detection will have false positives.
     */
    std::optional<uint256> hashActivationBlock;
};

typedef std::variant<CScript> FundingStreamAddress;

/**
 * Index into Params.vFundingStreams.
 *
 * Being array indices, these MUST be numbered consecutively.
 */
enum FundingStreamIndex : uint32_t {
    FS_ZIP214_BP,
    FS_ZIP214_ZF,
    FS_ZIP214_MG,
    MAX_FUNDING_STREAMS,
};
const auto FIRST_FUNDING_STREAM = FS_ZIP214_BP;

enum FundingStreamError {
    CANOPY_NOT_ACTIVE,
    ILLEGAL_RANGE,
    INSUFFICIENT_ADDRESSES,
};

class FundingStream
{
private:
    int startHeight;
    int endHeight;
    std::vector<FundingStreamAddress> addresses;

    FundingStream(int startHeight, int endHeight, const std::vector<FundingStreamAddress>& addresses):
        startHeight(startHeight), endHeight(endHeight), addresses(addresses) { }
public:
    FundingStream(const FundingStream& fs):
        startHeight(fs.startHeight), endHeight(fs.endHeight), addresses(fs.addresses) { }

    static std::variant<FundingStream, FundingStreamError> ValidateFundingStream(
        const Consensus::Params& params,
        const int startHeight,
        const int endHeight,
        const std::vector<FundingStreamAddress>& addresses
    );

    static FundingStream ParseFundingStream(
        const Consensus::Params& params,
        const KeyConstants& keyConstants,
        const int startHeight,
        const int endHeight,
        const std::vector<std::string>& strAddresses);

    int GetStartHeight() const { return startHeight; };
    int GetEndHeight() const { return endHeight; };
    const std::vector<FundingStreamAddress>& GetAddresses() const {
        return addresses;
    };

    FundingStreamAddress RecipientAddress(const Params& params, int nHeight) const;
};

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    /**
     * Returns true if the given network upgrade is active as of the given block
     * height. Caller must check that the height is >= 0 (and handle unknown
     * heights).
     */
    bool NetworkUpgradeActive(int nHeight, Consensus::UpgradeIndex idx) const;

    uint256 hashGenesisBlock;

    bool fCoinbaseMustBeShielded;

    int nSubsidyHalvingInterval;

    /**
     * Identify the halving index at the specified height. The result will be
     * negative during the slow-start period.
     */
    int Halving(int nHeight) const;

    /**
     * Get the block height of the specified halving.
     */
    int HalvingHeight(int halvingIndex) const;

    /**
     * Get the block height of the first block at which the community fee is
     * active.
     */
    int GetCommunityFeeStartHeight() const;

    /**
     * Get the block height of the last block at which the community fee is
     * active.
     */
    int GetLastCommunityFeeBlockHeight() const;

    int FundingPeriodIndex(int fundingStreamStartHeight, int nHeight) const;

    /** Used to check majorities for block version upgrade */
    int nMajorityEnforceBlockUpgrade;
    int nMajorityRejectBlockOutdated;
    int nMajorityWindow;
    NetworkUpgrade vUpgrades[MAX_NETWORK_UPGRADES];

    int nFundingPeriodLength;
    std::optional<FundingStream> vFundingStreams[MAX_FUNDING_STREAMS];
    void AddZIP207FundingStream(
        const KeyConstants& keyConstants,
        FundingStreamIndex idx,
        int startHeight,
        int endHeight,
        const std::vector<std::string>& addresses);

    /** Proof of work parameters */
    unsigned int nEquihashN = 144;
    unsigned int nEquihashK = 5;
    uint256 powLimit;
    std::optional<uint32_t> nPowAllowMinDifficultyBlocksAfterHeight;
    int64_t nPowAveragingWindow;
    int64_t nPowMaxAdjustDown;
    int64_t nPowMaxAdjustUp;
    int64_t nPowTargetSpacing;

    int64_t PoWTargetSpacing(int nHeight) const;
    int64_t AveragingWindowTimespan(int nHeight) const;
    int64_t MinActualTimespan(int nHeight) const;
    int64_t MaxActualTimespan(int nHeight) const;

    uint256 nMinimumChainWork;

    int vCommunityFeeStartHeight;
    int vCommunityFeeLastHeight;
};

} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
