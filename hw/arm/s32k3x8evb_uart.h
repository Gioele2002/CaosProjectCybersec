#ifndef HW_ARM_S32K3X8EVB_UART_H
#define HW_ARM_S32K3X8EVB_UART_H

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qom/object.h"

#define S32K3X8EVB_UART0_BASE    0x40328000  // Define your custom UART base address

// Define the custom UART registers (you may need to add more registers)
#define S32K3X8EVB_UART_BAUD    (S32K3X8EVB_UART0_BASE + 0x10)
#define S32K3X8EVB_UART_CTRL    (S32K3X8EVB_UART0_BASE + 0x18)
#define S32K3X8EVB_UART_DATA    (S32K3X8EVB_UART0_BASE + 0x1C)
#define S32K3X8EVB_UART_STAT    (S32K3X8EVB_UART0_BASE + 0x14)
#define UART_STATUS_TX_COMPLETE (1 << 0) // Example: TX complete flag
#define UART_CTRL_INTERRUPT_ENABLE (1 << 1) // Example: Interrupt enable bit


// Define the UART device structure
typedef struct S32K3X8EVBUartState {
    SysBusDevice parent_obj;
    MemoryRegion mem;
    uint32_t baud_rate;
    uint32_t control;
    uint32_t status;
    uint32_t data;
    FILE *output;
    qemu_irq irq; // Add interrupt line
    bool interrupt_enabled;  
} S32K3X8EVBUartState;

#define TYPE_S32K3X8EVB_UART "s32k3x8evb_uart"
OBJECT_DECLARE_TYPE(S32K3X8EVBUartState, s32k3x8evb_uart, S32K3X8EVB_UART)

// Function prototypes
uint64_t s32k3x8evb_uart_read(void *opaque, hwaddr addr, unsigned size);
void s32k3x8evb_uart_write(void *opaque, hwaddr addr, uint64_t val, unsigned size); 
void s32k3x8evb_uart_class_init(ObjectClass *klass, void *data);
void s32k3x8evb_uart_realize(DeviceState *dev, Error **errp);

#endif 