// Copyright (c) 2020 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include <consensus/funding.h>

namespace Consensus
{
/**
 * General information about each funding stream.
 * Ordered by Consensus::FundingStreamIndex.
 */
constexpr struct FSInfo FundingStreamInfo[Consensus::MAX_FUNDING_STREAMS] = {
    {
        .recipient = "Electric Coin Company",
        .specification = "https://zips.z.cash/zip-0214",
        .valueNumerator = 7,
        .valueDenominator = 100,
    },
    {
        .recipient = "Zcash Foundation",
        .specification = "https://zips.z.cash/zip-0214",
        .valueNumerator = 5,
        .valueDenominator = 100,
    },
    {
        .recipient = "Major Grants",
        .specification = "https://zips.z.cash/zip-0214",
        .valueNumerator = 8,
        .valueDenominator = 100,
    }
};

static constexpr bool validateFundingStreamInfo(uint32_t idx) {
    return (idx >= Consensus::MAX_FUNDING_STREAMS || (
        FundingStreamInfo[idx].valueNumerator < FundingStreamInfo[idx].valueDenominator &&
        FundingStreamInfo[idx].valueNumerator < (INT64_MAX / (MAX_MONEY / 1000)) &&
        validateFundingStreamInfo(idx + 1)));
}
static_assert(
    validateFundingStreamInfo(Consensus::FIRST_FUNDING_STREAM),
    "Invalid FundingStreamInfo");

CAmount FSInfo::Value(CAmount blockSubsidy) const
{
    // Integer division is floor division for nonnegative integers in C++
    return CAmount((blockSubsidy * valueNumerator) / valueDenominator);
}

std::set<FundingStreamElement> GetActiveFundingStreamElements(
    int nHeight,
    CAmount blockSubsidy,
    const Consensus::Params& params)
{
    std::set<std::pair<FundingStreamAddress, CAmount>> requiredElements;

    // Funding streams are disabled if Canopy is not active.
    if (params.NetworkUpgradeActive(nHeight, Consensus::UPGRADE_CANOPY)) {
        for (uint32_t idx = Consensus::FIRST_FUNDING_STREAM; idx < Consensus::MAX_FUNDING_STREAMS; idx++) {
            // The following indexed access is safe as Consensus::MAX_FUNDING_STREAMS is used
            // in the definition of vFundingStreams.
            auto fs = params.vFundingStreams[idx];
            // Funding period is [startHeight, endHeight)
            if (fs && nHeight >= fs.value().GetStartHeight() && nHeight < fs.value().GetEndHeight()) {
                requiredElements.insert(std::make_pair(
                    fs.value().RecipientAddress(params, nHeight),
                    FundingStreamInfo[idx].Value(blockSubsidy)));
            }
        }
    }

    return requiredElements;
};

std::vector<FSInfo> GetActiveFundingStreams(
    int nHeight,
    const Consensus::Params& params)
{
    std::vector<FSInfo> activeStreams;

    // Funding streams are disabled if Canopy is not active.
    if (params.NetworkUpgradeActive(nHeight, Consensus::UPGRADE_CANOPY)) {
        for (uint32_t idx = Consensus::FIRST_FUNDING_STREAM; idx < Consensus::MAX_FUNDING_STREAMS; idx++) {
            auto fs = params.vFundingStreams[idx];
            if (fs && nHeight >= fs.value().GetStartHeight() && nHeight < fs.value().GetEndHeight()) {
                activeStreams.push_back(FundingStreamInfo[idx]);
            }
        }
    }

    return activeStreams;
};

} // namespace Consensus
