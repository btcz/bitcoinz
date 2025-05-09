// Copyright (c) 2016 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "utiltest.h"

#include "consensus/upgrades.h"
#include "transaction_builder.h"

#include <array>

// Sprout
CMutableTransaction GetValidSproutReceiveTransaction(
                                const libzcash::SproutSpendingKey& sk,
                                CAmount value,
                                bool randomInputs,
                                uint32_t versionGroupId, /* = SAPLING_VERSION_GROUP_ID */
                                int32_t version /* = SAPLING_TX_VERSION */) {
    // We removed the ability to create pre-Sapling Sprout transactions
    assert(version >= SAPLING_TX_VERSION);

    CMutableTransaction mtx;
    mtx.fOverwintered = true;
    mtx.nVersionGroupId = versionGroupId;
    mtx.nVersion = version;
    mtx.vin.resize(2);
    if (randomInputs) {
        mtx.vin[0].prevout.hash = GetRandHash();
        mtx.vin[1].prevout.hash = GetRandHash();
    } else {
        mtx.vin[0].prevout.hash = uint256S("0000000000000000000000000000000000000000000000000000000000000001");
        mtx.vin[1].prevout.hash = uint256S("0000000000000000000000000000000000000000000000000000000000000002");
    }
    mtx.vin[0].prevout.n = 0;
    mtx.vin[1].prevout.n = 0;

    // Generate an ephemeral keypair.
    uint256 joinSplitPubKey;
    unsigned char joinSplitPrivKey[crypto_sign_SECRETKEYBYTES];
    crypto_sign_keypair(joinSplitPubKey.begin(), joinSplitPrivKey);
    mtx.joinSplitPubKey = joinSplitPubKey;

    std::array<libzcash::JSInput, 2> inputs = {
        libzcash::JSInput(), // dummy input
        libzcash::JSInput() // dummy input
    };

    std::array<libzcash::JSOutput, 2> outputs = {
        libzcash::JSOutput(sk.address(), value),
        libzcash::JSOutput(sk.address(), value)
    };

    // Prepare JoinSplits
    uint256 rt;
    JSDescription jsdesc {mtx.joinSplitPubKey, rt,
                          inputs, outputs, 2*value, 0, false};
    mtx.vJoinSplit.push_back(jsdesc);

    // Consider: The following is a bit misleading (given the name of this function)
    // and should perhaps be changed, but currently a few tests in test_wallet.cpp
    // depend on this happening.
    if (version >= 4) {
        // Shielded Output
        OutputDescription od;
        mtx.vShieldedOutput.push_back(od);
    }

    // Empty output script.
    uint32_t consensusBranchId = SPROUT_BRANCH_ID;
    CScript scriptCode;
    CTransaction signTx(mtx);
    uint256 dataToBeSigned = SignatureHash(scriptCode, signTx, NOT_AN_INPUT, SIGHASH_ALL, 0, consensusBranchId);

    // Add the signature
    assert(crypto_sign_detached(&mtx.joinSplitSig[0], NULL,
                                dataToBeSigned.begin(), 32,
                                joinSplitPrivKey
                               ) == 0);

    return mtx;
}

CWalletTx GetValidSproutReceive(const libzcash::SproutSpendingKey& sk,
                                CAmount value,
                                bool randomInputs,
                                uint32_t versionGroupId, /* = SAPLING_VERSION_GROUP_ID */
                                int32_t version /* = SAPLING_TX_VERSION */)
{
    CMutableTransaction mtx = GetValidSproutReceiveTransaction(
        sk, value, randomInputs, versionGroupId, version
    );
    CTransaction tx {mtx};
    CWalletTx wtx {NULL, tx};
    return wtx;
}

