# qemu_stm32f407_soc

Emulate your stm32f407 SoC with QEMU.

## Build QEMU

Install QEMU:

```bash
./install.sh
```

Build binaries:

```bash
mkdir build
cd build
../qemu/configure --target-list=aarch64-linux-user,arm-linux-user,arm-softmmu
make -j20
```

## Run application

```bash
./build/qemu-system-arm -machine stm32f407g_disc -kernel kernel.elf -serial stdio
```

## Make a patch file

```bash
cd qemu
git add -N hw/arm/stm32f407g_disc.c
git diff > ../qemu.patch
```
