// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "clientversion.h"
#include "init.h"
#include "main.h"
#include "net.h"
#include "netbase.h"
#include "rpcserver.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#endif

#include <stdint.h>

#include <boost/assign/list_of.hpp>

#include <univalue.h>

#include "zcash/Address.hpp"

using namespace std;

/**
 * Return current blockchain status, wallet balance, address balance and the last 200 transactions
**/
UniValue getalldata(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "getalldata\n"
            "Return current blockchain status, wallet balance, address balance and the last 200 transactions.\n"
            "\nArguments:\n"
            "1. \"infoSelection\"     (integer, optional)\n"
            "                         Not provided: Return info\n"
            "                         Value of 0: Return address, balance, transactions and network\n"
            "                         Value of 1: Return address, balance and network\n"
            "                         Value of 2: Return transactions and network\n"
            "\nResult:\n"
            "{\n"
            "  \"connectionCount\": 8,\n"
            "  \"besttime\": 1536035862,\n"
            "  \"bestblockhash\": \"0000001230937d124f83751d0565f4e2b14525665e720bdcc0a62726a71c44b6\",\n"
            "  \"transparentbalance\": \"0.001\",\n"
            "  \"privatebalance\": \"0.00\",\n"
            "  \"lockedbalance\": \"0.00\",\n"
            "  \"totalbalance\": \"0.001\",\n"
            "  \"unconfirmedbalance\": \"0.00\",\n"
            "  \"immaturebalance\": \"0.00\",\n"
            "  \"addressbalance\": [\n"
            "    {\n"
            "        \"t1P8tafpz8rcBz7RntTWjQanka6sojk8bWf\": {\n"
            "            \"amount\": 0.00100000,\n"
            "                \"ismine\": true\n"
            "        }\n"
            "    }\n"
            "  ],\n"
            "  \"listtransactions\": [\n"
            "    {\n"
            "        \"account\": \"\",\n"
            "            \"address\": \"t1P8tafpz8rcBz7RntTWjQanka6sojk8bWf\",\n"
            "            \"category\": \"receive\",\n"
            "            \"amount\": 0.00100000,\n"
            "            \"vout\": 0,\n"
            "            \"confirmations\": 0,\n"
            "            \"txid\": \"011abbccb177bede25a51b85e7178d8dddab36bdf6fa9f07a05bf6fab97c6b0e\",\n"
            "            \"walletconflicts\": [\n"
            "            ],\n"
            "            \"time\": 1536036490,\n"
            "            \"timereceived\": 1536036490,\n"
            "            \"vjoinsplit\": [\n"
            "            ],\n"
            "            \"size\": 225\n"
            "    }\n"
            "  ]\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getalldata", "0")
            + HelpExampleRpc("getalldata", "0")
        );

    LOCK(cs_main);

    UniValue returnObj(UniValue::VOBJ);
    int connectionCount = 0;
    {
        LOCK2(cs_main, cs_vNodes);
        connectionCount = (int)vNodes.size();
    }

    int nMinDepth = 1;
    CAmount nBalance = getBalanceTaddr("", nMinDepth, true);
    CAmount nPrivateBalance = getBalanceZaddr("", nMinDepth, true);
    CAmount nLockedCoin = pwalletMain->GetLockedCoins();

    CAmount nTotalBalance = nBalance + nPrivateBalance + nLockedCoin;

    returnObj.push_back(Pair("connectionCount", connectionCount));
    returnObj.push_back(Pair("besttime", chainActive.Tip()->GetBlockTime()));
    returnObj.push_back(Pair("bestblockhash", chainActive.Tip()->GetBlockHash().GetHex()));
    returnObj.push_back(Pair("transparentbalance", FormatMoney(nBalance)));
    returnObj.push_back(Pair("privatebalance", FormatMoney(nPrivateBalance)));
    returnObj.push_back(Pair("lockedbalance", FormatMoney(nLockedCoin)));
    returnObj.push_back(Pair("totalbalance", FormatMoney(nTotalBalance)));
    returnObj.push_back(Pair("unconfirmedbalance", FormatMoney(pwalletMain->GetUnconfirmedBalance())));
    returnObj.push_back(Pair("immaturebalance", FormatMoney(pwalletMain->GetImmatureBalance())));

    //get address balance
    nBalance = 0;

    //get all t address
    UniValue addressbalance(UniValue::VARR);
    UniValue addrlist(UniValue::VOBJ);

    if (params.size() > 0 && (params[0].get_int() == 1 || params[0].get_int() == 0))
    {
      BOOST_FOREACH(const PAIRTYPE(CBitcoinAddress, CAddressBookData)& item, pwalletMain->mapAddressBook)
      {
          UniValue addr(UniValue::VOBJ);
          const CBitcoinAddress& address = item.first;
          CTxDestination dest = address.Get();
          isminetype mine = pwalletMain ? IsMine(*pwalletMain, dest) : ISMINE_NO;

          const string& strName = item.second.name;
          nBalance = getBalanceTaddr(address.ToString(), nMinDepth, false);

          addr.push_back(Pair("amount", ValueFromAmount(nBalance)));
          addr.push_back(Pair("ismine", (mine & ISMINE_SPENDABLE) ? true : false));
          addrlist.push_back(Pair(address.ToString(), addr));
      }

      //address grouping
      {
          LOCK2(cs_main, pwalletMain->cs_wallet);

          map<CTxDestination, CAmount> balances = pwalletMain->GetAddressBalances();
          BOOST_FOREACH(set<CTxDestination> grouping, pwalletMain->GetAddressGroupings())
          {
              BOOST_FOREACH(CTxDestination address, grouping)
              {
                  UniValue addr(UniValue::VOBJ);
                  const string& strName = CBitcoinAddress(address).ToString();
                  if(addrlist.exists(strName))
                      continue;
                  isminetype mine = pwalletMain ? IsMine(*pwalletMain, address) : ISMINE_NO;

                  nBalance = getBalanceTaddr(strName, nMinDepth, false);

                  addr.push_back(Pair("amount", ValueFromAmount(nBalance)));
                  addr.push_back(Pair("ismine", (mine & ISMINE_SPENDABLE) ? true : false));
                  addrlist.push_back(Pair(strName, addr));
              }
          }
      }

      //get all z address
      std::set<libzcash::PaymentAddress> addresses;
      pwalletMain->GetPaymentAddresses(addresses);
      for (auto addr : addresses ) {
          if (pwalletMain->HaveSpendingKey(addr)) {
              UniValue address(UniValue::VOBJ);
              const string& strName = CZCPaymentAddress(addr).ToString();
              nBalance = getBalanceZaddr(strName, nMinDepth, false);
              address.push_back(Pair("amount", ValueFromAmount(nBalance)));
              address.push_back(Pair("ismine", true));
              addrlist.push_back(Pair(strName, address));
          }
      }
    }

    addressbalance.push_back(addrlist);
    returnObj.push_back(Pair("addressbalance", addressbalance));


    //get transactions
    string strAccount = "";
    int nCount = 200;
    int nFrom = 0;
    isminefilter filter = ISMINE_SPENDABLE;

    UniValue trans(UniValue::VARR);
    if (params.size() > 0 && (params[0].get_int() == 2 || params[0].get_int() == 0))
    {
        std::list<CAccountingEntry> acentries;
        CWallet::TxItems txOrdered = pwalletMain->OrderedTxItems(acentries, strAccount);

        // iterate backwards until we have nCount items to return:
        for (CWallet::TxItems::reverse_iterator it = txOrdered.rbegin(); it != txOrdered.rend(); ++it)
        {
            CWalletTx *const pwtx = (*it).second.first;
            if (pwtx != 0)
                ListTransactions(*pwtx, strAccount, 0, true, trans, filter);
            CAccountingEntry *const pacentry = (*it).second.second;
            if (pacentry != 0)
                AcentryToJSON(*pacentry, strAccount, trans);

            if ((int)trans.size() >= (nCount+nFrom)) break;
        }

        // trans is newest to oldest
        if (nFrom > (int)trans.size())
            nFrom = trans.size();
        if ((nFrom + nCount) > (int)trans.size())
            nCount = trans.size() - nFrom;

        vector<UniValue> arrTmp = trans.getValues();

        vector<UniValue>::iterator first = arrTmp.begin();
        std::advance(first, nFrom);
        vector<UniValue>::iterator last = arrTmp.begin();
        std::advance(last, nFrom+nCount);

        if (last != arrTmp.end()) arrTmp.erase(last, arrTmp.end());
        if (first != arrTmp.begin()) arrTmp.erase(arrTmp.begin(), first);

        std::reverse(arrTmp.begin(), arrTmp.end()); // Return oldest to newest

        trans.clear();
        trans.setArray();
        trans.push_backV(arrTmp);
    }

    returnObj.push_back(Pair("listtransactions", trans));

    return returnObj;
}

