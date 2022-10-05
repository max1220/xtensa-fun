/* Linux boot Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp32/rom/cache.h"


static void IRAM_ATTR map_flash_and_jump()
{
	unsigned long drom_load_addr_aligned = 0x3f400000;
	unsigned long drom_addr_aligned = 0x00000000;
	unsigned long drom_page_count = (0x00400000 - drom_addr_aligned) / 0x10000;
	unsigned long irom_load_addr_aligned = 0x400d0000;
	unsigned long irom_addr_aligned = 0x00040000;
	unsigned long irom_page_count = (0x00400000 - irom_addr_aligned) / 0x10000;

	Cache_Read_Disable(0);
	Cache_Flush(0);
	cache_flash_mmu_set(0, 0, irom_load_addr_aligned, irom_addr_aligned, 64, irom_page_count);
	cache_flash_mmu_set(0, 0, drom_load_addr_aligned, drom_addr_aligned, 64, drom_page_count);
	Cache_Read_Enable(0);

	asm volatile ("jx %0" :: "r"(irom_load_addr_aligned));
}

void app_main()
{
	printf("\n========== ESP32 LINUX BOOTLOADER ==========\n");
	vTaskSuspendAll();
    map_flash_and_jump();
    //xTaskResumeAll();
}
