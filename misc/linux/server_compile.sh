#!/bin/bash

set -e

export BUILD_CLIENT="${BUILD_CLIENT:-0}"
export BUILD_SERVER="${BUILD_SERVER:-1}"
export USE_CURL="${USE_CURL:-1}"
export USE_CODEC_OPUS="${USE_CODEC_OPUS:-1}"
export USE_VOIP="${USE_VOIP:-1}"
export COPYDIR="${COPYDIR:-~/quakewars}"
QWREMOTE="${QWREMOTE:-https://github.com/ioid3-games/ioid3-qw.git}"
MAKE_OPTS="${MAKE_OPTS:--j2}"

if ! [ -x "$(command -v git)" ] || ! [ -x "$(command -v make)" ]; then
        echo "This build script requires 'git' and 'make' to be installed." >&2
        echo "Please install them through your normal package installation system." >&2
        exit 1
fi

echo " This build process requires all of the Quake Wars dependencies necessary for a Quake Wars server.
 If you do not have the necessary dependencies the build will fail.

 Please post a message to http://forum.clover.moe/ asking for help and include whatever error messages you received during the compile phase.

 We will be building from the git repo at ${QWREMOTE}
 The resulting binary will be installed to ${COPYDIR}

 If you need to change these, please set variables as follows:

 QWREMOTE=https://github.com/something/something.git COPYDIR=~/somewhere $0"

BUILD_DIR="$(mktemp -d)"
trap "rm -rf $BUILD_DIR" EXIT

while true; do
        read -p "Are you ready to compile Quake Wars from ${QWREMOTE}, and have it installed into $COPYDIR? " yn
        case $yn in
                [Yy]*)
                        git clone $QWREMOTE $BUILD_DIR/quakewars
                        cd $BUILD_DIR/quakewars
                        make $MAKE_OPTS
                        make copyfiles
                        exit
                        ;;
                [Nn]*)
                        echo "aborting installation."
                        exit
                        ;;
                *)
                        echo "Please answer yes or no."
                        ;;
        esac
done
