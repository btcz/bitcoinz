// Copyright (c) 2020 The Bitcoinz Community
// Copyright (c) 2019 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "params.h"

#include <amount.h>
#include <key_io.h>
#include <script/standard.h>
#include "upgrades.h"

#include <variant>

namespace Consensus {
    bool Params::NetworkUpgradeActive(int nHeight, Consensus::UpgradeIndex idx) const {
        return NetworkUpgradeState(nHeight, *this, idx) == UPGRADE_ACTIVE;
    }

    int Params::Halving(int nHeight) const {
        return (nHeight / nSubsidyHalvingInterval);
    }

    /**
     * This method determines the block height of the `halvingIndex`th
     * halving, as known at the specified `nHeight` block height.
     *
     * Previous implementations of this logic were specialized to the
     * first halving.
     */
    int Params::HalvingHeight(int halvingIndex) const {
        assert(halvingIndex > 0);

        return (nSubsidyHalvingInterval * halvingIndex);
    }

    int Params::GetCommunityFeeStartHeight() const {
        return vCommunityFeeStartHeight;
    }

    int Params::GetLastCommunityFeeBlockHeight() const {
        return vCommunityFeeLastHeight;
    }

    int Params::FundingPeriodIndex(int fundingStreamStartHeight, int nHeight) const {
        int firstHalvingHeight = HalvingHeight(1);

        // If the start height of the funding period is not aligned to a multiple of the
        // funding period length, the first funding period will be shorter than the
        // funding period length.
        auto startPeriodOffset = (fundingStreamStartHeight - firstHalvingHeight) % nFundingPeriodLength;
        if (startPeriodOffset < 0) startPeriodOffset += nFundingPeriodLength; // C++ '%' is remainder, not modulus!

        return (nHeight - fundingStreamStartHeight + startPeriodOffset) / nFundingPeriodLength;
    }

    std::variant<FundingStream, FundingStreamError> FundingStream::ValidateFundingStream(
        const Consensus::Params& params,
        const int startHeight,
        const int endHeight,
        const std::vector<FundingStreamAddress>& addresses
    ) {
        if (!params.NetworkUpgradeActive(startHeight, Consensus::UPGRADE_CANOPY)) {
            return FundingStreamError::CANOPY_NOT_ACTIVE;
        }

        if (endHeight < startHeight) {
            return FundingStreamError::ILLEGAL_RANGE;
        }

        if (params.FundingPeriodIndex(startHeight, endHeight - 1) >= addresses.size()) {
            return FundingStreamError::INSUFFICIENT_ADDRESSES;
        }

        return FundingStream(startHeight, endHeight, addresses);
    };

    class GetFundingStreamOrThrow {
    public:
        FundingStream operator()(const FundingStream& fs) const {
            return fs;
        }

        FundingStream operator()(const FundingStreamError& e) const {
            switch (e) {
                case FundingStreamError::CANOPY_NOT_ACTIVE:
                    throw std::runtime_error("Canopy network upgrade not active at funding stream start height.");
                case FundingStreamError::ILLEGAL_RANGE:
                    throw std::runtime_error("Illegal start/end height combination for funding stream.");
                case FundingStreamError::INSUFFICIENT_ADDRESSES:
                    throw std::runtime_error("Insufficient payment addresses to fully exhaust funding stream.");
                default:
                    throw std::runtime_error("Unrecognized error validating funding stream.");
            };
        }
    };

    FundingStream FundingStream::ParseFundingStream(
        const Consensus::Params& params,
        const KeyConstants& keyConstants,
        const int startHeight,
        const int endHeight,
        const std::vector<std::string>& strAddresses)
    {
        KeyIO keyIO(keyConstants);

        // Parse the address strings into concrete types.
        std::vector<FundingStreamAddress> addresses;
        for (auto addr : strAddresses) {
            auto taddr = keyIO.DecodeDestination(addr);
            if (IsValidDestination(taddr)) {
                addresses.push_back(GetScriptForDestination(taddr));
            }
        }

        auto validationResult = FundingStream::ValidateFundingStream(params, startHeight, endHeight, addresses);
        return std::visit(GetFundingStreamOrThrow(), validationResult);
    };

    void Params::AddZIP207FundingStream(
        const KeyConstants& keyConstants,
        FundingStreamIndex idx,
        int startHeight,
        int endHeight,
        const std::vector<std::string>& strAddresses)
    {
        if (startHeight >= 0) {
            vFundingStreams[idx] = FundingStream::ParseFundingStream(*this, keyConstants, startHeight, endHeight, strAddresses);
        }
    };

    FundingStreamAddress FundingStream::RecipientAddress(const Consensus::Params& params, int nHeight) const
    {
        auto addressIndex = params.FundingPeriodIndex(startHeight, nHeight);

        assert(addressIndex >= 0 && addressIndex < addresses.size());
        return addresses[addressIndex];
    };

    int64_t Params::PoWTargetSpacing(int nHeight) const {
        return nPowTargetSpacing;
    }

    int64_t Params::AveragingWindowTimespan(int nHeight) const {
        return nPowAveragingWindow * PoWTargetSpacing(nHeight);
    }

    int64_t Params::MinActualTimespan(int nHeight) const {
        return (AveragingWindowTimespan(nHeight) * (100 - nPowMaxAdjustUp)) / 100;
    }

    int64_t Params::MaxActualTimespan(int nHeight) const {
        return (AveragingWindowTimespan(nHeight) * (100 + nPowMaxAdjustDown)) / 100;
    }
}
