// Copyright (c) 2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "chainparams.h"
#include "main.h"

#include "test/test_bitcoin.h"

#include <boost/signals2/signal.hpp>
#include <boost/test/unit_test.hpp>


BOOST_FIXTURE_TEST_SUITE(main_tests, TestingSetup)

const CAmount INITIAL_SUBSIDY = 12500 * COIN;

static int GetTotalHalvings(const Consensus::Params& consensusParams) {
    return 64;
}

static void TestBlockSubsidyHalvings(const Consensus::Params& consensusParams)
{
    int nHeight = 0;
    BOOST_CHECK_EQUAL(GetBlockSubsidy(nHeight, consensusParams), INITIAL_SUBSIDY);
    CAmount nPreviousSubsidy = INITIAL_SUBSIDY;
    for (int nHalvings = 1; nHalvings < GetTotalHalvings(consensusParams); nHalvings++) {
        nHeight = nHalvings * consensusParams.nSubsidyHalvingInterval;
        BOOST_CHECK_EQUAL(GetBlockSubsidy(nHeight - 1, consensusParams), nPreviousSubsidy);
        CAmount nSubsidy = GetBlockSubsidy(nHeight, consensusParams);
        BOOST_CHECK(nSubsidy <= INITIAL_SUBSIDY);
        BOOST_CHECK_EQUAL(nSubsidy, nPreviousSubsidy / 2);
        nPreviousSubsidy = nSubsidy;
    }
    BOOST_CHECK_EQUAL(GetBlockSubsidy(nHeight, consensusParams), 0);
}

static void TestBlockSubsidyHalvings(int nSubsidyHalvingInterval)
{
    Consensus::Params consensusParams;
    consensusParams.nSubsidyHalvingInterval = nSubsidyHalvingInterval;
    TestBlockSubsidyHalvings(consensusParams);
}

BOOST_AUTO_TEST_CASE(block_subsidy_test)
{
    TestBlockSubsidyHalvings(Params(CBaseChainParams::MAIN).GetConsensus()); // As in main
    TestBlockSubsidyHalvings(840000);
    TestBlockSubsidyHalvings(150); // As in regtest
    TestBlockSubsidyHalvings(1000); // Just another interval
}

BOOST_AUTO_TEST_CASE(subsidy_limit_test)
{
    const Consensus::Params& consensusParams = Params(CBaseChainParams::MAIN).GetConsensus();

    CAmount nSum = 0;
    int nHeight = 0;

    // Regular mining
    CAmount nSubsidy;
    do {
        nSubsidy = GetBlockSubsidy(nHeight, consensusParams);
        BOOST_CHECK(nSubsidy <= INITIAL_SUBSIDY);
        nSum += nSubsidy;
        BOOST_ASSERT(MoneyRange(nSum));
        ++nHeight;
    } while (nSubsidy > 0);

    // Changing the block interval from 10 to 2.5 minutes causes truncation
    // effects to occur earlier (from the 9th halving interval instead of the
    // 11th), decreasing the total monetary supply by 0.0693 ZEC.
    // BOOST_CHECK_EQUAL(nSum, 2099999997690000ULL);
    // Reducing the interval further to 1.25 minutes has a similar effect,
    // decreasing the total monetary supply by another 0.09240 ZEC.
    // BOOST_CHECK_EQUAL(nSum, 2099999990760000ULL);
    BOOST_CHECK_EQUAL(nSum, 2099999999988240000LL);
}

bool ReturnFalse() { return false; }
bool ReturnTrue() { return true; }

BOOST_AUTO_TEST_CASE(test_combiner_all)
{
    boost::signals2::signal<bool (), CombinerAll> Test;
    BOOST_CHECK(Test());
    Test.connect(&ReturnFalse);
    BOOST_CHECK(!Test());
    Test.connect(&ReturnTrue);
    BOOST_CHECK(!Test());
    Test.disconnect(&ReturnFalse);
    BOOST_CHECK(Test());
    Test.disconnect(&ReturnTrue);
    BOOST_CHECK(Test());
}

BOOST_AUTO_TEST_SUITE_END()
