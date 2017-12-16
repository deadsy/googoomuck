//-----------------------------------------------------------------------------
/*

SPI/I2S Driver

*/
//-----------------------------------------------------------------------------

#ifndef SPI_H
#define SPI_H

//-----------------------------------------------------------------------------

#ifndef STM32F4_SOC_H
#warning "please include this file using the toplevel stm32f4_soc.h"
#endif

//-----------------------------------------------------------------------------
// I2S Defines

// operating modes (I2SCFG)
#define I2S_MODE_SLAVE_TX (0U << 8)
#define I2S_MODE_SLAVE_RX (1U << 8)
#define I2S_MODE_MASTER_TX (2U << 8)
#define I2S_MODE_MASTER_RX (3U << 8)

// i2s standard (I2SSTD, PCMSYNC)
#define I2S_STANDARD_PHILIPS (0U << 4)
#define I2S_STANDARD_MSB (1U << 4)
#define I2S_STANDARD_LSB (2U << 4)
#define I2S_STANDARD_PCM_SHORT ((3U << 4) | (0U << 7))
#define I2S_STANDARD_PCM_LONG ((3U << 4) | (1U << 7))

// data format (DATLEN, CHLEN)
#define I2S_DATAFORMAT_16B ((0U << 1) | (0U << 0))
#define I2S_DATAFORMAT_16B_EXTENDED ((0U << 1) | (1U << 0))
#define I2S_DATAFORMAT_24B ((1U << 1) | (1U << 1))
#define I2S_DATAFORMAT_32B ((2U << 1) | (1U << 1))

// clock polarity (CKPOL)
#define I2S_CPOL_LOW (0U << 3)
#define I2S_CPOL_HIGH (1U << 3)

// master clock output enable
#define I2S_MCLKOUTPUT_ENABLE (1U << 9)
#define I2S_MCLKOUTPUT_DISABLE (0U << 9)

// dma control
#define I2S_DMA_OFF ((0U << 1) | (0U << 0))
#define I2S_DMA_TX ((1U << 1) | (0U << 0))
#define I2S_DMA_RX ((0U << 1) | (1U << 0))
#define I2S_DMA_RXTX ((1U << 1) | (1U << 0))

//-----------------------------------------------------------------------------

// define non-reserved register bits
#define CR1_MASK (0xffffU)
#define CR2_MASK (0xf7U)
#define SR_MASK (0x1ffU)
#define DR_MASK (0xffffU)
#define CRCPR_MASK (0xffffU)
#define RXCRCR_MASK (0xffffU)
#define TXCRCR_MASK (0xffffU)
#define I2SCFGR_MASK (0xfbfU)
#define I2SPR_MASK (0x3ffU)

//-----------------------------------------------------------------------------
// General

void spi_enable(uint32_t base);

//-----------------------------------------------------------------------------
// I2S Driver API

struct i2s_cfg {
	uint32_t base;		// base address of spi/i2s peripheral
	uint32_t mode;		// operating mode
	uint32_t standard;	// standard used
	uint32_t data_format;	// data format
	uint32_t mckoe;		// is the master clock output enabled?
	uint32_t cpol;		// clock polarity
	uint32_t fs;		// audio sample rate
	uint32_t dma;		// dma control
};

struct i2s_drv {
	struct i2s_cfg cfg;	// configuration values
	SPI_TypeDef *regs;	// SPI/I2S peripheral registers
};

static inline void i2s_enable(struct i2s_drv *i2s) {
	i2s->regs->I2SCFGR |= (1 << 10);	// I2SE
}

static inline void i2s_disable(struct i2s_drv *i2s) {
	i2s->regs->I2SCFGR &= ~(1 << 10);	// I2SE
}

int i2s_init(struct i2s_drv *i2s, struct i2s_cfg *cfg);
uint32_t i2s_get_fsclk(struct i2s_drv *i2s);
//int i2s_wr(struct i2s_drv *i2s, int16_t val);

int set_i2sclk(uint32_t fs);
uint32_t get_i2sclk(void);

//-----------------------------------------------------------------------------
// SPI Driver API

struct spi_cfg {
	uint32_t base;		// base address of spi peripheral
};

struct spi_drv {
	SPI_TypeDef *regs;	// SPI/I2S peripheral registers
};

int spi_init(struct spi_drv *spi, struct spi_cfg *cfg);
int spi_rd(struct spi_drv *spi, uint16_t * data);
void spi_rd_block(struct spi_drv *spi, uint16_t * data);
int spi_wr(struct spi_drv *spi, uint16_t data);
void spi_wr_block(struct spi_drv *spi, uint16_t data);

//-----------------------------------------------------------------------------

#endif				// SPI_H

//-----------------------------------------------------------------------------
