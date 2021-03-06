//-----------------------------------------------------------------------------
/*

Axoloti Synth Board (STM32F427xG)

http://www.axoloti.com/

CPU: STM32F427IGH6 (G=1MiB flash)
Codec: ADAU1361BCPZ (96kHz, 24 bit stereo codec)
SDRAM: AS4C4M16SA-7BCN (4Mx16bit SDRAM)

Notes:

With sysclk=180MHz (cpu rate) we can't set pll48clk=48MHz, and that would
prevent the USB from working properly. So, we derate to sysclk=168 MHz
allowing us to set pll48clk=48MHz.

*/
//-----------------------------------------------------------------------------

#include "stm32f4_soc.h"
#include "audio.h"
#include "debounce.h"
#include "ggm.h"
#include "utils.h"
#include "io.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------
// IO configuration

static const struct gpio_info gpios[] = {
	// leds
	{IO_LED_RED, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_LED_GREEN, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF0, 0},
	// push buttons
	{IO_SWITCH_1, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_PD, GPIO_AF0, 0},
	{IO_SWITCH_2, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_PD, GPIO_AF0, 0},
	// serial port
	{IO_UART_TX, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_HI, GPIO_PUPD_NONE, GPIO_AF8, 0},
	{IO_UART_RX, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_HI, GPIO_PUPD_NONE, GPIO_AF8, 0},
	// audio
	{IO_AUDIO_I2C_SCL, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_AUDIO_I2C_SDA, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_AUDIO_MCLK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_AUDIO_ADC, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{IO_AUDIO_LRCLK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{IO_AUDIO_BCLK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{IO_AUDIO_DAC, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
};

//-----------------------------------------------------------------------------

static struct ggm synth;

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
		gpio_toggle(IO_LED_GREEN);
	}
	// sample debounced inputs every 16 ms
	if ((ticks & 15) == 0) {
		debounce_isr();
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
	// Activate the Over-Drive mode
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
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

#define SWITCH_1_BIT 0
#define SWITCH_2_BIT 1

// handle a key down
void debounce_on_handler(uint32_t bits) {
	if (bits & (1 << SWITCH_1_BIT)) {
		event_wr(EVENT_TYPE_KEY_DN | 1U, NULL);
	}
	if (bits & (1 << SWITCH_2_BIT)) {
		event_wr(EVENT_TYPE_KEY_DN | 2U, NULL);
	}
}

// handle a key up
void debounce_off_handler(uint32_t bits) {
	if (bits & (1 << SWITCH_1_BIT)) {
		event_wr(EVENT_TYPE_KEY_UP | 1U, NULL);
	}
	if (bits & (1 << SWITCH_2_BIT)) {
		event_wr(EVENT_TYPE_KEY_UP | 2U, NULL);
	}
}

// map the gpio inputs to be debounced into the 32 bit debounce state
uint32_t debounce_input(void) {
	return (gpio_rd(IO_SWITCH_1) << SWITCH_1_BIT) | (gpio_rd(IO_SWITCH_2) << SWITCH_2_BIT);
}

//-----------------------------------------------------------------------------

static struct adc_cfg test_adc_cfg = {
	.base = ADC1_BASE,
};

static struct adc_drv test_adc;

//-----------------------------------------------------------------------------
// random number generator

#if 0
// errors callback
static void rng_err_callback(struct rng_drv *rng, uint32_t errors) {
	DBG("rng error 0x%08x\r\n", errors);
}

// data callback
static void rng_data_callback(struct rng_drv *rng, uint32_t data) {
	// do something with the data
}
void HASH_RNG_IRQHandler(void) {
	rng_isr(&ggm_rng);
}
#endif

static struct rng_cfg ggm_rng_cfg = {
	.mode = RNG_MODE_POLLED,
	//.err_callback = rng_err_callback,
	//.data_callback = rng_data_callback,
};

static struct rng_drv ggm_rng;

//-----------------------------------------------------------------------------
// midi port (on USART2)

struct usart_cfg midi_serial_cfg = {
	.base = USART6_BASE,
	.baud = 31250,
	.data = 8,
	.parity = 0,
	.stop = 1,
};

struct usart_drv midi_serial;

void USART6_IRQHandler(void) {
	usart_isr(&midi_serial);
}

//-----------------------------------------------------------------------------

static void dump_clocks() {
	DBG("sysclk %d Hz\r\n", HAL_RCC_GetSysClockFreq());
	DBG("pclk1 %d Hz\r\n", HAL_RCC_GetPCLK1Freq());
	DBG("pclk2 %d Hz\r\n", HAL_RCC_GetPCLK2Freq());
	DBG("i2sclk %d Hz\r\n", get_i2sclk());
}

//-----------------------------------------------------------------------------

int main(void) {
	uint32_t val;
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

	rc = usart_init(&midi_serial, &midi_serial_cfg);
	if (rc != 0) {
		DBG("usart_init failed %d\r\n", rc);
		goto exit;
	}
	// setup the interrupts for the serial port
	HAL_NVIC_SetPriority(USART6_IRQn, 10, 0);
	NVIC_EnableIRQ(USART6_IRQn);

	rc = rng_init(&ggm_rng, &ggm_rng_cfg);
	if (rc != 0) {
		DBG("rng_init failed %d\r\n", rc);
		goto exit;
	}
#if 0
	// Setup HASH_RNG interrupt
	HAL_NVIC_SetPriority(HASH_RNG_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(HASH_RNG_IRQn);
#endif
	rng_enable(&ggm_rng);

	rc = adc_init(&test_adc, &test_adc_cfg);
	if (rc != 0) {
		DBG("adc_init failed %d\r\n", rc);
		goto exit;
	}

	rc = audio_init(&ggm_audio);
	if (rc != 0) {
		DBG("audio_init failed %d\r\n", rc);
		goto exit;
	}

	rc = ggm_init(&synth, &ggm_audio, &midi_serial);
	if (rc != 0) {
		DBG("ggm_init failed %d\r\n", rc);
		goto exit;
	}

	rc = audio_start(&ggm_audio);
	if (rc != 0) {
		DBG("audio_start failed %d\r\n", rc);
		goto exit;
	}
	// seed the PRNG
	rc = rng_rd(&ggm_rng, 1, &val);
	if (rc == 0) {
		rand_init(val);
	} else {
		DBG("rng_rd failed %d\r\n", rc);
		goto exit;
	}

	dump_clocks();

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
