#include "hw/arm/s32k3x8evb_can.h"
#include "hw/sysbus.h"
#include "qemu/units.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "hw/irq.h"

static const MemoryRegionOps s32k3x8evb_can_ops = {
    .read = s32k3x8evb_can_read,
    .write = s32k3x8evb_can_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

/* CAN Read Function */
uint64_t s32k3x8evb_can_read(void *opaque, hwaddr addr, unsigned size) {
    S32K3X8EVBCanState *s = opaque;

    switch (addr) {
        case 0x00:
            return s->mcr;
        case 0x04:
            return s->ctrl;
        case 0x08:  // Status Register
            qemu_log_mask(LOG_UNIMP, "🔍 Reading CAN STATUS: 0x%08X\n",(uint32_t) s->status);
            return s->status | CAN_STATUS_TX_READY;  // Ensure TX_READY is always set
        case 0x10:
            return s->txbuf;
        case 0x14:
            return s->rxbuf;
        case 0x18:
            return s->irq;
        default:
            qemu_log_mask(LOG_GUEST_ERROR, "Invalid CAN read at addr 0x%" PRIx64 "\n", addr);
            return 0;
    }
}


/* CAN Write Function */
void s32k3x8evb_can_write(void *opaque, hwaddr addr, uint64_t val, unsigned size) {
    S32K3X8EVBCanState *s = opaque;

    qemu_log_mask(LOG_UNIMP, "CAN WRITE: 0x%016" PRIX64 " to addr 0x%" PRIx64 "\n", val, addr);

    switch (addr) {
        case 0x00:  // MCR
            s->mcr = val;
            break;

        case 0x04:  // Control Register
            s->ctrl = val;
            break;

        case 0x08:  // Status Register (shouldn't be written to in real hardware)
            s->status = val;
            break;

        case 0x10:  // TX Buffer
            s->txbuf = val;

            // Ensure TX is marked READY
            s->status |= CAN_STATUS_TX_READY;
            qemu_log_mask(LOG_UNIMP, "✅ CAN TX_READY FLAG SET in QEMU: 0x%08X\n", (uint32_t)s->status);

            // Simulate Loopback Mode if Enabled
            if (s->ctrl & CAN_CTRL_LOOPBACK) {
                s->rxbuf = val;
                s->status |= CAN_STATUS_RX_READY;
                qemu_log_mask(LOG_UNIMP, "✅ Loopback Mode: RXBUF = 0x%08X\n", (uint32_t)s->rxbuf);
            }

            // Trigger Interrupt (
            qemu_set_irq(s->can_irq, 1);
            break;

        case 0x14:  // RX Buffer (read-only)
            s->rxbuf = val;
            break;

        default:
            qemu_log_mask(LOG_UNIMP, "Invalid CAN write at addr 0x%" PRIx64 "\n", addr);
            break;
    }
}


/* CAN Initialization */
void s32k3x8evb_can_realize(DeviceState *dev, Error **errp) {
    S32K3X8EVBCanState *s = S32K3X8EVB_CAN(dev);

    s->mcr = 0;
    s->ctrl = 0;
    s->status = 0;
    s->txbuf = 0;
    s->rxbuf = 0;
    s->irq = 0;

    /* Initialize MMIO Region */
    memory_region_init_io(&s->mem, OBJECT(s), &s32k3x8evb_can_ops, s, "s32k3x8evb_can", 0x1000);

    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mem);
    sysbus_mmio_map(SYS_BUS_DEVICE(s), 0, S32K3X8EVB_CAN0_BASE);
    sysbus_init_irq(SYS_BUS_DEVICE(s), &s->can_irq);
}


/* Device Class Initialization */
void s32k3x8evb_can_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = s32k3x8evb_can_realize;
}

/* Register CAN */
static const TypeInfo s32k3x8evb_can_class_info = {
    .name = TYPE_S32K3X8EVB_CAN,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K3X8EVBCanState),
    .class_init = s32k3x8evb_can_class_init,
};

static void s32k3x8evb_can_register_types(void) {
    type_register_static(&s32k3x8evb_can_class_info);
}

type_init(s32k3x8evb_can_register_types);