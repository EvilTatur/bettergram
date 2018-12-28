#!/bin/bash
#
# This script fetches ApiId and ApiHash values from TelegramPrivate/custom_api_id.h
# file and transmits them to refresh.sh file
#

set -e

pushd `dirname $0` > /dev/null
FULL_PATH=`pwd`
popd > /dev/null

TELEGRAM_PRIVATE_PATH=`realpath $FULL_PATH/../../../TelegramPrivate`
CUSTOM_API_ID_FILE_PATH=$TELEGRAM_PRIVATE_PATH/custom_api_id.h

if [ ! -f $CUSTOM_API_ID_FILE_PATH ]
then
  >&2 echo "ERROR: the file $CUSTOM_API_ID_FILE_PATH does not exist"
  exit 1
fi

YOUR_API_ID=`cat $CUSTOM_API_ID_FILE_PATH | grep "ApiId" | sed "s/.*=//g" | sed "s/;//g" | sed "s/[[:blank:]]//g"`

YOUR_API_HASH=`cat $CUSTOM_API_ID_FILE_PATH | grep "ApiHash" | sed "s/.*=//g" | sed "s/;//g" | sed "s/\"//g" | sed "s/[[:blank:]]//g"`

if [ -z "$YOUR_API_ID" ]
then
  >&2 echo "ERROR: the ApiId does not exist or empty at the file $CUSTOM_API_ID_FILE_PATH"
  exit 1
fi

if [ -z "$YOUR_API_HASH" ]
then
  >&2 echo "ERROR: the ApiHash does not exist or empty at the file $CUSTOM_API_ID_FILE_PATH"
  exit 1
fi

$FULL_PATH/refresh.sh --api-id $YOUR_API_ID --api-hash $YOUR_API_HASH
