#!/bin/bash

sudo apt-get install python3 python3-pip git libglib2.0-dev libfdt-dev \
    libpixman-1-dev zlib1g-dev ninja-build git-email libaio-dev \
    libbluetooth-devlibcapstone-dev libbrlapi-dev libbz2-dev libcap-ng-dev \
    libcurl4-gnutls-dev libgtk-3-dev libibverbs-dev libjpeg8-dev \
    libncurses5-dev libnuma-dev librbd-dev librdmacm-dev libsasl2-dev \
    libsdl2-dev libseccomp-dev libsnappy-dev libssh-dev libvde-dev \
    libvdeplug-dev libvte-2.91-dev libxen-dev liblzo2-dev valgrind xfslibs-dev \
    libnfs-dev libiscsi-dev -y

python3 -m pip install tomli

git submodule update --init --recursive
