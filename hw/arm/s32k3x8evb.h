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
#include "qemu/units.h"

/* Base addresses and sizes */
#define FLASH_BASE    0x00400000
#define FLASH_SIZE    0x00800000 // 8 MB

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

/* Machine state and class structure */
typedef struct S32K3X8EVBMachineClass {
    MachineClass parent_class;
} S32K3X8EVBMachineClass;

typedef struct S32K3X8EVBMachineState {
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
} S32K3X8EVBMachineState;
