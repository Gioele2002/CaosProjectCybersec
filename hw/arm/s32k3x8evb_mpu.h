#ifndef HW_ARM_S32K3X8EVB_MPU_H
#define HW_ARM_S32K3X8EVB_MPU_H

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qom/object.h"

/* MPU Base Address */
#define S32K3X8EVB_MPU_BASE  0x40278000  

/* MPU Register Offsets */
#define MPU_CESR_OFFSET      0x00  /* Control/Error Status Register */
#define MPU_EAR_OFFSET       0x10  /* Error Address Register */
#define MPU_EDR_OFFSET       0x14  /* Error Detail Register */
#define MPU_REGION0_RBAR     0x20  /* Region 0 Base Address Register */
#define MPU_REGION0_RASR     0x24  /* Region 0 Attribute/Size Register */
#define MPU_REGION1_RBAR     0x30  /* Region 1 Base Address Register */
#define MPU_REGION1_RASR     0x34  /* Region 1 Attribute/Size Register */

/* MPU Region Attributes */
#define MPU_NO_ACCESS      0x00000000  /* No Access (Causes Fault) */
#define MPU_READ_ONLY      0x00000001  /* Read-Only */
#define MPU_READ_WRITE     0x00000002  /* Read/Write Allowed */
#define MPU_EXECUTE_NEVER  0x00000004  /* No Execute */

/* MPU Control Register Bit Masks */
#define MPU_CESR_ENABLE    (1 << 0)  /* Enable MPU in QEMU */
#define MPU_REGION_ENABLE  (1 << 1)  /* Enable Region Protection */

#define MPU_MAX_REGIONS 2

/* MPU Device State Structure */
typedef struct S32K3X8EVBMPUState {
    SysBusDevice parent_obj;
    MemoryRegion mem;

    uint32_t cesr;
    uint32_t ear;
    uint32_t edr;

    uint32_t region_base[MPU_MAX_REGIONS];
    uint32_t region_attr[MPU_MAX_REGIONS];

    qemu_irq mpu_irq;
} S32K3X8EVBMPUState;

#define TYPE_S32K3X8EVB_MPU "s32k3x8evb_mpu"
OBJECT_DECLARE_TYPE(S32K3X8EVBMPUState, s32k3x8evb_mpu, S32K3X8EVB_MPU)

/* Function Prototypes */
uint64_t s32k3x8evb_mpu_read(void *opaque, hwaddr addr, unsigned size);
void s32k3x8evb_mpu_write(void *opaque, hwaddr addr, uint64_t val, unsigned size);
void s32k3x8evb_mpu_realize(DeviceState *dev, Error **errp);
void s32k3x8evb_mpu_class_init(ObjectClass *klass, void *data);
bool mpu_is_access_allowed(S32K3X8EVBMPUState *s, uint32_t addr, uint32_t access_type);

#endif /* HW_ARM_S32K3X8EVB_MPU_H */
