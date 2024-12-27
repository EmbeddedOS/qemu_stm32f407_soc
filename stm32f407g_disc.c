#include "qemu/osdep.h"

#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/arm/boot.h"
#include "hw/arm/armv7m.h"
#include "hw/qdev-clock.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "qom/object.h"
#include "hw/char/stm32f2xx_usart.h"
#include "hw/misc/unimp.h"
#include <string.h>

/* Helper defines ------------------------------------------------------------*/
#define __FILENAME__ (strrchr(__FILE__, '/') ? \
    strrchr(__FILE__, '/') + 1 : __FILE__)

#define __LOG(fmt, ...) \
    printf("[%s:%d][%s()]: " fmt "\n", \
           __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

/* Public defines ------------------------------------------------------------*/
#define TYPE_STM32F407_SOC "stm32f407-soc"
#define MACHINE_NAME "stm32f407g_disc-machine"
#define BOARD_DESCRIPTION "STM32F407G-DISCOVERY 1 Board (Cortex-M4)"
#define SYSCLK_FRQ 32000000ULL      /* 32MHz. */
#define FLASH_BASE_ADDRESS 0x08000000
#define FLASH_SIZE (1024 * 1024)    /* 1MB. */
#define SRAM_BASE_ADDRESS 0x20000000
#define SRAM_SIZE (128 * 1024)
#define CCM_BASE_ADDRESS 0x10000000
#define CCM_SIZE (64 * 1024)

#define NUM_USARTS 7

/**
 * @brief   - This macro declares the device type and define a inline function
 *            STM32F407_SOC() to upcast object from parent type pointer.
 */
OBJECT_DECLARE_SIMPLE_TYPE(STM32F407State, STM32F407_SOC)

/* Main SoC Object structures ------------------------------------------------*/
struct STM32F407State
{
    SysBusDevice parent_obj;
    ARMv7MState armv7m;
    STM32F2XXUsartState usarts[NUM_USARTS];
    Clock *sysclk;
    Clock *refclk;

    MemoryRegion ccm;
    MemoryRegion sram;
    MemoryRegion flash;
    MemoryRegion flash_alias;
};


static const uint32_t usart_addr[] = { 0x40011000, 0x40004400, 0x40004800,
                                       0x40004C00, 0x40005000, 0x40011400,
                                       0x40007800, 0x40007C00 };

/* Private functions & methods -----------------------------------------------*/

/* Realizers -----------------------------------------------------------------*/
static void stm32f407_soc_realize(DeviceState *dev_soc, Error **errp)
{
    STM32F407State *s = STM32F407_SOC(dev_soc);
    MemoryRegion *system_memory = get_system_memory();
    DeviceState *dev, *armv7m;
    SysBusDevice *busdev;
    Error *err = NULL;

    clock_set_mul_div(s->refclk, 8, 1);
    clock_set_source(s->refclk, s->sysclk);

    memory_region_init_rom(&s->flash, OBJECT(dev_soc), "STM32F407.flash",
                           FLASH_SIZE, &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }
    memory_region_init_alias(&s->flash_alias, OBJECT(dev_soc),
                             "STM32F407.flash.alias", &s->flash, 0,
                             FLASH_SIZE);

    memory_region_add_subregion(system_memory, FLASH_BASE_ADDRESS, &s->flash);
    memory_region_add_subregion(system_memory, 0, &s->flash_alias);

    memory_region_init_ram(&s->sram, NULL, "STM32F407.sram", SRAM_SIZE,
                           &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }
    memory_region_add_subregion(system_memory, SRAM_BASE_ADDRESS, &s->sram);

    memory_region_init_ram(&s->ccm, NULL, "STM32F407.ccm", CCM_SIZE,
                           &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }
    memory_region_add_subregion(system_memory, CCM_BASE_ADDRESS, &s->ccm);

    armv7m = DEVICE(&s->armv7m);
    qdev_prop_set_uint32(armv7m, "num-irq", 96);
    qdev_prop_set_string(armv7m, "cpu-type", ARM_CPU_TYPE_NAME("cortex-m4"));
    qdev_prop_set_bit(armv7m, "enable-bitband", true);
    qdev_connect_clock_in(armv7m, "cpuclk", s->sysclk);
    qdev_connect_clock_in(armv7m, "refclk", s->refclk);

    object_property_set_link(OBJECT(&s->armv7m), "memory",
                             OBJECT(system_memory), &error_abort);

    if (!sysbus_realize(SYS_BUS_DEVICE(&s->armv7m), errp)) {
        return;
    }

    for (int i = 0; i < NUM_USARTS; i++) {
        dev = DEVICE(&(s->usarts[i]));
        qdev_prop_set_chr(dev, "chardev", serial_hd(i));
        if (!sysbus_realize(SYS_BUS_DEVICE(&s->usarts[i]), errp)) {
            return;
        }
        busdev = SYS_BUS_DEVICE(dev);
        sysbus_mmio_map(busdev, 0, usart_addr[i]);
    }
}

/* Constructors --------------------------------------------------------------*/
static void stm32f407_soc_initfn(Object *obj)
{
    __LOG("Invoked!");
    STM32F407State *s = STM32F407_SOC(obj);

    /* 1. Add object properties. */
    object_initialize_child(obj, "armv7m", &s->armv7m, TYPE_ARMV7M);

    for (int i = 0; i < NUM_USARTS; i++) {
        object_initialize_child(obj, "usart[*]", &s->usarts[i],
                                TYPE_STM32F2XX_USART);
    }

    s->sysclk = qdev_init_clock_in(DEVICE(s), "sysclk", NULL, NULL, 0);
    s->refclk = qdev_init_clock_in(DEVICE(s), "refclk", NULL, NULL, 0);
};

static void stm32f407_soc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    /* Override parent's realize method. */
    dc->realize = stm32f407_soc_realize;
};