CWalletTx GetInvalidCommitmentSproutReceive(const libzcash::SproutSpendingKey& sk,
                                CAmount value,
                                bool randomInputs,
                                uint32_t versionGroupId, /* = SAPLING_VERSION_GROUP_ID */
                                int32_t version /* = SAPLING_TX_VERSION */)
{
    CMutableTransaction mtx = GetValidSproutReceiveTransaction(
        sk, value, randomInputs, versionGroupId, version
    );
    mtx.vJoinSplit[0].commitments[0] = uint256();
    mtx.vJoinSplit[0].commitments[1] = uint256();
    CTransaction tx {mtx};
    CWalletTx wtx {NULL, tx};
    return wtx;
}

libzcash::SproutNote GetSproutNote(const libzcash::SproutSpendingKey& sk,
                                   const CTransaction& tx, size_t js, size_t n) {
    ZCNoteDecryption decryptor {sk.receiving_key()};
    auto hSig = tx.vJoinSplit[js].h_sig(tx.joinSplitPubKey);
    auto note_pt = libzcash::SproutNotePlaintext::decrypt(
        decryptor,
        tx.vJoinSplit[js].ciphertexts[n],
        tx.vJoinSplit[js].ephemeralKey,
        hSig,
        (unsigned char) n);
    return note_pt.note(sk.address());
}

CWalletTx GetValidSproutSpend(const libzcash::SproutSpendingKey& sk,
                              const libzcash::SproutNote& note,
                              CAmount value) {
    CMutableTransaction mtx;
    mtx.fOverwintered = true;
    mtx.nVersionGroupId = SAPLING_VERSION_GROUP_ID;
    mtx.nVersion = SAPLING_TX_VERSION;
    mtx.vout.resize(2);
    mtx.vout[0].nValue = value;
    mtx.vout[1].nValue = 0;

    // Generate an ephemeral keypair.
    uint256 joinSplitPubKey;
    unsigned char joinSplitPrivKey[crypto_sign_SECRETKEYBYTES];
    crypto_sign_keypair(joinSplitPubKey.begin(), joinSplitPrivKey);
    mtx.joinSplitPubKey = joinSplitPubKey;

    // Fake tree for the unused witness
    SproutMerkleTree tree;

    libzcash::JSOutput dummyout;
    libzcash::JSInput dummyin;

    {
        if (note.value() > value) {
            libzcash::SproutSpendingKey dummykey = libzcash::SproutSpendingKey::random();
            libzcash::SproutPaymentAddress dummyaddr = dummykey.address();
            dummyout = libzcash::JSOutput(dummyaddr, note.value() - value);
        } else if (note.value() < value) {
            libzcash::SproutSpendingKey dummykey = libzcash::SproutSpendingKey::random();
            libzcash::SproutPaymentAddress dummyaddr = dummykey.address();
            libzcash::SproutNote dummynote(dummyaddr.a_pk, (value - note.value()), uint256(), uint256());
            tree.append(dummynote.cm());
            dummyin = libzcash::JSInput(tree.witness(), dummynote, dummykey);
        }
    }

    tree.append(note.cm());

    std::array<libzcash::JSInput, 2> inputs = {
        libzcash::JSInput(tree.witness(), note, sk),
        dummyin
    };

    std::array<libzcash::JSOutput, 2> outputs = {
        dummyout, // dummy output
        libzcash::JSOutput() // dummy output
    };

    // Prepare JoinSplits
    uint256 rt = tree.root();
    JSDescription jsdesc {mtx.joinSplitPubKey, rt,
                          inputs, outputs, 0, value, false};
    mtx.vJoinSplit.push_back(jsdesc);

    // Empty output script.
    uint32_t consensusBranchId = SPROUT_BRANCH_ID;
    CScript scriptCode;
    CTransaction signTx(mtx);
    uint256 dataToBeSigned = SignatureHash(scriptCode, signTx, NOT_AN_INPUT, SIGHASH_ALL, 0, consensusBranchId);

    // Add the signature
    assert(crypto_sign_detached(&mtx.joinSplitSig[0], NULL,
                                dataToBeSigned.begin(), 32,
                                joinSplitPrivKey
                               ) == 0);
    CTransaction tx {mtx};
    CWalletTx wtx {NULL, tx};
    return wtx;
}

