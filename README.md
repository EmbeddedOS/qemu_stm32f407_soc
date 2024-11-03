# qemu_stm32f407_soc

Emulate your stm32f407 SoC with QEMU.

## Build QEMU

Install dependencies:

```bash
sudo apt-get install python3 python3-pip -y
sudo apt-get install git libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev ninja-build -y
sudo apt-get install git-email -y
sudo apt-get install libaio-dev libbluetooth-dev libcapstone-dev libbrlapi-dev libbz2-dev -y
sudo apt-get install libcap-ng-dev libcurl4-gnutls-dev libgtk-3-dev -y
sudo apt-get install libibverbs-dev libjpeg8-dev libncurses5-dev libnuma-dev -y
sudo apt-get install librbd-dev librdmacm-dev -y
sudo apt-get install libsasl2-dev libsdl2-dev libseccomp-dev libsnappy-dev libssh-dev -y
sudo apt-get install libvde-dev libvdeplug-dev libvte-2.91-dev libxen-dev liblzo2-dev -y
sudo apt-get install valgrind xfslibs-dev -y
sudo apt-get install libnfs-dev libiscsi-dev -y

python3 -m pip install tomli
```

```bash
git submodule update --init --recursive

cd build
../qemu/configure --target-list=aarch64-linux-user,arm-linux-user,arm-softmmu
```
