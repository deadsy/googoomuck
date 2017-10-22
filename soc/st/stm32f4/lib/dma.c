//-----------------------------------------------------------------------------
/*

DMA Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#define DEBUG

#include "stm32f4_soc.h"
#include "utils.h"
#include "logging.h"

//-----------------------------------------------------------------------------

#define DMA_SxCR_MASK   0x0fefffffU
#define DMA_SxNDTR_MASK 0x0000ffffU
#define DMA_SxFCR_MASK  0x000000bfU

//-----------------------------------------------------------------------------

// return a pointer to the dma controller registers (1,2)
static inline DMA_TypeDef *dma_controller_regs(int controller) {
	static const uint32_t regs[] = { 0, DMA1_BASE, DMA2_BASE };
	return (DMA_TypeDef *) regs[controller];
}

// return a pointer to the dma stream registers (0..7)
static inline DMA_Stream_TypeDef *dma_stream_regs(int controller, int stream) {
	static const uint32_t ofs[] = { 0x10, 0x28, 0x40, 0x58, 0x70, 0x88, 0xA0, 0xB8 };
	return (DMA_Stream_TypeDef *) ((uint32_t) dma_controller_regs(controller) + ofs[stream]);
}

//-----------------------------------------------------------------------------

// enable the clock to the dma module
static void dma_module_enable(int controller) {
	if (controller == 1) {
		RCC->AHB1ENR |= (1 << 21 /*DMA1EN */ );
	} else if (controller == 2) {
		RCC->AHB1ENR |= (1 << 22 /*DMA2EN */ );
	}
}

//-----------------------------------------------------------------------------

void dma_enable(struct dma_drv *dma) {
	dma->sregs->CR |= (1 << 0 /*EN*/);
}

void dma_disable(struct dma_drv *dma) {
	dma->sregs->CR &= ~(1 << 0 /*EN*/);
}

//-----------------------------------------------------------------------------

int dma_init(struct dma_drv *dma, struct dma_cfg *cfg) {
	uint32_t val;

	memset(dma, 0, sizeof(struct dma_drv));
	dma->cfg = *cfg;
	dma->cregs = dma_controller_regs(dma->cfg.controller);
	dma->sregs = dma_stream_regs(dma->cfg.controller, dma->cfg.stream);

	// enable the dma controller
	dma_module_enable(dma->cfg.controller);

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
	val |= (1 << 4 /*TCIE*/);	// Transfer complete interrupt enable (on)
	val |= (1 << 3 /*HTIE*/);	// Half transfer interrupt enable (on)
	val |= (1 << 2 /*TEIE*/);	// Transfer error interrupt enable (on)
	val |= (1 << 1 /*DMEIE*/);	// Direct mode error interrupt enable (on)
	val |= (0 << 0 /*EN*/);	// Stream enable / flag stream ready when read low (disabled)
	dma->sregs->CR &= ~(1 << 0 /*EN*/);	// allow modifications
	reg_rmw(&dma->sregs->CR, DMA_SxCR_MASK, val);

	dma->sregs->NDTR = 0;
	dma->sregs->PAR = dma->cfg.par;
	dma->sregs->M0AR = dma->cfg.mar;
	dma->sregs->M1AR = 0;

	// SxFCR setup
	val = (1 << 7 /*FEIE*/);	// FIFO error interrupt enable (enabled)
	val |= (1 << 2 /*DMDIS*/);	// Direct mode disable (disabled)
	val |= (3 << 0 /*FTH*/);	// FIFO threshold selection (full)
	reg_rmw(&dma->sregs->FCR, DMA_SxFCR_MASK, val);

	return 0;
}

//-----------------------------------------------------------------------------
