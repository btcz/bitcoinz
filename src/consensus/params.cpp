// Copyright (c) 2020 The Bitcoinz Community
// Copyright (c) 2019 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "params.h"

#include "upgrades.h"

namespace Consensus {
    bool Params::NetworkUpgradeActive(int nHeight, Consensus::UpgradeIndex idx) const {
        return NetworkUpgradeState(nHeight, *this, idx) == UPGRADE_ACTIVE;
    }

    int Params::Halving(int nHeight) const {
        // zip208
        // Halving(height) :=
        // floor((height - SlowStartShift) / PreBlossomHalvingInterval), if not IsBlossomActivated(height)
        // floor((BlossomActivationHeight - SlowStartShift) / PreBlossomHalvingInterval + (height - BlossomActivationHeight) / PostBlossomHalvingInterval), otherwise
        if (NetworkUpgradeActive(nHeight, Consensus::UPGRADE_BLOSSOM)) {
            int64_t blossomActivationHeight = vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight;
            // Ideally we would say:
            // halvings = (blossomActivationHeight - consensusParams.SubsidySlowStartShift()) / consensusParams.nPreBlossomSubsidyHalvingInterval
            //     + (nHeight - blossomActivationHeight) / consensusParams.nPostBlossomSubsidyHalvingInterval;
            // But, (blossomActivationHeight - consensusParams.SubsidySlowStartShift()) / consensusParams.nPreBlossomSubsidyHalvingInterval
            // would need to be treated as a rational number in order for this to work.
            // Define scaledHalvings := halvings * consensusParams.nPostBlossomSubsidyHalvingInterval;
            int64_t scaledHalvings = ((blossomActivationHeight - SubsidySlowStartShift()) * Consensus::BLOSSOM_POW_TARGET_SPACING_RATIO)
                + (nHeight - blossomActivationHeight);
            return (int) (scaledHalvings / nPostBlossomSubsidyHalvingInterval);
        } else {
            return (nHeight - SubsidySlowStartShift()) / nPreBlossomSubsidyHalvingInterval;
        }
    }

    int64_t Params::PoWTargetSpacing(int nHeight) const {
        // zip208
        // PoWTargetSpacing(height) :=
        // PreBlossomPoWTargetSpacing, if not IsBlossomActivated(height)
        // PostBlossomPoWTargetSpacing, otherwise.
        bool blossomActive = NetworkUpgradeActive(nHeight, Consensus::UPGRADE_BLOSSOM);
        return blossomActive ? nPostBlossomPowTargetSpacing : nPreBlossomPowTargetSpacing;
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
