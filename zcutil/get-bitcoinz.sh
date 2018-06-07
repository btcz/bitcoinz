#!/bin/bash

sudo apt -y update
sudo apt-get install -y libc6-dev g++-multilib python p7zip-full pwgen jq curl
cd ~

if [ -f bitcoinz.zip ]
then
    rm bitcoinz.zip
fi
wget -O bitcoinz.zip `curl -s 'https://api.github.com/repos/bitcoinz-pod/bitcoinz/releases/latest' | jq -r '.assets[].browser_download_url' | egrep "bitcoinz.+x64.zip"`
7z x -y bitcoinz.zip
chmod -R a+x ~/bitcoinz-pkg
rm bitcoinz.zip

cd ~/bitcoinz-pkg
./fetch-params.sh

if ! [[ -d ~/.bitcoinz ]]
then
    mkdir -p ~/.bitcoinz
fi

if ! [[ -f ~/.bitcoinz/bitcoinz.conf ]]
then
    echo "rpcuser=rpc`pwgen 15 1`" > ~/.bitcoinz/bitcoinz.conf
    echo "rpcpassword=rpc`pwgen 15 1`" >> ~/.bitcoinz/bitcoinz.conf
fi

./bitcoinzd
