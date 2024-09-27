#include <gtest/gtest.h>

#include "main.h"
#include "utilmoneystr.h"
#include "chainparams.h"
#include "fs.h"
#include "utilstrencodings.h"
#include "zcash/Address.hpp"
#include "wallet/wallet.h"
#include "amount.h"
#include <memory>
#include <string>
#include <set>
#include <vector>
#include "util.h"
#include "utiltest.h"

// To run tests:
// ./bitcoinz-gtest --gtest_filter="FoundersRewardTest.*"

//
// Enable this test to generate and print 48 testnet 2-of-3 multisig addresses.
// The output can be copied into chainparams.cpp.
// The temporary wallet file can be renamed as wallet.dat and used for testing with bitcoinzd.
//
#if 0
TEST(FoundersRewardTest, create_testnet_2of3multisig) {
    SelectParams(CBaseChainParams::TESTNET);
    fs::path pathTemp = fs::temp_directory_path() / fs::unique_path();
    fs::create_directories(pathTemp);
    mapArgs["-datadir"] = pathTemp.string();
    bool fFirstRun;
    auto pWallet = std::make_shared<CWallet>("wallet.dat");
    ASSERT_EQ(DB_LOAD_OK, pWallet->LoadWallet(fFirstRun));
    pWallet->TopUpKeyPool();
    std::cout << "Test wallet and logs saved in folder: " << pathTemp.native() << std::endl;

    int numKeys = 48;
    std::vector<CPubKey> pubkeys;
    pubkeys.resize(3);
    CPubKey newKey;
    std::vector<std::string> addresses;
    for (int i = 0; i < numKeys; i++) {
        ASSERT_TRUE(pWallet->GetKeyFromPool(newKey));
        pubkeys[0] = newKey;
        pWallet->SetAddressBook(newKey.GetID(), "", "receive");

        ASSERT_TRUE(pWallet->GetKeyFromPool(newKey));
        pubkeys[1] = newKey;
        pWallet->SetAddressBook(newKey.GetID(), "", "receive");

        ASSERT_TRUE(pWallet->GetKeyFromPool(newKey));
        pubkeys[2] = newKey;
        pWallet->SetAddressBook(newKey.GetID(), "", "receive");

        CScript result = GetScriptForMultisig(2, pubkeys);
        ASSERT_FALSE(result.size() > MAX_SCRIPT_ELEMENT_SIZE);
        CScriptID innerID(result);
        pWallet->AddCScript(result);
        pWallet->SetAddressBook(innerID, "", "receive");

        std::string address = EncodeDestination(innerID);
        addresses.push_back(address);
    }

    // Print out the addresses, 4 on each line.
    std::string s = "vCommunityFeeAddress = {\n";
    int i=0;
    int colsPerRow = 4;
    ASSERT_TRUE(numKeys % colsPerRow == 0);
    int numRows = numKeys/colsPerRow;
    for (int row=0; row<numRows; row++) {
        s += "    ";
        for (int col=0; col<colsPerRow; col++) {
            s += "\"" + addresses[i++] + "\", ";
        }
        s += "\n";
    }
    s += "    };";
    std::cout << s << std::endl;

    pWallet->Flush(true);
}
#endif

// Utility method to check the number of unique addresses from 1 to maxHeight
void checkNumberOfUniqueAddresses(int nUnique) {
    std::set<std::string> addresses;
    for (int i = 1; i <= Params().GetLastCommunityFeeBlockHeight(); i++) {
        addresses.insert(Params().GetCommunityFeeAddressAtHeight(i));
    }
    EXPECT_EQ(addresses.size(), nUnique);
}


TEST(FoundersRewardTest, General) {
    SelectParams(CBaseChainParams::TESTNET);

    CChainParams params = Params();

    // Fourth testnet reward:
    // address = t2EwBFfC96DCiCAcJuEqGUbUes8rTNmaD6Q
    // script.ToString() = OP_HASH160 5bfbeb4df59710514b7004041e75ad287dad9bc8 OP_EQUAL
    // HexStr(script) = a9145bfbeb4df59710514b7004041e75ad287dad9bc887
    EXPECT_EQ(HexStr(params.GetCommunityFeeScriptAtHeight(1)), "a91465a7c41acd34d55e7001a02d68c39f5470ae38cf87");
    EXPECT_EQ(params.GetCommunityFeeAddressAtHeight(1), "t2FpKCWt95LAPVRed61YbBny9yz5nqexLGN");
    EXPECT_EQ(HexStr(params.GetCommunityFeeScriptAtHeight(53126)), "a9145bfbeb4df59710514b7004041e75ad287dad9bc887");
    EXPECT_EQ(params.GetCommunityFeeAddressAtHeight(53126), "t2EwBFfC96DCiCAcJuEqGUbUes8rTNmaD6Q");
    EXPECT_EQ(HexStr(params.GetCommunityFeeScriptAtHeight(53127)), "a9145bfbeb4df59710514b7004041e75ad287dad9bc887");
    EXPECT_EQ(params.GetCommunityFeeAddressAtHeight(53127), "t2EwBFfC96DCiCAcJuEqGUbUes8rTNmaD6Q");

    int minHeight = params.GetCommunityFeeStartHeight();
    int maxHeight = params.GetLastCommunityFeeBlockHeight();

    // If the block height parameter is out of bounds, there is an assert.
    EXPECT_DEATH(params.GetCommunityFeeScriptAtHeight(0), "nHeight");
    EXPECT_DEATH(params.GetCommunityFeeScriptAtHeight(maxHeight+1), "nHeight");
    EXPECT_DEATH(params.GetCommunityFeeAddressAtHeight(0), "nHeight");
    EXPECT_DEATH(params.GetCommunityFeeAddressAtHeight(maxHeight+1), "nHeight");
}

