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

struct dma_cfg {
	int controller;		// dma controller to use
	int stream;		// dma stream to use
};

struct dma_drv {
	DMA_TypeDef *cregs;	// controller registers
	DMA_Stream_TypeDef *sregs;	// stream registers
};

//-----------------------------------------------------------------------------

int dma_init(struct dma_drv *dma, struct dma_cfg *cfg);

//-----------------------------------------------------------------------------

#endif				// DMA_H

//-----------------------------------------------------------------------------
