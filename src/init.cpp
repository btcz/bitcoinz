// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include "init.h"
#include "crypto/common.h"
#include "addrman.h"
#include "amount.h"
#include "checkpoints.h"
#include "compat/sanity.h"
#include "consensus/upgrades.h"
#include "consensus/validation.h"
#include "experimental_features.h"
#include "fs.h"
#include "httpserver.h"
#include "httprpc.h"
#include "key.h"
#ifdef ENABLE_MINING
#include "key_io.h"
#endif
#include "main.h"
#include "mempool_limit.h"
#include "metrics.h"
#include "miner.h"
#include "net.h"
#include "policy/policy.h"
#include "rpc/server.h"
#include "rpc/register.h"
#include "script/standard.h"
#include "scheduler.h"
#include "txdb.h"
#include "torcontrol.h"
#include "ui_interface.h"
#include "util.h"
#include "utilmoneystr.h"
#include "validationinterface.h"
#ifdef ENABLE_WALLET
#include "key_io.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#endif
#include "warnings.h"
#include <stdint.h>
#include <stdio.h>

#ifndef WIN32
#include <signal.h>
#endif

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/bind/bind.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/thread.hpp>

#if ENABLE_ZMQ
#include "zmq/zmqnotificationinterface.h"
#endif

#include "librustzcash.h"

using namespace boost::placeholders;

bool fFeeEstimatesInitialized = false;
static const bool DEFAULT_PROXYRANDOMIZE = true;
static const bool DEFAULT_REST_ENABLE = false;
static const bool DEFAULT_DISABLE_SAFEMODE = false;
static const bool DEFAULT_STOPAFTERBLOCKIMPORT = false;

// The time that the wallet will wait for the block index to load
// during startup before timing out.
static const int64_t WALLET_INITIAL_SYNC_TIMEOUT = 1000 * 60 * 60 * 2;

#if ENABLE_ZMQ
static CZMQNotificationInterface* pzmqNotificationInterface = NULL;
#endif

#ifdef WIN32
// Win32 LevelDB doesn't use file descriptors, and the ones used for
// accessing block files don't count towards the fd_set size limit
// anyway.
#define MIN_CORE_FILEDESCRIPTORS 0
#else
#define MIN_CORE_FILEDESCRIPTORS 150
#endif

/** Used to pass flags to the Bind() function */
enum BindFlags {
    BF_NONE         = 0,
    BF_EXPLICIT     = (1U << 0),
    BF_REPORT_ERROR = (1U << 1),
    BF_WHITELIST    = (1U << 2),
};

static const char* FEE_ESTIMATES_FILENAME="fee_estimates.dat";
CClientUIInterface uiInterface; // Declared but not defined in ui_interface.h

//////////////////////////////////////////////////////////////////////////////
//
// Shutdown
//

//
// Thread management and startup/shutdown:
//
// The network-processing threads are all part of a thread group
// created by AppInit().
//
// A clean exit happens when StartShutdown() or the SIGTERM
// signal handler sets fRequestShutdown, which triggers
// the DetectShutdownThread(), which interrupts the main thread group.
// DetectShutdownThread() then exits, which causes AppInit() to
// continue (it .joins the shutdown thread).
// Shutdown() is then
// called to clean up database connections, and stop other
// threads that should only be stopped after the main network-processing
// threads have exited.
//
// Note that if running -daemon the parent process returns from AppInit2
// before adding any threads to the threadGroup, so .join_all() returns
// immediately and the parent exits from main().
//

std::atomic<bool> fRequestShutdown(false);

void StartShutdown()
{
    fRequestShutdown = true;
}
bool ShutdownRequested()
{
    return fRequestShutdown;
}

class CCoinsViewErrorCatcher : public CCoinsViewBacked
{
public:
    CCoinsViewErrorCatcher(CCoinsView* view) : CCoinsViewBacked(view) {}
    bool GetCoins(const uint256 &txid, CCoins &coins) const {
        try {
            return CCoinsViewBacked::GetCoins(txid, coins);
        } catch(const std::runtime_error& e) {
            uiInterface.ThreadSafeMessageBox(_("Error reading from database, shutting down."), "", CClientUIInterface::MSG_ERROR);
            LogPrintf("Error reading from database: %s\n", e.what());
            // Starting the shutdown sequence and returning false to the caller would be
            // interpreted as 'entry not found' (as opposed to unable to read data), and
            // could lead to invalid interpretation. Just exit immediately, as we can't
            // continue anyway, and all writes should be atomic.
            abort();
        }
    }
    // Writes do not need similar protection, as failure to write is handled by the caller.
};

static CCoinsViewDB *pcoinsdbview = NULL;
static CCoinsViewErrorCatcher *pcoinscatcher = NULL;
static boost::scoped_ptr<ECCVerifyHandle> globalVerifyHandle;

static boost::thread_group threadGroup;
static CScheduler scheduler;

void Interrupt()
{
    InterruptHTTPServer();
    InterruptHTTPRPC();
    InterruptRPC();
    InterruptREST();
    InterruptTorControl();
}

void Shutdown()
{
    LogPrintf("%s: In progress...\n", __func__);
    static CCriticalSection cs_Shutdown;
    TRY_LOCK(cs_Shutdown, lockShutdown);
    if (!lockShutdown)
        return;

    /// Note: Shutdown() must be able to handle cases in which AppInit2() failed part of the way,
    /// for example if the data directory was found to be locked.
    /// Be sure that anything that writes files or flushes caches only does this if the respective
    /// module was initialized.
    RenameThread("bitcoinz-shutoff");
    mempool.AddTransactionsUpdated(1);

    StopHTTPRPC();
    StopREST();
    StopRPC();
    StopHTTPServer();
#ifdef ENABLE_WALLET
    if (pwalletMain)
        pwalletMain->Flush(false);
#endif
#ifdef ENABLE_MINING
    GenerateBitcoins(false, 0, Params());
#endif
    StopNode();
    StopTorControl();

    // After everything has been shut down, but before things get flushed, stop the
    // CScheduler/checkqueue threadGroup
    threadGroup.interrupt_all();
    threadGroup.join_all();

    UnregisterNodeSignals(GetNodeSignals());

    if (fFeeEstimatesInitialized)
    {
        fs::path est_path = GetDataDir() / FEE_ESTIMATES_FILENAME;
        CAutoFile est_fileout(fsbridge::fopen(est_path, "wb"), SER_DISK, CLIENT_VERSION);
        if (!est_fileout.IsNull())
            mempool.WriteFeeEstimates(est_fileout);
        else
            LogPrintf("%s: Failed to write fee estimates to %s\n", __func__, est_path.string());
        fFeeEstimatesInitialized = false;
    }

    {
        LOCK(cs_main);
        if (pcoinsTip != NULL) {
            FlushStateToDisk();
        }
        delete pcoinsTip;
        pcoinsTip = NULL;
        delete pcoinscatcher;
        pcoinscatcher = NULL;
        delete pcoinsdbview;
        pcoinsdbview = NULL;
        delete pblocktree;
        pblocktree = NULL;
    }
#ifdef ENABLE_WALLET
    if (pwalletMain)
        pwalletMain->Flush(true);
#endif

#if ENABLE_ZMQ
    if (pzmqNotificationInterface) {
        UnregisterValidationInterface(pzmqNotificationInterface);
        delete pzmqNotificationInterface;
        pzmqNotificationInterface = NULL;
    }
#endif

#ifndef WIN32
    try {
        fs::remove(GetPidFile());
    } catch (const fs::filesystem_error& e) {
        LogPrintf("%s: Unable to remove pidfile: %s\n", __func__, e.what());
    }
#endif
    UnregisterAllValidationInterfaces();
#ifdef ENABLE_WALLET
    delete pwalletMain;
    pwalletMain = NULL;
#endif
    globalVerifyHandle.reset();
    ECC_Stop();
    LogPrintf("%s: done\n", __func__);
}

/**
 * Signal handlers are very limited in what they are allowed to do, so:
 */
void HandleSIGTERM(int)
{
    fRequestShutdown = true;
}

void HandleSIGHUP(int)
{
    fReopenDebugLog = true;
}

bool static InitError(const std::string &str)
{
    uiInterface.ThreadSafeMessageBox(str, "", CClientUIInterface::MSG_ERROR);
    return false;
}

bool static InitWarning(const std::string &str)
{
    uiInterface.ThreadSafeMessageBox(str, "", CClientUIInterface::MSG_WARNING);
    return true;
}

bool static Bind(const CService &addr, unsigned int flags) {
    if (!(flags & BF_EXPLICIT) && IsLimited(addr))
        return false;
    std::string strError;
    if (!BindListenPort(addr, strError, (flags & BF_WHITELIST) != 0)) {
        if (flags & BF_REPORT_ERROR)
            return InitError(strError);
        return false;
    }
    return true;
}

void OnRPCStopped()
{
    cvBlockChange.notify_all();
    LogPrint(BCLog::RPC, "RPC stopped.\n");
}

void OnRPCPreCommand(const CRPCCommand& cmd)
{
    // Observe safe mode
    std::string strWarning = GetWarnings("rpc");
    if (strWarning != "" && !GetBoolArg("-disablesafemode", DEFAULT_DISABLE_SAFEMODE) &&
        !cmd.okSafeMode)
        throw JSONRPCError(RPC_FORBIDDEN_BY_SAFE_MODE, std::string("Safe mode: ") + strWarning);
}

