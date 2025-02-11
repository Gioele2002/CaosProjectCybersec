#ifndef HW_ARM_S32K3X8EVB_CAN_H
#define HW_ARM_S32K3X8EVB_CAN_H

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qom/object.h"

#define S32K3X8EVB_CAN0_BASE   0x40304000  // Base address for CAN0

// CAN Register Offsets
#define S32K3X8EVB_CAN_MCR     (S32K3X8EVB_CAN0_BASE + 0x00)  // Module Configuration Register
#define S32K3X8EVB_CAN_CTRL    (S32K3X8EVB_CAN0_BASE + 0x04)  // Control Register
#define S32K3X8EVB_CAN_STATUS  (S32K3X8EVB_CAN0_BASE + 0x08)  // Status Register
#define S32K3X8EVB_CAN_TXBUF   (S32K3X8EVB_CAN0_BASE + 0x10)  // Transmit Buffer
#define S32K3X8EVB_CAN_RXBUF   (S32K3X8EVB_CAN0_BASE + 0x14)  // Receive Buffer
#define S32K3X8EVB_CAN_IRQ     (S32K3X8EVB_CAN0_BASE + 0x18)  // Interrupt Register

// Bit Masks
#define CAN_STATUS_TX_READY   (1 << 0) // TX Ready flag
#define CAN_STATUS_RX_READY   (1 << 1) // RX Ready flag
#define CAN_CTRL_LOOPBACK     (1 << 2) // Loopback mode

typedef struct S32K3X8EVBCanState {
    SysBusDevice parent_obj;
    MemoryRegion mem;
    uint32_t mcr;      // Module Configuration Register
    uint32_t ctrl;     // Control Register
    uint32_t status;   // Status Register
    uint32_t txbuf;    // Transmit Buffer
    uint32_t rxbuf;    // Receive Buffer
    uint32_t irq;      // Interrupt Register
    qemu_irq can_irq;  // CAN IRQ line
} S32K3X8EVBCanState;

#define TYPE_S32K3X8EVB_CAN "s32k3x8evb_can"
OBJECT_DECLARE_TYPE(S32K3X8EVBCanState, s32k3x8evb_can, S32K3X8EVB_CAN)

// Function prototypes
uint64_t s32k3x8evb_can_read(void *opaque, hwaddr addr, unsigned size);
void s32k3x8evb_can_write(void *opaque, hwaddr addr, uint64_t val, unsigned size);
void s32k3x8evb_can_realize(DeviceState *dev, Error **errp);
void s32k3x8evb_can_class_init(ObjectClass *klass, void *data);

#endif /* HW_ARM_S32K3X8EVB_CAN_H */