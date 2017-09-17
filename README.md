# BitcoinZ - Your Financial Freedom
Community gift to the World.

Type: Cryptocurrency
Ticker: BTZ
Algo: Equihash
Max supply 21B coins / Current supply: 12500 coins every 2.5 minutes
Tech: zcl spirit + zcash white paper + bitcoin. 
Current block size is similar to BCC/BCH (BTZ = 2MB every 2.5 mins ~ BCC/BCH = 8MB every 10 min)
Mother coins: ZCL, ZEC, BTC

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

BitcoinZ is unfinished and highly experimental. Use at your own risk.

==========================

Zclassic v1.0.10-1

NOTICE, the default ports have changed! The p2p port is now 8033 and rpcport is 8023

What is Zclassic?
----------------
Zclassic is financial freedom.

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

About
--------------

[Zclassic](http://zclassic.org/), like [Zcash](https://z.cash/), is an implementation of the "Zerocash" protocol.
Based on Bitcoin's code, it intends to offer a far higher standard of privacy
through a sophisticated zero-knowledge proving scheme that preserves
confidentiality of transaction metadata. Technical details are available
in the Zcash [Protocol Specification](https://github.com/zcash/zips/raw/master/protocol/protocol.pdf).

This software is the Zclassic client. It downloads and stores the entire history
of Zclassic transactions; depending on the speed of your computer and network
connection, the synchronization process could take a day or more once the
blockchain has reached a significant size.

Security Warnings
-----------------

See important security warnings in
[doc/security-warnings.md](doc/security-warnings.md).

**Zclassic and Zcash are unfinished and highly experimental.** Use at your own risk.

Deprecation Policy
------------------

This release is considered deprecated 16 weeks after the release day. There
is an automatic deprecation shutdown feature which will halt the node some
time after this 16 week time period. The automatic feature is based on block
height and can be explicitly disabled.

Where do I begin?
-----------------
We have a guide for joining the main Zclassic network:
https://github.com/z-classic/zclassic/wiki/1.0-User-Guide

### Need Help?

* See the documentation at the [Zclassic Wiki](https://github.com/z-classic/zclassic/wiki)
  for help and more information.
* Ask for help on the [Zclassic](http://zcltalk.tech/index.php) forum.

### Want to participate in development?

* Code review is welcome!
* If you want to get to know us join our slack: http://zclassic.herokuapp.com/


Participation in the Zcash project is subject to a
[Code of Conduct](code_of_conduct.md).

Building
--------

Build Zcash along with most dependencies from source by running
./zcutil/build.sh. Currently only Linux is officially supported.

License
-------

For license information see the file [COPYING](COPYING).