std::string HelpMessage(HelpMessageMode mode)
{
    const bool showDebug = GetBoolArg("-help-debug", false);

    // When adding new options to the categories, please keep and ensure alphabetical ordering.
    // Do not translate _(...) -help-debug options, many technical terms, and only a very small audience, so is unnecessary stress to translators

    std::string strUsage = HelpMessageGroup(_("Options:"));
    strUsage += HelpMessageOpt("-?", _("Print this help message and exit"));
    strUsage += HelpMessageOpt("-version", _("Print version and exit"));
    strUsage += HelpMessageOpt("-alertnotify=<cmd>", _("Execute command when a relevant alert is received or we see a really long fork (%s in cmd is replaced by message)"));
    strUsage += HelpMessageOpt("-blocknotify=<cmd>", _("Execute command when the best block changes (%s in cmd is replaced by block hash)"));
    if (showDebug)
        strUsage += HelpMessageOpt("-blocksonly", strprintf(_("Whether to reject transactions from network peers. Automatic broadcast and rebroadcast of any transactions from inbound peers is disabled, unless '-whitelistforcerelay' is '1', in which case whitelisted peers' transactions will be relayed. RPC transactions are not affected. (default: %u)"), DEFAULT_BLOCKSONLY));
    strUsage += HelpMessageOpt("-checkblocks=<n>", strprintf(_("How many blocks to check at startup (default: %u, 0 = all)"), DEFAULT_CHECKBLOCKS));
    strUsage += HelpMessageOpt("-checklevel=<n>", strprintf(_("How thorough the block verification of -checkblocks is (0-4, default: %u)"), DEFAULT_CHECKLEVEL));
    strUsage += HelpMessageOpt("-conf=<file>", strprintf(_("Specify configuration file. Relative paths will be prefixed by datadir location. (default: %s)"), BITCOIN_CONF_FILENAME));
    if (mode == HMM_BITCOIND)
    {
#ifndef WIN32
        strUsage += HelpMessageOpt("-daemon", _("Run in the background as a daemon and accept commands"));
#endif
    }
    strUsage += HelpMessageOpt("-datadir=<dir>", _("Specify data directory"));
    strUsage += HelpMessageOpt("-exportdir=<dir>", _("Specify directory to be used when exporting data"));
    strUsage += HelpMessageOpt("-dbcache=<n>", strprintf(_("Set database cache size in megabytes (%d to %d, default: %d)"), nMinDbCache, nMaxDbCache, nDefaultDbCache));
    strUsage += HelpMessageOpt("-feefilter", strprintf(_("Tell other nodes to filter invs to us by our mempool min fee (default: %u)"), DEFAULT_FEEFILTER));
    strUsage += HelpMessageOpt("-loadblock=<file>", _("Imports blocks from external blk000??.dat file on startup"));
    strUsage += HelpMessageOpt("-debuglogfile=<file>", strprintf(_("Specify location of debug log file. Relative paths will be prefixed by a net-specific datadir location. (default: %s)"), DEFAULT_DEBUGLOGFILE));
    strUsage += HelpMessageOpt("-maxorphantx=<n>", strprintf(_("Keep at most <n> unconnectable transactions in memory (default: %u)"), DEFAULT_MAX_ORPHAN_TRANSACTIONS));
    strUsage += HelpMessageOpt("-par=<n>", strprintf(_("Set the number of script verification threads (%u to %d, 0 = auto, <0 = leave that many cores free, default: %d)"),
        -GetNumCores(), MAX_SCRIPTCHECK_THREADS, DEFAULT_SCRIPTCHECK_THREADS));
#ifndef WIN32
    strUsage += HelpMessageOpt("-pid=<file>", strprintf(_("Specify pid file. Relative paths will be prefixed by a net-specific datadir location. (default: %s)"), BITCOIN_PID_FILENAME));
#endif
    strUsage += HelpMessageOpt("-prune=<n>", strprintf(_("Reduce storage requirements by pruning (deleting) old blocks. This mode disables wallet support and is incompatible with -txindex. "
            "Warning: Reverting this setting requires re-downloading the entire blockchain. "
            "(default: 0 = disable pruning blocks, >%u = target size in MiB to use for block files)"), MIN_DISK_SPACE_FOR_BLOCK_FILES / 1024 / 1024));
#ifdef ENABLE_WALLET
    strUsage += HelpMessageOpt("-reindex-chainstate", _("Rebuild chain state from the currently indexed blocks (implies -rescan unless pruning or unless -rescan=0 is explicitly specified"));
    strUsage += HelpMessageOpt("-reindex", _("Rebuild chain state and block index from the blk*.dat files on disk (implies -rescan unless pruning or unless -rescan=0 is explicitly specified)"));
#else
    strUsage += HelpMessageOpt("-reindex-chainstate", _("Rebuild chain state from the currently indexed blocks"));
    strUsage += HelpMessageOpt("-reindex", _("Rebuild chain state and block index from the blk*.dat files on disk"));
#endif
#ifndef WIN32
    strUsage += HelpMessageOpt("-sysperms", _("Create new files with system default permissions, instead of umask 077 (only effective with disabled wallet functionality)"));
#endif
    strUsage += HelpMessageOpt("-txexpirynotify=<cmd>", _("Execute command when transaction expires (%s in cmd is replaced by transaction id)"));
    strUsage += HelpMessageOpt("-txindex", strprintf(_("Maintain a full transaction index, used by the getrawtransaction rpc call (default: %u)"), DEFAULT_TXINDEX));

    strUsage += HelpMessageGroup(_("Connection options:"));
    strUsage += HelpMessageOpt("-addnode=<ip>", _("Add a node to connect to and attempt to keep the connection open"));
    strUsage += HelpMessageOpt("-banscore=<n>", strprintf(_("Threshold for disconnecting misbehaving peers (default: %u)"), DEFAULT_BANSCORE_THRESHOLD));
    strUsage += HelpMessageOpt("-bantime=<n>", strprintf(_("Number of seconds to keep misbehaving peers from reconnecting (default: %u)"), DEFAULT_MISBEHAVING_BANTIME));
    strUsage += HelpMessageOpt("-bind=<addr>", _("Bind to given address and always listen on it. Use [host]:port notation for IPv6"));
    strUsage += HelpMessageOpt("-connect=<ip>", _("Connect only to the specified node(s); -noconnect or -connect=0 alone to disable automatic connections"));
    strUsage += HelpMessageOpt("-discover", _("Discover own IP addresses (default: 1 when listening and no -externalip or -proxy)"));
    strUsage += HelpMessageOpt("-dns", _("Allow DNS lookups for -addnode, -seednode and -connect") + " " + strprintf(_("(default: %u)"), DEFAULT_NAME_LOOKUP));
    strUsage += HelpMessageOpt("-dnsseed", _("Query for peer addresses via DNS lookup, if low on addresses (default: 1 unless -connect/-noconnect)"));
    strUsage += HelpMessageOpt("-externalip=<ip>", _("Specify your own public address"));
    strUsage += HelpMessageOpt("-forcednsseed", strprintf(_("Always query for peer addresses via DNS lookup (default: %u)"), DEFAULT_FORCEDNSSEED));
    strUsage += HelpMessageOpt("-listen", _("Accept connections from outside (default: 1 if no -proxy or -connect/-noconnect)"));
    strUsage += HelpMessageOpt("-listenonion", strprintf(_("Automatically create Tor hidden service (default: %d)"), DEFAULT_LISTEN_ONION));
    strUsage += HelpMessageOpt("-maxconnections=<n>", strprintf(_("Maintain at most <n> connections to peers (default: %u)"), DEFAULT_MAX_PEER_CONNECTIONS));
    strUsage += HelpMessageOpt("-maxreceivebuffer=<n>", strprintf(_("Maximum per-connection receive buffer, <n>*1000 bytes (default: %u)"), DEFAULT_MAXRECEIVEBUFFER));
    strUsage += HelpMessageOpt("-maxsendbuffer=<n>", strprintf(_("Maximum per-connection send buffer, <n>*1000 bytes (default: %u)"), DEFAULT_MAXSENDBUFFER));
    strUsage += HelpMessageOpt("-mempoolevictionmemoryminutes=<n>", strprintf(_("The number of minutes before allowing rejected transactions to re-enter the mempool. (default: %u)"), DEFAULT_MEMPOOL_EVICTION_MEMORY_MINUTES));
    strUsage += HelpMessageOpt("-mempooltxcostlimit=<n>",strprintf(_("An upper bound on the maximum size in bytes of all transactions in the mempool. (default: %s)"), DEFAULT_MEMPOOL_TOTAL_COST_LIMIT));
    strUsage += HelpMessageOpt("-onion=<ip:port>", strprintf(_("Use separate SOCKS5 proxy to reach peers via Tor hidden services (default: %s)"), "-proxy"));
    strUsage += HelpMessageOpt("-onlynet=<net>", _("Only connect to nodes in network <net> (ipv4, ipv6 or onion)"));
    strUsage += HelpMessageOpt("-permitbaremultisig", strprintf(_("Relay non-P2SH multisig (default: %u)"), DEFAULT_PERMIT_BAREMULTISIG));
    strUsage += HelpMessageOpt("-peerbloomfilters", strprintf(_("Support filtering of blocks and transaction with bloom filters (default: %u)"), DEFAULT_PEERBLOOMFILTERS));
    if (showDebug)
        strUsage += HelpMessageOpt("-enforcenodebloom", strprintf("Enforce minimum protocol version to limit use of bloom filters (default: %u)", DEFAULT_ENFORCENODEBLOOM));
    strUsage += HelpMessageOpt("-port=<port>", strprintf(_("Listen for connections on <port> (default: %u or testnet: %u)"),
        Params(CBaseChainParams::MAIN).GetDefaultPort(), Params(CBaseChainParams::TESTNET).GetDefaultPort()));
    strUsage += HelpMessageOpt("-proxy=<ip:port>", _("Connect through SOCKS5 proxy"));
    strUsage += HelpMessageOpt("-proxyrandomize", strprintf(_("Randomize credentials for every proxy connection. This enables Tor stream isolation (default: %u)"), DEFAULT_PROXYRANDOMIZE));
    strUsage += HelpMessageOpt("-seednode=<ip>", _("Connect to a node to retrieve peer addresses, and disconnect"));
    strUsage += HelpMessageOpt("-timeout=<n>", strprintf(_("Specify connection timeout in milliseconds (minimum: 1, default: %d)"), DEFAULT_CONNECT_TIMEOUT));
    strUsage += HelpMessageOpt("-torcontrol=<ip>:<port>", strprintf(_("Tor control port to use if onion listening enabled (default: %s)"), DEFAULT_TOR_CONTROL));
    strUsage += HelpMessageOpt("-torpassword=<pass>", _("Tor control port password (default: empty)"));
    strUsage += HelpMessageOpt("-whitebind=<addr>", _("Bind to given address and whitelist peers connecting to it. Use [host]:port notation for IPv6"));
    strUsage += HelpMessageOpt("-whitelist=<netmask>", _("Whitelist peers connecting from the given netmask or IP address. Can be specified multiple times.") +
        " " + _("Whitelisted peers cannot be DoS banned and their transactions are always relayed, even if they are already in the mempool, useful e.g. for a gateway"));
    strUsage += HelpMessageOpt("-whitelistrelay", strprintf(_("Accept relayed transactions received from whitelisted inbound peers even when not relaying transactions (default: %d)"), DEFAULT_WHITELISTRELAY));
    strUsage += HelpMessageOpt("-whitelistforcerelay", strprintf(_("Force relay of transactions from whitelisted inbound peers even they violate local relay policy (default: %d)"), DEFAULT_WHITELISTFORCERELAY));
    strUsage += HelpMessageOpt("-maxuploadtarget=<n>", strprintf(_("Tries to keep outbound traffic under the given target (in MiB per 24h), 0 = no limit (default: %d)"), DEFAULT_MAX_UPLOAD_TARGET));

#ifdef ENABLE_WALLET
    strUsage += CWallet::GetWalletHelpString(showDebug);
#endif

#if ENABLE_ZMQ
    strUsage += HelpMessageGroup(_("ZeroMQ notification options:"));
    strUsage += HelpMessageOpt("-zmqpubhashblock=<address>", _("Enable publish hash block in <address>"));
    strUsage += HelpMessageOpt("-zmqpubhashtx=<address>", _("Enable publish hash transaction in <address>"));
    strUsage += HelpMessageOpt("-zmqpubrawblock=<address>", _("Enable publish raw block in <address>"));
    strUsage += HelpMessageOpt("-zmqpubrawtx=<address>", _("Enable publish raw transaction in <address>"));
#endif

    strUsage += HelpMessageGroup(_("Debugging/Testing options:"));
    strUsage += HelpMessageOpt("-uacomment=<cmt>", _("Append comment to the user agent string"));
    if (showDebug)
    {
        strUsage += HelpMessageOpt("-checkblockindex", strprintf("Do a full consistency check for mapBlockIndex, setBlockIndexCandidates, chainActive and mapBlocksUnlinked occasionally. Also sets -checkmempool (default: %u)", Params(CBaseChainParams::MAIN).DefaultConsistencyChecks()));
        strUsage += HelpMessageOpt("-checkmempool=<n>", strprintf("Run checks every <n> transactions (default: %u)", Params(CBaseChainParams::MAIN).DefaultConsistencyChecks()));
        strUsage += HelpMessageOpt("-checkpoints", strprintf("Disable expensive verification for known chain history (default: %u)", DEFAULT_CHECKPOINTS_ENABLED));
        strUsage += HelpMessageOpt("-disablesafemode", strprintf("Disable safemode, override a real safe mode event (default: %u)", DEFAULT_DISABLE_SAFEMODE));
        strUsage += HelpMessageOpt("-testsafemode", strprintf("Force safe mode (default: %u)", DEFAULT_TESTSAFEMODE));
        strUsage += HelpMessageOpt("-dropmessagestest=<n>", "Randomly drop 1 of every <n> network messages");
        strUsage += HelpMessageOpt("-fuzzmessagestest=<n>", "Randomly fuzz 1 of every <n> network messages");
        strUsage += HelpMessageOpt("-stopafterblockimport", strprintf("Stop running after importing blocks from disk (default: %u)", DEFAULT_STOPAFTERBLOCKIMPORT));
        strUsage += HelpMessageOpt("-limitancestorcount=<n>", strprintf("Do not accept transactions if number of in-mempool ancestors is <n> or more (default: %u)", DEFAULT_ANCESTOR_LIMIT));
        strUsage += HelpMessageOpt("-limitancestorsize=<n>", strprintf("Do not accept transactions whose size with all in-mempool ancestors exceeds <n> kilobytes (default: %u)", DEFAULT_ANCESTOR_SIZE_LIMIT));
        strUsage += HelpMessageOpt("-limitdescendantcount=<n>", strprintf("Do not accept transactions if any ancestor would have <n> or more in-mempool descendants (default: %u)", DEFAULT_DESCENDANT_LIMIT));
        strUsage += HelpMessageOpt("-limitdescendantsize=<n>", strprintf("Do not accept transactions if any ancestor would have more than <n> kilobytes of in-mempool descendants (default: %u).", DEFAULT_DESCENDANT_SIZE_LIMIT));
        strUsage += HelpMessageOpt("-nuparams=hexBranchId:activationHeight", "Use given activation height for specified network upgrade (regtest-only)");
        strUsage += HelpMessageOpt(
            "-fundingstream=streamId:startHeight:endHeight:comma_delimited_addresses",
            "Use given addresses for block subsidy share paid to the funding stream with id <streamId> (regtest-only)");
    }
    strUsage += HelpMessageOpt("-debug=<category>", strprintf(_("Output debugging information (default: %u, supplying <category> is optional)"), 0) + ". " +
        _("If <category> is not supplied or if <category> = 1, output all debugging information.") + " " + _("<category> can be:") + " " + ListLogCategories() + ".");
    strUsage += HelpMessageOpt("-experimentalfeatures", _("Enable use of experimental features"));
    strUsage += HelpMessageOpt("-help-debug", _("Show all debugging options (usage: --help -help-debug)"));
    strUsage += HelpMessageOpt("-logips", strprintf(_("Include IP addresses in debug output (default: %u)"), DEFAULT_LOGIPS));
    strUsage += HelpMessageOpt("-logtimestamps", strprintf(_("Prepend debug output with timestamp (default: %u)"), DEFAULT_LOGTIMESTAMPS));
    if (showDebug)
    {
        strUsage += HelpMessageOpt("-mocktime=<n>", "Replace actual time with <n> seconds since epoch (default: 0)");
        strUsage += HelpMessageOpt("-maxsigcachesize=<n>", strprintf("Limit size of signature cache to <n> MiB (default: %u)", DEFAULT_MAX_SIG_CACHE_SIZE));
        strUsage += HelpMessageOpt("-maxtipage=<n>", strprintf("Maximum tip age in seconds to consider node in initial block download (default: %u)", DEFAULT_MAX_TIP_AGE));
    }
    strUsage += HelpMessageOpt("-minrelaytxfee=<amt>", strprintf(_("Transactions must have at least this fee rate (in %s per 1000 bytes) for relaying, mining and transaction creation (default: %s). This is not the only fee constraint."),
        CURRENCY_UNIT, FormatMoney(DEFAULT_MIN_RELAY_TX_FEE)));
    strUsage += HelpMessageOpt("-maxtxfee=<amt>", strprintf(_("Maximum total fees (in %s) to use in a single wallet transaction or raw transaction; setting this too low may abort large transactions (default: %s)"),
        CURRENCY_UNIT, FormatMoney(DEFAULT_TRANSACTION_MAXFEE)));
    strUsage += HelpMessageOpt("-printtoconsole", _("Send trace/debug info to console instead of debug.log file"));
    if (showDebug)
    {
        strUsage += HelpMessageOpt("-printpriority", strprintf("Log transaction fee per kB when mining blocks (default: %u)", DEFAULT_PRINTPRIORITY));
    }
    strUsage += HelpMessageOpt("-shrinkdebugfile", _("Shrink debug.log file on client startup (default: 1 when no -debug)"));

    AppendParamsHelpMessages(strUsage, showDebug);

    strUsage += HelpMessageGroup(_("Node relay options:"));
    strUsage += HelpMessageOpt("-datacarrier", strprintf(_("Relay and mine data carrier transactions (default: %u)"), DEFAULT_ACCEPT_DATACARRIER));
    strUsage += HelpMessageOpt("-datacarriersize", strprintf(_("Maximum size of data in data carrier transactions we relay and mine (default: %u)"), MAX_OP_RETURN_RELAY));

    strUsage += HelpMessageGroup(_("Block creation options:"));
    strUsage += HelpMessageOpt("-blockminsize=<n>", strprintf(_("Set minimum block size in bytes (default: %u)"), DEFAULT_BLOCK_MIN_SIZE));
    strUsage += HelpMessageOpt("-blockmaxsize=<n>", strprintf(_("Set maximum block size in bytes (default: %d)"), DEFAULT_BLOCK_MAX_SIZE));
    if (GetBoolArg("-help-debug", false))
        strUsage += HelpMessageOpt("-blockversion=<n>", strprintf("Override block version to test forking scenarios (default: %d)", (int)CBlock::CURRENT_VERSION));

    if (showDebug)
        strUsage += HelpMessageOpt("-nodebug", "Turn off debugging messages, same as -debug=0");

#ifdef ENABLE_MINING
    strUsage += HelpMessageGroup(_("Mining options:"));
    strUsage += HelpMessageOpt("-gen", strprintf(_("Generate coins (default: %u)"), DEFAULT_GENERATE));
    strUsage += HelpMessageOpt("-genproclimit=<n>", strprintf(_("Set the number of threads for coin generation if enabled (-1 = all cores, default: %d)"), DEFAULT_GENERATE_THREADS));
    strUsage += HelpMessageOpt("-equihashsolver=<name>", _("Specify the Equihash solver to be used if enabled (default: \"default\")"));
    strUsage += HelpMessageOpt("-mineraddress=<addr>", _("Send mined coins to a specific single address"));
    strUsage += HelpMessageOpt("-minetolocalwallet", strprintf(
            _("Require that mined blocks use a coinbase address in the local wallet (default: %u)"),
 #ifdef ENABLE_WALLET
            1
 #else
            0
 #endif
            ));
#endif

    strUsage += HelpMessageGroup(_("RPC server options:"));
    strUsage += HelpMessageOpt("-server", _("Accept command line and JSON-RPC commands"));
    strUsage += HelpMessageOpt("-rest", strprintf(_("Accept public REST requests (default: %u)"), DEFAULT_REST_ENABLE));
    strUsage += HelpMessageOpt("-rpcbind=<addr>", _("Bind to given address to listen for JSON-RPC connections. Use [host]:port notation for IPv6. This option can be specified multiple times (default: bind to all interfaces)"));
    strUsage += HelpMessageOpt("-rpccookiefile=<loc>", _("Location of the auth cookie. Relative paths will be prefixed by a net-specific datadir location. (default: data dir)"));
    strUsage += HelpMessageOpt("-rpcuser=<user>", _("Username for JSON-RPC connections"));
    strUsage += HelpMessageOpt("-rpcpassword=<pw>", _("Password for JSON-RPC connections"));
    strUsage += HelpMessageOpt("-rpcauth=<userpw>", _("Username and hashed password for JSON-RPC connections. The field <userpw> comes in the format: <USERNAME>:<SALT>$<HASH>. A canonical python script is included in share/rpcuser. This option can be specified multiple times"));
    strUsage += HelpMessageOpt("-rpcport=<port>", strprintf(_("Listen for JSON-RPC connections on <port> (default: %u or testnet: %u)"), 8232, 18232));
    strUsage += HelpMessageOpt("-rpcallowip=<ip>", _("Allow JSON-RPC connections from specified source. Valid for <ip> are a single IP (e.g. 1.2.3.4), a network/netmask (e.g. 1.2.3.4/255.255.255.0) or a network/CIDR (e.g. 1.2.3.4/24). This option can be specified multiple times"));
    strUsage += HelpMessageOpt("-rpcthreads=<n>", strprintf(_("Set the number of threads to service RPC calls (default: %d)"), DEFAULT_HTTP_THREADS));
    if (showDebug) {
        strUsage += HelpMessageOpt("-rpcworkqueue=<n>", strprintf("Set the depth of the work queue to service RPC calls (default: %d)", DEFAULT_HTTP_WORKQUEUE));
        strUsage += HelpMessageOpt("-rpcservertimeout=<n>", strprintf("Timeout during HTTP requests (default: %d)", DEFAULT_HTTP_SERVER_TIMEOUT));
    }

    // Disabled until we can lock notes and also tune performance of the prover which by default uses multiple threads
    //strUsage += HelpMessageOpt("-rpcasyncthreads=<n>", strprintf(_("Set the number of threads to service Async RPC calls (default: %d)"), 1));

    if (mode == HMM_BITCOIND) {
        strUsage += HelpMessageGroup(_("Metrics Options (only if -daemon and -printtoconsole are not set):"));
        strUsage += HelpMessageOpt("-showmetrics", _("Show metrics on stdout (default: 1 if running in a console, 0 otherwise)"));
        strUsage += HelpMessageOpt("-metricsui", _("Set to 1 for a persistent metrics screen, 0 for sequential metrics output (default: 1 if running in a console, 0 otherwise)"));
        strUsage += HelpMessageOpt("-metricsrefreshtime", strprintf(_("Number of seconds between metrics refreshes (default: %u if running in a console, %u otherwise)"), 1, 600));
    }

    return strUsage;
}

