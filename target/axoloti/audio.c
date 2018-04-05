//-----------------------------------------------------------------------------
/*

Audio Control for the STM32F4 Discovery Board

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "audio.h"
#include "ggm.h"
#include "io.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct audio_drv ggm_audio;

//-----------------------------------------------------------------------------
// DMA setup

// errors callback
static void audio_err_callback(struct dma_drv *dma, uint32_t errors) {
	DBG("dma error 0x%08x\r\n", errors);
}

// half transfer callback
static void audio_ht_callback(struct dma_drv *dma, int idx) {
	// dma is reading from the top half, so fill the bottom half
	int rc = event_wr(EVENT_TYPE_AUDIO | AUDIO_BLOCK_SIZE, &ggm_audio.buffer[0]);
	if (rc != 0) {
		DBG("event_wr error for ht callback\r\n");
	}
}

// transfer complete callback
static void audio_tc_callback(struct dma_drv *dma, int idx) {
	// dma is reading from the bottom half, so fill the top half
	int rc = event_wr(EVENT_TYPE_AUDIO | AUDIO_BLOCK_SIZE, &ggm_audio.buffer[HALF_AUDIO_BUFFER_SIZE]);
	if (rc != 0) {
		DBG("event_wr error for tc callback\r\n");
	}
}

// DMA configuration
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
	.src = (uint32_t) ggm_audio.buffer,
	.dst = (uint32_t) & SPI3->DR,
	.nitems = AUDIO_BUFFER_SIZE,
	.err_callback = audio_err_callback,
	.ht_callback = audio_ht_callback,
	.tc_callback = audio_tc_callback,
};

void DMA1_Stream7_IRQHandler(void) {
	dma_isr(&ggm_audio.dma);
}

//-----------------------------------------------------------------------------

// I2S setup
static struct i2s_cfg audio_i2s_cfg = {
	.base = SPI3_BASE,
	.mode = I2S_MODE_MASTER_TX,
	.standard = I2S_STANDARD_PHILIPS,
	.data_format = I2S_DATAFORMAT_16B,
	.mckoe = I2S_MCLKOUTPUT_ENABLE,
	.cpol = I2S_CPOL_LOW,
	.fs = AUDIO_SAMPLE_RATE,
	.dma = I2S_DMA_TX,
};

//-----------------------------------------------------------------------------

// I2C setup
static struct i2c_cfg audio_i2c_cfg = {
	.scl = IO_AUDIO_I2C_SCL,
	.sda = IO_AUDIO_I2C_SDA,
	.delay = 20,
};

//-----------------------------------------------------------------------------

// cs43l22 DAC setup
static struct cs4x_cfg audio_dac_cfg = {
	.adr = 0x94,
	.rst = IO_AUDIO_RESET,
	.out = DAC_OUTPUT_AUTO,
};

//-----------------------------------------------------------------------------

int audio_init(struct audio_drv *audio) {
	int rc = 0;

	// setup the dma to feed the i2s
	rc = dma_init(&audio->dma, &audio_dma_cfg);
	if (rc != 0) {
		DBG("dma_init failed %d\r\n", rc);
		goto exit;
	}
	// Setup DMA1_Stream7 interrupt
	HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 6, 0);

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
	// setup the stats
	memset(&audio->stats, 0, sizeof(struct audio_stats));
	audio->stats.min = AUDIO_BUFFER_SIZE;

	// setup the buffer
	memset(audio->buffer, 0, sizeof(int16_t) * AUDIO_BUFFER_SIZE);

 exit:
	return rc;
}

//-----------------------------------------------------------------------------

int audio_start(struct audio_drv *audio) {
	int rc = 0;

	// Enable DMA1 Stream7 interrupt.
	HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);

	// start the dma
	dma_enable(&audio->dma);

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

// clip and convert samples to the -32768..32767 range.
static int16_t clip_convert(float x) {
	return (int16_t) __SSAT((int32_t) (x * 32767.f), 16);
}

// write l/r channel samples to the audio output buffer
void audio_wr(int16_t * dst, size_t n, float *ch_l, float *ch_r) {
	unsigned int i;
	for (i = 0; i < n; i++) {
		*dst++ = clip_convert(ch_l[i]);
		*dst++ = clip_convert(ch_r[i]);
	}
}

//-----------------------------------------------------------------------------

// report some metrics for realtime audio performance
void audio_stats(struct audio_drv *audio, int16_t * buf) {
	struct audio_stats *stats = &audio->stats;
	// where are we in the DMA buffer?
	int ndtr = (int)dma_ndtr(&audio->dma);
	int margin = -1;

	stats->buffers += 1;

	// We call this just after we have created a new buffer of samples and copied them to the audio buffer.
	// We have to work faster than the DMA is reading the other buffer half.
	// Let's see how close the DMA is to finishing:
	// That gives us a margin for how much we beat the deadline.

	if (buf == audio->buffer) {
		// we just wrote to the lower buffer
		if (ndtr > HALF_AUDIO_BUFFER_SIZE) {
			// dma is reading in the lower half- oops!
			stats->underrun += 1;
		} else {
			// dma is still reading in the top half
			margin = ndtr;
		}
	} else {
		// we just wrote to the upper buffer
		if (ndtr < HALF_AUDIO_BUFFER_SIZE) {
			// dma is reading in the top half- oops!
			stats->underrun += 1;
		} else {
			margin = ndtr - (int)HALF_AUDIO_BUFFER_SIZE;
		}
	}

	if (margin >= 0) {
		// record the last N_MARGINS for an average
		stats->margins[stats->idx] = margin;
		stats->idx = (stats->idx + 1) & (N_MARGINS - 1);
		// record max and min margin
		if (margin < stats->min) {
			stats->min = margin;
		}
		if (margin > stats->max) {
			stats->max = margin;
		}
	}
	// print a periodic stats message
	if ((stats->buffers & ((1 << 10) - 1)) == 0) {
		// work out the average
		unsigned int i;
		int ave = 0;
		for (i = 0; i < N_MARGINS; i++) {
			ave += stats->margins[i];
		}
		ave /= N_MARGINS;
		DBG("margin min %d max %d ave %d underruns %d\r\n", stats->min, stats->max, ave, stats->underrun);
	}
}

//-----------------------------------------------------------------------------

// set the master volume
void audio_master_volume(struct audio_drv *audio, uint8_t vol) {
	DBG("audio_master_volume %d\r\n", vol);
	cs4x_master_volume(&audio->dac, vol);
}

//-----------------------------------------------------------------------------
