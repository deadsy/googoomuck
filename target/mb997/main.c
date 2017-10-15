//-----------------------------------------------------------------------------
/*

*/
//-----------------------------------------------------------------------------

#include "stm32f4xx_hal.h"
#include "stm32f4_soc.h"
#include "SEGGER_RTT.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------
// gpio configuration info

// standard board GPIO
#define LED_GREEN       GPIO_NUM(PORTD, 12)
#define LED_AMBER       GPIO_NUM(PORTD, 13)
#define LED_RED         GPIO_NUM(PORTD, 14)
#define LED_BLUE        GPIO_NUM(PORTD, 15)
#define PUSH_BUTTON     GPIO_NUM(PORTA, 0)	// 0 = open, 1 = pressed

// i2c bus
#define AUDIO_I2C_SCL    GPIO_NUM(PORTB, 6)
#define AUDIO_I2C_SDA    GPIO_NUM(PORTB, 9)

static const GPIO_INFO gpio_info[] = {
	// leds
	{LED_RED, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FAST, 0, GPIO_PIN_RESET},
	{LED_BLUE, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FAST, 0, GPIO_PIN_RESET},
	{LED_GREEN, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FAST, 0, GPIO_PIN_RESET},
	{LED_AMBER, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FAST, 0, GPIO_PIN_RESET},
	// push buttons
	{PUSH_BUTTON, GPIO_MODE_IT_FALLING, GPIO_NOPULL, 0, 0, -1},
	// audio i2c
	{AUDIO_I2C_SCL, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FAST, 0, -1},
	{AUDIO_I2C_SDA, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FAST, 0, -1},
};

//-----------------------------------------------------------------------------

static struct i2cbus audio_i2c;

//-----------------------------------------------------------------------------

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t * file, uint32_t line) {
	while (1) ;
}
#endif

void Error_Handler(void) {
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

void SysTick_Handler(void) {
	uint32_t ticks = HAL_GetTick();
	// blink the green led every 512 ms
	if ((ticks & 511) == 0) {
		gpio_toggle(LED_GREEN);
	}
	HAL_IncTick();
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

int main(void) {

	uint8_t buf[1];

	int i = 0;

	HAL_Init();
	SystemClock_Config();
	SEGGER_RTT_Init();

	gpio_init(gpio_info, sizeof(gpio_info) / sizeof(GPIO_INFO));
	i2c_init(&audio_i2c, AUDIO_I2C_SCL, AUDIO_I2C_SDA);

	DBG("response 34 %d\r\n", i2c_rd_buf(&audio_i2c, 0x34, buf, sizeof(buf)));
	DBG("response 94 %d\r\n", i2c_rd_buf(&audio_i2c, 0x94, buf, sizeof(buf)));
	DBG("response 96 %d\r\n", i2c_rd_buf(&audio_i2c, 0x96, buf, sizeof(buf)));

	for (i = 0; i < 256; i += 2) {
		if (i2c_scan(&audio_i2c, i) != 0) {
			DBG("device found %02x\r\n", i);
		}
	}

	i = 0;
	while (1) {
		DBG("in the while loop %d\r\n", i);
		mdelay(2000);
		i += 1;
	}

	return 0;
}

//-----------------------------------------------------------------------------
