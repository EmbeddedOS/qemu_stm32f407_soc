# qemu_stm32f407_soc

Emulate your stm32f407 SoC with QEMU.

## Build QEMU

```bash
mkdir build
cd build
../qemu/configure --target-list=aarch64-linux-user,arm-linux-user,arm-softmmu
make -j4
```
