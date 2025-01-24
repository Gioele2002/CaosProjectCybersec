#include "hw/arm/s32k3x8evb_uart.h"
#include "hw/sysbus.h"
#include "qemu/units.h"
#include "qemu/error-report.h"
#include "qemu/log.h" 
#include "hw/irq.h"

// Define the read and write operations for MemoryRegionOps
static const MemoryRegionOps s32k3x8evb_uart_ops = {
    .read = s32k3x8evb_uart_read,
    .write = s32k3x8evb_uart_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

// UART interrupt handler for TX
static void uart_tx_interrupt(S32K3X8EVBUartState *s) {
    if (s->interrupt_enabled && (s->status & UART_STATUS_TX_COMPLETE)) {
        qemu_irq_pulse(s->irq); // Trigger interrupt
        qemu_log_mask(LOG_UNIMP, "UART interrupt triggered (status: 0x%x)\n", s->status);
    }
}



// Read and write functions for UART
 uint64_t s32k3x8evb_uart_read(void *opaque, hwaddr addr, unsigned size)
{
    S32K3X8EVBUartState *s = opaque;

    switch (addr) {
        case S32K3X8EVB_UART_BAUD:
            return s->baud_rate;
        case S32K3X8EVB_UART_CTRL:
            return s->control;
        case S32K3X8EVB_UART_STAT:
            return s->status;
        case S32K3X8EVB_UART_DATA:
            return s->data;
        default:
            return 0;
    }
}

 void s32k3x8evb_uart_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    S32K3X8EVBUartState *s = opaque;
      qemu_log_mask(LOG_UNIMP, "UART write: addr=0x%" PRIx64 ", val=0x%" PRIx64 "\n", addr, val);

    switch (addr) {
        case S32K3X8EVB_UART_BAUD:
            s->baud_rate = val;
            break;
        case S32K3X8EVB_UART_CTRL:
            s->control = val;
            s->interrupt_enabled = (val & UART_CTRL_INTERRUPT_ENABLE) != 0;
            break;
        case S32K3X8EVB_UART_STAT:
            s->status = val;
            break;
        case S32K3X8EVB_UART_DATA:
            s->data = val;
             if (s->output) {
                fputc((char)val, s->output); // Send character to the output
                fflush(s->output);          // Ensure immediate output
            }
            s->status |= UART_STATUS_TX_COMPLETE; // Set TX complete flag
            uart_tx_interrupt(s); // Trigger the interrupt
           
            break;
        default:
         qemu_log_mask(LOG_UNIMP, "Invalid UART write address: 0x%" PRIx64 "\n", addr);
            break;
    }
}

// Initialize the custom UART device
 void s32k3x8evb_uart_realize(DeviceState *dev, Error **errp)
{
    S32K3X8EVBUartState *s = S32K3X8EVB_UART(dev);
    s->baud_rate = 115200; // Default baud rate
    s->control = 0;
    s->status = 0;
    s->data = 0;
    s->output = stdout; // Set output to stdout

    // Initialize and add the MMIO region
    memory_region_init_io(&s->mem, NULL, &s32k3x8evb_uart_ops, (SysBusDevice *)s, "s32k3x8evb_uart", sizeof(S32K3X8EVBUartState)); 
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mem); 

    sysbus_init_irq(SYS_BUS_DEVICE(s), &s->irq); // Initialize IRQ line
}

// Define the device class
 void s32k3x8evb_uart_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = s32k3x8evb_uart_realize;
}

// Define the device class type
static const TypeInfo s32k3x8evb_uart_class_info = {
    .name = TYPE_S32K3X8EVB_UART,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K3X8EVBUartState),
    .class_init = s32k3x8evb_uart_class_init,
};

// Register the device type
static void s32k3x8evb_uart_register_types(void)
{
    type_register_static(&s32k3x8evb_uart_class_info);
}

type_init(s32k3x8evb_uart_register_types);