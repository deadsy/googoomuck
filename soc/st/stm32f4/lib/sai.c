//-----------------------------------------------------------------------------
/*

Serial Audio Interface (SAI) Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "stm32f4_soc.h"
#include "utils.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// define the non-reserved register bits
#define SAI_CR1_MASK (0xfb3fefU)
#define SAI_CR2_MASK (0xffffU)
#define SAI_FRCR_MASK (0x67fffU)
#define SAI_SLOTR_MASK (0xffff0fdfU)
#define SAI_IMR_MASK (0x7fU)
#define SAI_CLRFR_MASK (0x77U)

int sai_init(struct sai_drv *sai, struct sai_cfg *cfg) {
	uint32_t val;

	memset(sai, 0, sizeof(struct sai_drv));
	sai->cfg = *cfg;
	sai->global = (SAI_TypeDef *) cfg->base;
	sai->blocka = (SAI_Block_TypeDef *) (cfg->base + 0x04);
	sai->blockb = (SAI_Block_TypeDef *) (cfg->base + 0x24);

	// global

	// block a

	// setup CR1
	val = 0;
	val |= (0 << 20 /*MCJDIV*/);	// Master clock divider
	val |= (0 << 19 /*NODIV*/);	// No divider
	val |= (0 << 17 /*DMAEN*/);	// DMA enable
	val |= (0 << 16 /*SAIAEN*/);	// Audio block A enable
	val |= (0 << 13 /*OutDri */ );	// Output drive
	val |= (0 << 12 /*MONO*/);	// Mono mode
	val |= (0 << 10 /*SYNCEN*/);	// Synchronization enable
	val |= (0 << 9 /*CKSTR*/);	// Clock strobing edge
	val |= (0 << 8 /*LSBFIRST*/);	// Least significant bit first
	val |= (0 << 5 /*DS*/);	// Data size
	val |= (0 << 2 /*PRTCFG*/);	// Protocol configuration
	val |= (0 << 0 /*MODE*/);	// Audio block mode
	reg_rmw(&sai->blocka->CR1, SAI_CR1_MASK, val);

	// setup CR2
	val = 0;
	val |= (0 << 14 /*COMP*/);	// Companding mode
	val |= (0 << 13 /*CPL*/);	// Complement bit
	val |= (0 << 7 /*MUTECN*/);	// Mute counter
	val |= (0 << 6 /*MUTEVAL*/);	// Mute value
	val |= (0 << 5 /*MUTE*/);	// Mute
	val |= (0 << 4 /*TRIS*/);	// Tristate management on data line
	val |= (0 << 3 /*FFLUS*/);	// FIFO flush
	val |= (0 << 0 /*FTH*/);	// FIFO threshold
	reg_rmw(&sai->blocka->CR2, SAI_CR2_MASK, val);

	// setup FRCR
	val = 0;
	val |= (0 << 18 /*FSOFF*/);	// Frame synchronization offset
	val |= (0 << 17 /*FSPOL*/);	// Frame synchronization polarity
	val |= (0 << 16 /*FSDEF*/);	// Frame synchronization definition
	val |= (0 << 8 /*FSALL*/);	// Frame synchronization active level length
	val |= (0 << 0 /*FRL*/);	// Frame length
	reg_rmw(&sai->blocka->FRCR, SAI_FRCR_MASK, val);

	// setup SLOTR
	val = 0;
	val |= (0 << 16 /*SLOTEN*/);	// Slot enable
	val |= (0 << 8 /*NBSLOT*/);	// Number of slots in an audio frame
	val |= (0 << 6 /*SLOTSZ*/);	// Slot size
	val |= (0 << 0 /*FBOFF*/);	// First bit offset
	reg_rmw(&sai->blocka->SLOTR, SAI_SLOTR_MASK, val);

	// setup IMR
	val = 0;
	val |= (0 << 6 /*LFSDET*/);	// Late frame synchronization detection interrupt enable
	val |= (0 << 5 /*AFSDETIE*/);	// Anticipated frame synchronization detection interrupt enable
	val |= (0 << 4 /*CNRDYIE*/);	// Codec not ready interrupt enable
	val |= (0 << 3 /*FREQIE*/);	// FIFO request interrupt enable
	val |= (0 << 2 /*WCKCFG*/);	// Wrong clock configuration interrupt enable
	val |= (0 << 1 /*MUTEDET*/);	// Mute detection interrupt enable
	val |= (0 << 0 /*OVRUDRIE*/);	// Overrun/underrun interrupt enable
	reg_rmw(&sai->blocka->IMR, SAI_IMR_MASK, val);

	// setup CLRFR
	val = 0;
	val |= (1 << 6 /*LFSDET*/);	// Clear late frame synchronization detection flag
	val |= (1 << 5 /*CAFSDET*/);	// Clear anticipated frame synchronization detection flag
	val |= (1 << 4 /*CNRDY*/);	// Clear codec not ready flag
	val |= (1 << 2 /*CKCFG*/);	// Clear wrong clock configuration flag
	val |= (1 << 1 /*MUTEDET*/);	// Mute detection flag
	val |= (1 << 0 /*OVRUDR*/);	// Clear overrun / underrun
	reg_rmw(&sai->blocka->CLRFR, SAI_CLRFR_MASK, val);

	// DR?

	// block b

	// setup CR1
	val = 0;
	val |= (0 << 20 /*MCJDIV*/);	// Master clock divider
	val |= (0 << 19 /*NODIV*/);	// No divider
	val |= (0 << 17 /*DMAEN*/);	// DMA enable
	val |= (0 << 16 /*SAIAEN*/);	// Audio block A enable
	val |= (0 << 13 /*OutDri */ );	// Output drive
	val |= (0 << 12 /*MONO*/);	// Mono mode
	val |= (0 << 10 /*SYNCEN*/);	// Synchronization enable
	val |= (0 << 9 /*CKSTR*/);	// Clock strobing edge
	val |= (0 << 8 /*LSBFIRST*/);	// Least significant bit first
	val |= (0 << 5 /*DS*/);	// Data size
	val |= (0 << 2 /*PRTCFG*/);	// Protocol configuration
	val |= (0 << 0 /*MODE*/);	// Audio block mode
	reg_rmw(&sai->blockb->CR1, SAI_CR1_MASK, val);

	// setup CR2
	val = 0;
	val |= (0 << 14 /*COMP*/);	// Companding mode
	val |= (0 << 13 /*CPL*/);	// Complement bit
	val |= (0 << 7 /*MUTECN*/);	// Mute counter
	val |= (0 << 6 /*MUTEVAL*/);	// Mute value
	val |= (0 << 5 /*MUTE*/);	// Mute
	val |= (0 << 4 /*TRIS*/);	// Tristate management on data line
	val |= (0 << 3 /*FFLUS*/);	// FIFO flush
	val |= (0 << 0 /*FTH*/);	// FIFO threshold
	reg_rmw(&sai->blockb->CR2, SAI_CR2_MASK, val);

	// setup FRCR
	val = 0;
	val |= (0 << 18 /*FSOFF*/);	// Frame synchronization offset
	val |= (0 << 17 /*FSPOL*/);	// Frame synchronization polarity
	val |= (0 << 16 /*FSDEF*/);	// Frame synchronization definition
	val |= (0 << 8 /*FSALL*/);	// Frame synchronization active level length
	val |= (0 << 0 /*FRL*/);	// Frame length
	reg_rmw(&sai->blockb->FRCR, SAI_FRCR_MASK, val);

	// setup SLOTR
	val = 0;
	val |= (0 << 16 /*SLOTEN*/);	// Slot enable
	val |= (0 << 8 /*NBSLOT*/);	// Number of slots in an audio frame
	val |= (0 << 6 /*SLOTSZ*/);	// Slot size
	val |= (0 << 0 /*FBOFF*/);	// First bit offset
	reg_rmw(&sai->blockb->SLOTR, SAI_SLOTR_MASK, val);

	// setup IMR
	val = 0;
	val |= (0 << 6 /*LFSDET*/);	// Late frame synchronization detection interrupt enable
	val |= (0 << 5 /*AFSDETIE*/);	// Anticipated frame synchronization detection interrupt enable
	val |= (0 << 4 /*CNRDYIE*/);	// Codec not ready interrupt enable
	val |= (0 << 3 /*FREQIE*/);	// FIFO request interrupt enable
	val |= (0 << 2 /*WCKCFG*/);	// Wrong clock configuration interrupt enable
	val |= (0 << 1 /*MUTEDET*/);	// Mute detection interrupt enable
	val |= (0 << 0 /*OVRUDRIE*/);	// Overrun/underrun interrupt enable
	reg_rmw(&sai->blockb->IMR, SAI_IMR_MASK, val);

	// setup CLRFR
	val = 0;
	val |= (1 << 6 /*LFSDET*/);	// Clear late frame synchronization detection flag
	val |= (1 << 5 /*CAFSDET*/);	// Clear anticipated frame synchronization detection flag
	val |= (1 << 4 /*CNRDY*/);	// Clear codec not ready flag
	val |= (1 << 2 /*CKCFG*/);	// Clear wrong clock configuration flag
	val |= (1 << 1 /*MUTEDET*/);	// Mute detection flag
	val |= (1 << 0 /*OVRUDR*/);	// Clear overrun / underrun
	reg_rmw(&sai->blockb->CLRFR, SAI_CLRFR_MASK, val);

	// DR?

	return 0;
}

//-----------------------------------------------------------------------------
