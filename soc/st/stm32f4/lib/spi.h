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

// For test/debug puposes we implement a bit-banged SPI driver.
// Don't use this normally - it's inefficient.
//#define SPI_DRIVER_BITBANG
#define SPI_DRIVER_HW

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

#if defined(SPI_DRIVER_HW)

// spi modes
#define SPI_MODE_SLAVE (0U << 2)
#define SPI_MODE_MASTER (1U << 2)

// clock polarity
#define SPI_CPOL_LO (0U << 1)
#define SPI_CPOL_HI (1U << 1)

// clock phase
#define SPI_CPHA_CLK1 (0U << 0)
#define SPI_CPHA_CLK2 (1U << 0)

// data frame format
#define SPI_DFF (1U << 11)

// msb/lsb first
#define SPI_MSB_FIRST (0U << 7)
#define SPI_LSB_FIRST (1U << 7)

// baud rate divisors (F_PCLK/N)
#define SPI_BAUD_DIV2   (0U << 3)
#define SPI_BAUD_DIV4   (1U << 3)
#define SPI_BAUD_DIV8   (2U << 3)
#define SPI_BAUD_DIV16  (3U << 3)
#define SPI_BAUD_DIV32  (4U << 3)
#define SPI_BAUD_DIV64  (5U << 3)
#define SPI_BAUD_DIV128 (6U << 3)
#define SPI_BAUD_DIV256 (7U << 3)

// enable
#define SPI_SPE (1U << 6)

struct spi_cfg {
	uint32_t base;		// base address of spi peripheral
	uint32_t mode;		// master/slave mode
	uint32_t cpol;		// clock polarity
	uint32_t cpha;		// clock phase
	uint32_t lsb;		// msb/lsb first
	uint32_t div;		// baud rate divisor
};

struct spi_drv {
	struct spi_cfg cfg;	// configuration values
	SPI_TypeDef *regs;	// SPI/I2S peripheral registers
	int bits;		// current 8/16 bit mode
};

#elif defined(SPI_DRIVER_BITBANG)

struct spi_cfg {
	int clk;		// clock gpio
	int mosi;		// mosi gpio
	int miso;		// miso gpio
	int cpol;		// clock polarity, 0 = normally low, 1 = normally high
	int cpha;		// clock edge to capture miso on, 0 = 1st edge, 1 = 2nd edge
	int lsb;		// least significant bit first
	int delay;		// clock delay in usecs
};

struct spi_drv {
	struct spi_cfg cfg;
};

#else
#error "what kind of SPI driver are we building?"
#endif

int spi_init(struct spi_drv *spi, struct spi_cfg *cfg);
void spi_wait4_done(struct spi_drv *spi);

void spi_tx8(struct spi_drv *spi, uint8_t data, size_t n);
void spi_tx16(struct spi_drv *spi, uint16_t data, size_t n);
void spi_txbuf8(struct spi_drv *spi, const uint8_t * buf, size_t n);
void spi_txbuf16(struct spi_drv *spi, const uint16_t * buf, size_t n);

void spi_rx8(struct spi_drv *spi, uint8_t * data);
void spi_rxbuf8(struct spi_drv *spi, uint8_t * buf, size_t n);

//-----------------------------------------------------------------------------

#endif				// SPI_H

//-----------------------------------------------------------------------------
