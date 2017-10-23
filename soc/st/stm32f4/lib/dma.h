//-----------------------------------------------------------------------------
/*

DMA Driver

*/
//-----------------------------------------------------------------------------

#ifndef DMA_H
#define DMA_H

//-----------------------------------------------------------------------------

#ifndef STM32F4_SOC_H
#warning "please include this file using the toplevel stm32f4_soc.h"
#endif

//-----------------------------------------------------------------------------

typedef struct {
	volatile uint32_t ISR;	// DMA interrupt status register
	volatile uint32_t reserved;
	volatile uint32_t IFCR;	// DMA interrupt flag clear register
} DMA_Interrupt_TypeDef;

//-----------------------------------------------------------------------------

struct dma_cfg {
	int controller;		// dma controller to use
	int stream;		// dma stream to use
	uint32_t par;		// peripheral address register
	uint32_t mar;		// memory address register
};

struct dma_drv {
	struct dma_cfg cfg;
	DMA_Interrupt_TypeDef *iregs;	// interrupt registers
	DMA_Stream_TypeDef *sregs;	// stream registers
};

//-----------------------------------------------------------------------------

// enable a dma stream
static inline void dma_enable(struct dma_drv *dma) {
	dma->sregs->CR |= (1 << 0 /*EN*/);
}

// disable a dma stream
static inline void dma_disable(struct dma_drv *dma) {
	dma->sregs->CR &= ~(1 << 0 /*EN*/);
}

//-----------------------------------------------------------------------------

int dma_init(struct dma_drv *dma, struct dma_cfg *cfg);
int dma_start(struct dma_drv *dma, uint32_t dst, uint32_t src, uint32_t length);

//-----------------------------------------------------------------------------

#endif				// DMA_H

//-----------------------------------------------------------------------------
