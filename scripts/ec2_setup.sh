#!/usr/bin/env bash

set -e
set -x

#
# Dependencies
#

# Packages
sudo yum groupinstall -y "Development Tools"
sudo yum install -y \
     openssl openssl-devel \
     gmp-devel procps-devel cmake3 \
     python3 python3-devel \
     boost-devel \
     emacs-nox
sudo ln -sf /usr/bin/cmake3 /usr/local/bin/cmake

# Library directories
sudo sh -c 'echo /usr/local/lib > /etc/ld.so.conf.d/local-x86_64.conf'
sudo ldconfig

# Install grpc
scripts/install_grpc /usr/local v1.31.x

# Install and activate nodejs
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.34.0/install.sh | bash
. ~/.nvm/nvm.sh
nvm install 10

# Setup ganache
pushd zeth_contracts
npm config set python python2.7
npm config set engine-strict true
npm config set unsafe-perm true
npm install --unsafe-perm
popd

#
# Zeth-specific setup
#

# Setup client
pushd client
python3 -m venv env
. env/bin/activate
make setup
sudo ln -sf ~/.solcx/solc-* /usr/local/bin/solc
popd

# Prover server
mkdir -p build
pushd build
    CMAKE_PREFIX_PATH=/usr/local cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j"$(($(nproc)+1))" prover_server zeth-tool
popd
