# This dockerfile is used to create Docker image based on Ubuntu 16.04
# to build Bettergram application for Ubuntu 16.04 and above.
#
# Use the following commands in order to build and run the Docker image:
# $ docker build -t bettergram/ubuntu:16.04 .
# $ docker run -v `realpath ../../../../../TelegramPrivate`:/opt/bettergram/TelegramPrivate -v release:/opt/bettergram/release -ti bettergram/ubuntu:16.04
# $ ./update_and_rebuild_bettergram.sh
#
# Maintainer: Ildar Gilmanov <dev@ddwarf.org>
# See also https://bettergram.io
#

FROM ubuntu:16.04

LABEL maintainer="Ildar Gilmanov <dev@ddwarf.org>" \
      description="Creates a Docker image based on Ubuntu 16.04 for building Bettergram application. See https://bettergram.io"

WORKDIR /opt/bettergram

COPY apt_get_install.sh apt_get_install.sh
RUN ./apt_get_install.sh

ENV MAKE_THREADS_CNT=-j4

COPY prepare_not_patched_libraries.sh prepare_not_patched_libraries.sh
RUN ./prepare_not_patched_libraries.sh

# At this point we clone Bettergram repository and use patches from it,
# so when those patches are changed at the git repository
# we should rebuild the docker image
COPY prepare_patched_libraries.sh prepare_patched_libraries.sh
RUN ./prepare_patched_libraries.sh

COPY update_and_rebuild_bettergram.sh update_and_rebuild_bettergram.sh

# You can fetch new code and rebuild it with the following command:
# $ update_and_rebuild_bettergram.sh

ENTRYPOINT [ "/bin/bash" ]
