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
static void dma_module_enable(int controller) {
	if (controller == 1) {
		RCC->AHB1ENR |= (1 << 21 /*DMA1EN */ );
	} else if (controller == 2) {
		RCC->AHB1ENR |= (1 << 22 /*DMA2EN */ );
	}
}

//-----------------------------------------------------------------------------

// return a pointer to the dma base registers (1,2)
static inline DMA_TypeDef *dma_base_regs(struct dma_drv *dma) {
	static const uint32_t regs[] = { 0, DMA1_BASE, DMA2_BASE };
	return (DMA_TypeDef *) regs[dma->cfg.controller];
}

// return a pointer to the dma stream registers (0..7)
static inline DMA_Stream_TypeDef *dma_stream_regs(struct dma_drv *dma) {
	static const uint8_t ofs[] = { 0x10, 0x28, 0x40, 0x58, 0x70, 0x88, 0xA0, 0xB8 };
	return (DMA_Stream_TypeDef *) ((uint32_t) dma_base_regs(dma) + ofs[dma->cfg.stream]);
}

// return a pointer to the dma interrupt registers
static inline DMA_Interrupt_TypeDef *dma_interrupt_regs(struct dma_drv *dma) {
	uint8_t ofs = dma->cfg.stream & 4;
	return (DMA_Interrupt_TypeDef *) ((uint32_t) dma_base_regs(dma) + ofs);
}

//-----------------------------------------------------------------------------

// clear dma interrupt flags
static inline void dma_clr_interrupt_flags(struct dma_drv *dma) {
	static const uint8_t shift[] = { 0, 6, 16, 22 };
	dma->iregs->IFCR = 0x3d << shift[dma->cfg.stream & 3];
}

//-----------------------------------------------------------------------------

// setup a peripheral/memory transfer
static void dma_peripheral_transfer(struct dma_drv *dma, uint32_t dst, uint32_t src, uint32_t length) {
	uint32_t dir;
	// clear double buffering
	dma->sregs->CR &= ~(1 << 18 /*DBM*/);
	// set the length to transfer
	dma->sregs->NDTR = length;
	dir = (dma->sregs->CR >> 6 /*DIR*/) & 3;
	if (dir == 0) {
		// peripheral to memory
		dma->sregs->PAR = src;
		dma->sregs->M0AR = dst;
	} else if (dir == 1) {
		// memory to peripheral
		dma->sregs->PAR = dst;
		dma->sregs->M0AR = src;
	}
}

// start a peripheral/memory transfer
int dma_start(struct dma_drv *dma, uint32_t dst, uint32_t src, uint32_t length) {
	dma_peripheral_transfer(dma, dst, src, length);
	dma_enable(dma);
	return 0;
}

//-----------------------------------------------------------------------------

// define the non-reserved register bits
#define DMA_SxCR_MASK   0x0fefffffU
#define DMA_SxNDTR_MASK 0x0000ffffU
#define DMA_SxFCR_MASK  0x000000bfU

int dma_init(struct dma_drv *dma, struct dma_cfg *cfg) {
	uint32_t val;

	memset(dma, 0, sizeof(struct dma_drv));
	cfg->stream &= 7;
	cfg->controller &= 3;
	dma->cfg = *cfg;
	dma->iregs = dma_interrupt_regs(dma);
	dma->sregs = dma_stream_regs(dma);

	// enable the dma controller
	dma_module_enable(dma->cfg.controller);

	// disable the stream to allow modifications
	dma_disable(dma);

	// SxCR setup
	val = (0 << 25 /*CHSEL*/);	// Channel selection (0)
	val |= (0 << 23 /*MBURST*/);	// Memory burst transfer (single)
	val |= (0 << 21 /*PBURST*/);	// Peripheral burst transfer (single)
	val |= (0 << 19 /*CT*/);	// Current target (only in double buffer mode)
	val |= (0 << 18 /*DBM*/);	// Double buffer mode (off)
	val |= (2 << 16 /*PL*/);	// Priority level (high)
	val |= (0 << 15 /*PINCOS*/);	// Peripheral increment offset size (PSIZE)
	val |= (1 << 13 /*MSIZE*/);	// Memory data size (16 bits)
	val |= (1 << 11 /*PSIZE*/);	// Peripheral data size (16 bits)
	val |= (1 << 10 /*MINC*/);	// Memory increment mode (on)
	val |= (0 << 9 /*PINC*/);	// Peripheral increment mode (off)
	val |= (1 << 8 /*CIRC*/);	// Circular mode (on)
	val |= (1 << 6 /*DIR*/);	// Data transfer direction (memory to peripheral)
	val |= (0 << 5 /*PFCTRL*/);	// Peripheral flow controller (dma)
	val |= (0 << 4 /*TCIE*/);	// Transfer complete interrupt enable (off)
	val |= (0 << 3 /*HTIE*/);	// Half transfer interrupt enable (off)
	val |= (0 << 2 /*TEIE*/);	// Transfer error interrupt enable (off)
	val |= (0 << 1 /*DMEIE*/);	// Direct mode error interrupt enable (off)
	val |= (0 << 0 /*EN*/);	// Stream enable / flag stream ready when read low (disabled)
	reg_rmw(&dma->sregs->CR, DMA_SxCR_MASK, val);

	dma->sregs->NDTR = 0;
	dma->sregs->PAR = 0;
	dma->sregs->M0AR = 0;
	dma->sregs->M1AR = 0;

	// SxFCR setup
	val = (0 << 7 /*FEIE*/);	// FIFO error interrupt enable (off)
	val |= (1 << 2 /*DMDIS*/);	// Direct mode disable (disabled)
	val |= (3 << 0 /*FTH*/);	// FIFO threshold selection (full)
	reg_rmw(&dma->sregs->FCR, DMA_SxFCR_MASK, val);

	// clear interrupt flags
	dma_clr_interrupt_flags(dma);

	return 0;
}

//-----------------------------------------------------------------------------
