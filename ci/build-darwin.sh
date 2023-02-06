#!/bin/bash

# use homebrew toolchain
brew update
brew install llvm

ARCHIVE_ID=fastcws-darwin-$BUILD_TYPE-$(git rev-parse --short HEAD)
INSTALL_DIR=./$ARCHIVE_ID
ARCHIVE_FILENAME=./output/$ARCHIVE_ID.tar.gz

# build & install the project
mkdir -p build $INSTALL_DIR
cmake -B ./build -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DCMAKE_TOOLCHAIN_FILE=$(pwd)/ci/brew-llvm.toolchain.cmake
cmake --build ./build --config $BUILD_TYPE
cmake --install ./build --config $BUILD_TYPE

# copy license
cp LICENSE $INSTALL_DIR/

# package the binary
mkdir -p output
tar -czvf $ARCHIVE_FILENAME $ARCHIVE_ID

