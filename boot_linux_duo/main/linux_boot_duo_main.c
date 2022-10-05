/* Linux boot Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdkconfig.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "soc/reg_base.h"
#include "soc/dport_reg.h"
#include "soc/dport_access.h"
#include "soc/uart_reg.h"

#include "esp32/rom/cache.h"

#define LINUX_BOOT_DUO_DEBUG 1

#define PRINT_REG(_reg_name) \
	printf("Register " #_reg_name " at 0x%lx = 0x%lx\n", \
	(uint32_t)(_reg_name), \
	_DPORT_REG_READ(_reg_name));

static void dump_interrupt_status_registers()
{
	printf("\nInterrupt status registers: \n");
	PRINT_REG(DPORT_PRO_INTR_STATUS_0_REG)
	PRINT_REG(DPORT_PRO_INTR_STATUS_1_REG)
	PRINT_REG(DPORT_PRO_INTR_STATUS_2_REG)
	PRINT_REG(DPORT_APP_INTR_STATUS_0_REG)
	PRINT_REG(DPORT_APP_INTR_STATUS_1_REG)
	PRINT_REG(DPORT_APP_INTR_STATUS_2_REG)
}

static void dump_interrupt_map_registers()
{
	printf("\nInterrupt configuration registers: \n");
	PRINT_REG(DPORT_PRO_UART_INTR_MAP_REG)
	PRINT_REG(DPORT_PRO_UART1_INTR_MAP_REG)
	PRINT_REG(DPORT_PRO_UART2_INTR_MAP_REG)
	PRINT_REG(DPORT_APP_UART_INTR_MAP_REG)
	PRINT_REG(DPORT_APP_UART1_INTR_MAP_REG)
	PRINT_REG(DPORT_APP_UART2_INTR_MAP_REG)
}

static void dump_uart_interrupt_config_registers()
{
	printf("\nUART Interrupt configuration: \n");
	PRINT_REG(UART_INT_RAW_REG(0))
	PRINT_REG(UART_INT_ST_REG(0))
	PRINT_REG(UART_INT_ENA_REG(0))
	PRINT_REG(UART_INT_CLR_REG(0))
	PRINT_REG(UART_INT_RAW_REG(1))
	PRINT_REG(UART_INT_ST_REG(1))
	PRINT_REG(UART_INT_ENA_REG(1))
	PRINT_REG(UART_INT_CLR_REG(1))
	PRINT_REG(UART_INT_RAW_REG(2))
	PRINT_REG(UART_INT_ST_REG(2))
	PRINT_REG(UART_INT_ENA_REG(2))
	PRINT_REG(UART_INT_CLR_REG(2))
}

static void dump_mmu_regs()
{
	printf("\nMMU registers: \n");
	PRINT_REG(DPORT_PRO_DRAM_HL)
	PRINT_REG(DPORT_APP_DRAM_HL)
	PRINT_REG(DPORT_PRO_DRAM_SPLIT)
	PRINT_REG(DPORT_PRO_DRAM_SPLIT)
}


// map the flash regions for the APP CPU
static void IRAM_ATTR map_flash()
{
	// The flash memory mappings needed to load Linux
	//TODO: Hardcoded values
	unsigned long drom_vaddr = 0x3f400000;
	unsigned long drom_paddr = 0x00000000;
	unsigned long drom_page_count = (0x00400000 - drom_paddr) / 0x10000;
	unsigned long irom_vaddr = 0x400d0000;
	unsigned long irom_paddr = 0x00040000;
	unsigned long irom_page_count = (0x00400000 - irom_paddr) / 0x10000;

#if LINUX_BOOT_DUO_DEBUG
	printf("Mapping flash: DROM V=%lx P=%lx(%ld pages), IROM V=%lx P=%lx(%ld pages)\n",
		drom_vaddr,
		drom_paddr,
		drom_page_count,
		irom_vaddr,
		irom_paddr,
		irom_page_count
	);
#endif

	// suspend FreeRTOS while changing cache parameters just in case
	vTaskSuspendAll();
	// Need to be carefull not to use any library functions here!

	// Map the flash on the APP CPU at the correct address
	Cache_Read_Disable(1);
	Cache_Flush(1);
	cache_flash_mmu_set(1, 0, irom_vaddr, irom_paddr, 64, irom_page_count);
	cache_flash_mmu_set(1, 0, drom_vaddr, drom_paddr, 64, drom_page_count);
	Cache_Read_Enable(1);

	// resume FreeRTOS
	xTaskResumeAll();
}

// TODO
static void IRAM_ATTR map_ram()
{

#if LINUX_BOOT_DUO_DEBUG
	printf("Mapping RAM(TODO)\n");
#endif
}

// start running Linux on the APP CPU.
// This will leave FreeRTOS running on the main CPU.
static void IRAM_ATTR start_APP_cpu(uint32_t entry)
{
	// Check if the APP CPU is enabled.
	if (DPORT_REG_GET_BIT(DPORT_APPCPU_CTRL_B_REG, DPORT_APPCPU_CLKGATE_EN))
	{
		printf("APP CPU already running!\n");
		return;
	}

#if LINUX_BOOT_DUO_DEBUG
	printf("Resetting APP CPU\n");
#endif
	// reset the APP CPU
	DPORT_REG_SET_BIT(DPORT_APPCPU_CTRL_A_REG, DPORT_APPCPU_RESETTING);
	DPORT_REG_CLR_BIT(DPORT_APPCPU_CTRL_A_REG, DPORT_APPCPU_RESETTING);

#if LINUX_BOOT_DUO_DEBUG
	printf("Setting entry point to: %lx\n", entry);
#endif
	// set entrypoint to linux.
	DPORT_WRITE_PERI_REG(DPORT_APPCPU_CTRL_D_REG, entry);

#if LINUX_BOOT_DUO_DEBUG
	printf("Enabling APP CPU!\n");
#endif
	// Enable the APP CPU, by enabling it's clock.
	DPORT_SET_PERI_REG_MASK(DPORT_APPCPU_CTRL_B_REG, DPORT_APPCPU_CLKGATE_EN);
}


void app_main()
{
	dump_interrupt_status_registers();
	dump_interrupt_map_registers();
	dump_uart_interrupt_config_registers();

	// (This is PRO CPU)
	printf("\n========== ESP32 LINUX DUO BOOTLOADER ==========\n");

	// map flash
	map_flash();
	map_ram();

	//start Linux on APP CPU
	start_APP_cpu(0x400d0000);

	// (Linux should now be running on APP CPU)
	printf("\n========== /ESP32 LINUX DUO BOOTLOADER ==========\n");

	// vTaskSuspendAll();

	// show some message that the FreeRTOS kernel is still alive
	while (1) {
#if LINUX_BOOT_DUO_DEBUG
		printf("\nFreeRTOS still kicking around!\n");
#endif
		vTaskDelay(3);
	}

	//TODO: provide interfaces to Linux via some kind of API?
}
