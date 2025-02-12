#include "hw/arm/s32k3x8evb_mpu.h"
#include "hw/arm/s32k3x8evb.h"
#include "hw/sysbus.h"
#include "sysemu/reset.h"
#include "qemu/units.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "hw/irq.h"
#include <stdlib.h>
#include "inttypes.h"

/* Memory region operations for the MPU registers */
static const MemoryRegionOps s32k3x8evb_mpu_ops = {
    .read       = s32k3x8evb_mpu_read,
    .write      = s32k3x8evb_mpu_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

/* MPU Read Function */
uint64_t s32k3x8evb_mpu_read(void *opaque, hwaddr addr, unsigned size) {
    S32K3X8EVBMPUState *s = opaque;

    /* Bypass the protection check for configuration registers */
    if (addr != MPU_CESR_OFFSET &&
        addr != MPU_EAR_OFFSET &&
        addr != MPU_EDR_OFFSET &&
        addr != MPU_REGION0_RBAR &&
        addr != MPU_REGION0_RASR &&
        addr != MPU_REGION1_RBAR &&
        addr != MPU_REGION1_RASR)
    {
        if (!mpu_is_access_allowed(s, addr, MPU_READ_ONLY)) {
            /* Record fault: save the faulting offset and set an error code */
            s->ear = addr;
            s->edr = 0xDEAD;
            qemu_log_mask(LOG_GUEST_ERROR, "❌ MPU FAULT: Unauthorized READ at 0x%08X\n", (uint32_t)addr);
            return 0;
        }
    }

    switch (addr) {
        case MPU_CESR_OFFSET:
            return s->cesr;
        case MPU_EAR_OFFSET:
            return s->ear;
        case MPU_EDR_OFFSET:
            return s->edr;
        case MPU_REGION0_RBAR:
            return s->region_base[0];
        case MPU_REGION0_RASR:
            return s->region_attr[0];
        case MPU_REGION1_RBAR:
            return s->region_base[1];
        case MPU_REGION1_RASR:
            return s->region_attr[1];
        default:
            qemu_log_mask(LOG_GUEST_ERROR, "Invalid MPU read at addr 0x%" PRIx64 "\n", addr);
            return 0;
    }
}

/* MPU Write Function */
void s32k3x8evb_mpu_write(void *opaque, hwaddr addr, uint64_t val, unsigned size) {
    S32K3X8EVBMPUState *s = opaque;

    /* For configuration registers, bypass the protection check */
    switch (addr) {
        case MPU_CESR_OFFSET:
            s->cesr = val | MPU_CESR_ENABLE;
            qemu_log_mask(LOG_UNIMP, "✅ MPU Enabled! MPU_CESR: 0x%08X\n", s->cesr);
            break;
        case MPU_EAR_OFFSET:
            s->ear = val;
            qemu_log_mask(LOG_UNIMP, "✅ MPU EAR set to 0x%08X\n", s->ear);
            break;
        case MPU_EDR_OFFSET:
            s->edr = val;
            qemu_log_mask(LOG_UNIMP, "✅ MPU EDR set to 0x%08X\n", s->edr);
            break;
        case MPU_REGION0_RBAR:
            s->region_base[0] = val & 0xFFFFFFF0;
            qemu_log_mask(LOG_UNIMP, "✅ MPU Region 0 Base set to 0x%08X\n", s->region_base[0]);
            break;
        case MPU_REGION0_RASR:
            s->region_attr[0] = val & 0xF;
            qemu_log_mask(LOG_UNIMP, "✅ MPU Region 0 Attributes set to 0x%X\n", s->region_attr[0]);
            break;
        case MPU_REGION1_RBAR:
            s->region_base[1] = val & 0xFFFFFFF0;
            qemu_log_mask(LOG_UNIMP, "✅ MPU Region 1 Base set to 0x%08X\n", s->region_base[1]);
            break;
        case MPU_REGION1_RASR:
            s->region_attr[1] = val & 0xF;
            qemu_log_mask(LOG_UNIMP, "✅ MPU Region 1 Attributes set to 0x%X\n", s->region_attr[1]);
            break;
        default:
            qemu_log_mask(LOG_GUEST_ERROR, "❌ MPU FAULT: Unauthorized WRITE at 0x%08X\n", (uint32_t)addr);
            break;
    }
}

/* MPU Access Check Function */
bool mpu_is_access_allowed(S32K3X8EVBMPUState *s, uint32_t addr, uint32_t access_type) {
    for (int i = 0; i < MPU_MAX_REGIONS; i++) {
        if (addr >= s->region_base[i] && addr < (s->region_base[i] + 0x1000)) {
            if ((s->region_attr[i] & access_type) == 0) {
                return false;
            }
        }
    }
    return true;
}

/* MPU Initialization (Realize Function) */
void s32k3x8evb_mpu_realize(DeviceState *dev, Error **errp) {
    S32K3X8EVBMPUState *s = S32K3X8EVB_MPU(dev);

    s->cesr = MPU_CESR_ENABLE;
    s->ear = 0;
    s->edr = 0;
    for (int i = 0; i < MPU_MAX_REGIONS; i++) {
        s->region_base[i] = 0;
        s->region_attr[i] = 0;
    }

    memory_region_init_io(&s->mem, OBJECT(s), &s32k3x8evb_mpu_ops, s, "s32k3x8evb_mpu", 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mem);
    sysbus_mmio_map(SYS_BUS_DEVICE(s), 0, S32K3X8EVB_MPU_BASE);
    sysbus_init_irq(SYS_BUS_DEVICE(s), &s->mpu_irq);

}

/* Class Initialization Function */
void s32k3x8evb_mpu_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = s32k3x8evb_mpu_realize;
}

/* QEMU Type Registration */
static const TypeInfo s32k3x8evb_mpu_class_info = {
    .name          = TYPE_S32K3X8EVB_MPU,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K3X8EVBMPUState),
    .class_init    = s32k3x8evb_mpu_class_init,
};

static void s32k3x8evb_mpu_register_types(void) {
    type_register_static(&s32k3x8evb_mpu_class_info);
}

type_init(s32k3x8evb_mpu_register_types);
