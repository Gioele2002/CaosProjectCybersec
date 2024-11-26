#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/arm/armv7m.h"
//#include "hw/arm/arm.h"
#include "hw/arm/boot.h"
#include "hw/sysbus.h"
//#include "hw/char/serial.h"
//#include "hw/net/can.h"
#include "hw/boards.h"
#include "sysemu/sysemu.h"
#include "sysemu/qtest.h"
#include "exec/address-spaces.h"
#include "qemu/units.h"
#include "qemu/cutils.h"
#include "qemu/error-report.h"
#include "hw/boards.h"
#include "hw/qdev-clock.h"
#include "qapi/qmp/qlist.h"
#include "qom/object.h"
#include "hw/qdev-properties.h"

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
    
    DeviceState *armv7m;

     /* Initialize CPU (Cortex-M7) */
     //This function initializes the CPU (Cortex-M7) as a child object of the board (s), which is represented by the structure S32K3X8EVBState.
    object_initialize_child(OBJECT(s), "armv7m", &s->armv7m, TYPE_ARMV7M);
    //This line casts the armv7m object (which was initialized as part of S32K3X8EVBState) to a DeviceState type, which is a common type for all devices in QEMU.
    armv7m = DEVICE(&s->armv7m);
    //This function connects the system clock (s->sysclk) to the CPU (armv7m), which allows the CPU to run at the specified frequency.
    qdev_connect_clock_in(armv7m, "cpuclk", s->sysclk);
    // This function sets a property of the CPU device (armv7m). Specifically, it sets the cpu-type property to the CPU type defined in the machine.
    qdev_prop_set_string(armv7m, "cpu-type", machine->cpu_type);
    //This function sets a property of the CPU to enable a feature (in this case, the ARM bit-banding feature).
    qdev_prop_set_bit(armv7m, "enable-bitband", true);
    //This sets the memory property of the CPU to point to the system memory, meaning that the CPU will use the system memory region to fetch instructions and data.
    object_property_set_link(OBJECT(&s->armv7m), "memory",
                             OBJECT(get_system_memory()), &err);
    //This function finalizes the initialization of the CPU and ensures it is properly connected to the system bus so that it can interact with other devices.
    sysbus_realize(SYS_BUS_DEVICE(&s->armv7m), &err);
    /*This function assigns the parent bus for the device (armv7m) to the system bus. The parent bus connects the CPU to the rest of the system, allowing it to interact with other devices and memory regions.*/
    qdev_set_parent_bus(DEVICE(&s->armv7m), sysbus_get_default(), &err);

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
   // s->uart = sysbus_create_simple("pl011", UART_BASE, NULL);

    /* Initialize CAN */
    //s->can = sysbus_create_simple("can_sja1000", CAN_BASE, NULL);

    /* Load firmware into Flash */
    armv7m_load_kernel(ARM_CPU(armv7m), machine->kernel_filename, 0, FLASH_SIZE);
}


/* Machine registration function */
static void s32k3x8evb_machine_init(MachineClass *mc)
{
    mc->desc = "NXP S32K348 EVB Board";
    mc->init = s32k3x8evb_init;
    mc->default_cpu_type = "cortex-m7";
    mc->default_ram_size = SRAM0_SIZE + SRAM1_SIZE + SRAM2_SIZE;  // Total SRAM size
}

//Registration of the board 
DEFINE_MACHINE("s32k3x8evb", s32k3x8evb_machine_init)
