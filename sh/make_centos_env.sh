#!/usr/bin/env bash

BASEDIR=$(cd $(dirname $0);pwd)
SCRIPT_NAME="[make_centos_env]"
TMP_DIR=/tmp/msgpack_rpc_c

mkdir $TMP_DIR
cd $TMP_DIR

echo "$SCRIPT_NAME trying to install msgpack-c.."
git clone https://github.com/msgpack/msgpack-c.git
cd msgpack-c
./bootstrap
./configure CXXFLAGS="-std=c++11"
make
sudo make install

echo "$SCRIPT_NAME making clean msgpack-c directory.."
cd $TMP_DIR
rm -rf msgpack-c

echo "$SCRIPT_NAME trying to install mpio.."
git clone https://github.com/frsyuki/mpio.git
cd mpio
./bootstrap
./configure
make
sudo make install

echo "$SCRIPT_NAME making clean mpio directory.."
cd $TMP_DIR
rm -rf mpio

echo "$SCRIPT_NAME Refreshing the cache of shared library.."
sudo ldconfig

echo "$SCRIPT_NAME trying to install msgpack-rpc-cpp.."
git clone https://github.com/msgpack-rpc/msgpack-rpc-cpp.git
cd msgpack-rpc-cpp
./bootstrap  # if needed
./configure
make
sudo make install

echo "$SCRIPT_NAME Refreshing the cache of shared library.."
sudo ldconfig

echo "$SCRIPT_NAME making clean msgpack-rpc-cpp directory.."
cd $TMP_DIR
rm -rf msgpack-rpc-cpp

echo "$SCRIPT_NAME Making clean a build directory.."
cd $BASEDIR
rm -rf $TMP_DIR

echo "$SCRIPT_NAME Trying to install waf.."
sudo yum install -y waf

echo "$SCRIPT_NAME Trying to install jansson.."
sudo yum install -y jansson

echo "$SCRIPT_NAME Finish.."
