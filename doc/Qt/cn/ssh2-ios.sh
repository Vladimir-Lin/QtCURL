#!/bin/sh
# one step compilation for iPhone ARMv7

export ZOS=/Users/Foxman/CIOS/Development/ZOS
export ARX=ARMv7
export PLT=iPhone

tar xvf $ZOS/Requires/libSSH2/libssh2-1.4.3.tar.gz

cd libssh2-1.4.3

export IOS=/Developer/Platforms/iPhoneOS.platform/Developer
export IROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS7.0.sdk
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

./configure --host=arm-apple-darwin --disable-shared --enable-static --with-libssl-prefix=/Users/Foxman/Temp/openssl --with-libz-prefix=/Users/Foxman/Temp/zlib
make
