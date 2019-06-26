# Development stopped

# [Bettergram][bettergram] – Official Messenger

This is the complete source code and the build instructions for the official desktop client for the [Bettergram][bettergram] messenger, based on the [Telegram API][telegram_api] and the [MTProto][telegram_proto] secure protocol.

<a href="http://www.youtube.com/watch?feature=player_embedded&v=olGflYVZPkI
" target="_blank"><img src="http://img.youtube.com/vi/olGflYVZPkI/0.jpg" 
alt="Bettergram Explained In 60 Seconds" width="240" height="180" border="10" /></a>

The source code is published under GPLv3 with OpenSSL exception, the license is available [here][license].

## Supported systems

* Windows XP - Windows 10 (**not** RT)
* Mac OS X 10.8 - Mac OS X 10.11
* Ubuntu 12.04 - Ubuntu 18.10
* Fedora 22 - Fedora 29
* [Snappy](https://snapcraft.io/telegram-desktop)

## Third-party

* Qt 5.3.2 and 5.6.2, slightly patched ([LGPL](http://doc.qt.io/qt-5/lgpl.html))
* OpenSSL 1.0.1g ([OpenSSL License](https://www.openssl.org/source/license.html))
* zlib 1.2.8 ([zlib License](http://www.zlib.net/zlib_license.html))
* libexif 0.6.20 ([LGPL](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html))
* LZMA SDK 9.20 ([public domain](http://www.7-zip.org/sdk.html))
* liblzma ([public domain](http://tukaani.org/xz/))
* Google Breakpad ([License](https://chromium.googlesource.com/breakpad/breakpad/+/master/LICENSE))
* Google Crashpad ([Apache License 2.0](https://chromium.googlesource.com/crashpad/crashpad/+/master/LICENSE))
* GYP ([BSD License](https://github.com/bnoordhuis/gyp/blob/master/LICENSE))
* Ninja ([Apache License 2.0](https://github.com/ninja-build/ninja/blob/master/COPYING))
* OpenAL Soft ([LGPL](https://github.com/kcat/openal-soft/blob/master/COPYING))
* Opus codec ([BSD License](http://www.opus-codec.org/license/))
* FFmpeg ([LGPL](https://www.ffmpeg.org/legal.html))
* Guideline Support Library ([MIT License](https://github.com/Microsoft/GSL/blob/master/LICENSE))
* Mapbox Variant ([BSD License](https://github.com/mapbox/variant/blob/master/LICENSE))
* Range-v3 ([Boost License](https://github.com/ericniebler/range-v3/blob/master/LICENSE.txt))
* Open Sans font ([Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0.html))
* Emoji alpha codes ([MIT License](https://github.com/emojione/emojione/blob/master/extras/alpha-codes/LICENSE.md))
* Catch test framework ([Boost License](https://github.com/philsquared/Catch/blob/master/LICENSE.txt))
* xxHash ([BSD License](https://github.com/Cyan4973/xxHash/blob/dev/LICENSE))

## Build instructions

* [Build instructions][building]
* [Visual Studio 2017][msvc]
* [Xcode 10][xcode]
* [GYP/CMake on GNU/Linux][cmake]

[//]: # (LINKS)
[bettergram]: https://bettergram.io
[telegram_api]: https://core.telegram.org
[telegram_proto]: https://core.telegram.org/mtproto
[license]: LICENSE
[building]: docs/building.md
[msvc]: docs/building-msvc.md
[xcode]: docs/building-xcode.md
[xcode_old]: docs/building-xcode-old.md
[cmake]: docs/building-cmake.md
