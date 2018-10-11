#!/bin/bash
#
# This script installs GCC 7.2, CMake 3.2 and other Bettergram dependencies
# that we can fetch from Ubuntu repositories on Ubuntu 16.04.
#
# This script should be run with sudo.
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

if [ "$EUID" -ne 0 ]
then
  >&2 echo "ERROR: Please run as root"
  exit 1
fi

echo ""
echo "Trying to install GCC 7..."
echo ""

apt-get update \
  && apt-get -y install gcc g++ \
  && apt-get install -y software-properties-common \
  && add-apt-repository ppa:ubuntu-toolchain-r/test \
  && apt-get update \
  && apt-get install -y gcc-7 g++-7 \
  && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 60 \
  && add-apt-repository --remove ppa:ubuntu-toolchain-r/test

check_result "GCC 7 is installed" "Unable to install GCC 7"

echo ""
echo "Trying to install Bettergram dependencies from Ubuntu repositories..."
echo ""

apt-get install -y \
  git \
  libexif-dev \
  liblzma-dev \
  libz-dev \
  libssl-dev \
  libappindicator-dev \
  libunity-dev \
  libicu-dev \
  libdee-dev \
  libdrm-dev \
  dh-autoreconf \
  autoconf \
  automake \
  build-essential \
  libass-dev \
  libfreetype6-dev \
  libgpac-dev \
  libsdl1.2-dev \
  libtheora-dev \
  libtool \
  libva-dev \
  libvdpau-dev \
  libvorbis-dev \
  libxcb1-dev \
  libxcb-image0-dev \
  libxcb-shm0-dev \
  libxcb-xfixes0-dev \
  libxcb-keysyms1-dev \
  libxcb-icccm4-dev \
  libxcb-render-util0-dev \
  libxcb-util0-dev \
  libxrender-dev \
  libasound-dev \
  libpulse-dev \
  libxcb-sync0-dev \
  libxcb-randr0-dev \
  libx11-xcb-dev \
  libffi-dev \
  libncurses5-dev \
  pkg-config \
  texi2html \
  zlib1g-dev \
  yasm \
  cmake \
  xutils-dev \
  bison \
  python-xcbgen \
  libbsd-dev

check_result "Bettergram dependencies are installed from Ubuntu repositories" "Unable to install Bettergram dependencies from Ubuntu repositories"

exit 0