static void BlockNotifyCallback(bool initialSync, const CBlockIndex *pBlockIndex)
{
    if (initialSync || !pBlockIndex)
        return;

    std::string strCmd = GetArg("-blocknotify", "");

    boost::replace_all(strCmd, "%s", pBlockIndex->GetBlockHash().GetHex());
    boost::thread t(runCommand, strCmd); // thread runs free
}

static void TxExpiryNotifyCallback(const uint256& txid)
{
    std::string strCmd = GetArg("-txexpirynotify", "");

    boost::replace_all(strCmd, "%s", txid.GetHex());
    boost::thread t(runCommand, strCmd); // thread runs free
}

static bool fHaveGenesis = false;
static boost::mutex cs_GenesisWait;
static CConditionVariable condvar_GenesisWait;

static void BlockNotifyGenesisWait(bool, const CBlockIndex *pBlockIndex)
{
    if (pBlockIndex != NULL) {
        {
            boost::unique_lock<boost::mutex> lock_GenesisWait(cs_GenesisWait);
            fHaveGenesis = true;
        }
        condvar_GenesisWait.notify_all();
    }
}

struct CImportingNow
{
    CImportingNow() {
        assert(fImporting == false);
        fImporting = true;
    }

    ~CImportingNow() {
        assert(fImporting == true);
        fImporting = false;
    }
};


