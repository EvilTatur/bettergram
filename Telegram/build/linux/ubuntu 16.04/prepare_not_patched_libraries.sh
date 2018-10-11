#!/bin/bash
#
# This script prepares libraries needed for building Bettergram application
# on Ubuntu 16.04. Please note that we do not change these libraries,
# just fetch, build and install them.
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
echo "Trying to create Libraries directory..."
echo ""

mkdir Libraries && cd Libraries

check_result "Libraries directory is created" "Unable to create Libraries directory"

echo ""
echo "Trying to clone range-v3 git respository..."
echo ""

git clone https://github.com/ericniebler/range-v3

check_result "range-v3 git repository is cloned" "Unable to clone range-v3 git repository"

echo ""
echo "Trying to build zlib..."
echo ""

git clone https://github.com/telegramdesktop/zlib.git \
  && cd zlib \
  && ./configure \
  && make $MAKE_THREADS_CNT \
  && $SUDO make install \
  && cd ..

check_result "zlib is built" "Unable to build zlib"

echo ""
echo "Trying to build opus..."
echo ""

git clone https://github.com/xiph/opus \
  && cd opus \
  && git checkout v1.2.1 \
  && ./autogen.sh \
  && ./configure \
  && make $MAKE_THREADS_CNT \
  && $SUDO make install \
  && cd ..

check_result "opus is built" "Unable to build opus"

echo ""
echo "Trying to build libva..."
echo ""

git clone https://github.com/01org/libva.git \
 && cd libva \
 && ./autogen.sh --enable-static \
 && make $MAKE_THREADS_CNT \
 && $SUDO make install \
 && cd ..

check_result "libva is built" "Unable to build libva"

echo ""
echo "Trying to build libvdpau..."
echo ""

git clone git://anongit.freedesktop.org/vdpau/libvdpau \
  && cd libvdpau \
  && ./autogen.sh --enable-static \
  && make $MAKE_THREADS_CNT \
  && $SUDO make install \
  && cd ..

check_result "libvdpau is built" "Unable to build libvdpau"

echo ""
echo "Trying to build FFmpeg..."
echo ""

