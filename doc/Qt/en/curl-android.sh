#!/bin/sh
# one step compilation for android

export ZOS=/home/CIOS/Development/ZOS
export ARX=ARMv7
export PLT=Android

tar xvf $ZOS/Requires/curl/curl-7.35.0.tar.gz

cd curl-7.35.0

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

./configure \
--host=$ACC \
--disable-shared \
--enable-static \
--enable-optimize \
--enable-ipv6 \
--disable-curldebug \
--prefix=/home/Builds/cURL
make
make install