// If we're using -prune with -reindex, then delete block files that will be ignored by the
// reindex.  Since reindexing works by starting at block file 0 and looping until a blockfile
// is missing, do the same here to delete any later block files after a gap.  Also delete all
// rev files since they'll be rewritten by the reindex anyway.  This ensures that vinfoBlockFile
// is in sync with what's actually on disk by the time we start downloading, so that pruning
// works correctly.
void CleanupBlockRevFiles()
{
    using namespace fs;
    std::map<std::string, path> mapBlockFiles;

    // Glob all blk?????.dat and rev?????.dat files from the blocks directory.
    // Remove the rev files immediately and insert the blk file paths into an
    // ordered map keyed by block file index.
    LogPrintf("Removing unusable blk?????.dat and rev?????.dat files for -reindex with -prune\n");
    path blocksdir = GetDataDir() / "blocks";
    for (directory_iterator it(blocksdir); it != directory_iterator(); it++) {
        if (is_regular_file(*it) &&
            it->path().filename().string().length() == 12 &&
            it->path().filename().string().substr(8,4) == ".dat")
        {
            if (it->path().filename().string().substr(0,3) == "blk")
                mapBlockFiles[it->path().filename().string().substr(3,5)] = it->path();
            else if (it->path().filename().string().substr(0,3) == "rev")
                remove(it->path());
        }
    }

    // Remove all block files that aren't part of a contiguous set starting at
    // zero by walking the ordered map (keys are block file indices) by
    // keeping a separate counter.  Once we hit a gap (or if 0 doesn't exist)
    // start removing block files.
    int nContigCounter = 0;
    for (const std::pair<std::string, path>& item : mapBlockFiles) {
        if (atoi(item.first) == nContigCounter) {
            nContigCounter++;
            continue;
        }
        remove(item.second);
    }
}

void ThreadStartWalletNotifier()
{
    CBlockIndex *pindexLastTip{nullptr};

    // If the wallet is compiled in and enabled, we want to start notifying
    // from the block which corresponds with the wallet's view of the chain
    // tip. In particular, we want to handle the case where the node shuts
    // down uncleanly, and on restart the chain's tip is potentially up to
    // an hour of chain sync older than the wallet's tip. We assume here
    // that there is only a single wallet connected to the validation
    // interface, which is currently true.
#ifdef ENABLE_WALLET
    if (pwalletMain)
    {
        std::optional<uint256> walletBestBlockHash;
        {
            LOCK(pwalletMain->cs_wallet);
            walletBestBlockHash = pwalletMain->GetPersistedBestBlock();
        }

        if (walletBestBlockHash.has_value()) {
            int64_t slept{0};
            bool isReindexing{true};
            auto timedOut = [&]() -> bool {
                MilliSleep(500);
                // re-check whether we're reindexing
                if (isReindexing) {
                    LOCK(cs_main);
                    pblocktree->ReadReindexing(isReindexing);
                }

                // once we're out of reindexing, we can start incrementing the slept counter
                if (!isReindexing) {
                    slept += 500;
                }

                if (slept > WALLET_INITIAL_SYNC_TIMEOUT) {
                    auto errmsg = strprintf(
                            "The wallet's best block hash %s was not detected in restored chain state. "
                            "Giving up; please restart with `-rescan`.",
                            walletBestBlockHash.value().GetHex());

                    LogPrintf("*** %s: %s", __func__, errmsg);
                    uiInterface.ThreadSafeMessageBox(
                        _("Error: A fatal wallet synchronization error occurred, see debug.log for details"),
                        "", CClientUIInterface::MSG_ERROR);
                    StartShutdown();
                    return true;
                }

                return false;
            };

            // Wait until we've found the block that the wallet identifies as its
            // best block.
            while (true) {
                boost::this_thread::interruption_point();

                {
                    LOCK(cs_main);
                    BlockMap::iterator mi = mapBlockIndex.find(walletBestBlockHash.value());
                    if (mi != mapBlockIndex.end()) {
                        pindexLastTip = (*mi).second;
                        assert(pindexLastTip != nullptr);
                        break;
                    }
                }

                if (timedOut()) {
                    return;
                }
            }

            // We cannot progress with wallet notification until the chain tip is no
            // more than 100 blocks behind pindexLastTip. This can occur if the node
            // shuts down abruptly without being able to write out chainActive; the
            // node writes chain data out roughly hourly, while the wallet writes it
            // every 10 minutes. We need to wait for ThreadImport to catch up, or any
            // missing blocks to be fetched from peers.
            while (true) {
                boost::this_thread::interruption_point();

                {
                    LOCK(cs_main);
                    const CBlockIndex *pindexFork = chainActive.FindFork(pindexLastTip);
                    // We know we have the genesis block.
                    assert(pindexFork != nullptr);

                    if (pindexLastTip->nHeight < pindexFork->nHeight ||
                        pindexLastTip->nHeight - pindexFork->nHeight < 100)
                    {
                        break;
                    }
                }

                if (timedOut()) {
                    return;
                }
            }
        }
    } else {
        LOCK(cs_main);
        pindexLastTip = chainActive.Tip();
    }
#else
    {
        LOCK(cs_main);
        pindexLastTip = chainActive.Tip();
    }
#endif

    // Become the thread that notifies listeners of transactions that have been
    // recently added to the mempool, or have been added to or removed from the
    // chain.
    ThreadNotifyWallets(pindexLastTip);
}

void ThreadImport(std::vector<fs::path> vImportFiles)
{
    const CChainParams& chainparams = Params();
    RenameThread("bitcoinz-loadblk");
    CImportingNow imp;

    // -reindex
    if (fReindex) {
        nSizeReindexed = 0;  // will be modified inside LoadExternalBlockFile
        // Find the summary size of all block files first
        int nFile = 0;
        size_t fullSize = 0;
        while (true) {
            CDiskBlockPos pos(nFile, 0);
            fs::path blkFile = GetBlockPosFilename(pos, "blk");
            if (!fs::exists(blkFile))
                break; // No block files left to reindex
            nFile++;
            fullSize += fs::file_size(blkFile);
        }
        nFullSizeToReindex = std::max<size_t>(1, fullSize);
        nFile = 0;
        while (true) {
            CDiskBlockPos pos(nFile, 0);
            if (!fs::exists(GetBlockPosFilename(pos, "blk")))
                break; // No block files left to reindex
            FILE *file = OpenBlockFile(pos, true);
            if (!file)
                break; // This error is logged in OpenBlockFile
            LogPrintf("Reindexing block file blk%05u.dat...\n", (unsigned int)nFile);
            LoadExternalBlockFile(chainparams, file, &pos);
            nFile++;
        }
        pblocktree->WriteReindexing(false);
        fReindex = false;
        nSizeReindexed = 0;
        nFullSizeToReindex = 1;
        LogPrintf("Reindexing finished\n");
        // To avoid ending up in a situation without genesis block, re-try initializing (no-op if reindexing worked):
        InitBlockIndex(chainparams);
    }

    // hardcoded $DATADIR/bootstrap.dat
    fs::path pathBootstrap = GetDataDir() / "bootstrap.dat";
    if (fs::exists(pathBootstrap)) {
        FILE *file = fsbridge::fopen(pathBootstrap, "rb");
        if (file) {
            fs::path pathBootstrapOld = GetDataDir() / "bootstrap.dat.old";
            LogPrintf("Importing bootstrap.dat...\n");
            LoadExternalBlockFile(chainparams, file);
            RenameOver(pathBootstrap, pathBootstrapOld);
        } else {
            LogPrintf("Warning: Could not open bootstrap file %s\n", pathBootstrap.string());
        }
    }

    // -loadblock=
    for (const fs::path& path : vImportFiles) {
        FILE *file = fsbridge::fopen(path, "rb");
        if (file) {
            LogPrintf("Importing blocks file %s...\n", path.string());
            LoadExternalBlockFile(chainparams,file);
        } else {
            LogPrintf("Warning: Could not open blocks file %s\n", path.string());
        }
    }

    // scan for better chains in the block chain database, that are not yet connected in the active best chain
    CValidationState state;
    if (!ActivateBestChain(state, chainparams)) {
        LogPrintf("Failed to connect best block");
        StartShutdown();
    }

    if (GetBoolArg("-stopafterblockimport", DEFAULT_STOPAFTERBLOCKIMPORT)) {
        LogPrintf("Stopping after block import\n");
        StartShutdown();
    }
}

/** Sanity checks
 *  Ensure that Bitcoin is running in a usable environment with all
 *  necessary library support.
 */
bool InitSanityCheck(void)
{
    if(!ECC_InitSanityCheck()) {
        InitError("Elliptic curve cryptography sanity check failure. Aborting.");
        return false;
    }
    if (!glibc_sanity_test() || !glibcxx_sanity_test())
        return false;

    return true;
}


static void ZC_LoadParams(
    const CChainParams& chainparams
)
{
    struct timeval tv_start, tv_end;
    float elapsed;

    fs::path sapling_spend = ZC_GetParamsDir() / "sapling-spend.params";
    fs::path sapling_output = ZC_GetParamsDir() / "sapling-output.params";
    fs::path sprout_groth16 = ZC_GetParamsDir() / "sprout-groth16.params";

    if (!(
        fs::exists(sapling_spend) &&
        fs::exists(sapling_output) &&
        fs::exists(sprout_groth16)
    )) {
        uiInterface.ThreadSafeMessageBox(strprintf(
            _("Cannot find the BitcoinZ network parameters in the following directory:\n"
              "%s\n"
              "Please run 'bitcoinz-fetch-params' or './zcutil/fetch-params.sh' and then restart."),
                ZC_GetParamsDir()),
            "", CClientUIInterface::MSG_ERROR);
        StartShutdown();
        return;
    }

    static_assert(
        sizeof(fs::path::value_type) == sizeof(codeunit),
        "librustzcash not configured correctly");
    auto sapling_spend_str = sapling_spend.native();
    auto sapling_output_str = sapling_output.native();
    auto sprout_groth16_str = sprout_groth16.native();

    LogPrintf("Loading Sapling (Spend) parameters from %s\n", sapling_spend.string().c_str());
    LogPrintf("Loading Sapling (Output) parameters from %s\n", sapling_output.string().c_str());
    LogPrintf("Loading Sapling (Sprout Groth16) parameters from %s\n", sprout_groth16.string().c_str());
    gettimeofday(&tv_start, 0);

    librustzcash_init_zksnark_params(
        reinterpret_cast<const codeunit*>(sapling_spend_str.c_str()),
        sapling_spend_str.length(),
        "8270785a1a0d0bc77196f000ee6d221c9c9894f55307bd9357c3f0105d31ca63991ab91324160d8f53e2bbd3c2633a6eb8bdf5205d822e7f3f73edac51b2b70c",
        reinterpret_cast<const codeunit*>(sapling_output_str.c_str()),
        sapling_output_str.length(),
        "657e3d38dbb5cb5e7dd2970e8b03d69b4787dd907285b5a7f0790dcc8072f60bf593b32cc2d1c030e00ff5ae64bf84c5c3beb84ddc841d48264b4a171744d028",
        reinterpret_cast<const codeunit*>(sprout_groth16_str.c_str()),
        sprout_groth16_str.length(),
        "e9b238411bd6c0ec4791e9d04245ec350c9c5744f5610dfcce4365d5ca49dfefd5054e371842b3f88fa1b9d7e8e075249b3ebabd167fa8b0f3161292d36c180a"
    );

    gettimeofday(&tv_end, 0);
    elapsed = float(tv_end.tv_sec-tv_start.tv_sec) + (tv_end.tv_usec-tv_start.tv_usec)/float(1000000);
    LogPrintf("Loaded Sapling parameters in %fs seconds.\n", elapsed);
}