git clone https://github.com/FFmpeg/FFmpeg.git ffmpeg \
  && cd ffmpeg \
  && git checkout release/3.4 \
  && ./configure \
    --prefix=/usr/local \
    --disable-programs \
    --disable-doc \
    --disable-everything \
    --enable-protocol=file \
    --enable-libopus \
    --enable-decoder=aac \
    --enable-decoder=aac_latm \
    --enable-decoder=aasc \
    --enable-decoder=flac \
    --enable-decoder=gif \
    --enable-decoder=h264 \
    --enable-decoder=h264_vdpau \
    --enable-decoder=mp1 \
    --enable-decoder=mp1float \
    --enable-decoder=mp2 \
    --enable-decoder=mp2float \
    --enable-decoder=mp3 \
    --enable-decoder=mp3adu \
    --enable-decoder=mp3adufloat \
    --enable-decoder=mp3float \
    --enable-decoder=mp3on4 \
    --enable-decoder=mp3on4float \
    --enable-decoder=mpeg4 \
    --enable-decoder=mpeg4_vdpau \
    --enable-decoder=msmpeg4v2 \
    --enable-decoder=msmpeg4v3 \
    --enable-decoder=opus \
    --enable-decoder=pcm_alaw \
    --enable-decoder=pcm_alaw_at \
    --enable-decoder=pcm_f32be \
    --enable-decoder=pcm_f32le \
    --enable-decoder=pcm_f64be \
    --enable-decoder=pcm_f64le \
    --enable-decoder=pcm_lxf \
    --enable-decoder=pcm_mulaw \
    --enable-decoder=pcm_mulaw_at \
    --enable-decoder=pcm_s16be \
    --enable-decoder=pcm_s16be_planar \
    --enable-decoder=pcm_s16le \
    --enable-decoder=pcm_s16le_planar \
    --enable-decoder=pcm_s24be \
    --enable-decoder=pcm_s24daud \
    --enable-decoder=pcm_s24le \
    --enable-decoder=pcm_s24le_planar \
    --enable-decoder=pcm_s32be \
    --enable-decoder=pcm_s32le \
    --enable-decoder=pcm_s32le_planar \
    --enable-decoder=pcm_s64be \
    --enable-decoder=pcm_s64le \
    --enable-decoder=pcm_s8 \
    --enable-decoder=pcm_s8_planar \
    --enable-decoder=pcm_u16be \
    --enable-decoder=pcm_u16le \
    --enable-decoder=pcm_u24be \
    --enable-decoder=pcm_u24le \
    --enable-decoder=pcm_u32be \
    --enable-decoder=pcm_u32le \
    --enable-decoder=pcm_u8 \
    --enable-decoder=pcm_zork \
    --enable-decoder=vorbis \
    --enable-decoder=wavpack \
    --enable-decoder=wmalossless \
    --enable-decoder=wmapro \
    --enable-decoder=wmav1 \
    --enable-decoder=wmav2 \
    --enable-decoder=wmavoice \
    --enable-encoder=libopus \
    --enable-hwaccel=h264_vaapi \
    --enable-hwaccel=h264_vdpau \
    --enable-hwaccel=mpeg4_vaapi \
    --enable-hwaccel=mpeg4_vdpau \
    --enable-parser=aac \
    --enable-parser=aac_latm \
    --enable-parser=flac \
    --enable-parser=h264 \
    --enable-parser=mpeg4video \
    --enable-parser=mpegaudio \
    --enable-parser=opus \
    --enable-parser=vorbis \
    --enable-demuxer=aac \
    --enable-demuxer=flac \
    --enable-demuxer=gif \
    --enable-demuxer=h264 \
    --enable-demuxer=mov \
    --enable-demuxer=mp3 \
    --enable-demuxer=ogg \
    --enable-demuxer=wav \
    --enable-muxer=ogg \
    --enable-muxer=opus \
  && make $MAKE_THREADS_CNT \
  && $SUDO make install \
  && cd ..

check_result "FFmpeg is built" "Unable to build FFmpeg"

echo ""
echo "Trying to build portaudio..."
echo ""

git clone https://git.assembla.com/portaudio.git \
 && cd portaudio \
 && git checkout 396fe4b669 \
 && ./configure \
 && make $MAKE_THREADS_CNT \
 && $SUDO make install \
 && cd ..

check_result "portaudio is built" "Unable to build portaudio"

echo ""
echo "Trying to build openal-soft..."
echo ""

git clone git://repo.or.cz/openal-soft.git \
  && cd openal-soft \
  && git checkout v1.18 \
  && cd build \
  && cmake -D LIBTYPE:STRING=STATIC .. \
  && make $MAKE_THREADS_CNT \
  && $SUDO make install \
  && cd ../..

check_result "openal-soft is built" "Unable to build openal-soft"

echo ""
echo "Trying to build openssl..."
echo ""

# For Ubuntu 14.04
# git checkout OpenSSL_1_0_1-stable
# For Ubuntu 16.04, 18.04 or Linux Mint 18.3
# git checkout OpenSSL_1_0_2-stable

git clone https://github.com/openssl/openssl \
  && cd openssl \
  && git checkout OpenSSL_1_0_2-stable \
  && ./config \
  && make $MAKE_THREADS_CNT \
  && $SUDO make install \
  && cd ..

check_result "openssl is built" "Unable to build openssl"

echo ""
echo "Trying to build libxkbcommon..."
echo ""

git clone https://github.com/xkbcommon/libxkbcommon.git \
  && cd libxkbcommon \
  && ./autogen.sh --disable-x11 \
  && make $MAKE_THREADS_CNT \
  && $SUDO make install \
  && cd ..

check_result "libxkbcommon is built" "Unable to build libxkbcommon"
