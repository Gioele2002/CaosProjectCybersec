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
        case 0x00: return s->mcr;
        case 0x04: return s->ctrl;
        case 0x08: return s->status;
        case 0x10: return s->txbuf;
        case 0x14: return s->rxbuf;
        case 0x18: return s->irq;
        default:
            qemu_log_mask(LOG_GUEST_ERROR, "Invalid CAN read at 0x%" PRIx64 "\n", addr);
            return 0;
    }
}

/* CAN Write Function */
void s32k3x8evb_can_write(void *opaque, hwaddr addr, uint64_t val, unsigned size) {
    S32K3X8EVBCanState *s = opaque;
    qemu_log_mask(LOG_UNIMP, "CAN WRITE: addr=0x%" PRIx64 ", val=0x%" PRIx64 "\n", addr, val);

    switch (addr) {
        case 0x00:
            s->mcr = val;
            break;
        case 0x04:
            s->ctrl = val;
            break;
        case 0x08:
            s->status = val;
            break;
        case 0x10:
            s->txbuf = val;
            if (s->ctrl & CAN_CTRL_LOOPBACK) {
                s->rxbuf = val;  // Loopback mode: Directly receive the message
                s->status |= CAN_STATUS_RX_READY;
            }
            s->status |= CAN_STATUS_TX_READY;
            qemu_set_irq(s->can_irq, 1); // Trigger IRQ
            break;
        case 0x14:
            s->rxbuf = val;
            break;
        default:
            qemu_log_mask(LOG_UNIMP, "Invalid CAN write at 0x%" PRIx64 "\n", addr);
            break;
    }
}

/* CAN Initialization */
void s32k3x8evb_can_realize(DeviceState *dev, Error **errp) {
    S32K3X8EVBCanState *s = S32K3X8EVB_CAN(dev);
    s->mcr = 0;
    s->ctrl = 0;
    s->status = CAN_STATUS_TX_READY;
    s->txbuf = 0;
    s->rxbuf = 0;
    s->irq = 0;

    memory_region_init_io(&s->mem, OBJECT(s), &s32k3x8evb_can_ops, s, "s32k3x8evb_can", 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mem);
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
