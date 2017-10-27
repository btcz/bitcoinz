# BitcoinZ - Your Financial Freedom
Community gift to the World.

**Keep running wallet to strengthen the BitcoinZ network. Backup your wallet in many locations & keep your coins wallet offline.**

![BTCZ](https://ip.bitcointalk.org/?u=https%3A%2F%2Fi.imgur.com%2FXflfHDN.png&t=582&c=-VhG9Suykc7RCA)

- Type: Cryptocurrency
- Ticker: BTCZ
- Algo: Equihash
- Max supply 21B coins / Current supply: 12500 coins every 2.5 minutes
- Current block size is similar to BCC/BCH (BTCZ = 2MB every 2.5 mins ~ BCC/BCH = 8MB every 10 min)
- Technology / Mother coins: ZCL spirit, ZEC privacy, BTC fundamentals

## BitcoinZ Principles: 
- Anonymous
It is recommended to use anonymous zaddr for all transactions. Public taddrs transactions are also allowed.
- All contributors are welcome and everyone is volunteer - no premine or dev taxes (Satoshi has about 10% of BTC marketcap!)
- Store of value - always POW and never POS
- Fully decentralized cryptocurrency (no root domain, no main website, no main dev team)
- Decentralized development
Developers are self organized in development pods. Every new dev or team is welcome. All implementations which follow consensus are allowed. No need to register - just create new pod account (e.g. github) and convince community.
- Decentralized mining
As a miner you should not use the biggest pools to follow main principles.
- Easy to mine
Equihash algorithm.
You can use your Desktop PC to mine BitcoinZ. Most profitable is GPU mining.
- Decentalized Exchanges
All exchanges are allowed. The best one are decentralized. We plan to implement fully decentralized XCAT exchange.
- Pure community coin
- Always immutable - no way to change history!

[bitcointalk.org ANN thread](https://bitcointalk.org/index.php?topic=2166510.new#new)
- Post your nodes https://github.com/bitcoinz-pod/bitcoinz/issues/1
- Post pools: https://github.com/bitcoinz-pod/bitcoinz/issues/2
- Post exchanges: https://github.com/bitcoinz-pod/bitcoinz/issues/3
- Explorers
https://btzexplorer.blockhub.info

### Other wallets
- [BitcoinZ Windows Wallet](https://github.com/bitcoinz-pod/bitcoinz-windows-wallet/releases)
- [BitcoinZ Cold Wallet](https://github.com/bitcoinz-pod/zgenerate/releases) - Windows binary is available. All you need to own all coin in your address is only private key.

## Roadmap
- Windows build
- Windows GUI wallet
- Anonymous messaging channels

### Ports:
- RPC port: 1979
- P2P port: 1989

Install
-----------------
### Linux

### [Quick guide for beginners](https://github.com/bitcoinz-pod/bitcoinz/wiki/Quick-guide-for-beginners)

Get dependencies
```{r, engine='bash'}
sudo apt-get install \
      build-essential pkg-config libc6-dev m4 g++-multilib \
      autoconf libtool ncurses-dev unzip git python \
      zlib1g-dev wget bsdmainutils automake
```

Install
```{r, engine='bash'}
# Build
./zcutil/build.sh -j$(nproc)
# fetch key
./zcutil/fetch-params.sh
# Run
./src/zcashd
```

### Windows
Windows build is maintained in [bitcoinz-win project](https://github.com/bitcoinz-pod/bitcoinz-win).

Security Warnings
-----------------

**BitcoinZ is experimental and a work-in-progress.** Use at your own risk.
