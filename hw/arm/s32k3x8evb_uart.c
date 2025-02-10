#include "hw/arm/s32k3x8evb_uart.h"
#include "hw/sysbus.h"
#include "qemu/units.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "hw/irq.h"

/* UART MMIO Operations */
static const MemoryRegionOps s32k3x8evb_uart_ops = {
    .read = s32k3x8evb_uart_read,
    .write = s32k3x8evb_uart_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

/* UART TX Interrupt */
static void uart_tx_interrupt(S32K3X8EVBUartState *s) {
    if (s->interrupt_enabled && (s->status & UART_STATUS_TX_COMPLETE)) {
        qemu_set_irq(s->irq, 1); // Assert IRQ
        qemu_log_mask(LOG_UNIMP, "UART interrupt triggered (status: 0x%x)\n", s->status);
    }
}

/* Read Function */
uint64_t s32k3x8evb_uart_read(void *opaque, hwaddr addr, unsigned size) {
    S32K3X8EVBUartState *s = opaque;

    switch (addr) {
        case 0x10: // BAUD RATE
            return s->baud_rate;
        case 0x18: // CONTROL
            return s->control;
        case 0x14: // STATUS
            qemu_log_mask(LOG_UNIMP, "UART STATUS READ at 0x%" PRIx64 ": 0x%x\n", addr, s->status);
            return s->status;  // ✅ Ensure it returns TX_COMPLETE if set
        case 0x1C: // DATA (Dummy read)
            return s->data;
        default:
            qemu_log_mask(LOG_GUEST_ERROR, "Invalid UART read at address 0x%" PRIx64 "\n", addr);
            return 0;
    }
}



/* Write Function */
void s32k3x8evb_uart_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    S32K3X8EVBUartState *s = opaque;
    
    qemu_log_mask(LOG_UNIMP, "UART WRITE: addr=0x%" PRIx64 ", val=0x%" PRIx64 "\n", addr, val);

    switch (addr) {
        case 0x10: // BAUD RATE
            s->baud_rate = val;
            break;

        case 0x18: // CONTROL
            s->control = val;
            s->interrupt_enabled = (val & UART_CTRL_INTERRUPT_ENABLE) != 0;
            break;

        case 0x1C: // DATA (Transmit)
            s->data = val;
            
            qemu_log_mask(LOG_UNIMP, "UART OUTPUT: %c\n", (char)val);

            if (s->output) {
                fputc((char)val, s->output);
                fflush(s->output);
            }

            // ✅ **Set TX_COMPLETE Flag**
            s->status |= UART_STATUS_TX_COMPLETE;
            uart_tx_interrupt(s);  // Trigger IRQ if enabled
            break;

        case 0x14: // STATUS (Clear TX_COMPLETE)
            qemu_log_mask(LOG_UNIMP, "Clearing TX_COMPLETE flag\n");
            s->status &= ~UART_STATUS_TX_COMPLETE; // ✅ **Clear TX_COMPLETE when written**
            break;

        default:
            qemu_log_mask(LOG_UNIMP, "Invalid UART write address: 0x%" PRIx64 "\n", addr);
            break;
    }
}



/* UART Initialization */
void s32k3x8evb_uart_realize(DeviceState *dev, Error **errp) {
    S32K3X8EVBUartState *s = S32K3X8EVB_UART(dev);
    s->baud_rate = 115200;
    s->control = 0;
    s->status = UART_STATUS_TX_COMPLETE;  // Ensure TX is ready initially
    s->data = 0;
    s->output = stdout;

    // Initialize MMIO region (Ensure size is correct!)
    memory_region_init_io(&s->mem, OBJECT(s), &s32k3x8evb_uart_ops, s,
                          "s32k3x8evb_uart", 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mem);
    sysbus_init_irq(SYS_BUS_DEVICE(s), &s->irq); 
    /* Ensure correct UART memory mapping */
    sysbus_mmio_map(SYS_BUS_DEVICE(s), 0, S32K3X8EVB_UART0_BASE);
}


/* Device Class Initialization */
 void s32k3x8evb_uart_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = s32k3x8evb_uart_realize;
}

/* Register Device Type */
static const TypeInfo s32k3x8evb_uart_class_info = {
    .name = TYPE_S32K3X8EVB_UART,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K3X8EVBUartState),
    .class_init = s32k3x8evb_uart_class_init,
};

/* Register the UART */
static void s32k3x8evb_uart_register_types(void) {
    type_register_static(&s32k3x8evb_uart_class_info);
}

type_init(s32k3x8evb_uart_register_types);