/**
 * @note Do not add or change anything in the information returned by this
 * method. `getinfo` exists for backwards-compatibility only. It combines
 * information from wildly different sources in the program, which is a mess,
 * and is thus planned to be deprecated eventually.
 *
 * Based on the source of the information, new information should be added to:
 * - `getblockchaininfo`,
 * - `getnetworkinfo` or
 * - `getwalletinfo`
 *
 * Or alternatively, create a specific query method for the information.
 **/
UniValue getinfo(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getinfo\n"
            "Returns an object containing various state info.\n"
            "\nResult:\n"
            "{\n"
            "  \"version\": xxxxx,           (numeric) the server version\n"
            "  \"protocolversion\": xxxxx,   (numeric) the protocol version\n"
            "  \"walletversion\": xxxxx,     (numeric) the wallet version\n"
            "  \"balance\": xxxxxxx,         (numeric) the total Zcash balance of the wallet\n"
            "  \"blocks\": xxxxxx,           (numeric) the current number of blocks processed in the server\n"
            "  \"timeoffset\": xxxxx,        (numeric) the time offset\n"
            "  \"connections\": xxxxx,       (numeric) the number of connections\n"
            "  \"proxy\": \"host:port\",     (string, optional) the proxy used by the server\n"
            "  \"difficulty\": xxxxxx,       (numeric) the current difficulty\n"
            "  \"testnet\": true|false,      (boolean) if the server is using testnet or not\n"
            "  \"keypoololdest\": xxxxxx,    (numeric) the timestamp (seconds since GMT epoch) of the oldest pre-generated key in the key pool\n"
            "  \"keypoolsize\": xxxx,        (numeric) how many new keys are pre-generated\n"
            "  \"unlocked_until\": ttt,      (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked\n"
            "  \"paytxfee\": x.xxxx,         (numeric) the transaction fee set in " + CURRENCY_UNIT + "/kB\n"
            "  \"relayfee\": x.xxxx,         (numeric) minimum relay fee for non-free transactions in " + CURRENCY_UNIT + "/kB\n"
            "  \"errors\": \"...\"           (string) any error messages\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getinfo", "")
            + HelpExampleRpc("getinfo", "")
        );