bool AppInitServers()
{
    RPCServer::OnStopped(&OnRPCStopped);
    RPCServer::OnPreCommand(&OnRPCPreCommand);
    if (!InitHTTPServer())
        return false;
    if (!StartRPC())
        return false;
    if (!StartHTTPRPC())
        return false;
    if (GetBoolArg("-rest", DEFAULT_REST_ENABLE) && !StartREST())
        return false;
    if (!StartHTTPServer())
        return false;
    return true;
}

// Parameter interaction based on rules
void InitParameterInteraction()
{
    // when specifying an explicit binding address, you want to listen on it
    // even when -connect or -proxy is specified
    if (mapArgs.count("-bind")) {
        if (SoftSetBoolArg("-listen", true))
            LogPrintf("%s: parameter interaction: -bind set -> setting -listen=1\n", __func__);
    }
    if (mapArgs.count("-whitebind")) {
        if (SoftSetBoolArg("-listen", true))
            LogPrintf("%s: parameter interaction: -whitebind set -> setting -listen=1\n", __func__);
    }

    if (mapArgs.count("-connect") && mapMultiArgs["-connect"].size() > 0) {
        // when only connecting to trusted nodes, do not seed via DNS, or listen by default
        if (SoftSetBoolArg("-dnsseed", false))
            LogPrintf("%s: parameter interaction: -connect set -> setting -dnsseed=0\n", __func__);
        if (SoftSetBoolArg("-listen", false))
            LogPrintf("%s: parameter interaction: -connect set -> setting -listen=0\n", __func__);
    }

    if (mapArgs.count("-proxy")) {
        // to protect privacy, do not listen by default if a default proxy server is specified
        if (SoftSetBoolArg("-listen", false))
            LogPrintf("%s: parameter interaction: -proxy set -> setting -listen=0\n", __func__);
        // to protect privacy, do not discover addresses by default
        if (SoftSetBoolArg("-discover", false))
            LogPrintf("%s: parameter interaction: -proxy set -> setting -discover=0\n", __func__);
    }

    if (!GetBoolArg("-listen", DEFAULT_LISTEN)) {
        // do not try to retrieve public IP when not listening (pointless)
        if (SoftSetBoolArg("-discover", false))
            LogPrintf("%s: parameter interaction: -listen=0 -> setting -discover=0\n", __func__);
        if (SoftSetBoolArg("-listenonion", false))
            LogPrintf("%s: parameter interaction: -listen=0 -> setting -listenonion=0\n", __func__);
    }

    if (mapArgs.count("-externalip")) {
        // if an explicit public IP is specified, do not try to find others
        if (SoftSetBoolArg("-discover", false))
            LogPrintf("%s: parameter interaction: -externalip set -> setting -discover=0\n", __func__);
    }

#ifdef ENABLE_WALLET
    // -rescan only affects the wallet.

    if (GetBoolArg("-salvagewallet", false)) {
        // Rewrite just private keys: rescan to find transactions
        if (SoftSetBoolArg("-rescan", true))
            LogPrintf("%s: parameter interaction: -salvagewallet=1 -> setting -rescan=1\n", __func__);
    }

    // -zapwallettx implies a rescan
    if (GetBoolArg("-zapwallettxes", false)) {
        if (SoftSetBoolArg("-rescan", true))
            LogPrintf("%s: parameter interaction: -zapwallettxes=<mode> -> setting -rescan=1\n", __func__);
    }

    if (GetBoolArg("-reindex", false) && !GetArg("-prune", 0)) {
        if (SoftSetBoolArg("-rescan", true))
            LogPrintf("%s: parameter interaction: -reindex=1 and not pruning -> setting -rescan=1\n", __func__);
    }

    if (GetBoolArg("-reindex-chainstate", false) && !GetArg("-prune", 0)) {
        if (SoftSetBoolArg("-rescan", true))
            LogPrintf("%s: parameter interaction: -reindex-chainstate=1 and not pruning -> setting -rescan=1\n", __func__);
    }
#endif

    // disable walletbroadcast and whitelistrelay in blocksonly mode
    if (GetBoolArg("-blocksonly", DEFAULT_BLOCKSONLY)) {
        if (SoftSetBoolArg("-whitelistrelay", false))
            LogPrintf("%s: parameter interaction: -blocksonly=1 -> setting -whitelistrelay=0\n", __func__);
#ifdef ENABLE_WALLET
        if (SoftSetBoolArg("-walletbroadcast", false))
            LogPrintf("%s: parameter interaction: -blocksonly=1 -> setting -walletbroadcast=0\n", __func__);
#endif
    }

    // Forcing relay from whitelisted hosts implies we will accept relays from them in the first place.
    if (GetBoolArg("-whitelistforcerelay", DEFAULT_WHITELISTFORCERELAY)) {
        if (SoftSetBoolArg("-whitelistrelay", true))
            LogPrintf("%s: parameter interaction: -whitelistforcerelay=1 -> setting -whitelistrelay=1\n", __func__);
    }
}

void InitLogging()
{
    fPrintToConsole = GetBoolArg("-printtoconsole", false);
    fLogTimestamps = GetBoolArg("-logtimestamps", true);
    fLogIPs = GetBoolArg("-logips", false);

    LogPrintf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    LogPrintf("BitcoinZ version %s (%s)\n", FormatFullVersion(), CLIENT_DATE);
}

/** Initialize bitcoin.
 *  @pre Parameters should be parsed and config file should be read.
 */
bool AppInit2()
{
    // ********************************************************* Step 1: setup
#ifdef _MSC_VER
    // Turn off Microsoft heap dump noise
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, CreateFileA("NUL", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0));
#endif
#if _MSC_VER >= 1400
    // Disable confusing "helpful" text message on abort, Ctrl-C
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#ifdef WIN32
    // Enable Data Execution Prevention (DEP)
    SetProcessDEPPolicy(PROCESS_DEP_ENABLE);
#endif

    if (!SetupNetworking())
        return InitError("Initializing networking failed");

#ifndef WIN32
    if (GetBoolArg("-sysperms", false)) {
#ifdef ENABLE_WALLET
        if (!GetBoolArg("-disablewallet", false))
            return InitError("-sysperms is not allowed in combination with enabled wallet functionality");
#endif
    } else {
        umask(077);
    }

    // Clean shutdown on SIGTERM
    assert(fRequestShutdown.is_lock_free());
    struct sigaction sa;
    sa.sa_handler = HandleSIGTERM;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Reopen debug.log on SIGHUP
    assert(fReopenDebugLog.is_lock_free());
    struct sigaction sa_hup;
    sa_hup.sa_handler = HandleSIGHUP;
    sigemptyset(&sa_hup.sa_mask);
    sa_hup.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &sa_hup, NULL);

    // Ignore SIGPIPE, otherwise it will bring the daemon down if the client closes unexpectedly
    signal(SIGPIPE, SIG_IGN);
#endif

    std::set_new_handler(new_handler_terminate);

    // ********************************************************* Step 2: parameter interactions
    const CChainParams& chainparams = Params();

    // also see: InitParameterInteraction()

    // Set this early so that experimental features are correctly enabled/disabled
    auto err = InitExperimentalMode();
    if (err) {
        return InitError(err.value());
    }

    // if using block pruning, then disable txindex
    if (GetArg("-prune", 0)) {
        if (GetBoolArg("-txindex", DEFAULT_TXINDEX))
            return InitError(_("Prune mode is incompatible with -txindex."));
#ifdef ENABLE_WALLET
        if (GetBoolArg("-rescan", false)) {
           return InitError(_("Rescans are not possible in pruned mode. You will need to use -reindex which will download the whole blockchain again."));
        }
#endif
    }

    // Make sure enough file descriptors are available
    int nBind = std::max((int)mapArgs.count("-bind") + (int)mapArgs.count("-whitebind"), 1);
    int nUserMaxConnections = GetArg("-maxconnections", DEFAULT_MAX_PEER_CONNECTIONS);
    nMaxConnections = std::max(nUserMaxConnections, 0);

    // Trim requested connection counts, to fit into system limitations
    nMaxConnections = std::max(std::min(nMaxConnections, FD_SETSIZE - nBind - MIN_CORE_FILEDESCRIPTORS), 0);
    int nFD = RaiseFileDescriptorLimit(nMaxConnections + MIN_CORE_FILEDESCRIPTORS);
    if (nFD < MIN_CORE_FILEDESCRIPTORS)
        return InitError(_("Not enough file descriptors available."));
    nMaxConnections = std::min(nFD - MIN_CORE_FILEDESCRIPTORS, nMaxConnections);

    if (nMaxConnections < nUserMaxConnections)
        InitWarning(strprintf(_("Reducing -maxconnections from %d to %d, because of system limitations."), nUserMaxConnections, nMaxConnections));

    // ********************************************************* Step 3: parameter-to-internal-flags

    if (mapMultiArgs.count("-debug") > 0) {
        // Special-case: if -debug=0/-nodebug is set, turn off debugging messages
        const std::vector<std::string>& categories = mapMultiArgs.at("-debug");

        if (!(GetBoolArg("-nodebug", false) || find(categories.begin(), categories.end(), std::string("0")) != categories.end())) {
            for (const auto& cat : categories) {
                uint32_t flag;
                if (!GetLogCategory(&flag, &cat)) {
                    InitWarning(strprintf(_("Unsupported logging category %s.\n"), cat));
                }
                logCategories |= flag;
            }
        }

        // Special case: if debug=zrpcunsafe, implies debug=zrpc, so add it to debug categories
        if (!(GetBoolArg("-nodebug", false) || find(categories.begin(), categories.end(), std::string("zrpcunsafe")) != categories.end())) {
            std::string zrpcString = "zrpc";
            if (find(categories.begin(), categories.end(), zrpcString) == categories.end()) {
                InitWarning(_("Logging category 'zrpcunsafe' requires 'zrpc'\n"));
                uint32_t flag;
                if (GetLogCategory(&flag, &zrpcString)) {
                    logCategories |= flag;
                }
            }
        }
    }

    // Check for -debugnet
    if (GetBoolArg("-debugnet", false))
        InitWarning(_("Unsupported argument -debugnet ignored, use -debug=net."));
    // Check for -socks - as this is a privacy risk to continue, exit here
    if (mapArgs.count("-socks"))
        return InitError(_("Unsupported argument -socks found. Setting SOCKS version isn't possible anymore, only SOCKS5 proxies are supported."));
    // Check for -tor - as this is a privacy risk to continue, exit here
    if (GetBoolArg("-tor", false))
        return InitError(_("Unsupported argument -tor found, use -onion."));

    if (GetBoolArg("-benchmark", false))
        InitWarning(_("Unsupported argument -benchmark ignored, use -debug=bench."));

    // Checkmempool and checkblockindex default to true in regtest mode
    int ratio = std::min<int>(std::max<int>(GetArg("-checkmempool", chainparams.DefaultConsistencyChecks() ? 1 : 0), 0), 1000000);
    if (ratio != 0) {
        mempool.setSanityCheck(1.0 / ratio);
    }

    int64_t mempoolTotalCostLimit = GetArg("-mempooltxcostlimit", DEFAULT_MEMPOOL_TOTAL_COST_LIMIT);
    int64_t mempoolEvictionMemorySeconds = GetArg("-mempoolevictionmemoryminutes", DEFAULT_MEMPOOL_EVICTION_MEMORY_MINUTES) * 60;
    mempool.SetMempoolCostLimit(mempoolTotalCostLimit, mempoolEvictionMemorySeconds);

    fCheckBlockIndex = GetBoolArg("-checkblockindex", chainparams.DefaultConsistencyChecks());
    fCheckpointsEnabled = GetBoolArg("-checkpoints", DEFAULT_CHECKPOINTS_ENABLED);

    // -par=0 means autodetect, but nScriptCheckThreads==0 means no concurrency
    nScriptCheckThreads = GetArg("-par", DEFAULT_SCRIPTCHECK_THREADS);
    if (nScriptCheckThreads <= 0)
        nScriptCheckThreads += GetNumCores();
    if (nScriptCheckThreads <= 1)
        nScriptCheckThreads = 0;
    else if (nScriptCheckThreads > MAX_SCRIPTCHECK_THREADS)
        nScriptCheckThreads = MAX_SCRIPTCHECK_THREADS;

    fServer = GetBoolArg("-server", false);

    // block pruning; get the amount of disk space (in MiB) to allot for block & undo files
    int64_t nSignedPruneTarget = GetArg("-prune", 0) * 1024 * 1024;
    if (nSignedPruneTarget < 0) {
        return InitError(_("Prune cannot be configured with a negative value."));
    }
    nPruneTarget = (uint64_t) nSignedPruneTarget;
    if (nPruneTarget) {
        if (nPruneTarget < MIN_DISK_SPACE_FOR_BLOCK_FILES) {
            return InitError(strprintf(_("Prune configured below the minimum of %d MiB.  Please use a higher number."), MIN_DISK_SPACE_FOR_BLOCK_FILES / 1024 / 1024));
        }
        LogPrintf("Prune configured to target %uMiB on disk for block and undo files.\n", nPruneTarget / 1024 / 1024);
        fPruneMode = true;
    }

    RegisterAllCoreRPCCommands(tableRPC);
