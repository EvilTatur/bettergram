#!/bin/bash
#
# This script prepares libraries needed for building Bettergram application
# on Ubuntu 16.04. Please note that we change these
# libraries with patches from Bettergram repository.
#
# This script may be run with or without sudo,
# but at the second case we will ask sudo password in order to install results.
#
# We can use it at dockerfile.
#
# Maintainer: Ildar Gilmanov <dev@ddwarf.org>
# See also https://bettergram.io
#

# Params: 
# $1:	Success text
# $2:	Error text
check_result()
{
  res=$?

  if [ $res -eq 0 ]
  then    
    if [ ! -z "$1" ]
    then
      echo ""
      echo "SUCCESS: $1"
      echo ""
    fi
  else
    >&2 echo ""
    >&2 echo "ERROR: Script failed. Return value: '$res'. $2"
    exit 1
  fi
}

if [ "$EUID" -eq 0 ]
then
  SUDO=""
else
  SUDO=sudo
fi

echo ""
echo "Trying to clone Bettergram git repository..."
echo ""

git clone --recursive https://github.com/bettergram/bettergram.git

check_result "Bettergram git repository is cloned" "Unable to clone Bettergram git repository"

echo ""
echo "Trying to enter to Libraries directory..."
echo ""

cd Libraries

check_result "We are at the Libraries directory" "Unable to enter to Libraries directory"

echo ""
echo "Trying to build Qt 5.6.2..."
echo ""

git clone git://code.qt.io/qt/qt5.git qt5_6_2 \
  && cd qt5_6_2 \
  && perl init-repository --module-subset=qtbase,qtimageformats \
  && git checkout v5.6.2 \
  && cd qtimageformats && git checkout v5.6.2 && cd .. \
  && cd qtbase && git checkout v5.6.2 && cd .. \
  && cd qtbase && git apply ../../../bettergram/Telegram/Patches/qtbase_5_6_2.diff && cd .. \
  && cd qtbase/src/plugins/platforminputcontexts \
  && git clone https://github.com/telegramdesktop/fcitx.git \
  && git clone https://github.com/telegramdesktop/hime.git \
  && cd ../../../.. \
  && OPENSSL_LIBS='-L/usr/local/ssl/lib -lssl -lcrypto' \
  && ./configure \
    -prefix "/usr/local/tdesktop/Qt-5.6.2" \
    -release \
    -force-debug-info \
    -opensource \
    -confirm-license \
    -qt-zlib \
    -qt-libpng \
    -qt-libjpeg \
    -qt-freetype \
    -qt-harfbuzz \
    -qt-pcre \
    -qt-xcb \
    -qt-xkbcommon-x11 \
    -no-opengl \
    -no-gtkstyle \
    -static \
    -openssl-linked \
    -nomake examples \
    -nomake tests \
  && make $MAKE_THREADS_CNT \
  && $SUDO make install \
  && cd .. \
  && rm -rf qt5_6_2

check_result "Qt 5.6.2 is built" "Unable to build Qt 5.6.2"

echo ""
echo "Trying to build gyp..."
echo ""

git clone https://chromium.googlesource.com/external/gyp \
  && cd gyp \
  && git checkout 702ac58e47 \
  && git apply ../../bettergram/Telegram/Patches/gyp.diff \
  && cd ..

check_result "gyp is built" "Unable to build gyp"

echo ""
echo "Trying to build breakpad..."
echo ""

git clone https://chromium.googlesource.com/breakpad/breakpad \
  && cd breakpad \
  && git checkout bc8fb886 \
  && git clone https://chromium.googlesource.com/linux-syscall-support src/third_party/lss \
  && cd src/third_party/lss \
  && git checkout a91633d1 \
  && cd ../../.. \
  && ./configure \
  && make $MAKE_THREADS_CNT \
  && $SUDO make install \
  && cd src/tools \
  && ../../../gyp/gyp \
    --depth=. \
    --generator-output=.. \
    -Goutput_dir=../out \
    tools.gyp \
    --format=cmake \
  && cd ../../out/Default

check_result "breakpad is built" "Unable to build breakpad"

# cmake returns error due the google changed gtest project
cmake . && make $MAKE_THREADS_CNT dump_syms

cd ../../../..

exit 0