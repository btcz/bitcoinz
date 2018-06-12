#!/bin/bash

set -e

docker build -f ./qa/Dockerfile.test -t btcz/bitcoinz-test .
docker run -it --rm btcz/bitcoinz-test ./qa/bitcoinz/full_test_suite.py
docker run -it --rm btcz/bitcoinz-test ./qa/pull-tester/rpc-tests.sh
