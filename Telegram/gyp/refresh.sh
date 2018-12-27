#!/usr/bin/env bash
set -e

pushd `dirname $0` > /dev/null
FullScriptPath=`pwd`
popd > /dev/null

python $FullScriptPath/generate.py $1 $2 $3 $4 $5 $6

BUILD_TARGET_GENERAL_DESCRIPTION="You can change the build target at the Telegram/build/target file."

MAC_STORE_DESCRIPTION="It will make .app directory ready for Apple App Store.
You also need the following files:
* TelegramPrivate/mac_development_team"

MAC_DMG_DESCRIPTION="It will make and sign .app directory and .dmg file.
You also need the following files:
* TelegramPrivate/mac_certificate_identity
* TelegramPrivate/mac_development_team
* TelegramPrivate/BettergramDmg.json
* TelegramPrivate/Bettergram.icns"

CHANGE_TO_MAC_STORE_DESCRIPTION="
If you want to deliver the application through the Apple App Store
then just write the following content to the Telegram/build/target file: macstore
$MAC_STORE_DESCRIPTION"

CHANGE_TO_MAC_DMG_DESCRIPTION="
If you want to deliver the application outside the Apple App Store
then just write the following content to the Telegram/build/target file: macdmg
$MAC_DMG_DESCRIPTION"

CHANGE_TO_EMTPY_DESCRIPTION="
Or If you want just to build the application
then clear the content of the Telegram/build/target file.

To get more information please check the docs/building-xcode.md file."

  if [ "$MySystem" == "Darwin" ]; then
    if [ -n "$BuildTarget" ]; then
      echo "Build target:" $BuildTarget
    fi

    if [ "$BuildTarget" == "macstore" ]; then
      echo ""
      echo "$MAC_STORE_DESCRIPTION"
      echo ""
      echo "$BUILD_TARGET_GENERAL_DESCRIPTION"
      echo "$CHANGE_TO_MAC_DMG_DESCRIPTION"
      echo "$CHANGE_TO_EMTPY_DESCRIPTION"
    else if [ "$BuildTarget" == "macdmg" ]; then
      echo ""
      echo "$MAC_DMG_DESCRIPTION"
      echo ""
      echo "$BUILD_TARGET_GENERAL_DESCRIPTION"
      echo "$CHANGE_TO_MAC_STORE_DESCRIPTION"
      echo "$CHANGE_TO_EMTPY_DESCRIPTION"
    else
      echo ""
      echo "At macOS you can build the application for Apple App Store or deliver it though your site."
      echo "$BUILD_TARGET_GENERAL_DESCRIPTION"
      echo "$CHANGE_TO_MAC_STORE_DESCRIPTION"
      echo "$CHANGE_TO_MAC_DMG_DESCRIPTION"
      echo "$CHANGE_TO_EMTPY_DESCRIPTION"
    fi
    fi
  fi


exit
