#ifndef S32K3X8EVB_H
#define S32K3X8EVB_H

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/arm/armv7m.h"
#include "hw/arm/boot.h"
#include "hw/sysbus.h"
#include "hw/boards.h"
#include "sysemu/sysemu.h"
#include "sysemu/qtest.h"
#include "exec/address-spaces.h"
#include "qemu/units.h"
#include "qemu/cutils.h"
#include "qemu/error-report.h"
#include "hw/qdev-clock.h"
#include "qapi/qmp/qlist.h"
#include "qom/object.h"
#include "hw/qdev-properties.h"

/* === Memory Base Addresses === */
#define FLASH_BASE       0x00400000
#define FLASH_SIZE       0x00800000  // 8 MB

#define SRAM0_BASE       0x20400000
#define SRAM0_SIZE       0x00040000  // 256 KB

#define SRAM1_BASE       0x20440000
#define SRAM1_SIZE       0x00040000  // 256 KB

#define SRAM2_BASE       0x20480000
#define SRAM2_SIZE       0x00040000  // 256 KB

#define ITCM_BASE        0x00000000
#define ITCM_SIZE        0x00010000  // 64 KB

#define DTCM_BASE        0x20000000
#define DTCM_SIZE        0x00020000  // 128 KB


/* === IRQ Numbers === */
#define IRQ_UART0        32
#define IRQ_CAN0         64

/* === CPU & Clock === */
#define DEFAULT_CPU      "cortex-m7"
#define SYSCLK_FRQ       160000000ULL  // System clock frequency (160 MHz)


/* === Machine State Structure === */
typedef struct S32K3X8EVBMachineClass {
    MachineClass parent_class;
} S32K3X8EVBMachineClass;

typedef struct S32K3X8EVBMachineState {
    MachineState parent;
    
    /* CPU */
    ARMv7MState armv7m;

    /* Memory Regions */
    MemoryRegion flash;
    MemoryRegion itcm;
    MemoryRegion dtcm;
    MemoryRegion sram0;
    MemoryRegion sram1;
    MemoryRegion sram2;

    /* Peripherals */
    DeviceState *uart;
    DeviceState *can;
    qemu_irq irq;

    /* Clock */
    Clock *sysclk;
} S32K3X8EVBMachineState;

#endif /* S32K3X8EVB_H */
