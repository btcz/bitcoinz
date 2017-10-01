#!/bin/bash

sudo apt -y update
sudo apt-get install -y libc6-dev g++-multilib python p7zip-full pwgen
cd ~
wget https://github.com/bitcoinz-pod/bitcoinz/releases/download/1.1.1/bitcoinz1_1_1_x64.zip
7z x bitcoinz1_1_1_x64.zip
rm bitcoinz1_1_1_x64.zip
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
cd ~/bitcoinz-pkg
./zcashd