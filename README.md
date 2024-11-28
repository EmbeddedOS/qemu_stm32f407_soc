# qemu_stm32f407_soc

Emulate your stm32f407 SoC with QEMU.

## Build QEMU

```bash
mkdir build
cd build
../qemu/configure --target-list=aarch64-linux-user,arm-linux-user,arm-softmmu
make -j4
./build/qemu-system-arm -machine stm32f407g_disc -kernel boot.elf
```

## Make a patch file

```bash
cd qemu
git add -N hw/arm/stm32f407g_disc.c
git diff > ../qemu.patch
```
