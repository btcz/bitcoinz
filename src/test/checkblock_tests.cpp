// Copyright (c) 2013-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "clientversion.h"
#include "consensus/validation.h"
#include "consensus/merkle.h"
#include "fs.h"
#include "main.h"
#include "proof_verifier.h"
#include "test/test_bitcoin.h"
#include "utiltime.h"
#include "util.h"
#include "zcash/Proof.hpp"

#include <cstdio>
#include <utility>

#include <boost/test/unit_test.hpp>


BOOST_FIXTURE_TEST_SUITE(CheckBlock_tests, BasicTestingSetup)

bool read_block(const std::string& filename, CBlock& block)
{
    fs::path testFile = fs::current_path() / "data" / filename;
#ifdef TEST_DATA_DIR
    if (!fs::exists(testFile))
    {
        testFile = fs::path(BOOST_PP_STRINGIZE(TEST_DATA_DIR)) / filename;
    }
#endif
    FILE* fp = fsbridge::fopen(testFile, "rb");
    if (!fp) return false;

    fseek(fp, 8, SEEK_SET); // skip msgheader/size

    CAutoFile filein(fp, SER_DISK, CLIENT_VERSION);
    if (filein.IsNull()) return false;

    filein >> block;

    return true;
}

BOOST_AUTO_TEST_CASE(May15)
{
    // Putting a 1MB binary file in the git repository is not a great
    // idea, so this test is only run if you manually download
    // test/data/Mar12Fork.dat from
    // http://sourceforge.net/projects/bitcoin/files/Bitcoin/blockchain/Mar12Fork.dat/download
    unsigned int tMay15 = 1368576000;
    SetMockTime(tMay15); // Test as if it was right at May 15

    CBlock forkingBlock;
    if (read_block("Mar12Fork.dat", forkingBlock))
    {
        CValidationState state;

        // After May 15'th, big blocks are OK:
        forkingBlock.nTime = tMay15; // Invalidates PoW
        auto verifier = ProofVerifier::Strict();
        BOOST_CHECK(CheckBlock(forkingBlock, state, Params(), verifier, false, false));
    }

    SetMockTime(0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(CheckBlock_cache_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(blocks_without_joinsplits_keep_the_validation_cache)
{
    CBlock block = Params().GenesisBlock();
    block.fChecked = false;
    CValidationState preliminary;
    auto disabled = ProofVerifier::Disabled();
    BOOST_REQUIRE(CheckBlock(block, preliminary, Params(), disabled));
    // There are no Sprout proofs to omit in this block, so the preliminary
    // result remains reusable without repeating PoW or Merkle validation.
    BOOST_CHECK(block.fChecked);

    CValidationState finalState;
    auto strict = ProofVerifier::Strict();
    BOOST_REQUIRE(CheckBlock(block, finalState, Params(), strict));
    BOOST_CHECK(block.fChecked);
}

BOOST_AUTO_TEST_CASE(partial_header_checks_do_not_cache_full_validation)
{
    for (const auto checks : {std::make_pair(false, true),
                              std::make_pair(true, false),
                              std::make_pair(false, false)}) {
        CBlock block = Params().GenesisBlock();
        block.fChecked = false;
        CValidationState state;
        auto strict = ProofVerifier::Strict();
        BOOST_REQUIRE(CheckBlock(block, state, Params(), strict,
                                 checks.first, checks.second));
        BOOST_CHECK(!block.fChecked);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(CheckBlock_historical_proof_tests, JoinSplitTestingSetup)

BOOST_AUTO_TEST_CASE(strict_result_survives_preliminary_check_and_disk_roundtrip)
{
    // Public historical block from issue #118. This fixture is only checked
    // locally; it is never submitted to a node or broadcast to a network.
    CBlock block;
    BOOST_REQUIRE(CheckBlock_tests::read_block("bitcoinz-1789967.dat", block));
    BOOST_REQUIRE_EQUAL(block.GetHash().GetHex(),
        "0000002e04e291438a422e2d5e9d6d67ac87059afefeb03136ff9f21711bb359");

    auto strict = ProofVerifier::Strict();
    CValidationState directState;
    BOOST_REQUIRE(!CheckBlock(block, directState, Params(), strict));
    const std::string rejection = directState.GetRejectReason();
    BOOST_REQUIRE_EQUAL(rejection, "bad-txns-joinsplit-verification-failed");
    BOOST_CHECK(!block.fChecked);

    auto disabled = ProofVerifier::Disabled();
    CValidationState preliminaryState;
    BOOST_REQUIRE(CheckBlock(block, preliminaryState, Params(), disabled));
    BOOST_CHECK(!block.fChecked);

    CValidationState afterPreliminaryState;
    BOOST_CHECK(!CheckBlock(block, afterPreliminaryState, Params(), strict));
    BOOST_CHECK_EQUAL(afterPreliminaryState.GetRejectReason(), rejection);

    CDataStream disk(SER_DISK, CLIENT_VERSION);
    disk << block;
    CBlock fromDisk;
    disk >> fromDisk;
    CValidationState diskState;
    BOOST_CHECK(!CheckBlock(fromDisk, diskState, Params(), strict));
    BOOST_CHECK_EQUAL(diskState.GetRejectReason(), rejection);
}

// Restore global arguments even if a BOOST_REQUIRE aborts a test.
struct HistoricalCompatibilityOption {
    const std::map<std::string, std::string> saved = mapArgs;
    HistoricalCompatibilityOption() { mapArgs["-experimentalaccept1789967"] = "1"; }
    ~HistoricalCompatibilityOption() { mapArgs = saved; }
};

BOOST_AUTO_TEST_CASE(historical_opt_in_is_not_a_strict_cache_result)
{
    HistoricalCompatibilityOption option;
    CBlock block;
    BOOST_REQUIRE(CheckBlock_tests::read_block("bitcoinz-1789967.dat", block));
    auto strict = ProofVerifier::Strict();
    auto disabled = ProofVerifier::Disabled();
    CValidationState preliminary, accepted;
    BOOST_REQUIRE(CheckBlock(block, preliminary, Params(), disabled));
    BOOST_CHECK(!block.fChecked);
    BOOST_REQUIRE(CheckBlock(block, accepted, Params(), strict));
    BOOST_CHECK(!block.fChecked);

    CDataStream disk(SER_DISK, CLIENT_VERSION);
    disk << block;
    CBlock fromDisk;
    disk >> fromDisk;
    CValidationState restored;
    BOOST_REQUIRE(CheckBlock(fromDisk, restored, Params(), strict));
    BOOST_CHECK(!fromDisk.fChecked);

    mapArgs["-experimentalaccept1789967"] = "0";
    for (const CBlock* candidate : {&block, &fromDisk}) {
        CValidationState rejected;
        BOOST_CHECK(!CheckBlock(*candidate, rejected, Params(), strict));
        BOOST_CHECK_EQUAL(rejected.GetRejectReason(), "bad-txns-joinsplit-verification-failed");
    }
}

BOOST_AUTO_TEST_CASE(historical_opt_in_does_not_cover_other_headers_or_networks)
{
    HistoricalCompatibilityOption option;
    CBlock block;
    BOOST_REQUIRE(CheckBlock_tests::read_block("bitcoinz-1789967.dat", block));
    auto strict = ProofVerifier::Strict();
    CBlock changedHeader = block;
    ++changedHeader.nTime;
    CValidationState changed;
    BOOST_CHECK(!CheckBlock(changedHeader, changed, Params(), strict, false, true));
    BOOST_CHECK_EQUAL(changed.GetRejectReason(), "bad-txns-joinsplit-verification-failed");

    for (const std::string& network : {CBaseChainParams::TESTNET, CBaseChainParams::REGTEST}) {
        CValidationState wrongNetwork;
        BOOST_CHECK(!CheckBlock(block, wrongNetwork, Params(network), strict, false, true));
        BOOST_CHECK_EQUAL(wrongNetwork.GetRejectReason(), "bad-txns-joinsplit-verification-failed");
    }
    // Standalone/mempool transaction validation remains strict even with opt-in.
    CValidationState transaction;
    BOOST_CHECK(!CheckTransaction(block.vtx.at(1), transaction, strict));
    BOOST_CHECK_EQUAL(transaction.GetRejectReason(), "bad-txns-joinsplit-verification-failed");
}

BOOST_AUTO_TEST_CASE(historical_opt_in_always_binds_the_body_to_the_header)
{
    HistoricalCompatibilityOption option;
    CBlock block;
    BOOST_REQUIRE(CheckBlock_tests::read_block("bitcoinz-1789967.dat", block));
    // Retain the historical header but replace its body with genesis's body.
    // Partial-check callers must not accidentally use the header as a wildcard.
    block.vtx = Params().GenesisBlock().vtx;
    auto strict = ProofVerifier::Strict();
    CValidationState changedBody;
    BOOST_CHECK(!CheckBlock(block, changedBody, Params(), strict, false, false));
    BOOST_CHECK_EQUAL(changedBody.GetRejectReason(), "bad-txnmrklroot");
    BOOST_CHECK(!block.fChecked);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(CheckBlock_inflation_block_tests, JoinSplitTestingSetup)

// Mainnet block 1860181 created 105 billion BTCZ through Sprout JoinSplits
// with all-zero proofs. Like the fixture above, it is only checked locally.
static CBlock ReadInflationBlock()
{
    CBlock block;
    BOOST_REQUIRE(CheckBlock_tests::read_block("bitcoinz-1860181.dat", block));
    BOOST_REQUIRE_EQUAL(block.GetHash().GetHex(),
        "000001963db0caee911c3409e3d1f1b868cb3fbfacc7e7e7422d5d3b80cbb37d");
    return block;
}

BOOST_AUTO_TEST_CASE(inflation_block_proofs_fail_after_a_preliminary_check)
{
    CBlock block = ReadInflationBlock();
    auto disabled = ProofVerifier::Disabled();
    CValidationState preliminary;
    BOOST_REQUIRE(CheckBlock(block, preliminary, Params(), disabled));
    BOOST_CHECK(!block.fChecked);

    // This is the call ConnectBlock makes. Before the fix it returned true
    // from the cache and the zero proofs were never verified.
    auto strict = ProofVerifier::Strict();
    CValidationState state;
    BOOST_CHECK(!CheckBlock(block, state, Params(), strict));
    BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-txns-joinsplit-verification-failed");
}

BOOST_AUTO_TEST_CASE(inflation_block_is_rejected_without_a_ban)
{
    CBlock block = ReadInflationBlock();
    uint256 prevHash = block.hashPrevBlock;
    CBlockIndex prev;
    prev.nHeight = 1860180;
    prev.phashBlock = &prevHash;

    CValidationState state;
    BOOST_CHECK(!ContextualCheckBlock(block, state, Params(), &prev));
    BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-blk-inflation");
    int nDoS = -1;
    BOOST_CHECK(state.IsInvalid(nDoS));
    BOOST_CHECK_EQUAL(nDoS, 0);
}

BOOST_AUTO_TEST_CASE(joinsplits_are_disabled_on_mainnet_from_height_1860181)
{
    CBlock block = ReadInflationBlock();
    const CTransaction& tx = block.vtx.at(1);
    BOOST_REQUIRE_EQUAL(tx.vJoinSplit.size(), 1);
    bool (*notInIBD)(const CChainParams&) = [](const CChainParams&) { return false; };

    CValidationState disabled;
    BOOST_CHECK(!ContextualCheckTransaction(tx, disabled, Params(), 1860181, 100, notInIBD));
    BOOST_CHECK_EQUAL(disabled.GetRejectReason(), "bad-txns-joinsplit-disabled");

    CValidationState before;
    ContextualCheckTransaction(tx, before, Params(), 1860180, 100, notInIBD);
    BOOST_CHECK(before.GetRejectReason() != "bad-txns-joinsplit-disabled");

    for (const std::string& network : {CBaseChainParams::TESTNET, CBaseChainParams::REGTEST}) {
        CValidationState other;
        ContextualCheckTransaction(tx, other, Params(network), 1860181, 100, notInIBD);
        BOOST_CHECK(other.GetRejectReason() != "bad-txns-joinsplit-disabled");
    }
}

BOOST_AUTO_TEST_SUITE_END()