#ifdef ENABLE_WALLET
    bool fDisableWallet = GetBoolArg("-disablewallet", false);
    if (!fDisableWallet)
        RegisterWalletRPCCommands(tableRPC);
#endif

    nConnectTimeout = GetArg("-timeout", DEFAULT_CONNECT_TIMEOUT);
    if (nConnectTimeout <= 0)
        nConnectTimeout = DEFAULT_CONNECT_TIMEOUT;

    // Fee rate in zatoshis per 1000 bytes required for mempool acceptance and relay.
    // TODO(update when ZIP 317 is implemented):
    // If you are mining, be careful setting this. If you set it too low then a
    // transaction spammer can cheaply fill blocks. It should be set above the real
    if (mapArgs.count("-minrelaytxfee"))
    {
        CAmount n = 0;
        if (ParseMoney(mapArgs["-minrelaytxfee"], n))
            ::minRelayTxFee = CFeeRate(n);
        else
            return InitError(strprintf(_("Invalid amount for -minrelaytxfee=<amount>: '%s'"), mapArgs["-minrelaytxfee"]));
    }

#ifdef ENABLE_WALLET
    if (!CWallet::ParameterInteraction())
        return false;
#endif // ENABLE_WALLET

    fIsBareMultisigStd = GetBoolArg("-permitbaremultisig", DEFAULT_PERMIT_BAREMULTISIG);
    fAcceptDatacarrier = GetBoolArg("-datacarrier", DEFAULT_ACCEPT_DATACARRIER);
    nMaxDatacarrierBytes = GetArg("-datacarriersize", nMaxDatacarrierBytes);

    // Option to startup with mocktime set (used for regression testing):
    SetMockTime(GetArg("-mocktime", 0)); // SetMockTime(0) is a no-op

    if (GetBoolArg("-peerbloomfilters", DEFAULT_PEERBLOOMFILTERS))
        nLocalServices |= NODE_BLOOM;

    nMaxTipAge = GetArg("-maxtipage", DEFAULT_MAX_TIP_AGE);

    KeyIO keyIO(chainparams);
#ifdef ENABLE_MINING
    if (mapArgs.count("-mineraddress")) {
        CTxDestination addr = keyIO.DecodeDestination(mapArgs["-mineraddress"]);
        if (!IsValidDestination(addr)) {
            return InitError(strprintf(
                _("Invalid address for -mineraddress=<addr>: '%s' (must be a transparent address)"),
                mapArgs["-mineraddress"]));
        }
    }
#endif

    if (GetArg("-blockprioritysize", 0) != 0) {
        InitWarning(_("The argument -blockprioritysize is no longer supported."));
    }

    if (!mapMultiArgs["-nuparams"].empty()) {
        // Allow overriding network upgrade parameters for testing
        if (Params().NetworkIDString() != "regtest") {
            return InitError("Network upgrade parameters may only be overridden on regtest.");
        }
        const std::vector<std::string>& deployments = mapMultiArgs["-nuparams"];
        for (auto i : deployments) {
            std::vector<std::string> vDeploymentParams;
            boost::split(vDeploymentParams, i, boost::is_any_of(":"));
            if (vDeploymentParams.size() != 2) {
                return InitError("Network upgrade parameters malformed, expecting hexBranchId:activationHeight");
            }
            int nActivationHeight;
            if (!ParseInt32(vDeploymentParams[1], &nActivationHeight)) {
                return InitError(strprintf("Invalid nActivationHeight (%s)", vDeploymentParams[1]));
            }
            bool found = false;
            // Exclude Sprout from upgrades
            for (auto i = Consensus::BASE_SPROUT + 1; i < Consensus::MAX_NETWORK_UPGRADES; ++i)
            {
                if (vDeploymentParams[0].compare(HexInt(NetworkUpgradeInfo[i].nBranchId)) == 0) {
                    UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex(i), nActivationHeight);
                    found = true;
                    LogPrintf("Setting network upgrade activation parameters for %s to height=%d\n", vDeploymentParams[0], nActivationHeight);
                    break;
                }
            }
            if (!found) {
                return InitError(strprintf("Invalid network upgrade (%s)", vDeploymentParams[0]));
            }
        }
    }

    if (!mapMultiArgs["-fundingstream"].empty()) {
        // Allow overriding network upgrade parameters for testing
        if (Params().NetworkIDString() != "regtest") {
            return InitError("Funding stream parameters may only be overridden on regtest.");
        }
        const std::vector<std::string>& streams = mapMultiArgs["-fundingstream"];
        for (auto i : streams) {
            std::vector<std::string> vStreamParams;
            boost::split(vStreamParams, i, boost::is_any_of(":"));
            if (vStreamParams.size() != 4) {
                return InitError("Funding stream parameters malformed, expecting streamId:startHeight:endHeight:comma_delimited_addresses");
            }
            int nFundingStreamId;
            if (!ParseInt32(vStreamParams[0], &nFundingStreamId) ||
                    nFundingStreamId < Consensus::FIRST_FUNDING_STREAM ||
                    nFundingStreamId >= Consensus::MAX_FUNDING_STREAMS) {
                return InitError(strprintf("Invalid streamId (%s)", vStreamParams[0]));
            }

            int nStartHeight;
            if (!ParseInt32(vStreamParams[1], &nStartHeight)) {
                return InitError(strprintf("Invalid funding stream start height (%s)", vStreamParams[1]));
            }

            int nEndHeight;
            if (!ParseInt32(vStreamParams[2], &nEndHeight)) {
                return InitError(strprintf("Invalid funding stream end height (%s)", vStreamParams[2]));
            }

            std::vector<std::string> vStreamAddrs;
            boost::split(vStreamAddrs, vStreamParams[3], boost::is_any_of(","));

            auto fs = Consensus::FundingStream::ParseFundingStream(
                    Params().GetConsensus(), Params(), nStartHeight, nEndHeight, vStreamAddrs);

            UpdateFundingStreamParameters((Consensus::FundingStreamIndex) nFundingStreamId, fs);
        }
    }

    // ********************************************************* Step 4: application initialization: dir lock, daemonize, pidfile, debug log

    // Initialize libsodium
    if (init_and_check_sodium() == -1) {
        return false;
    }

    // Initialize elliptic curve code
    ECC_Start();
    globalVerifyHandle.reset(new ECCVerifyHandle());

    // Sanity check
    if (!InitSanityCheck())
        return InitError(_("Initialization sanity check failed. BitcoinZ is shutting down."));

    std::string strDataDir = GetDataDir().string();

    // Make sure only a single Bitcoin process is using the data directory.
    fs::path pathLockFile = GetDataDir() / ".lock";
    FILE* file = fsbridge::fopen(pathLockFile, "a"); // empty lock file; created if it doesn't exist.
    if (file) fclose(file);

    try {
        static boost::interprocess::file_lock lock(pathLockFile.string().c_str());
        if (!lock.try_lock())
            return InitError(strprintf(_("Cannot obtain a lock on data directory %s. BitcoinZ is probably already running."), strDataDir));
    } catch(const boost::interprocess::interprocess_exception& e) {
        return InitError(strprintf(_("Cannot obtain a lock on data directory %s. BitcoinZ is probably already running.") + " %s.", strDataDir, e.what()));
    }

#ifndef WIN32
    CreatePidFile(GetPidFile(), getpid());
#endif
    if (GetBoolArg("-shrinkdebugfile", logCategories != BCLog::NONE)) {
        // Do this first since it both loads a bunch of debug.log into memory,
        // and because this needs to happen before any other debug.log printing
        ShrinkDebugFile();
    }

    if (fPrintToDebugLog) {
        if (!OpenDebugLog()) {
            return InitError(strprintf("Could not open debug log file %s", GetDebugLogPath().string()));
        }
    }

#ifdef ENABLE_WALLET
    LogPrintf("Using BerkeleyDB version %s\n", DbEnv::version(0, 0, 0));
#endif
    if (!fLogTimestamps)
        LogPrintf("Startup time: %s\n", DateTimeStrFormat("%Y-%m-%d %H:%M:%S", GetTime()));
    LogPrintf("Default data directory %s\n", GetDefaultDataDir().string());
    LogPrintf("Using data directory %s\n", strDataDir);
    LogPrintf("Using config file %s\n", GetConfigFile(GetArg("-conf", BITCOIN_CONF_FILENAME)).string());
    LogPrintf("Using at most %i connections (%i file descriptors available)\n", nMaxConnections, nFD);
    std::ostringstream strErrors;

    InitSignatureCache();

    LogPrintf("Using %u threads for script verification\n", nScriptCheckThreads);
    if (nScriptCheckThreads) {
        for (int i=0; i<nScriptCheckThreads-1; i++)
            threadGroup.create_thread(&ThreadScriptCheck);
    }

    // Start the lightweight task scheduler thread
    CScheduler::Function serviceLoop = boost::bind(&CScheduler::serviceQueue, &scheduler);
    threadGroup.create_thread(boost::bind(&TraceThread<CScheduler::Function>, "scheduler", serviceLoop));

    // Count uptime
    MarkStartTime();

    if ((chainparams.NetworkIDString() != "regtest") &&
            GetBoolArg("-showmetrics", isatty(STDOUT_FILENO)) &&
            !fPrintToConsole && !GetBoolArg("-daemon", false)) {
        // Start the persistent metrics interface
        ConnectMetricsScreen();
        threadGroup.create_thread(&ThreadShowMetricsScreen);
    }

    // Initialize BitcoinZ circuit parameters
    ZC_LoadParams(chainparams);

    /* Start the RPC server already.  It will be started in "warmup" mode
     * and not really process calls already (but it will signify connections
     * that the server is there and will be ready later).  Warmup mode will
     * be disabled when initialisation is finished.
     */
    if (fServer)
    {
        uiInterface.InitMessage.connect(SetRPCWarmupStatus);
        if (!AppInitServers())
            return InitError(_("Unable to start HTTP server. See debug log for details."));
    }

    int64_t nStart;

    // ********************************************************* Step 5: verify wallet database integrity
