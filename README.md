# BitcoinZ
**Keep running wallet to strengthen the BitcoinZ network. Backup your wallet in many locations & keep your coins wallet offline.**

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
# Clone Bitcoinz Repository
git clone https://github.com/btcz/bitcoinz
# Build
cd bitcoinz/
./zcutil/build.sh -j$(nproc)
# fetch key
./zcutil/fetch-params.sh
# Run
./src/zcashd
# Test getting information about the network
cd src/
./zcash-cli getmininginfo
# Test creating new transparent address
./zcash-cli getnewaddress
# Test creating new private address
./zcash-cli z_getnewaddress
# Test checking transparent balance
./zcash-cli getbalance
# Test checking total balance 
./zcash-cli z_gettotalbalance
# Check all available wallet commands
./zcash-cli help
# Get more info about a single wallet command
./zcash-cli help "The-command-you-want-to-learn-more-about"
./zcash-cli help "getbalance"
```

### Docker

Build
```
$ docker build -t btcz/bitcoinz .
```

Create a data directory on your local drive and create a bitcoinz.conf config file
```
$ mkdir -p /ops/volumes/bitcoinz/data
$ touch /ops/volumes/bitcoinz/data/bitcoinz.conf
$ chown -R 999:999 /ops/volumes/bitcoinz/data
```

Create bitcoinz.conf config file and run the application
```
$ docker run -d --name bitcoinz-node \
  -v bitcoinz.conf:/bitcoinz/data/bitcoinz.conf \
  -p 1989:1989 -p 127.0.0.1:1979:1979 \
  btcz/bitcoinz
```

Verify bitcoinz-node is running
```
$ docker ps
CONTAINER ID        IMAGE                  COMMAND                  CREATED             STATUS              PORTS                                              NAMES
31868a91456d        btcz/bitcoinz          "zcashd --datadir=..."   2 hours ago         Up 2 hours          127.0.0.1:1979->1979/tcp, 0.0.0.0:1989->1989/tcp   bitcoinz-node
```

Follow the logs
```
docker logs -f bitcoinz-node
```

The cli command is a wrapper to zcash-cli that works with an already running Docker container
```
docker exec -it bitcoinz-node cli help
```

## Using a Dockerfile
If you'd like to have a production btc/bitcoinz image with a pre-baked configuration
file, use of a Dockerfile is recommended:

```
FROM btcz/bitcoinz
COPY bitcoinz.conf /bitcoinz/data/bitcoinz.conf
```

Then, build with `docker build -t my-bitcoinz .` and run.

### Windows
Windows build is maintained in [bitcoinz-win project](https://github.com/bitcoinz-pod/bitcoinz-win).

Security Warnings
-----------------

**BitcoinZ is experimental and a work-in-progress.** Use at your own risk.
