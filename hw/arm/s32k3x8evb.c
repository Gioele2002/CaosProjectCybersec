#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/arm/arm.h"
#include "hw/arm/boot.h"
#include "hw/sysbus.h"
#include "hw/char/serial.h"
#include "hw/net/can.h"
#include "hw/boards.h"
#include "sysemu/sysemu.h"
#include "sysemu/qtest.h"
#include "exec/address-spaces.h"

/* Base addresses and sizes */
#define FLASH_BASE    0x00400000
#define FLASH_SIZE    0x00800000  // 8 MB

#define SRAM0_BASE    0x20400000
#define SRAM0_SIZE    0x00040000  // 256 KB

#define SRAM1_BASE    0x20440000
#define SRAM1_SIZE    0x00040000  // 256 KB

#define SRAM2_BASE    0x20480000
#define SRAM2_SIZE    0x00040000  // 256 KB

#define ITCM_BASE     0x00000000
#define ITCM_SIZE     0x00020000  // 128 KB

#define DTCM_BASE     0x20000000
#define DTCM_SIZE     0x00040000  // 256 KB

#define UART_BASE     0x4006A000
#define CAN_BASE      0x40024000

#define SYSCLK_FRQ    160000000ULL // System clock frequency (160 MHz)

/* Machine state structure */
typedef struct S32K3X8EVBState {
    MachineState parent;
    //Cpu state
    ARMv7MState armv7m;
    MemoryRegion flash;
    MemoryRegion itcm;
    MemoryRegion dtcm;
    MemoryRegion sram0;
    MemoryRegion sram1;
    MemoryRegion sram2;
    
    //Uart, Can
    DeviceState *uart;
    DeviceState *can;
    
    //Clock
    Clock *sysclk;
} S32K3X8EVBState;

#define TYPE_S32K3X8EVB "s32k3x8evb"
OBJECT_DECLARE_SIMPLE_TYPE(S32K3X8EVBState, S32K3X8EVB)


/* Initialization function for the board */
static void s32k3x8evb_init(MachineState *machine)
{
    S32K3X8EVBState *s = S32K3X8EVB(machine);
    Error *err = NULL;

    /* Initialize CPU */
    const char *cpu_model = machine->cpu_model ? machine->cpu_model : "cortex-m7";
    DeviceState *cpu = qemu_init_cpu(cpu_model);  // Initialize the Cortex-M7 CPU state
    qdev_set_parent_bus(cpu, sysbus_get_default());

    /* Initialize system clock */
    s->sysclk = clock_new(OBJECT(machine), "SYSCLK");
    clock_set_hz(s->sysclk, SYSCLK_FRQ);

    /* Initialize Flash memory */
    memory_region_init_ram(&s->flash, NULL, "s32k3x8evb.flash", FLASH_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), FLASH_BASE, &s->flash);

    /* Initialize ITCM */
    memory_region_init_ram(&s->itcm, NULL, "s32k3x8evb.itcm", ITCM_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), ITCM_BASE, &s->itcm);

    /* Initialize DTCM */
    memory_region_init_ram(&s->dtcm, NULL, "s32k3x8evb.dtcm", DTCM_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), DTCM_BASE, &s->dtcm);

    /* Initialize SRAM0 */
    memory_region_init_ram(&s->sram0, NULL, "s32k3x8evb.sram0", SRAM0_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), SRAM0_BASE, &s->sram0);

    /* Initialize SRAM1 */
    memory_region_init_ram(&s->sram1, NULL, "s32k3x8evb.sram1", SRAM1_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), SRAM1_BASE, &s->sram1);

    /* Initialize SRAM2 */
    memory_region_init_ram(&s->sram2, NULL, "s32k3x8evb.sram2", SRAM2_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), SRAM2_BASE, &s->sram2);

    /* Initialize UART */
    s->uart = sysbus_create_simple("pl011", UART_BASE, NULL);

    /* Initialize CAN */
    s->can = sysbus_create_simple("can_sja1000", CAN_BASE, NULL);

    /* Load firmware into Flash */
    armv7m_load_kernel(ARM_CPU(cpu), machine->kernel_filename, 0, FLASH_SIZE);
}


/* Machine registration function */
static void s32k3x8evb_machine_init(MachineClass *mc)
{
    mc->desc = "NXP S32K348 EVB Board";
    mc->init = s32k3x8evb_init;
    mc->default_cpu_type = "cortex-m7";
    mc->default_ram_size = SRAM0_SIZE + SRAM1_SIZE + SRAM2_SIZE;  // Total SRAM size
}

DEFINE_MACHINE("s32k3x8evb", s32k3x8evb_machine_init)
