#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/arm/armv7m.h"
#include "hw/arm/boot.h"
#include "hw/sysbus.h"
#include "hw/arm/s32k3x8evb_uart.h"
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
#include "hw/arm/s32k3x8evb.h"
#include "qemu/log.h"


#define TYPE_S32K3X8EVB_MACHINE "s32k3x8evb-machine"
OBJECT_DECLARE_TYPE(S32K3X8EVBMachineState, S32K3X8EVBMachineClass, S32K3X8EVB_MACHINE)



/* Initialization function for the board */
static void s32k3x8evb_init(MachineState *machine)
{
    S32K3X8EVBMachineState *s = S32K3X8EVB_MACHINE(machine);
    Error *err = NULL;
    
    DeviceState *armv7m;
    
     /* Initialize system clock */
    s->sysclk = clock_new(OBJECT(machine), "SYSCLK");
    clock_set_hz(s->sysclk, SYSCLK_FRQ);
    /* Initialize Flash memory */
    memory_region_init_rom(&s->flash, NULL, "s32k3x8evb.flash", FLASH_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), FLASH_BASE, &s->flash);
    /* Initialize ITCM */
    memory_region_init_ram(&s->itcm, NULL, "s32k3x8evb.itcm", ITCM_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), ITCM_BASE, &s->itcm);

    /* Initialize DTCM */
    memory_region_init_ram(&s->dtcm, NULL, "s32k3x8evb.dtcm", DTCM_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), DTCM_BASE, &s->dtcm);

    /* Initialize SRAM0 */
    memory_region_init_ram(&s->sram0, NULL, "s32k3x8evb.sram0", SRAM0_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), SRAM0_BASE, &s->sram0);

    /* Initialize SRAM1 */
    memory_region_init_ram(&s->sram1, NULL, "s32k3x8evb.sram1", SRAM1_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), SRAM1_BASE, &s->sram1);

    /* Initialize SRAM2 */
    memory_region_init_ram(&s->sram2, NULL, "s32k3x8evb.sram2", SRAM2_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), SRAM2_BASE, &s->sram2);

     /* Initialize CPU (Cortex-M7) */
     //This function initializes the CPU (Cortex-M7) as a child object of the board (s), which is represented by the structure S32K3X8EVBState.
    object_initialize_child(OBJECT(s), "armv7m", &s->armv7m, TYPE_ARMV7M);
    //This line casts the armv7m object (which was initialized as part of S32K3X8EVBState) to a DeviceState type, which is a common type for all devices in QEMU.
    armv7m = DEVICE(&s->armv7m);
    //This function connects the system clock (s->sysclk) to the CPU (armv7m), which allows the CPU to run at the specified frequency.
    qdev_connect_clock_in(armv7m, "cpuclk", s->sysclk);
    // This function sets a property of the CPU device (armv7m). Specifically, it sets the cpu-type property to the CPU type defined in the machine.
    qdev_prop_set_string(armv7m, "cpu-type", machine->cpu_type);
    //This function sets a property of the CPU to enable a feature (in this case, the ARM bit-banding feature).
    qdev_prop_set_bit(armv7m, "enable-bitband", true);
    //This sets the memory property of the CPU to point to the system memory, meaning that the CPU will use the system memory region to fetch instructions and data.
    object_property_set_link(OBJECT(&s->armv7m), "memory",
                             OBJECT(get_system_memory()), &err);
    //This function finalizes the initialization of the CPU and ensures it is properly connected to the system bus so that it can interact with other devices.
    sysbus_realize(SYS_BUS_DEVICE(&s->armv7m), &err);
    /*This function assigns the parent bus for the device (armv7m) to the system bus. The parent bus connects the CPU to the rest of the system, allowing it to interact with other devices and memory regions.*/
    qdev_set_parent_bus(DEVICE(&s->armv7m), sysbus_get_default(), &err);

    /* Initialize UART */
   
    /* Initialize UART */
    s->uart = sysbus_create_simple(TYPE_S32K3X8EVB_UART, S32K3X8EVB_UART0_BASE, NULL); 
    s32k3x8evb_uart_realize(s->uart,&err); 

    /* Initialize UART Interrupt */
   qdev_init_gpio_out(DEVICE(s->uart), &s->irq, 1);
   /* Connect the UART's IRQ to the system bus */
   sysbus_connect_irq(SYS_BUS_DEVICE(s->uart), 0, s->irq);

 // Connect the UART device to the interrupt line of the board


    //UART0_TransmitString("UART Initialized Successfully.\n");

    /* Initialize CAN */
    //s->can = sysbus_create_simple("can_sja1000", CAN_BASE, NULL);

    /* Load firmware into Flash */
    armv7m_load_kernel(ARM_CPU(first_cpu), machine->kernel_filename, 0, FLASH_SIZE);
}


/* Machine registration function */
static void s32k3x8evb_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
   // S32K3X8EVBClass *smc = S32K3X8EVB_CLASS(oc);
     static const char * const valid_cpu_types[] = {
        ARM_CPU_TYPE_NAME("cortex-m7"),
        NULL
    };
    

    mc->desc = "NXP S32K348 EVB Board";
    mc->init = s32k3x8evb_init;
    mc->max_cpus = 1;
    mc->default_ram_size = 3 * SRAM0_SIZE;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-m7");
    mc->valid_cpu_types = valid_cpu_types;
    
   
}

static const TypeInfo s32k3x8evb_info = {
    .name = TYPE_S32K3X8EVB_MACHINE,
    .parent = TYPE_MACHINE,
    .abstract = false,
    .instance_size = sizeof(S32K3X8EVBMachineState),
    .class_size = sizeof(S32K3X8EVBMachineClass),
    .class_init = s32k3x8evb_class_init,
};

static void s32k3x8evb_machine_init(void)
{
  type_register_static(&s32k3x8evb_info);
}

//Registration of the board 
type_init(s32k3x8evb_machine_init);