static void stm32f407g_disc_init(MachineState *machine)
{
    DeviceState *dev = NULL;
    Clock *sysclk = NULL;

    /* 1. Create a source clock for sysclk. */
    sysclk = clock_new(OBJECT(machine), "SYSCLK");
    clock_set_hz(sysclk, SYSCLK_FRQ);

    /* 2. Create SoC. */
    dev = qdev_new(TYPE_STM32F407_SOC);

    /* 3. Connect SoC to clock source. Make sure you init `sysclk` when creating
     * the device. This step also HAVE TO be done before realize the device. */
    qdev_connect_clock_in(dev, "sysclk", sysclk);

    /* 4. Realize the SoC. */
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);

    /* 5. Load kernel to flash memory. */
    __LOG("Loading kernel: %s", machine->kernel_filename);
    armv7m_load_kernel(ARM_CPU(first_cpu),
                       machine->kernel_filename,
                       0, FLASH_SIZE);
}

static void stm32f407g_disc_class_init(ObjectClass *klass, void *data)
{
    MachineClass *mc = MACHINE_CLASS(klass);
    mc->desc = BOARD_DESCRIPTION;

    static const char *const valid_cpu_types[] = {
        ARM_CPU_TYPE_NAME("cortex-m4"),
        NULL};

    mc->init = stm32f407g_disc_init;
    mc->valid_cpu_types = valid_cpu_types;
}

/* Module Initializer --------------------------------------------------------*/
static const TypeInfo stm32f407_soc_info = {
    .name = TYPE_STM32F407_SOC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(STM32F407State),
    .instance_init = stm32f407_soc_initfn,
    .class_init = stm32f407_soc_class_init,
};

static const TypeInfo stm32f407g_disc = {
    .name = MACHINE_NAME,
    .parent = TYPE_MACHINE,
    .class_init = stm32f407g_disc_class_init};

static void stm32f407g_disc_types(void)
{
    type_register_static(&stm32f407_soc_info);
    type_register_static(&stm32f407g_disc);
}

type_init(stm32f407g_disc_types)
