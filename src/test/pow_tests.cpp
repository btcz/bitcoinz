// Copyright (c) 2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "main.h"
#include "pow.h"
#include "util.h"
#include "utiltest.h"
#include "test/test_bitcoin.h"

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)

/* Test calculation of next difficulty target with no constraints applying */
BOOST_AUTO_TEST_CASE(get_next_work)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();
    BOOST_CHECK_EQUAL(150, params.PoWTargetSpacing(0));

    int64_t nLastRetargetTime = 1000000000; // NOTE: Not an actual block time
    int64_t nThisTime = 1000003570;
    arith_uint256 bnAvg;
    bnAvg.SetCompact(0x1d00ffff);
    BOOST_CHECK_EQUAL(0x1d01352a,
    CalculateNextWorkRequired(bnAvg, nThisTime, nLastRetargetTime, params, 0));
}

/* Test the constraint on the upper bound for next work */
BOOST_AUTO_TEST_CASE(get_next_work_pow_limit)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    int64_t nLastRetargetTime = 1231006505;
    int64_t nThisTime = 1233061996;
    arith_uint256 bnAvg;
    bnAvg.SetCompact(0x1f07ffff);
    BOOST_CHECK_EQUAL(0x1f07ffff,
    CalculateNextWorkRequired(bnAvg, nThisTime, nLastRetargetTime, params, 0));
}

/* Test the constraint on the lower bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_lower_limit_actual)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    int64_t nLastRetargetTime = 1000000000; // NOTE: Not an actual block time
    // 17*150*(1 - PoWMaxAdjustUp*PoWDampingFactor) = 918
    // so we pick 917 to be outside of this window
    int64_t nThisTime = 100000917;
    arith_uint256 bnAvg;
    bnAvg.SetCompact(0x1c05a3f4);
    BOOST_CHECK_EQUAL(0x1c03b902,
    CalculateNextWorkRequired(bnAvg, nThisTime, nLastRetargetTime, params, 0));
}

/* Test the constraint on the upper bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_upper_limit_actual)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    int64_t nLastRetargetTime = 1000000000; // NOTE: Not an actual block time
    // 17*150*(1 + maxAdjustDown*PoWDampingFactor) = 5814
    int64_t nThisTime = 1000005815;
    arith_uint256 bnAvg;
    bnAvg.SetCompact(0x1c387f6f);
    BOOST_CHECK_EQUAL(0x1c4bb500,
    CalculateNextWorkRequired(bnAvg, nThisTime, nLastRetargetTime, params, 0));
}

void GetBlockProofEquivalentTimeImpl(const Consensus::Params& params) {
    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : NULL;
        blocks[i].nHeight = i;
        blocks[i].nTime = i ? blocks[i - 1].nTime + params.PoWTargetSpacing(i) : 1269211443;
        blocks[i].nBits = 0x207fffff; /* target 0x7fffff000... */
        blocks[i].nChainWork = i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i - 1]) : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[GetRand(10000)];
        CBlockIndex *p2 = &blocks[GetRand(10000)];
        CBlockIndex *p3 = &blocks[GetRand(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, params);
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test)
{
    SelectParams(CBaseChainParams::MAIN);
    GetBlockProofEquivalentTimeImpl(Params().GetConsensus());
}

BOOST_AUTO_TEST_SUITE_END()