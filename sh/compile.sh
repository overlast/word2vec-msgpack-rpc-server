#!/usr/bin/env bash

BASEDIR=$(cd $(dirname $0);pwd)
SCRIPT_NAME="[compile_using_waf]"

cd $BASEDIR/../

echo "$SCRIPT_NAME trying to uninstall.."
sudo waf uninstall

echo "$SCRIPT_NAME cleaning a build directory.."
waf clean

echo "$SCRIPT_NAME configure.."
waf configure

echo "$SCRIPT_NAME trying to build.."
waf build

echo "$SCRIPT_NAME trying to install.."
sudo waf install

echo "$SCRIPT_NAME Finish.."
