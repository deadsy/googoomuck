//-----------------------------------------------------------------------------
/*

Serial Audio Interface (SAI) Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "stm32f4_soc.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

int sai_init(struct sai_drv *sai, struct sai_cfg *cfg) {

	memset(sai, 0, sizeof(struct sai_drv));
	sai->cfg = *cfg;

	return 0;
}

//-----------------------------------------------------------------------------
