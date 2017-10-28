//-----------------------------------------------------------------------------
/*

Audio Control for the STM32F4 Discovery Board

*/
//-----------------------------------------------------------------------------

#include "audio.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

static int16_t audio_buffer[128];

//-----------------------------------------------------------------------------
// IO configuration

#define AUDIO_I2C_SCL   GPIO_NUM(PORTB, 6)
#define AUDIO_I2C_SDA   GPIO_NUM(PORTB, 9)
#define AUDIO_RESET     GPIO_NUM(PORTD, 4)
#define AUDIO_I2S_MCK   GPIO_NUM(PORTC, 7)
#define AUDIO_I2S_SCK   GPIO_NUM(PORTC, 10)
#define AUDIO_I2S_SD    GPIO_NUM(PORTC, 12)
#define AUDIO_I2S_WS    GPIO_NUM(PORTA, 4)

static const struct gpio_info gpios[] = {
	{AUDIO_RESET, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{AUDIO_I2C_SCL, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{AUDIO_I2C_SDA, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{AUDIO_I2S_MCK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{AUDIO_I2S_SCK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{AUDIO_I2S_SD, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{AUDIO_I2S_WS, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
};

//-----------------------------------------------------------------------------

// DMA setup
static struct dma_cfg audio_dma_cfg = {
	.controller = DMA1_BASE,
	.stream = 7,
	.chsel = DMA_CHSEL(0),
	.pl = DMA_PL(2),
	.dir = DMA_DIR_M2P,
	.msize = DMA_MSIZE(16),
	.psize = DMA_PSIZE(16),
	.mburst = DMA_MBURST_INCR1,
	.pburst = DMA_PBURST_INCR1,
	.minc = DMA_MINC_ON,
	.pinc = DMA_PINC_OFF,
	.circ = DMA_CIRC_ON,
	.pfctrl = DMA_PFCTRL_DMA,
	.fifo = DMA_FIFO_ENABLE,
	.fth = DMA_FTH(3),
	.src = (uint32_t) audio_buffer,
	.dst = (uint32_t) & SPI3->DR,
	.nitems = sizeof(audio_buffer) / sizeof(int16_t),
};

// I2S setup
static struct i2s_cfg audio_i2s_cfg = {
	.idx = 3,
	.mode = I2S_MODE_MASTER_TX,
	.standard = I2S_STANDARD_PHILIPS,
	.data_format = I2S_DATAFORMAT_16B,
	.mckoe = I2S_MCLKOUTPUT_ENABLE,
	.cpol = I2S_CPOL_LOW,
	.fs = AUDIO_SAMPLE_RATE,
};

// I2C setup
static struct i2c_cfg audio_i2c_cfg = {
	.scl = AUDIO_I2C_SCL,
	.sda = AUDIO_I2C_SDA,
	.delay = 20,
};

// cs43l22 DAC setup
static struct cs4x_cfg audio_dac_cfg = {
	.adr = 0x94,
	.rst = AUDIO_RESET,
	.out = DAC_OUTPUT_AUTO,
};

//-----------------------------------------------------------------------------

int audio_init(struct audio_drv *audio) {
	int rc = 0;

	// setup the io pins
	rc = gpio_init(gpios, sizeof(gpios) / sizeof(struct gpio_info));
	if (rc != 0) {
		DBG("gpio_init failed %d\r\n", rc);
		goto exit;
	}
	// setup the dma to feed the i2s
	rc = dma_init(&audio->dma, &audio_dma_cfg);
	if (rc != 0) {
		DBG("dma_init failed %d\r\n", rc);
		goto exit;
	}
	// Setup the i2s pll to generate i2s_clk
	rc = set_i2sclk(AUDIO_SAMPLE_RATE);
	if (rc != 0) {
		DBG("i2sclk_init failed %d\r\n", rc);
		goto exit;
	}
	DBG("i2sclk %d Hz\r\n", get_i2sclk());

	// setup the i2s interface
	rc = i2s_init(&audio->i2s, &audio_i2s_cfg);
	if (rc != 0) {
		DBG("i2s_init failed %d\r\n", rc);
		goto exit;
	}
	// setup the i2c bus used to control the dac
	rc = i2c_init(&audio->i2c, &audio_i2c_cfg);
	if (rc != 0) {
		DBG("i2c_init failed %d\r\n", rc);
		goto exit;
	}
	// setup the dac
	audio_dac_cfg.i2c = &audio->i2c;
	rc = cs4x_init(&audio->dac, &audio_dac_cfg);
	if (rc != 0) {
		DBG("cs4x_init failed %d\r\n", rc);
		goto exit;
	}

 exit:
	return rc;
}

//-----------------------------------------------------------------------------

int audio_start(struct audio_drv *audio) {
	int rc = 0;

	// start the i2s clocking
	i2s_enable(&audio->i2s);
	DBG("fs %d Hz\r\n", i2s_get_fsclk(&audio->i2s));

	// start the dac
	rc = cs4x_start(&audio->dac);
	if (rc != 0) {
		DBG("cs4x_start failed %d\r\n", rc);
		goto exit;
	}

 exit:
	return rc;
}

//-----------------------------------------------------------------------------

// write l/r channel samples (32-bit float) to the audio output
void audio_wr(struct audio_drv *audio, float ch_l, float ch_r) {
	int16_t xl, xr;
	// clip left channel
	ch_l = (ch_l > 1.0f) ? 1.0f : ch_l;
	ch_l = (ch_l < -1.0f) ? -1.0f : ch_l;
	// clip right channel
	ch_r = (ch_r > 1.0f) ? 1.0f : ch_r;
	ch_r = (ch_r < -1.0f) ? -1.0f : ch_r;
	// convert to a signed integer
	xl = (int16_t) (ch_l * 32767.0f);
	xr = (int16_t) (ch_r * 32767.0f);
	// write the samples to the i2s
	i2s_wr(&audio->i2s, xl);
	i2s_wr(&audio->i2s, xr);
}

//-----------------------------------------------------------------------------

// set the master volume
void audio_master_volume(struct audio_drv *audio, uint8_t vol) {
	DBG("audio_master_volume %d\r\n", vol);
	cs4x_master_volume(&audio->dac, vol);
}

//-----------------------------------------------------------------------------