#ifdef ENABLE_WALLET
    LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);
#else
    LOCK(cs_main);
#endif

    proxyType proxy;
    GetProxy(NET_IPV4, proxy);

    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("version", CLIENT_VERSION));
    obj.push_back(Pair("protocolversion", PROTOCOL_VERSION));
#ifdef ENABLE_WALLET
    if (pwalletMain) {
        obj.push_back(Pair("walletversion", pwalletMain->GetVersion()));
        obj.push_back(Pair("balance",       ValueFromAmount(pwalletMain->GetBalance())));
    }
#endif
    obj.push_back(Pair("blocks",        (int)chainActive.Height()));
    obj.push_back(Pair("timeoffset",    GetTimeOffset()));
    obj.push_back(Pair("connections",   (int)vNodes.size()));
    obj.push_back(Pair("proxy",         (proxy.IsValid() ? proxy.proxy.ToStringIPPort() : string())));
    obj.push_back(Pair("difficulty",    (double)GetDifficulty()));
    obj.push_back(Pair("testnet",       Params().TestnetToBeDeprecatedFieldRPC()));
#ifdef ENABLE_WALLET
    if (pwalletMain) {
        obj.push_back(Pair("keypoololdest", pwalletMain->GetOldestKeyPoolTime()));
        obj.push_back(Pair("keypoolsize",   (int)pwalletMain->GetKeyPoolSize()));
    }
    if (pwalletMain && pwalletMain->IsCrypted())
        obj.push_back(Pair("unlocked_until", nWalletUnlockTime));
    obj.push_back(Pair("paytxfee",      ValueFromAmount(payTxFee.GetFeePerK())));
#endif
    obj.push_back(Pair("relayfee",      ValueFromAmount(::minRelayTxFee.GetFeePerK())));
    obj.push_back(Pair("errors",        GetWarnings("statusbar")));
    return obj;
}

