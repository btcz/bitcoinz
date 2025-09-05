// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2016-2018 The Zcash developers
// Copyright (c) 2017-2025 The BitcoinZ Community
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef BITCOIN_KEYIO_H
#define BITCOIN_KEYIO_H

#include <chainparams.h>
#include <key.h>
#include <pubkey.h>
#include <script/standard.h>
#include <zcash/Address.hpp>

#include <vector>
#include <string>

class KeyIO {
private:
    const KeyConstants& keyConstants;

public:
    KeyIO(const KeyConstants& keyConstants): keyConstants(keyConstants) { }

    CKey DecodeSecret(const std::string& str);
    std::string EncodeSecret(const CKey& key);

    CExtKey DecodeExtKey(const std::string& str);
    std::string EncodeExtKey(const CExtKey& extkey);
    CExtPubKey DecodeExtPubKey(const std::string& str);
    std::string EncodeExtPubKey(const CExtPubKey& extpubkey);

    std::string EncodeDestination(const CTxDestination& dest);
    CTxDestination DecodeDestination(const std::string& str);

    bool IsValidDestinationString(const std::string& str);

    std::string EncodePaymentAddress(const libzcash::PaymentAddress& zaddr);
    libzcash::PaymentAddress DecodePaymentAddress(const std::string& str);
    bool IsValidPaymentAddressString(const std::string& str);

    std::string EncodeViewingKey(const libzcash::ViewingKey& vk);
    libzcash::ViewingKey DecodeViewingKey(const std::string& str);

    std::string EncodeSpendingKey(const libzcash::SpendingKey& zkey);
    libzcash::SpendingKey DecodeSpendingKey(const std::string& str);
};

#endif // BITCOIN_KEYIO_H
