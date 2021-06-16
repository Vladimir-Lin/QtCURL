#!/bin/sh
# one step compilation for iPhone ARMv7

export LTD=/Users/Foxman/Temp
export APN=curl
export ZOS=/Users/Foxman/CIOS/Development/ZOS
export ARX=ARMv7
export PLT=iPhone
export ACC=arm-apple-darwin
export XOS=Developer/Platforms/iPhoneOS.platform/Developer
export IOS=/$XOS
export IROOT=/Applications/Xcode.app/Contents/$XOS/SDKs/iPhoneOS7.0.sdk

tar xvf $ZOS/Requires/curl/curl-7.35.0.tar.gz

cd curl-7.35.0

export CC="$IOS/usr/llvm-gcc-4.2/bin/llvm-gcc-4.2 --sysroot=$IROOT"
export CXX="$IOS/usr/llvm-gcc-4.2/bin/llvm-g++-4.2 --sysroot=$IROOT"
export LD="$IOS/usr/bin/ld"
export AR="$IOS/usr/bin/ar"
export NM="$IOS/usr/bin/nm"
export RANLIB="$IOS/usr/bin/ranlib"
export STRIP="$IOS/usr/bin/strip"
export LIBTOOL="$IOS/usr/bin/libtool"
export CPPFLAGS="-arch armv7 -isysroot $IROOT -miphoneos-version-min=4.2"
export LIBS="-L$IROOT/usr/lib -L$IROOT/usr/lib/system"

./configure \
--host=$ACC \
--disable-shared \
--enable-static \
--enable-optimize \
--enable-ipv6 \
--disable-curldebug \
--prefix=$LTD/$APN
make