#ifdef ENABLE_WALLET
class DescribeAddressVisitor : public boost::static_visitor<UniValue>
{
public:
    UniValue operator()(const CNoDestination &dest) const { return UniValue(UniValue::VOBJ); }

    UniValue operator()(const CKeyID &keyID) const {
        UniValue obj(UniValue::VOBJ);
        CPubKey vchPubKey;
        obj.push_back(Pair("isscript", false));
        if (pwalletMain && pwalletMain->GetPubKey(keyID, vchPubKey)) {
            obj.push_back(Pair("pubkey", HexStr(vchPubKey)));
            obj.push_back(Pair("iscompressed", vchPubKey.IsCompressed()));
        }
        return obj;
    }

    UniValue operator()(const CScriptID &scriptID) const {
        UniValue obj(UniValue::VOBJ);
        CScript subscript;
        obj.push_back(Pair("isscript", true));
        if (pwalletMain && pwalletMain->GetCScript(scriptID, subscript)) {
            std::vector<CTxDestination> addresses;
            txnouttype whichType;
            int nRequired;
            ExtractDestinations(subscript, whichType, addresses, nRequired);
            obj.push_back(Pair("script", GetTxnOutputType(whichType)));
            obj.push_back(Pair("hex", HexStr(subscript.begin(), subscript.end())));
            UniValue a(UniValue::VARR);
            BOOST_FOREACH(const CTxDestination& addr, addresses)
                a.push_back(CBitcoinAddress(addr).ToString());
            obj.push_back(Pair("addresses", a));
            if (whichType == TX_MULTISIG)
                obj.push_back(Pair("sigsrequired", nRequired));
        }
        return obj;
    }
};
#endif

UniValue validateaddress(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "validateaddress \"zcashaddress\"\n"
            "\nReturn information about the given Zcash address.\n"
            "\nArguments:\n"
            "1. \"zcashaddress\"     (string, required) The Zcash address to validate\n"
            "\nResult:\n"
            "{\n"
            "  \"isvalid\" : true|false,         (boolean) If the address is valid or not. If not, this is the only property returned.\n"
            "  \"address\" : \"zcashaddress\",   (string) The Zcash address validated\n"
            "  \"scriptPubKey\" : \"hex\",       (string) The hex encoded scriptPubKey generated by the address\n"
            "  \"ismine\" : true|false,          (boolean) If the address is yours or not\n"
            "  \"isscript\" : true|false,        (boolean) If the key is a script\n"
            "  \"pubkey\" : \"publickeyhex\",    (string) The hex value of the raw public key\n"
            "  \"iscompressed\" : true|false,    (boolean) If the address is compressed\n"
            "  \"account\" : \"account\"         (string) DEPRECATED. The account associated with the address, \"\" is the default account\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("validateaddress", "\"1PSSGeFHDnKNxiEyFrD1wcEaHr9hrQDDWc\"")
            + HelpExampleRpc("validateaddress", "\"1PSSGeFHDnKNxiEyFrD1wcEaHr9hrQDDWc\"")
        );

#ifdef ENABLE_WALLET
    LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);
#else
    LOCK(cs_main);
#endif

    CBitcoinAddress address(params[0].get_str());
    bool isValid = address.IsValid();

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("isvalid", isValid));
    if (isValid)
    {
        CTxDestination dest = address.Get();
        string currentAddress = address.ToString();
        ret.push_back(Pair("address", currentAddress));

        CScript scriptPubKey = GetScriptForDestination(dest);
        ret.push_back(Pair("scriptPubKey", HexStr(scriptPubKey.begin(), scriptPubKey.end())));

#ifdef ENABLE_WALLET
        isminetype mine = pwalletMain ? IsMine(*pwalletMain, dest) : ISMINE_NO;
        ret.push_back(Pair("ismine", (mine & ISMINE_SPENDABLE) ? true : false));
        ret.push_back(Pair("iswatchonly", (mine & ISMINE_WATCH_ONLY) ? true: false));
        UniValue detail = boost::apply_visitor(DescribeAddressVisitor(), dest);
        ret.pushKVs(detail);
        if (pwalletMain && pwalletMain->mapAddressBook.count(dest))
            ret.push_back(Pair("account", pwalletMain->mapAddressBook[dest].name));
#endif
    }
    return ret;
}


