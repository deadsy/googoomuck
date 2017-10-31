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

// IRQ Status Flags
#define DMA_IRQ_FEIF (1U << 0)	// FIFO overrun/underrun
#define DMA_IRQ_DMEIF (1U << 2)	// Direct mode error
#define DMA_IRQ_TEIF (1U << 3)	// Transfer error
#define DMA_IRQ_HTIF (1U << 4)	// Half-transfer
#define DMA_IRQ_TCIF (1U << 5)	// Transfer complete

#define DMA_IRQ_ALL (DMA_IRQ_FEIF |DMA_IRQ_DMEIF |DMA_IRQ_TEIF |DMA_IRQ_HTIF |DMA_IRQ_TCIF)

typedef struct {
	volatile uint32_t ISR;	// DMA interrupt status register
	volatile uint32_t reserved;
	volatile uint32_t IFCR;	// DMA interrupt flag clear register
} DMA_Interrupt_TypeDef;

//-----------------------------------------------------------------------------

#define DMA_CHSEL(x) (((x) & 7) << 25)	// x = 0..7
#define DMA_PSIZE(x) ((((x) >> 4) & 3) << 11)	// x = 8,16,32
#define DMA_MSIZE(x) ((((x) >> 4) & 3) << 13)	// x = 8,16,32
#define DMA_PL(x) (((x) & 3) << 16)	// x = 0..3

#define DMA_MINC_ON (1U << 10)
#define DMA_MINC_OFF 0

#define DMA_PINC_ON (1U << 9)
#define DMA_PINC_OFF 0

#define DMA_CIRC_ON (1U << 8)
#define DMA_CIRC_OFF 0

#define DMA_PINCOS_4 (1U << 15)
#define DMA_PINCOS_PSIZE 0

#define DMA_PFCTRL_DMA (0U << 5)
#define DMA_PFCTRL_PERIPHERAL (1U << 5)

#define DMA_MBURST_INCR1 (0U << 23)
#define DMA_MBURST_INCR4 (1U << 23)
#define DMA_MBURST_INCR8 (2U << 23)
#define DMA_MBURST_INCR16 (3U << 23)

#define DMA_PBURST_INCR1 (0U << 21)
#define DMA_PBURST_INCR4 (1U << 21)
#define DMA_PBURST_INCR8 (2U << 21)
#define DMA_PBURST_INCR16 (3U << 21)

#define DMA_DIR_P2M (0U << 6)	// peripheral to memory
#define DMA_DIR_M2P (1U << 6)	// memory to peripheral
#define DMA_DIR_M2M (2U << 6)	// memory to memory

#define DMA_FIFO_ENABLE (1U << 2)
#define DMA_FIFO_DISABLE (0U << 2)

#define DMA_FTH(x) (((x) & 3) << 0)	// x = 0..3

struct dma_drv {
	DMA_Interrupt_TypeDef *iregs;	// interrupt registers
	DMA_Stream_TypeDef *sregs;	// stream registers
	int stream;		// stream number 0..7
	void (*err_callback) (struct dma_drv * dma, uint32_t errors);	// errors callback
	void (*ht_callback) (struct dma_drv * dma, int idx);	// half transfer callback
	void (*tc_callback) (struct dma_drv * dma, int idx);	// transfer complete callback
};

struct dma_cfg {
	uint32_t controller;	// controller base address
	int stream;		// stream number 0..7
	uint32_t chsel;		// channel selection
	uint32_t pl;		// priority level
	uint32_t dir;		// direction
	uint32_t msize;		// memory data size
	uint32_t psize;		// peripheral data size
	uint32_t mburst;	// memory burst transfer
	uint32_t pburst;	// peripheral burst transfer
	uint32_t minc;		// memory increment
	uint32_t pinc;		// peripheral increment
	uint32_t pincos;	// peripheral increment offset
	uint32_t circ;		// circular mode
	uint32_t pfctrl;	// peripheral flow control
	uint32_t fifo;		// fifo control
	uint32_t fth;		// fifo threshold
	uint32_t src;		// source address
	uint32_t dst;		// destination address
	uint32_t nitems;	// number of items in the buffer
	void (*err_callback) (struct dma_drv * dma, uint32_t errors);	// errors callback
	void (*ht_callback) (struct dma_drv * dma, int idx);	// half transfer callback
	void (*tc_callback) (struct dma_drv * dma, int idx);	// transfer complete callback
};

//-----------------------------------------------------------------------------

// return the current number of data transfer items
static inline uint32_t dma_ndtr(struct dma_drv *dma) {
	return dma->sregs->NDTR;
}

// enable a dma stream
// Note: enable the dma stream *before* enabling the peripheral
static inline void dma_enable(struct dma_drv *dma) {
	dma->sregs->CR |= DMA_SxCR_EN;
}

int dma_init(struct dma_drv *dma, struct dma_cfg *cfg);
int dma_disable(struct dma_drv *dma);
void dma_isr(struct dma_drv *dma);

//-----------------------------------------------------------------------------

#endif				// DMA_H

//-----------------------------------------------------------------------------
