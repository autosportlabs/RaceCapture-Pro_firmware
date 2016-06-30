#include "cpu_device.h"
#include "portmacro.h"
#include "printk.h"
#include <stm32f4xx_misc.h>
#include <stm32f4xx_rcc.h>
#include <core_cm4.h>
#include <stdint.h>
#include <app_info.h>

#define CPU_ID_REGISTER_START 	0x1FFF7A10
#define CPU_ID_REGISTER_END   	0x1FFF7A1C
#define ASCII(x)		(((x)&0xF) < 10) ? (((x)&0xF)+'0') : (((x)&0xF)-10+'A')
#define SERIAL_ID_BITS		96
#define SERIAL_ID_BUFFER_LEN	(((SERIAL_ID_BITS / 8) * 2) + 1)

/*
 * Set by f407_mem.ld linker script.  Somehow this is getting
 * altered to a value of 0x20020000.  Don't know why yet.
 * But we handle this below by masking off the top 12 bits.
 */
extern const uint32_t _flash_start;

static char cpu_id[SERIAL_ID_BUFFER_LEN];

static void init_cpu_id()
{
    uint32_t *p = (uint32_t *) CPU_ID_REGISTER_START;
    int i = 0, j = 0;

    while (p <= (uint32_t *) CPU_ID_REGISTER_END) {
        for (i = 0; i < 8; i++)
            cpu_id[7 - i + j] = ASCII(*p >> (i * 4));
        p++;
        j += 8;
    }

    cpu_id[SERIAL_ID_BUFFER_LEN - 1] = 0;
}

int cpu_device_init(void)
{
        NVIC_SetVectorTable(NVIC_VectTab_FLASH, _flash_start & 0x000FFFFF);
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
        init_cpu_id();
        return 1;
}

void cpu_device_reset(int bootloader)
{
    struct app_handshake_block *handshake =
        (struct app_handshake_block *)HANDSHAKE_ADDR;

    /* Clear any reset flags that might be present (i.e. watchdog) */
    RCC_ClearFlag();

    /* If bootloader mode is requested, Set the flag in the
     * handshake area */
    if (bootloader == 1) {
        handshake->loader_magic = LOADER_KEY;
    }

    NVIC_SystemReset();
}

const char *cpu_device_get_serialnumber(void)
{
    return cpu_id;
}

/* This is required by the STM32 libraries for their ASSERT macros to
 * work.  Useful if you need to catch HAL bugs */
void assert_failed(uint8_t* file, uint32_t line)
{
        portDISABLE_INTERRUPTS();
	while(1);
}