// Sapling
const Consensus::Params& RegtestActivateSapling() {
    SelectParams(CBaseChainParams::REGTEST);
    UpdateNetworkUpgradeParameters(Consensus::UPGRADE_OVERWINTER, Consensus::NetworkUpgrade::ALWAYS_ACTIVE);
    UpdateNetworkUpgradeParameters(Consensus::UPGRADE_SAPLING, Consensus::NetworkUpgrade::ALWAYS_ACTIVE);
    return Params().GetConsensus();
}

void RegtestDeactivateSapling() {
    UpdateNetworkUpgradeParameters(Consensus::UPGRADE_SAPLING, Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT);
    UpdateNetworkUpgradeParameters(Consensus::UPGRADE_OVERWINTER, Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT);
}

const Consensus::Params& RegtestActivateCanopy(bool updatePow, int canopyActivationHeight) {
    SelectParams(CBaseChainParams::REGTEST);
    UpdateNetworkUpgradeParameters(Consensus::UPGRADE_OVERWINTER, Consensus::NetworkUpgrade::ALWAYS_ACTIVE);
    UpdateNetworkUpgradeParameters(Consensus::UPGRADE_SAPLING, Consensus::NetworkUpgrade::ALWAYS_ACTIVE);
    UpdateNetworkUpgradeParameters(Consensus::UPGRADE_CANOPY, canopyActivationHeight);
    if (updatePow) {
        UpdateRegtestPow(32, 16, uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
    }
    return Params().GetConsensus();
}

const Consensus::Params& RegtestActivateCanopy() {
    return RegtestActivateCanopy(false, Consensus::NetworkUpgrade::ALWAYS_ACTIVE);
}

void RegtestDeactivateCanopy() {
    UpdateRegtestPow(0, 0, uint256S("0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f"));
    UpdateNetworkUpgradeParameters(Consensus::UPGRADE_CANOPY, Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT);
    UpdateNetworkUpgradeParameters(Consensus::UPGRADE_SAPLING, Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT);
    UpdateNetworkUpgradeParameters(Consensus::UPGRADE_OVERWINTER, Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT);
    SelectParams(CBaseChainParams::MAIN);
}

libzcash::SaplingExtendedSpendingKey GetTestMasterSaplingSpendingKey() {
    std::vector<unsigned char, secure_allocator<unsigned char>> rawSeed(32);
    HDSeed seed(rawSeed);
    return libzcash::SaplingExtendedSpendingKey::Master(seed);
}

CKey AddTestCKeyToKeyStore(CBasicKeyStore& keyStore) {
    KeyIO keyIO(Params());
    CKey tsk = keyIO.DecodeSecret(T_SECRET_REGTEST);
    keyStore.AddKey(tsk);
    return tsk;
}

TestSaplingNote GetTestSaplingNote(const libzcash::SaplingPaymentAddress& pa, CAmount value) {
    // Generate dummy Sapling note
    libzcash::SaplingNote note(pa, value);
    uint256 cm = note.cmu().value();
    SaplingMerkleTree tree;
    tree.append(cm);
    return { note, tree };
}

CWalletTx GetValidSaplingReceive(const Consensus::Params& consensusParams,
                                 CBasicKeyStore& keyStore,
                                 const libzcash::SaplingExtendedSpendingKey &sk,
                                 CAmount value) {
    // From taddr
    CKey tsk = AddTestCKeyToKeyStore(keyStore);
    auto scriptPubKey = GetScriptForDestination(tsk.GetPubKey().GetID());
    // To zaddr
    auto fvk = sk.expsk.full_viewing_key();
    auto pa = sk.DefaultAddress();

    auto builder = TransactionBuilder(consensusParams, 1, &keyStore);
    builder.SetFee(0);
    builder.AddTransparentInput(COutPoint(), scriptPubKey, value);
    builder.AddSaplingOutput(fvk.ovk, pa, value, {});

    CTransaction tx = builder.Build().GetTxOrThrow();
    CWalletTx wtx {NULL, tx};
    return wtx;
}
