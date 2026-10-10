// Copyright (c) 2013-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "clientversion.h"
#include "consensus/validation.h"
#include "fs.h"
#include "main.h"
#include "proof_verifier.h"
#include "test/test_bitcoin.h"
#include "utiltime.h"
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

BOOST_AUTO_TEST_SUITE_END()
