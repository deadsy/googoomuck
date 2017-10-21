//-----------------------------------------------------------------------------
/*

*/
//-----------------------------------------------------------------------------

#define DEBUG

#include "stm32f4_soc.h"
#include "cs43l22.h"
#include "logging.h"
#include "synth.h"

//-----------------------------------------------------------------------------
// IO configuration

// leds
#define LED_GREEN       GPIO_NUM(PORTD, 12)
#define LED_AMBER       GPIO_NUM(PORTD, 13)
#define LED_RED         GPIO_NUM(PORTD, 14)
#define LED_BLUE        GPIO_NUM(PORTD, 15)

// push button
#define PUSH_BUTTON     GPIO_NUM(PORTA, 0)	// 0 = open, 1 = pressed

// audio
#define AUDIO_I2C_SCL   GPIO_NUM(PORTB, 6)
#define AUDIO_I2C_SDA   GPIO_NUM(PORTB, 9)
#define AUDIO_RESET     GPIO_NUM(PORTD, 4)
#define AUDIO_I2S_MCK   GPIO_NUM(PORTC, 7)
#define AUDIO_I2S_SCK   GPIO_NUM(PORTC, 10)
#define AUDIO_I2S_SD    GPIO_NUM(PORTC, 12)
#define AUDIO_I2S_WS    GPIO_NUM(PORTA, 4)

static const struct gpio_info gpios[] = {
	// leds
	{LED_RED, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LED_BLUE, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LED_GREEN, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LED_AMBER, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	// push buttons
	{PUSH_BUTTON, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_PU, GPIO_AF0, 0},
	// audio codec
	{AUDIO_RESET, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{AUDIO_I2C_SCL, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{AUDIO_I2C_SDA, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{AUDIO_I2S_MCK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{AUDIO_I2S_SCK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{AUDIO_I2S_SD, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{AUDIO_I2S_WS, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
};

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
// Audio Initialisation
// Configuring for 44.1 KHz sample rate, 16 bits per sample, 2 channels.

#define AUDIO_SAMPLE_RATE 44100U

// from ./scripts/i2sclk.py
// 44099.506579: sn 429 sr 2 div 9 odd 1 (chlen = 16, mckoe = 1)
#define AUDIO_I2SPLLN 429U
#define AUDIO_I2SPLLR 2U
#define AUDIO_I2SDIV 9U
#define AUDIO_I2SODD 1U

// I2S setup
static struct i2s_cfg audio_i2s_cfg = {
	.idx = 3,
	.mode = I2S_MODE_MASTER_TX,
	.standard = I2S_STANDARD_PHILIPS,
	.data_format = I2S_DATAFORMAT_16B,
	.mckoe = I2S_MCLKOUTPUT_ENABLE,
	.fs = I2S_FS_44100,
	.cpol = I2S_CPOL_LOW,
	.div = AUDIO_I2SDIV,
	.odd = AUDIO_I2SODD,
	.fdx = 0,
};

static struct i2s_drv audio_i2s;

// I2C setup
static struct i2c_cfg audio_i2c_cfg = {
	.scl = AUDIO_I2C_SCL,
	.sda = AUDIO_I2C_SDA,
	.delay = 20,
};

static struct i2c_drv audio_i2c;

// cs43l22 DAC setup
static struct cs4x_cfg audio_dac_cfg = {
	.i2c = &audio_i2c,
	.adr = 0x94,
	.rst = AUDIO_RESET,
	.out = DAC_OUTPUT_AUTO,
};

static struct cs4x_drv audio_dac;

static int audio_init(void) {
	int rc = 0;

	// Setup the i2s pll to generate i2s_clk
	rc = set_i2sclk(AUDIO_I2SPLLN, AUDIO_I2SPLLR);
	if (rc != 0) {
		DBG("i2sclk_init failed %d\r\n", rc);
		goto exit;
	}
	DBG("i2sclk %d Hz\r\n", get_i2sclk());

	// setup the i2s interface
	rc = i2s_init(&audio_i2s, &audio_i2s_cfg);
	if (rc != 0) {
		DBG("i2s_init failed %d\r\n", rc);
		goto exit;
	}
	// start the i2s clocking
	i2s_enable(&audio_i2s);
	DBG("fs %d Hz\r\n", i2s_get_fsclk(&audio_i2s));

	// setup the i2c bus used to control the dac
	rc = i2c_init(&audio_i2c, &audio_i2c_cfg);
	if (rc != 0) {
		DBG("i2c_init failed %d\r\n", rc);
		goto exit;
	}
	// setup the dac
	rc = cs4x_init(&audio_dac, &audio_dac_cfg);
	if (rc != 0) {
		DBG("cs4x_init failed %d\r\n", rc);
		goto exit;
	}
	// start the dac
	rc = cs4x_play(&audio_dac);
	if (rc != 0) {
		DBG("cs4x_play failed %d\r\n", rc);
		goto exit;
	}
#if 0
	rc = cs4x_beep(&audio_dac);
	if (rc != 0) {
		DBG("cs4x_beep failed %d\r\n", rc);
		goto exit;
	}
#endif

 exit:
	return rc;
}

//-----------------------------------------------------------------------------

static struct osc_lut osc_sin0;
static struct osc_lut osc_sin1;
static struct osc_lut osc_sin2;

static void synth(void) {
	uint8_t notes[3];
	major_chord(notes, 60);

	osc_sin_init(&osc_sin0, midi_to_frequency(notes[0]), AUDIO_SAMPLE_RATE);
	osc_sin_init(&osc_sin1, midi_to_frequency(notes[1]), AUDIO_SAMPLE_RATE);
	osc_sin_init(&osc_sin2, midi_to_frequency(notes[2]), AUDIO_SAMPLE_RATE);

	while (1) {
		float x = lut_sample(&osc_sin0);
		x += lut_sample(&osc_sin1);
		x += lut_sample(&osc_sin2);
		x *= 2000.0f;
		i2s_wr(&audio_i2s, (int16_t) x);
		i2s_wr(&audio_i2s, (int16_t) x);
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

	rc = audio_init();
	if (rc != 0) {
		DBG("audio_init failed %d\r\n", rc);
		goto exit;
	}

	DBG("init good\r\n");

	synth();
	while (1) ;

 exit:
	while (1) ;
	return 0;
}

//-----------------------------------------------------------------------------