UniValue z_validateaddress(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "z_validateaddress \"zaddr\"\n"
            "\nReturn information about the given z address.\n"
            "\nArguments:\n"
            "1. \"zaddr\"     (string, required) The z address to validate\n"
            "\nResult:\n"
            "{\n"
            "  \"isvalid\" : true|false,      (boolean) If the address is valid or not. If not, this is the only property returned.\n"
            "  \"address\" : \"zaddr\",         (string) The z address validated\n"
            "  \"ismine\" : true|false,       (boolean) If the address is yours or not\n"
            "  \"payingkey\" : \"hex\",         (string) The hex value of the paying key, a_pk\n"
            "  \"transmissionkey\" : \"hex\",   (string) The hex value of the transmission key, pk_enc\n"

            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("z_validateaddress", "\"zcWsmqT4X2V4jgxbgiCzyrAfRT1vi1F4sn7M5Pkh66izzw8Uk7LBGAH3DtcSMJeUb2pi3W4SQF8LMKkU2cUuVP68yAGcomL\"")
            + HelpExampleRpc("z_validateaddress", "\"zcWsmqT4X2V4jgxbgiCzyrAfRT1vi1F4sn7M5Pkh66izzw8Uk7LBGAH3DtcSMJeUb2pi3W4SQF8LMKkU2cUuVP68yAGcomL\"")
        );


#ifdef ENABLE_WALLET
    LOCK2(cs_main, pwalletMain->cs_wallet);
#else
    LOCK(cs_main);
#endif

    bool isValid = false;
    bool isMine = false;
    std::string payingKey, transmissionKey;

    string strAddress = params[0].get_str();
    try {
        CZCPaymentAddress address(strAddress);
        libzcash::PaymentAddress addr = address.Get();

#ifdef ENABLE_WALLET
        isMine = pwalletMain->HaveSpendingKey(addr);
#endif
        payingKey = addr.a_pk.GetHex();
        transmissionKey = addr.pk_enc.GetHex();
        isValid = true;
    } catch (std::runtime_error e) {
        // address is invalid, nop here as isValid is false.
    }

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("isvalid", isValid));
    if (isValid)
    {
        ret.push_back(Pair("address", strAddress));
        ret.push_back(Pair("payingkey", payingKey));
        ret.push_back(Pair("transmissionkey", transmissionKey));
#ifdef ENABLE_WALLET
        ret.push_back(Pair("ismine", isMine));
#endif
    }
    return ret;
}


/**
 * Used by addmultisigaddress / createmultisig:
 */
CScript _createmultisig_redeemScript(const UniValue& params)
{
    int nRequired = params[0].get_int();
    const UniValue& keys = params[1].get_array();

    // Gather public keys
    if (nRequired < 1)
        throw runtime_error("a multisignature address must require at least one key to redeem");
    if ((int)keys.size() < nRequired)
        throw runtime_error(
            strprintf("not enough keys supplied "
                      "(got %u keys, but need at least %d to redeem)", keys.size(), nRequired));
    if (keys.size() > 16)
        throw runtime_error("Number of addresses involved in the multisignature address creation > 16\nReduce the number");
    std::vector<CPubKey> pubkeys;
    pubkeys.resize(keys.size());
    for (unsigned int i = 0; i < keys.size(); i++)
    {
        const std::string& ks = keys[i].get_str();
#ifdef ENABLE_WALLET
        // Case 1: Bitcoin address and we have full public key:
        CBitcoinAddress address(ks);
        if (pwalletMain && address.IsValid())
        {
            CKeyID keyID;
            if (!address.GetKeyID(keyID))
                throw runtime_error(
                    strprintf("%s does not refer to a key",ks));
            CPubKey vchPubKey;
            if (!pwalletMain->GetPubKey(keyID, vchPubKey))
                throw runtime_error(
                    strprintf("no full public key for address %s",ks));
            if (!vchPubKey.IsFullyValid())
                throw runtime_error(" Invalid public key: "+ks);
            pubkeys[i] = vchPubKey;
        }

        // Case 2: hex public key
        else
#endif
        if (IsHex(ks))
        {
            CPubKey vchPubKey(ParseHex(ks));
            if (!vchPubKey.IsFullyValid())
                throw runtime_error(" Invalid public key: "+ks);
            pubkeys[i] = vchPubKey;
        }
        else
        {
            throw runtime_error(" Invalid public key: "+ks);
        }
    }
    CScript result = GetScriptForMultisig(nRequired, pubkeys);

    if (result.size() > MAX_SCRIPT_ELEMENT_SIZE)
        throw runtime_error(
                strprintf("redeemScript exceeds size limit: %d > %d", result.size(), MAX_SCRIPT_ELEMENT_SIZE));

    return result;
}

