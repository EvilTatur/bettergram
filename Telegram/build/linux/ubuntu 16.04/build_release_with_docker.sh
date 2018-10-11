#!/bin/bash
#
# This script removes 'release' directory,
# creates docker image based on Ubuntu 16.04,
# builds the Bettergram application
# and copies the resulted archive to 'release' directory.
#
# This script should be run under usual (not root) user,
# but we ask sudo password at the beginning in order to work with docker.
#
# We should not use it at dockerfile.
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
  >&2 echo "ERROR: You should not run this script under root user or with sudo"
  exit 1
fi

echo ""
echo "Trying to remove 'release' directory, create docker image based on Ubuntu 16.04, build the Bettergram application and copy the resulted archive to 'release' directory..."
echo ""
echo "IMPORTANT: Please note that if the Bettergram GitHub repository is changed dramatically or if patches are changed then you should remove old docker image before start this script by using the following command: $ sudo docker image rm bettergram/ubuntu:16.04"
echo ""

USER_ID=$EUID

sudo bash -c "rm -rf release \
  && docker build -t bettergram/ubuntu:16.04 . \
  && docker run -v `realpath ../../../../../TelegramPrivate`:/opt/bettergram/TelegramPrivate -v release:/opt/bettergram/release -ti bettergram/ubuntu:16.04 udpate_and_rebuild_bettergram.sh"

check_result "The release build is created and you can find the results at the 'release' directory" "Unable to create release build"