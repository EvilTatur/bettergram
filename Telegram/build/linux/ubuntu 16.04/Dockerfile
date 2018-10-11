# This dockerfile is used to create Docker image based on Ubuntu 16.04
# to build Bettergram application for Ubuntu 16.04 and above.
#
# Use the following command in order to build the Docker image:
# $ docker build -t <your_user_name>/bettergram:ubuntu_16.04 .
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

COPY udpate_and_rebuild_bettergram.sh udpate_and_rebuild_bettergram.sh

COPY prepare_not_patched_libraries.sh prepare_not_patched_libraries.sh
RUN ./prepare_not_patched_libraries.sh

# At this point we clone Bettergram repository and use patches from it,
# so when those patches are changed at the git repository
# we should rebuild the docker image
COPY prepare_patched_libraries.sh prepare_patched_libraries.sh
RUN ./prepare_patched_libraries.sh

RUN cd bettergram \
    && Telegram/gyp/refresh.sh \
    && cd out/Release
#    && make $MAKE_THREADS_CNT

# The Bettergram is built for now.
# You can fetch new code and rebuild it again if you want it.
# Just call the following command:
# $ udpate_and_rebuild_bettergram.sh

ENTRYPOINT [ "/bin/bash" ]
