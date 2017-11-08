//-----------------------------------------------------------------------------
/*

MB997 Board (STM32F4_Discovery) based on the STM32F407xx SoC

*/
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "stm32f4_soc.h"
#include "led.h"
#include "debounce.h"

#include "board.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------
// IO configuration

// push button
#define PUSH_BUTTON GPIO_NUM(PORTA, 0)	// 0 = open, 1 = pressed

static const struct gpio_info gpios[] = {
	// leds
	{LED_RED, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LED_BLUE, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LED_GREEN, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LED_AMBER, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	// push buttons
	{PUSH_BUTTON, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
};

//-----------------------------------------------------------------------------

void Error_Handler(void) {
	DBG("Error_Handler\r\n");
	led_on(LED_RED);
	while (1) ;
}

//-----------------------------------------------------------------------------

void NMI_Handler(void) {
}
void HardFault_Handler(void) {
	while (1) ;
}
void MemManage_Handler(void) {
	while (1) ;
}
void BusFault_Handler(void) {
	while (1) ;
}
void UsageFault_Handler(void) {
	while (1) ;
}
void SVC_Handler(void) {
}
void DebugMon_Handler(void) {
}
void PendSV_Handler(void) {
}

//-----------------------------------------------------------------------------

static void SystemClock_Config(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	// Enable Power Control clock
	__PWR_CLK_ENABLE();

	// The voltage scaling allows optimizing the power consumption when the device is
	// clocked below the maximum system frequency, to update the voltage scaling value
	// regarding system frequency refer to product datasheet.
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	// Enable HSE Oscillator and activate PLL with HSE as source
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	// Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

//-----------------------------------------------------------------------------
// key debouncing (called from the system tick isr)

#define PUSH_BUTTON_BIT 0

// handle a key down
void debounce_on_handler(uint32_t bits) {
	if (bits & (1 << PUSH_BUTTON_BIT)) {
		led_on(LED_BLUE);
	}
}

// handle a key up
void debounce_off_handler(uint32_t bits) {
	if (bits & (1 << PUSH_BUTTON_BIT)) {
		led_off(LED_BLUE);
	}
}

// map the gpio inputs to be debounced into the 32 bit debounce state
uint32_t debounce_input(void) {
	return gpio_rd(PUSH_BUTTON) << PUSH_BUTTON_BIT;
}

//-----------------------------------------------------------------------------

void board_init(void) {
	int rc;

	HAL_Init();
	SystemClock_Config();

	rc = log_init();
	if (rc != 0) {
		goto exit;
	}

	rc = gpio_init(gpios, sizeof(gpios) / sizeof(struct gpio_info));
	if (rc != 0) {
		DBG("gpio_init failed %d\r\n", rc);
		goto exit;
	}

	rc = debounce_init();
	if (rc != 0) {
		DBG("debounce_init failed %d\r\n", rc);
		goto exit;
	}

	DBG("board_init ok\r\n");
	return;

 exit:
	return;
}

//-----------------------------------------------------------------------------

void board_leds(uint32_t on_mask, uint32_t off_mask) {
	if (on_mask & 1) {
		led_on(LED_GREEN);
	}
	if (off_mask & 1) {
		led_off(LED_GREEN);
	}
}

//-----------------------------------------------------------------------------

uint32_t board_buttons(void) {
	DBG("board_buttons\r\n");
	return 0;
}

//-----------------------------------------------------------------------------

void board_systick(void) {
	uint32_t ticks = HAL_GetTick();
	// sample debounced inputs every 16 ms
	if ((ticks & 15) == 0) {
		debounce_isr();
	}
	HAL_IncTick();
}

//-----------------------------------------------------------------------------