#ifdef ENABLE_WALLET
    if (!fDisableWallet) {
        if (!CWallet::Verify())
            return false;
    } // (!fDisableWallet)
#endif // ENABLE_WALLET
    // ********************************************************* Step 6: network initialization

    RegisterNodeSignals(GetNodeSignals());

    // sanitize comments per BIP-0014, format user agent and check total size
    std::vector<std::string> uacomments;
    for (std::string cmt : mapMultiArgs["-uacomment"])
    {
        if (cmt != SanitizeString(cmt, SAFE_CHARS_UA_COMMENT))
            return InitError(strprintf("User Agent comment (%s) contains unsafe characters.", cmt));
        uacomments.push_back(SanitizeString(cmt, SAFE_CHARS_UA_COMMENT));
    }
    strSubVersion = FormatSubVersion(CLIENT_NAME, CLIENT_VERSION, uacomments);
    if (strSubVersion.size() > MAX_SUBVERSION_LENGTH) {
        return InitError(strprintf("Total length of network version string %i exceeds maximum of %i characters. Reduce the number and/or size of uacomments.",
            strSubVersion.size(), MAX_SUBVERSION_LENGTH));
    }

    if (mapArgs.count("-onlynet")) {
        std::set<enum Network> nets;
        for (const std::string& snet : mapMultiArgs["-onlynet"]) {
            enum Network net = ParseNetwork(snet);
            if (net == NET_UNROUTABLE)
                return InitError(strprintf(_("Unknown network specified in -onlynet: '%s'"), snet));
            nets.insert(net);
        }
        for (int n = 0; n < NET_MAX; n++) {
            enum Network net = (enum Network)n;
            if (!nets.count(net))
                SetLimited(net);
        }
    }

    if (mapArgs.count("-whitelist")) {
        for (const std::string& net : mapMultiArgs["-whitelist"]) {
            CSubNet subnet(net);
            if (!subnet.IsValid())
                return InitError(strprintf(_("Invalid netmask specified in -whitelist: '%s'"), net));
            CNode::AddWhitelistedRange(subnet);
        }
    }

    bool proxyRandomize = GetBoolArg("-proxyrandomize", DEFAULT_PROXYRANDOMIZE);
    // -proxy sets a proxy for all outgoing network traffic
    // -noproxy (or -proxy=0) as well as the empty string can be used to not set a proxy, this is the default
    std::string proxyArg = GetArg("-proxy", "");
    SetLimited(NET_TOR);
    if (proxyArg != "" && proxyArg != "0") {
        proxyType addrProxy = proxyType(CService(proxyArg, 9050), proxyRandomize);
        if (!addrProxy.IsValid())
            return InitError(strprintf(_("Invalid -proxy address: '%s'"), proxyArg));

        SetProxy(NET_IPV4, addrProxy);
        SetProxy(NET_IPV6, addrProxy);
        SetProxy(NET_TOR, addrProxy);
        SetNameProxy(addrProxy);
        SetLimited(NET_TOR, false); // by default, -proxy sets onion as reachable, unless -noonion later
    }

    // -onion can be used to set only a proxy for .onion, or override normal proxy for .onion addresses
    // -noonion (or -onion=0) disables connecting to .onion entirely
    // An empty string is used to not override the onion proxy (in which case it defaults to -proxy set above, or none)
    std::string onionArg = GetArg("-onion", "");
    if (onionArg != "") {
        if (onionArg == "0") { // Handle -noonion/-onion=0
            SetLimited(NET_TOR); // set onions as unreachable
        } else {
            proxyType addrOnion = proxyType(CService(onionArg, 9050), proxyRandomize);
            if (!addrOnion.IsValid())
                return InitError(strprintf(_("Invalid -onion address: '%s'"), onionArg));
            SetProxy(NET_TOR, addrOnion);
            SetLimited(NET_TOR, false);
        }
    }

    // see Step 2: parameter interactions for more information about these
    fListen = GetBoolArg("-listen", DEFAULT_LISTEN);
    fDiscover = GetBoolArg("-discover", true);
    fNameLookup = GetBoolArg("-dns", DEFAULT_NAME_LOOKUP);

    bool fBound = false;
    if (fListen) {
        if (mapArgs.count("-bind") || mapArgs.count("-whitebind")) {
            for (const std::string& strBind : mapMultiArgs["-bind"]) {
                CService addrBind;
                if (!Lookup(strBind.c_str(), addrBind, GetListenPort(), false))
                    return InitError(strprintf(_("Cannot resolve -bind address: '%s'"), strBind));
                fBound |= Bind(addrBind, (BF_EXPLICIT | BF_REPORT_ERROR));
            }
            for (const std::string& strBind : mapMultiArgs["-whitebind"]) {
                CService addrBind;
                if (!Lookup(strBind.c_str(), addrBind, 0, false))
                    return InitError(strprintf(_("Cannot resolve -whitebind address: '%s'"), strBind));
                if (addrBind.GetPort() == 0)
                    return InitError(strprintf(_("Need to specify a port with -whitebind: '%s'"), strBind));
                fBound |= Bind(addrBind, (BF_EXPLICIT | BF_REPORT_ERROR | BF_WHITELIST));
            }
        }
        else {
            struct in_addr inaddr_any;
            inaddr_any.s_addr = INADDR_ANY;
            fBound |= Bind(CService(in6addr_any, GetListenPort()), BF_NONE);
            fBound |= Bind(CService(inaddr_any, GetListenPort()), !fBound ? BF_REPORT_ERROR : BF_NONE);
        }
        if (!fBound)
            return InitError(_("Failed to listen on any port. Use -listen=0 if you want this."));
    }

    if (mapArgs.count("-externalip")) {
        for (const std::string& strAddr : mapMultiArgs["-externalip"]) {
            CService addrLocal;
            if (Lookup(strAddr.c_str(), addrLocal, GetListenPort(), fNameLookup) && addrLocal.IsValid())
                AddLocal(addrLocal, LOCAL_MANUAL);
            else
                return InitError(strprintf(_("Cannot resolve -externalip address: '%s'"), strAddr));
        }
    }

    for (const std::string& strDest : mapMultiArgs["-seednode"])
        AddOneShot(strDest);

#if ENABLE_ZMQ
    pzmqNotificationInterface = CZMQNotificationInterface::CreateWithArguments(mapArgs);

    if (pzmqNotificationInterface) {
        RegisterValidationInterface(pzmqNotificationInterface);
    }
