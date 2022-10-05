#include <stdio.h>
#include <unistd.h>
#include <stdint.h>



// these definitions are adapted from esp-idf/components/soc/esp32/include/soc/dport_access.h
#define _DPORT_REG_READ(_r)        	(*(volatile uint32_t *)(_r))
#define _DPORT_REG_WRITE(_r, _v)   	(*(volatile uint32_t *)(_r)) = (_v)
#define DPORT_REG_GET_BIT(_r, _b)  	(DPORT_REG_READ(_r) & (_b))
#define DPORT_REG_SET_BIT(_r, _b)  	DPORT_REG_WRITE((_r), (DPORT_REG_READ(_r)|(_b)))
#define DPORT_REG_CLR_BIT(_r, _b)  	DPORT_REG_WRITE((_r), (DPORT_REG_READ(_r) & (~(_b))))

// these definitions are from esp-idf/components/soc/esp32/include/soc/reg_base.h
#define DR_REG_DPORT_BASE		0x3ff00000
#define DR_REG_UART_BASE		0x3ff40000
#define DR_REG_UART1_BASE		0x3ff50000
#define DR_REG_UART2_BASE		0x3ff6E000

// these definitions are from esp-idf/components/soc/esp32/include/soc/dport_reg.h
#define DPORT_PRO_INTR_STATUS_0_REG	(DR_REG_DPORT_BASE + 0x0EC)
#define DPORT_PRO_INTR_STATUS_1_REG	(DR_REG_DPORT_BASE + 0x0F0)
#define DPORT_PRO_INTR_STATUS_2_REG	(DR_REG_DPORT_BASE + 0x0F4)
#define DPORT_APP_INTR_STATUS_0_REG	(DR_REG_DPORT_BASE + 0x0F8)
#define DPORT_APP_INTR_STATUS_1_REG	(DR_REG_DPORT_BASE + 0x0FC)
#define DPORT_APP_INTR_STATUS_2_REG	(DR_REG_DPORT_BASE + 0x100)
#define DPORT_PRO_UART_INTR_MAP_REG	(DR_REG_DPORT_BASE + 0x18C)
#define DPORT_PRO_UART1_INTR_MAP_REG	(DR_REG_DPORT_BASE + 0x190)
#define DPORT_PRO_UART2_INTR_MAP_REG	(DR_REG_DPORT_BASE + 0x194)
#define DPORT_APP_UART_INTR_MAP_REG	(DR_REG_DPORT_BASE + 0x2A0)
#define DPORT_APP_UART1_INTR_MAP_REG	(DR_REG_DPORT_BASE + 0x2A4)
#define DPORT_APP_UART2_INTR_MAP_REG	(DR_REG_DPORT_BASE + 0x2A8)

// these definitions are from esp-idf/components/soc/esp32/include/soc/dport_reg.h
#define REG_UART_BASE( i )		(DR_REG_UART_BASE + (i) * 0x10000 + ( (i) > 1 ? 0xe000 : 0 ) )
#define UART_INT_RAW_REG(i)		(REG_UART_BASE(i) + 0x4)
#define UART_INT_ST_REG(i)		(REG_UART_BASE(i) + 0x8)
#define UART_INT_ENA_REG(i)		(REG_UART_BASE(i) + 0xC)
#define UART_INT_CLR_REG(i)		(REG_UART_BASE(i) + 0x10)



#define PRINT_REG(_reg_name) \
	printf("Register " #_reg_name " at 0x%lx = 0x%lx\n", \
	(uint32_t)(_reg_name), \
	_DPORT_REG_READ(_reg_name));



static void dump_interrupt_status_registers()
{
	printf("Interrupt status registers: \n");
	PRINT_REG(DPORT_PRO_INTR_STATUS_0_REG)
	PRINT_REG(DPORT_PRO_INTR_STATUS_1_REG)
	PRINT_REG(DPORT_PRO_INTR_STATUS_2_REG)
	PRINT_REG(DPORT_APP_INTR_STATUS_0_REG)
	PRINT_REG(DPORT_APP_INTR_STATUS_1_REG)
	PRINT_REG(DPORT_APP_INTR_STATUS_2_REG)
}

static void dump_interrupt_map_registers()
{
	printf("Interrupt configuration registers: \n");
	PRINT_REG(DPORT_PRO_UART_INTR_MAP_REG)
	PRINT_REG(DPORT_PRO_UART1_INTR_MAP_REG)
	PRINT_REG(DPORT_PRO_UART2_INTR_MAP_REG)
	PRINT_REG(DPORT_APP_UART_INTR_MAP_REG)
	PRINT_REG(DPORT_APP_UART1_INTR_MAP_REG)
	PRINT_REG(DPORT_APP_UART2_INTR_MAP_REG)
}

static void dump_uart_interrupt_config_registers()
{
	printf("UART Interrupt configuration: \n");
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








int main() {
	printf("Hello World!\n");
	dump_interrupt_status_registers();
	dump_interrupt_map_registers();
	dump_uart_interrupt_config_registers();
}
