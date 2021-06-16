#!/bin/sh
# one step compilation for android

export ZOS=/home/CIOS/Development/ZOS
export ARX=ARMv7
export PLT=Android

tar xvf $ZOS/Requires/libSSH2/libssh2-1.4.3.tar.gz

cd libssh2-1.4.3

export TKX=/home/Builds/androidsys
export ACC=arm-linux-androideabi
export ATC=bin/$ACC

export CC="$TKX/$ATC-gcc --sysroot=$TKX/sysroot"
export CXX="$TKX/$ATC-g++ --sysroot=$TKX/sysroot"
export AR="$TKX/$ATC-ar"
export LD="$TKX/$ATC-ld"
export RANLIB="$TKX/$ATC-ranlib"
export STRIP="$TKX/$ATC-strip"
export SYSROOT="$TKX/sysroot"
export PATH="$TKX/bin":$PATH

./configure --host=$ACC --disable-shared --enable-static --with-libssl-prefix=/home/Builds/openssl --with-libz-prefix=/home/Builds/zlib
make