TEST(FoundersRewardTest, RegtestGetLastBlockBlossom) {
    int blossomActivationHeight = Consensus::PRE_BLOSSOM_REGTEST_HALVING_INTERVAL / 2; // = 75
    auto params = RegtestActivateBlossom(false, blossomActivationHeight);
    int lastCFHeight = Params().GetLastCommunityFeeBlockHeight();
    EXPECT_EQ(0, params.Halving(lastCFHeight));
    RegtestDeactivateBlossom();
}

TEST(FoundersRewardTest, MainnetGetLastBlock) {
    SelectParams(CBaseChainParams::MAIN);
    auto params = Params().GetConsensus();
    int lastCFHeight = Params().GetLastCommunityFeeBlockHeight();
    EXPECT_EQ(1, params.Halving(lastCFHeight));
}

#define NUM_MAINNET_FOUNDER_ADDRESSES 100

TEST(FoundersRewardTest, Mainnet) {
    SelectParams(CBaseChainParams::MAIN);
    checkNumberOfUniqueAddresses(NUM_MAINNET_FOUNDER_ADDRESSES);
}


#define NUM_TESTNET_FOUNDER_ADDRESSES 100

TEST(FoundersRewardTest, Testnet) {
    SelectParams(CBaseChainParams::TESTNET);
    checkNumberOfUniqueAddresses(NUM_TESTNET_FOUNDER_ADDRESSES);
}


#define NUM_REGTEST_FOUNDER_ADDRESSES 1

TEST(FoundersRewardTest, Regtest) {
    SelectParams(CBaseChainParams::REGTEST);
    checkNumberOfUniqueAddresses(NUM_REGTEST_FOUNDER_ADDRESSES);
}



// Test that 5% community fee is fully rewarded in a defined period.
// On Mainnet: nHeight > 328500 && nHeight <= 1400000 (494687187.5 BTCZ)
TEST(FoundersRewardTest, SlowStartSubsidy) {
    SelectParams(CBaseChainParams::MAIN);
    CChainParams params = Params();

    int minHeight = params.GetCommunityFeeStartHeight();
    int maxHeight = params.GetLastCommunityFeeBlockHeight();

    CAmount totalSubsidy = 0;
    for (int nHeight = 1; nHeight <= maxHeight; nHeight++) {
        CAmount nSubsidy = GetBlockSubsidy(nHeight, params.GetConsensus()) * 0.05;
	if (nHeight > minHeight) {
            totalSubsidy += nSubsidy;
        }
    }

    EXPECT_EQ(totalSubsidy, 49468718750000000);
}


// For use with mainnet and testnet which each have 100 addresses.
// Verify the number of rewards each individual address receives.
// Since on the main network vCommunityFeeStartHeight does not start from 0,
// the first 22 elements of vCommunityFeeAddress are skipped.
void verifyNumberOfRewards(bool fMainNet) {
    CChainParams params = Params();

    int minHeight = params.GetCommunityFeeStartHeight();
    int maxHeight = params.GetLastCommunityFeeBlockHeight();

    std::map<std::string, CAmount> ms;
    for (int nHeight = 1; nHeight <= maxHeight; nHeight++) {
      std::string addr = params.GetCommunityFeeAddressAtHeight(nHeight);
      if (ms.count(addr) == 0) {
          ms[addr] = 0;
      }
      if (nHeight > minHeight) {
          ms[addr] = ms[addr] + GetBlockSubsidy(nHeight, params.GetConsensus()) * 0.05;
      }
    }

    if (fMainNet) {
        EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(0)], 0 * 625 * COIN);
        EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(1)], 0 * 625 * COIN);
        EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(22)], 0 * 625 * COIN);
        EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(23)], 7523 * 625 * COIN);
    } else {
        EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(0)], 12500 * 625 * COIN);
        EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(1)], 14001 * 625 * COIN);
        EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(22)], 14001 * 625 * COIN);
        EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(23)], 14001 * 625 * COIN);
    }

    for (int i = 24; i <= 58; i++) {
        EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(i)], 14001 * 625 * COIN);
    }

    EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(59)], 8731875 * COIN);

    for (int i = 60; i <= 98; i++) {
        EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(i)], 14001 * 312.5 * COIN);
    }

    EXPECT_EQ(ms[params.GetCommunityFeeAddressAtIndex(99)], 13902 * 312.5 * COIN);
}

// Verify the number of rewards going to each mainnet address
TEST(FoundersRewardTest, PerAddressRewardMainnet) {
    SelectParams(CBaseChainParams::MAIN);
    verifyNumberOfRewards(true);
}

// Verify the number of rewards going to each testnet address
TEST(FoundersRewardTest, PerAddressRewardTestnet) {
    SelectParams(CBaseChainParams::TESTNET);
    verifyNumberOfRewards(false);
}