#endif
    if (mapArgs.count("-maxuploadtarget")) {
        CNode::SetMaxOutboundTarget(
            chainparams.GetConsensus().nPowTargetSpacing,
            GetArg("-maxuploadtarget", DEFAULT_MAX_UPLOAD_TARGET)*1024*1024);
    }

    // ********************************************************* Step 7: load block chain

    fReindex = GetBoolArg("-reindex", false);
    bool fReindexChainState = GetBoolArg("-reindex-chainstate", false);

    fs::create_directories(GetDataDir() / "blocks");

    // cache size calculations
    int64_t nTotalCache = (GetArg("-dbcache", nDefaultDbCache) << 20);
    nTotalCache = std::max(nTotalCache, nMinDbCache << 20); // total cache cannot be less than nMinDbCache
    nTotalCache = std::min(nTotalCache, nMaxDbCache << 20); // total cache cannot be greater than nMaxDbcache
    int64_t nBlockTreeDBCache = nTotalCache / 8;
    if (nBlockTreeDBCache > (1 << 21) && !GetBoolArg("-txindex", DEFAULT_TXINDEX))
        nBlockTreeDBCache = (1 << 21); // block tree db cache shouldn't be larger than 2 MiB

    // https://github.com/bitpay/bitcoin/commit/c91d78b578a8700a45be936cb5bb0931df8f4b87#diff-c865a8939105e6350a50af02766291b7R1233
    if (GetBoolArg("-insightexplorer", false)) {
        if (!GetBoolArg("-txindex", DEFAULT_TXINDEX)) {
            return InitError(_("-insightexplorer requires -txindex."));
        }
        // increase cache if additional indices are needed
        nBlockTreeDBCache = nTotalCache * 3 / 4;
    }
    nTotalCache -= nBlockTreeDBCache;
    int64_t nCoinDBCache = std::min(nTotalCache / 2, (nTotalCache / 4) + (1 << 23)); // use 25%-50% of the remainder for disk cache
    nTotalCache -= nCoinDBCache;
    nCoinCacheUsage = nTotalCache; // the rest goes to in-memory cache
    LogPrintf("Cache configuration:\n");
    LogPrintf("* Using %.1fMiB for block index database\n", nBlockTreeDBCache * (1.0 / 1024 / 1024));
    LogPrintf("* Using %.1fMiB for chain state database\n", nCoinDBCache * (1.0 / 1024 / 1024));
    LogPrintf("* Using %.1fMiB for in-memory UTXO set\n", nCoinCacheUsage * (1.0 / 1024 / 1024));

    bool clearWitnessCaches = false;

    bool fLoaded = false;
    while (!fLoaded) {
        bool fReset = fReindex;
        std::string strLoadError;

        uiInterface.InitMessage(_("Loading block index..."));

        LOCK(cs_main);

        nStart = GetTimeMillis();
        do {
            try {
                UnloadBlockIndex();
                delete pcoinsTip;
                delete pcoinsdbview;
                delete pcoinscatcher;
                delete pblocktree;

                pblocktree = new CBlockTreeDB(nBlockTreeDBCache, false, fReindex);
                pcoinsdbview = new CCoinsViewDB(nCoinDBCache, false, fReindex || fReindexChainState);
                pcoinscatcher = new CCoinsViewErrorCatcher(pcoinsdbview);
                pcoinsTip = new CCoinsViewCache(pcoinscatcher);

                if (fReindex) {
                    pblocktree->WriteReindexing(true);
                    //If we're reindexing in prune mode, wipe away unusable block files and all undo data files
                    if (fPruneMode)
                        CleanupBlockRevFiles();
                }

                if (!LoadBlockIndex()) {
                    strLoadError = _("Error loading block database");
                    break;
                }

                // If the loaded chain has a wrong genesis, bail out immediately
                // (we're likely using a testnet datadir, or the other way around).
                if (!mapBlockIndex.empty() && mapBlockIndex.count(chainparams.GetConsensus().hashGenesisBlock) == 0)
                    return InitError(_("Incorrect or no genesis block found. Wrong datadir for network?"));

                // Initialize the block index (no-op if non-empty database was already loaded)
                if (!InitBlockIndex(chainparams)) {
                    strLoadError = _("Error initializing block database");
                    break;
                }

                // Check for changed -txindex state
                if (fTxIndex != GetBoolArg("-txindex", DEFAULT_TXINDEX)) {
                    strLoadError = _("You need to rebuild the database using -reindex-chainstate to change -txindex");
                    break;
                }

                // Check for changed -insightexplorer state
                bool fInsightExplorerPreviouslySet = false;
                pblocktree->ReadFlag("insightexplorer", fInsightExplorerPreviouslySet);
                if (fExperimentalInsightExplorer != fInsightExplorerPreviouslySet) {
                    strLoadError = _("You need to rebuild the database using -reindex to change -insightexplorer");
                    break;
                }

                // Check for changed -lightwalletd state
                bool fLightWalletdPreviouslySet = false;
                pblocktree->ReadFlag("lightwalletd", fLightWalletdPreviouslySet);
                if (fExperimentalLightWalletd != fLightWalletdPreviouslySet) {
                    strLoadError = _("You need to rebuild the database using -reindex to change -lightwalletd");
                    break;
                }

                // Check for changed -prune state.  What we are concerned about is a user who has pruned blocks
                // in the past, but is now trying to run unpruned.
                if (fHavePruned && !fPruneMode) {
                    strLoadError = _("You need to rebuild the database using -reindex to go back to unpruned mode. This will redownload the entire blockchain");
                    break;
                }

                if (!fReindex && chainActive.Tip() != NULL) {
                    uiInterface.InitMessage(_("Rewinding blocks if needed..."));
                    if (!RewindBlockIndex(chainparams, clearWitnessCaches)) {
                        strLoadError = _("Unable to rewind the database to a pre-upgrade state. You will need to redownload the blockchain");
                        break;
                    }
                }

                uiInterface.InitMessage(_("Verifying blocks..."));
                if (fHavePruned && GetArg("-checkblocks", DEFAULT_CHECKBLOCKS) > MIN_BLOCKS_TO_KEEP) {
                    LogPrintf("Prune: pruned datadir may not have more than %d blocks; -checkblocks=%d may fail\n",
                        MIN_BLOCKS_TO_KEEP, GetArg("-checkblocks", DEFAULT_CHECKBLOCKS));
                }

                CBlockIndex* tip = chainActive.Tip();
                if (tip && tip->nTime > GetTime() + MAX_FUTURE_BLOCK_TIME) {
                    strLoadError = _("The block database contains a block which appears to be from the future. "
                            "This may be due to your computer's date and time being set incorrectly. "
                            "Only rebuild the block database if you are sure that your computer's date and time are correct");
                    break;
                }

                if (!CVerifyDB().VerifyDB(chainparams, pcoinsdbview, GetArg("-checklevel", DEFAULT_CHECKLEVEL),
                              GetArg("-checkblocks", DEFAULT_CHECKBLOCKS))) {
                    strLoadError = _("Corrupted block database detected");
                    break;
                }
            } catch (const std::exception& e) {
                LogPrintf("%s\n", e.what());
                strLoadError = _("Error opening block database");
                break;
            }

            fLoaded = true;
        } while(false);

        if (!fLoaded) {
            // first suggest a reindex
            if (!fReset) {
                bool fRet = uiInterface.ThreadSafeQuestion(
                    strLoadError + ".\n\n" + _("Do you want to rebuild the block database now?"),
                    strLoadError + ".\nPlease restart with -reindex to recover.",
                    "", CClientUIInterface::MSG_ERROR | CClientUIInterface::BTN_ABORT);
                if (fRet) {
                    fReindex = true;
                    fRequestShutdown = false;
                } else {
                    LogPrintf("Aborted block database rebuild. Exiting.\n");
                    return false;
                }
            } else {
                return InitError(strLoadError);
            }
        }
    }

    // As LoadBlockIndex can take several minutes, it's possible the user
    // requested to kill the GUI during the last operation. If so, exit.
    // As the program has not fully started yet, Shutdown() is possibly overkill.
    if (fRequestShutdown)
    {
        LogPrintf("Shutdown requested. Exiting.\n");
        return false;
    }
    LogPrintf(" block index %15dms\n", GetTimeMillis() - nStart);

    fs::path est_path = GetDataDir() / FEE_ESTIMATES_FILENAME;
    CAutoFile est_filein(fsbridge::fopen(est_path, "rb"), SER_DISK, CLIENT_VERSION);
    // Allowed to fail as this file IS missing on first startup.
    if (!est_filein.IsNull())
        mempool.ReadFeeEstimates(est_filein);
    fFeeEstimatesInitialized = true;


    // ********************************************************* Step 8: load wallet
#ifdef ENABLE_WALLET
    if (fDisableWallet) {
        pwalletMain = NULL;
        LogPrintf("Wallet disabled!\n");
    } else {
        CWallet::InitLoadWallet(clearWitnessCaches || fReindex);
        if (!pwalletMain)
            return false;
    }
#else // ENABLE_WALLET
    LogPrintf("No wallet support compiled in!\n");
#endif // !ENABLE_WALLET

#ifdef ENABLE_MINING
 #ifndef ENABLE_WALLET
    if (GetBoolArg("-minetolocalwallet", false)) {
        return InitError(_("BitcoinZ was not built with wallet support. Set -minetolocalwallet=0 to use -mineraddress, or rebuild BitcoinZ with wallet support."));
    }
    if (GetArg("-mineraddress", "").empty() && GetBoolArg("-gen", false)) {
        return InitError(_("BitcoinZ was not built with wallet support. Set -mineraddress, or rebuild BitcoinZ with wallet support."));
    }
 #endif // !ENABLE_WALLET

    if (mapArgs.count("-mineraddress")) {
 #ifdef ENABLE_WALLET
        bool minerAddressInLocalWallet = false;
        if (pwalletMain) {
            // Address has already been validated
            CTxDestination addr = keyIO.DecodeDestination(mapArgs["-mineraddress"]);
            CKeyID keyID = std::get<CKeyID>(addr);
            minerAddressInLocalWallet = pwalletMain->HaveKey(keyID);
        }
        if (GetBoolArg("-minetolocalwallet", true) && !minerAddressInLocalWallet) {
            return InitError(_("-mineraddress is not in the local wallet. Either use a local address, or set -minetolocalwallet=0"));
        }
 #endif // ENABLE_WALLET

        // This is leveraging the fact that boost::signals2 executes connected
        // handlers in-order. Further up, the wallet is connected to this signal
        // if the wallet is enabled. The wallet's ScriptForMining handler does
        // nothing if -mineraddress is set, and GetScriptForMinerAddress() does
        // nothing if -mineraddress is not set (or set to an invalid address).
        //
        // The upshot is that when ScriptForMining(script) is called:
        // - If -mineraddress is set (whether or not the wallet is enabled), the
        //   CScript argument is set to -mineraddress.
        // - If the wallet is enabled and -mineraddress is not set, the CScript
        //   argument is set to a wallet address.
        // - If the wallet is disabled and -mineraddress is not set, the CScript
        //   argument is not modified; in practice this means it is empty, and
        //   GenerateBitcoins() returns an error.
        GetMainSignals().ScriptForMining.connect(GetScriptForMinerAddress);
    }
#endif // ENABLE_MINING

    // Spawn a thread that will wait for the chain state needed for
    // ThreadNotifyWallets to become available.
    threadGroup.create_thread(
        std::bind(&TraceThread<void (*)()>, "txnotify", &ThreadStartWalletNotifier)
    );

    // ********************************************************* Step 9: data directory maintenance

    // if pruning, unset the service bit and perform the initial blockstore prune
    // after any wallet rescanning has taken place.
    if (fPruneMode) {
        LogPrintf("Unsetting NODE_NETWORK on prune mode\n");
        nLocalServices &= ~NODE_NETWORK;
        if (!fReindex) {
            uiInterface.InitMessage(_("Pruning blockstore..."));
            PruneAndFlush();
        }
    }

    // ********************************************************* Step 10: import blocks

    if (!CheckDiskSpace())
        return false;

    // Either install a handler to notify us when genesis activates, or set fHaveGenesis directly.
    // No locking, as this happens before any background thread is started.
    if (chainActive.Tip() == NULL) {
        uiInterface.NotifyBlockTip.connect(BlockNotifyGenesisWait);
    } else {
        fHaveGenesis = true;
    }

    if (mapArgs.count("-blocknotify"))
        uiInterface.NotifyBlockTip.connect(BlockNotifyCallback);

    if (mapArgs.count("-txexpirynotify"))
        uiInterface.NotifyTxExpiration.connect(TxExpiryNotifyCallback);

    uiInterface.InitMessage(_("Activating best chain..."));
    // scan for better chains in the block chain database, that are not yet connected in the active best chain
    CValidationState state;
    if (!ActivateBestChain(state, chainparams))
        strErrors << "Failed to connect best block";

    std::vector<fs::path> vImportFiles;
    if (mapArgs.count("-loadblock"))
    {
        for (const std::string& strFile : mapMultiArgs["-loadblock"])
            vImportFiles.push_back(strFile);
    }
    threadGroup.create_thread(boost::bind(&ThreadImport, vImportFiles));

    // Wait for genesis block to be processed
    {
        boost::unique_lock<boost::mutex> lock(cs_GenesisWait);
        while (!fHaveGenesis) {
            condvar_GenesisWait.wait(lock);
        }
        uiInterface.NotifyBlockTip.disconnect(BlockNotifyGenesisWait);
    }
    if (!fHaveGenesis) {
        return false;
    }

    // ********************************************************* Step 11: start node

    if (!strErrors.str().empty())
        return InitError(strErrors.str());

    //// debug print
    {
        LOCK(cs_main);
        LogPrintf("mapBlockIndex.size() = %u\n", mapBlockIndex.size());
        LogPrintf("nBestHeight = %d\n", chainActive.Height());
    }
#ifdef ENABLE_WALLET
    if (pwalletMain)
    {
        LOCK(pwalletMain->cs_wallet);
        LogPrintf("setKeyPool.size() = %u\n",      pwalletMain->setKeyPool.size());
        LogPrintf("mapWallet.size() = %u\n",       pwalletMain->mapWallet.size());
        LogPrintf("mapAddressBook.size() = %u\n",  pwalletMain->mapAddressBook.size());
    }
#endif

    if (GetBoolArg("-listenonion", DEFAULT_LISTEN_ONION))
        StartTorControl(threadGroup, scheduler);

    StartNode(threadGroup, scheduler);

#ifdef ENABLE_MINING
    // Generate coins in the background
    GenerateBitcoins(GetBoolArg("-gen", DEFAULT_GENERATE), GetArg("-genproclimit", DEFAULT_GENERATE_THREADS), chainparams);
#endif

    // ********************************************************* Step 12: finished

    SetRPCWarmupFinished();
    uiInterface.InitMessage(_("Done loading"));

#ifdef ENABLE_WALLET
    if (pwalletMain) {
        // Add wallet transactions that aren't already in a block to mapTransactions
        pwalletMain->ReacceptWalletTransactions();

        // Run a thread to flush wallet periodically
        threadGroup.create_thread(boost::bind(&ThreadFlushWalletDB, boost::ref(pwalletMain->strWalletFile)));
    }
#endif

    return !fRequestShutdown;
}
