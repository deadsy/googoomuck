//-----------------------------------------------------------------------------
/*

DMA Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "stm32f4_soc.h"

//-----------------------------------------------------------------------------

// return a pointer to the dma controller registers (1,2)
static inline DMA_TypeDef *dma_controller_regs(int controller) {
	static const uint32_t regs[] = { 0, DMA1_BASE, DMA2_BASE };
	return (DMA_TypeDef *) regs[controller];
}

static inline DMA_Stream_TypeDef *dma_stream_regs(int controller, int stream) {
	static const uint32_t ofs[] = { 0x10, 0x28, 0x40, 0x58, 0x70, 0x88, 0xA0, 0xB8 };
	return (DMA_Stream_TypeDef *) ((uint32_t) dma_controller_regs(controller) + ofs[stream]);
}

//-----------------------------------------------------------------------------

int dma_init(struct dma_drv *dma, struct dma_cfg *cfg) {
	memset(dma, 0, sizeof(struct dma_drv));
	dma->cregs = dma_controller_regs(cfg->controller);
	dma->sregs = dma_stream_regs(cfg->controller, cfg->stream);

	return 0;
}

//-----------------------------------------------------------------------------
