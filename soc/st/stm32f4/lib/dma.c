//-----------------------------------------------------------------------------
/*

DMA Driver

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#define DEBUG

#include "stm32f4_soc.h"
#include "utils.h"
#include "logging.h"

//-----------------------------------------------------------------------------

// enable the clock to the dma controller module
static void dma_module_enable(uint32_t base) {
	if (base == DMA1_BASE) {
		RCC->AHB1ENR |= (1 << 21 /*DMA1EN */ );
	} else if (base == DMA2_BASE) {
		RCC->AHB1ENR |= (1 << 22 /*DMA2EN */ );
	}
}

//-----------------------------------------------------------------------------

// return a pointer to the dma stream registers (0..7)
static DMA_Stream_TypeDef *dma_stream_regs(uint32_t base, int stream) {
	static const uint8_t ofs[] = { 0x10, 0x28, 0x40, 0x58, 0x70, 0x88, 0xA0, 0xB8 };
	return (DMA_Stream_TypeDef *) (base + ofs[stream]);
}

// return a pointer to the dma interrupt registers
static DMA_Interrupt_TypeDef *dma_interrupt_regs(uint32_t base, int stream) {
	uint8_t ofs = stream & 4;
	return (DMA_Interrupt_TypeDef *) (base + ofs);
}

//-----------------------------------------------------------------------------

// clear dma interrupt flags
static void dma_clr_interrupt_flags(struct dma_drv *dma) {
	static const uint8_t shift[] = { 0, 6, 16, 22 };
	dma->iregs->IFCR = 0x3d << shift[dma->stream & 3];
}

//-----------------------------------------------------------------------------

#define DMA_EN_TIMEOUT 5	// milliseconds

// disable a dma stream
// Note: disable the dma stream *before* disabling the peripheral
int dma_disable(struct dma_drv *dma) {
	uint32_t ticks = HAL_GetTick();
	// disable
	dma->sregs->CR &= ~DMA_SxCR_EN;
	// wait for the enable to read 0
	while ((dma->sregs->CR & DMA_SxCR_EN) != 0) {
		if ((HAL_GetTick() - ticks) > DMA_EN_TIMEOUT) {
			return -1;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------

// define the non-reserved register bits
#define DMA_SxCR_MASK   0x0fefffffU
#define DMA_SxNDTR_MASK 0x0000ffffU
#define DMA_SxFCR_MASK  0x000000bfU

int dma_init(struct dma_drv *dma, struct dma_cfg *cfg) {
	uint32_t val;
	int rc = 0;

	memset(dma, 0, sizeof(struct dma_drv));
	dma->iregs = dma_interrupt_regs(cfg->controller, cfg->stream);
	dma->sregs = dma_stream_regs(cfg->controller, cfg->stream);
	dma->stream = cfg->stream;

	// enable the dma controller
	dma_module_enable(cfg->controller);

	// disable the stream to allow modifications
	rc = dma_disable(dma);
	if (rc != 0) {
		rc = -1;
		goto exit;
	}
	// SxCR setup
	val = cfg->chsel;	// Channel selection
	val |= cfg->mburst;	// Memory burst transfer
	val |= cfg->pburst;	// Peripheral burst transfer
	val |= (0 << 19 /*CT*/);	// Current target (only in double buffer mode)
	val |= (0 << 18 /*DBM*/);	// Double buffer mode (off)
	val |= cfg->pl;		// Priority level
	val |= cfg->pincos;	// Peripheral increment offset
	val |= cfg->msize;	// Memory data size
	val |= cfg->psize;	// Peripheral data size
	val |= cfg->minc;	// Memory increment mode
	val |= cfg->pinc;	// Peripheral increment mode
	val |= cfg->circ;	// Circular mode
	val |= cfg->dir;	// Data transfer direction
	val |= cfg->pfctrl;	// Peripheral flow controller
	// enable all interrupt conditions
	val |= (1 << 4 /*TCIE*/);	// Transfer complete interrupt enable (enabled)
	val |= (1 << 3 /*HTIE*/);	// Half transfer interrupt enable  (enabled)
	val |= (1 << 2 /*TEIE*/);	// Transfer error interrupt enable  (enabled)
	val |= (1 << 1 /*DMEIE*/);	// Direct mode error interrupt enable  (enabled)
	reg_rmw(&dma->sregs->CR, DMA_SxCR_MASK, val);

	// setup the address registers
	if (cfg->dir == DMA_DIR_P2M) {
		// peripheral to memory
		dma->sregs->PAR = cfg->src;
		dma->sregs->M0AR = cfg->dst;
	} else if (cfg->dir == DMA_DIR_M2P) {
		// memory to peripheral
		dma->sregs->PAR = cfg->dst;
		dma->sregs->M0AR = cfg->src;
	} else if (cfg->dir == DMA_DIR_M2M) {
		// TODO memory to memory
	} else {
		rc = -1;
		goto exit;
	}

	dma->sregs->NDTR = cfg->nitems;

	// SxFCR setup
	val = (1 << 7 /*FEIE*/);	// FIFO error interrupt enable (enabled)
	val |= cfg->fifo;	// fifo control (aka DMDIS, direct mode disable)
	val |= cfg->fth;	// FIFO threshold selection
	reg_rmw(&dma->sregs->FCR, DMA_SxFCR_MASK, val);

	// clear interrupt flags
	dma_clr_interrupt_flags(dma);

 exit:
	return rc;
}

//-----------------------------------------------------------------------------
