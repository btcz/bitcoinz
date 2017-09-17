# BitcoinZ - Your Financial Freedom
Community gift to the World.

- Type: Cryptocurrency
- Ticker: BTZ
- Algo: Equihash
- Max supply 21B coins / Current supply: 12500 coins every 2.5 minutes
- Tech: zcl spirit + zcash white paper + bitcoin. 
- Current block size is similar to BCC/BCH (BTZ = 2MB every 2.5 mins ~ BCC/BCH = 8MB every 10 min)
- Mother coins: ZCL, ZEC, BTC

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

## Roadmap
- Update to latest Zcash codebase
- Finish rebranding

Install
-----------------
### Linux

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
Get dependencies
```{r, engine='bash'}
sudo apt-get install \
      build-essential pkg-config libc6-dev m4 g++-multilib \
      autoconf libtool ncurses-dev unzip git python \
      zlib1g-dev wget bsdmainutils automake mingw-w64
```

Install (Cross-Compiled, building on Windows is not supported yet)
```{r, engine='bash'}
# Build
./zcutil/build-win.sh -j$(nproc)
```
The exe will save to `src` which you can then move to a windows machine

### Mac
Get dependencies
```{r, engine='bash'}
#install xcode
xcode-select --install

/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew install cmake autoconf libtool automake coreutils pkgconfig gmp wget

brew install gcc5 --without-multilib
```

Install
```{r, engine='bash'}
# Build
./zcutil/build-mac.sh -j$(nproc)
# fetch key
./zcutil/fetch-params.sh
# Run
./src/zcashd
```

Security Warnings
-----------------

See important security warnings in
[doc/security-warnings.md](doc/security-warnings.md).

BitcoinZ is unfinished and highly experimental. Use at your own risk.
