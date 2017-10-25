//-----------------------------------------------------------------------------
/*

MB997C Board

*/
//-----------------------------------------------------------------------------

#include "stm32f4_soc.h"
#include "audio.h"
#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------
// IO configuration

// leds
#define LED_GREEN       GPIO_NUM(PORTD, 12)
#define LED_AMBER       GPIO_NUM(PORTD, 13)
#define LED_RED         GPIO_NUM(PORTD, 14)
#define LED_BLUE        GPIO_NUM(PORTD, 15)

// push button
#define PUSH_BUTTON     GPIO_NUM(PORTA, 0)	// 0 = open, 1 = pressed

static const struct gpio_info gpios[] = {
	// leds
	{LED_RED, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LED_BLUE, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LED_GREEN, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LED_AMBER, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	// push buttons
	{PUSH_BUTTON, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_PU, GPIO_AF0, 0},
};

//-----------------------------------------------------------------------------

static struct audio_drv audio;

static struct ggm_cfg synth_cfg = {
	.audio = &audio,
	.fs = AUDIO_SAMPLE_RATE,
};

static struct ggm_state synth;

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
	int rc;

	HAL_Init();
	SystemClock_Config();

	rc = log_init();
	if (rc != 0) {
		DBG("log_init failed %d\r\n", rc);
		goto exit;
	}

	rc = gpio_init(gpios, sizeof(gpios) / sizeof(struct gpio_info));
	if (rc != 0) {
		DBG("gpio_init failed %d\r\n", rc);
		goto exit;
	}

	rc = audio_init(&audio);
	if (rc != 0) {
		DBG("audio_init failed %d\r\n", rc);
		goto exit;
	}

	rc = ggm_init(&synth, &synth_cfg);
	if (rc != 0) {
		DBG("ggm_init failed %d\r\n", rc);
		goto exit;
	}

	rc = audio_start(&audio);
	if (rc != 0) {
		DBG("audio_start failed %d\r\n", rc);
		goto exit;
	}

	DBG("init good\r\n");

	rc = ggm_run(&synth);
	if (rc != 0) {
		DBG("ggm_run exited %d\r\n", rc);
		goto exit;
	}

 exit:
	while (1) ;
	return 0;
}

//-----------------------------------------------------------------------------
