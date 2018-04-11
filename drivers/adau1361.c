//-----------------------------------------------------------------------------
/*

Analog Device ADAU1361 Stereo CODEC

*/
//-----------------------------------------------------------------------------

#include <string.h>

#define DEBUG

#include "adau1361.h"
#include "logging.h"

//-----------------------------------------------------------------------------
// volume controls
// Map 0..255 to the control value for a volume register.
// 0 is minium volume (or mute), 255 is maximum volume.

// set the master volume
int adau1361_master_volume(struct adau1361_drv *codec, uint8_t vol) {
	return 0;
}

//-----------------------------------------------------------------------------

int adau1361_init(struct adau1361_drv *codec, struct adau1361_cfg *cfg) {

	memset(codec, 0, sizeof(struct adau1361_drv));
	codec->cfg = *cfg;

	return 0;
}

//-----------------------------------------------------------------------------

int adau1361_start(struct adau1361_drv *codec) {
	return 0;
}

//-----------------------------------------------------------------------------
