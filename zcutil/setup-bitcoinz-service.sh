#!/bin/bash

if [ $# -eq 0 ]
then
    echo "BitcoinZ systemd unit setup."
    echo -e "Run:\n$0 user\nor install for current user\n$0 $USER"
    exit 1
fi

if id "$1" >/dev/null 2>&1
then
    echo "Installing BitcoinZ service for $1 user..."
else
    echo -e "User $1 does not exist.\nTo add user run the following command:\nsudo adduser --disabled-password --gecos '' $1"
    exit 1
fi

cat > /tmp/config_setup.sh << EOF
#!/bin/bash
if ! [[ -d ~/.bitcoinz ]]
then
    mkdir -p ~/.bitcoinz
fi

if ! [[ -f ~/.bitcoinz/bitcoinz.conf ]]
then
    echo "rpcuser=rpc`pwgen 15 1`" > ~/.bitcoinz/bitcoinz.conf
    echo "rpcpassword=rpc`pwgen 15 1`" >> ~/.bitcoinz/bitcoinz.conf
fi
EOF
chmod +x /tmp/config_setup.sh
sudo -H -u $1 /tmp/config_setup.sh
sudo -H -u $1 ~/bitcoinz-pkg/fetch-params.sh


cat > /etc/systemd/system/bitcoinz.service << EOF
[Unit]
Description=bitcoinz

[Service]
ExecStart=`cd ~; pwd`/bitcoinz-pkg/bitcoinzd
User=$1
Restart=always


[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable bitcoinz
systemctl start bitcoinz

systemctl status bitcoinz