UniValue createmultisig(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 2)
    {
        string msg = "createmultisig nrequired [\"key\",...]\n"
            "\nCreates a multi-signature address with n signature of m keys required.\n"
            "It returns a json object with the address and redeemScript.\n"

            "\nArguments:\n"
            "1. nrequired      (numeric, required) The number of required signatures out of the n keys or addresses.\n"
            "2. \"keys\"       (string, required) A json array of keys which are Zcash addresses or hex-encoded public keys\n"
            "     [\n"
            "       \"key\"    (string) Zcash address or hex-encoded public key\n"
            "       ,...\n"
            "     ]\n"

            "\nResult:\n"
            "{\n"
            "  \"address\":\"multisigaddress\",  (string) The value of the new multisig address.\n"
            "  \"redeemScript\":\"script\"       (string) The string value of the hex-encoded redemption script.\n"
            "}\n"

            "\nExamples:\n"
            "\nCreate a multisig address from 2 addresses\n"
            + HelpExampleCli("createmultisig", "2 \"[\\\"t16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\\\",\\\"t171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\\\"]\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("createmultisig", "2, \"[\\\"t16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\\\",\\\"t171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\\\"]\"")
        ;
        throw runtime_error(msg);
    }

    // Construct using pay-to-script-hash:
    CScript inner = _createmultisig_redeemScript(params);
    CScriptID innerID(inner);
    CBitcoinAddress address(innerID);

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("address", address.ToString()));
    result.push_back(Pair("redeemScript", HexStr(inner.begin(), inner.end())));

    return result;
}

UniValue verifymessage(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 3)
        throw runtime_error(
            "verifymessage \"zcashaddress\" \"signature\" \"message\"\n"
            "\nVerify a signed message\n"
            "\nArguments:\n"
            "1. \"zcashaddress\"    (string, required) The Zcash address to use for the signature.\n"
            "2. \"signature\"       (string, required) The signature provided by the signer in base 64 encoding (see signmessage).\n"
            "3. \"message\"         (string, required) The message that was signed.\n"
            "\nResult:\n"
            "true|false   (boolean) If the signature is verified or not.\n"
            "\nExamples:\n"
            "\nUnlock the wallet for 30 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"mypassphrase\" 30") +
            "\nCreate the signature\n"
            + HelpExampleCli("signmessage", "\"t14oHp2v54vfmdgQ3v3SNuQga8JKHTNi2a1\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"t14oHp2v54vfmdgQ3v3SNuQga8JKHTNi2a1\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("verifymessage", "\"t14oHp2v54vfmdgQ3v3SNuQga8JKHTNi2a1\", \"signature\", \"my message\"")
        );

    LOCK(cs_main);

    string strAddress  = params[0].get_str();
    string strSign     = params[1].get_str();
    string strMessage  = params[2].get_str();

    CBitcoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    bool fInvalid = false;
    vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

    if (fInvalid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey;
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig))
        return false;

    return (pubkey.GetID() == keyID);
}

UniValue setmocktime(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "setmocktime timestamp\n"
            "\nSet the local time to given timestamp (-regtest only)\n"
            "\nArguments:\n"
            "1. timestamp  (integer, required) Unix seconds-since-epoch timestamp\n"
            "   Pass 0 to go back to using the system time."
        );

    if (!Params().MineBlocksOnDemand())
        throw runtime_error("setmocktime for regression testing (-regtest mode) only");

    // cs_vNodes is locked and node send/receive times are updated
    // atomically with the time change to prevent peers from being
    // disconnected because we think we haven't communicated with them
    // in a long time.
    LOCK2(cs_main, cs_vNodes);

    RPCTypeCheck(params, boost::assign::list_of(UniValue::VNUM));
    SetMockTime(params[0].get_int64());

    uint64_t t = GetTime();
    BOOST_FOREACH(CNode* pnode, vNodes) {
        pnode->nLastSend = pnode->nLastRecv = t;
    }

    return NullUniValue;
}
