//-----------------------------------------------------------------------------
/*

Cirrus Logic CS43L22 Stereo DAC

*/
//-----------------------------------------------------------------------------

#include <string.h>

#define DEBUG

#include "cs43l22.h"
#include "stm32f4_soc.h"
#include "logging.h"
#include "utils.h"

//-----------------------------------------------------------------------------

#define CS43L22_REG_ID                              0x01
#define CS43L22_REG_Power_Ctl_1                     0x02
#define CS43L22_REG_Power_Ctl_2                     0x04
#define CS43L22_REG_Clocking_Ctl                    0x05
#define CS43L22_REG_Interface_Ctl_1                 0x06
#define CS43L22_REG_Interface_Ctl_2                 0x07
#define CS43L22_REG_Passthrough_A_Select            0x08
#define CS43L22_REG_Passthrough_B_Select            0x09
#define CS43L22_REG_Analog_ZC_and_SR_Settings       0x0A
#define CS43L22_REG_Passthrough_Gang_Control        0x0C
#define CS43L22_REG_Playback_Ctl_1                  0x0D
#define CS43L22_REG_Misc_Ctl                        0x0E
#define CS43L22_REG_Playback_Ctl_2                  0x0F
#define CS43L22_REG_Passthrough_A_Vol               0x14
#define CS43L22_REG_Passthrough_B_Vol               0x15
#define CS43L22_REG_PCMA_Vol                        0x1A
#define CS43L22_REG_PCMB_Vol                        0x1B
#define CS43L22_REG_BEEP_Freq_On_Time               0x1C
#define CS43L22_REG_BEEP_Vol_Off_Time               0x1D
#define CS43L22_REG_BEEP_Tone_Cfg                   0x1E
#define CS43L22_REG_Tone_Ctl                        0x1F
#define CS43L22_REG_Master_A_Vol                    0x20
#define CS43L22_REG_Master_B_Vol                    0x21
#define CS43L22_REG_Headphone_A_Volume              0x22
#define CS43L22_REG_Headphone_B_Volume              0x23
#define CS43L22_REG_Speaker_A_Volume                0x24
#define CS43L22_REG_Speaker_B_Volume                0x25
#define CS43L22_REG_Channel_Mixer_Swap              0x26
#define CS43L22_REG_Limit_Ctl_1_Thresholds          0x27
#define CS43L22_REG_Limit_Ctl_2_Release Rate        0x28
#define CS43L22_REG_Limiter_Attack_Rate             0x29
#define CS43L22_REG_Overflow_Clock_Status           0x2E
#define CS43L22_REG_Battery_Compensation            0x2F
#define CS43L22_REG_VP_Battery_Level                0x30
#define CS43L22_REG_Speaker_Status                  0x31
#define CS43L22_REG_Charge_Pump_Frequency           0x34

//-----------------------------------------------------------------------------

// read a dac register
static int cs4x_rd(struct cs4x_drv *dac, uint8_t reg, uint8_t * val) {
	uint8_t buf[1] = { reg };
	int rc;
	rc = i2c_wr(dac->cfg.i2c, dac->cfg.adr, buf, 1);
	if (rc != 0) {
		return rc;
	}
	rc = i2c_rd(dac->cfg.i2c, dac->cfg.adr, buf, 1);
	if (rc != 0) {
		return rc;
	}
	*val = buf[0];
	return 0;
}

// write a dac register
static int cs4x_wr(struct cs4x_drv *dac, uint8_t reg, uint8_t val) {
	uint8_t buf[2] = { reg, val };
	return i2c_wr(dac->cfg.i2c, dac->cfg.adr, buf, 2);
}

// read/modify/write a register
static int cs4x_rmw(struct cs4x_drv *dac, uint8_t reg, uint8_t mask, uint8_t val) {
	uint8_t x;
	int rc;
	rc = cs4x_rd(dac, reg, &x);
	if (rc != 0) {
		return rc;
	}
	x &= ~mask;
	x |= val & mask;
	return cs4x_wr(dac, reg, x);
}

// set bits in a register
static int cs4x_set(struct cs4x_drv *dac, uint8_t reg, uint8_t bits) {
	return cs4x_rmw(dac, reg, bits, 0xff);
}

// clear bits in a register
static int cs4x_clr(struct cs4x_drv *dac, uint8_t reg, uint8_t bits) {
	return cs4x_rmw(dac, reg, bits, 0);
}

//-----------------------------------------------------------------------------

// read and verify the device id
static int cs4x_id(struct cs4x_drv *dac) {
	uint8_t id;
	int rc;
	rc = cs4x_rd(dac, CS43L22_REG_ID, &id);
	if (rc != 0) {
		return rc;
	}
	if ((id & 0xf8) != 0xe0) {
		return -1;
	}
	return 0;
}

//-----------------------------------------------------------------------------

// set the output device
int cs4x_output(struct cs4x_drv *dac, unsigned int out) {
	const uint8_t ctrl[DAC_OUTPUT_MAX] = { 0xff, 0xfa, 0xaf, 0xaa, 0x05 };
	int rc;
	if (out >= DAC_OUTPUT_MAX) {
		out = DAC_OUTPUT_OFF;
	}
	rc = cs4x_wr(dac, CS43L22_REG_Power_Ctl_2, ctrl[out]);
	if (rc != 0) {
		return rc;
	}
	dac->cfg.out = out;
	return 0;
}

//-----------------------------------------------------------------------------
// volume controls
// Map 0..255 to the control value for a volume register.
// 0 is minium volume (or mute), 255 is maximum volume.

// set the master volume
int cs4x_master_volume(struct cs4x_drv *dac, uint8_t vol) {
	uint32_t x;
	int rc;
	x = (((281 - 52) << 16) / 255) * vol + (52 << 16);
	x >>= 16;
	x &= 255;
	rc = cs4x_wr(dac, CS43L22_REG_Master_A_Vol, x);
	rc |= cs4x_wr(dac, CS43L22_REG_Master_B_Vol, x);
	return rc;
}

// set the headphone volume
int cs4x_headphone_volume(struct cs4x_drv *dac, uint8_t vol) {
	uint32_t x;
	int rc;
	if (vol == 0) {
		x = 1;		// muted
	} else {
		x = (((257 - 52) << 16) / 255) * (vol - 1) + (52 << 16);
		x >>= 16;
		x &= 255;
	}
	rc = cs4x_wr(dac, CS43L22_REG_Headphone_A_Volume, x);
	rc |= cs4x_wr(dac, CS43L22_REG_Headphone_B_Volume, x);
	return rc;
}

// set the speaker volume
int cs4x_speaker_volume(struct cs4x_drv *dac, uint8_t vol) {
	uint32_t x;
	int rc;
	if (vol == 0) {
		x = 1;		// muted
	} else {
		x = (((257 - 64) << 16) / 255) * (vol - 1) + (64 << 16);
		x >>= 16;
		x &= 255;
	}
	rc = cs4x_wr(dac, CS43L22_REG_Speaker_A_Volume, x);
	rc |= cs4x_wr(dac, CS43L22_REG_Speaker_B_Volume, x);
	return rc;
}

// set the pcm volume
int cs4x_pcm_volume(struct cs4x_drv *dac, uint8_t vol) {
	uint32_t x;
	int rc;
	if (vol == 0) {
		x = 0x80;	// muted
	} else {
		x = (((281 - 25) << 16) / (255 - 1)) * (vol - 1) + (25 << 16);
		x >>= 16;
		x &= 255;
	}
	rc = cs4x_wr(dac, CS43L22_REG_PCMA_Vol, x);
	rc |= cs4x_wr(dac, CS43L22_REG_PCMB_Vol, x);
	return rc;
}

//-----------------------------------------------------------------------------
// mute on/off

static int cs4x_mute_on(struct cs4x_drv *dac) {
	int rc = cs4x_wr(dac, CS43L22_REG_Power_Ctl_2, 0xff);
	rc |= cs4x_headphone_volume(dac, 0);
	return rc;
}

static int cs4x_mute_off(struct cs4x_drv *dac) {
	int rc = cs4x_headphone_volume(dac, 0xff);
	rc |= cs4x_output(dac, dac->cfg.out);
	return rc;
}

//-----------------------------------------------------------------------------

int cs4x_init(struct cs4x_drv *dac, struct cs4x_cfg *cfg) {
	int rc;

	memset(dac, 0, sizeof(struct cs4x_drv));
	dac->cfg = *cfg;

	// 4.9 Recommended Power-Up Sequence (1,2)
	// reset the dac
	gpio_clr(dac->cfg.rst);
	gpio_set(dac->cfg.rst);

	rc = cs4x_id(dac);
	if (rc != 0) {
		DBG("cs4x bad device id %d\r\n", rc);
		goto exit;
	}
	// 4.9 Recommended Power-Up Sequence (4)
	// 4.11 Required Initialization Settings
	rc |= cs4x_wr(dac, 0, 0x99);
	rc |= cs4x_wr(dac, 0x47, 0x80);
	rc |= cs4x_set(dac, 0x32, 1 << 7);
	rc |= cs4x_clr(dac, 0x32, 1 << 7);
	rc |= cs4x_wr(dac, 0, 0);

	// set the output to AUTO
	rc |= cs4x_output(dac, DAC_OUTPUT_AUTO);
	// Clock configuration: Auto detection
	rc |= cs4x_wr(dac, CS43L22_REG_Clocking_Ctl, 0x81);
	// Set the Slave Mode and the audio Standard
	rc |= cs4x_wr(dac, CS43L22_REG_Interface_Ctl_1, 0x04);

	// Set the Master volume
	rc |= cs4x_master_volume(dac, 169);

	// If the Speaker is enabled, set the Mono mode and volume attenuation level
	if (dac->cfg.out != DAC_OUTPUT_OFF && dac->cfg.out != DAC_OUTPUT_HEADPHONE) {
		// Set the Speaker Mono mode
		rc |= cs4x_wr(dac, CS43L22_REG_Playback_Ctl_2, 0x06);
		rc |= cs4x_speaker_volume(dac, 0xff);
	}
	// Additional configuration for the CODEC. These configurations are done to reduce
	// the time needed for the Codec to power off. If these configurations are removed,
	// then a long delay should be added between powering off the Codec and switching
	// off the I2S peripheral MCLK clock (which is the operating clock for Codec).
	// If this delay is not inserted, then the codec will not shut down properly and
	// it results in high noise after shut down.

	// Disable the analog soft ramp
	rc |= cs4x_rmw(dac, CS43L22_REG_Analog_ZC_and_SR_Settings, 0x0f, 0x00);
	// Disable the digital soft ramp
	rc |= cs4x_wr(dac, CS43L22_REG_Misc_Ctl, 0x04);
	// Disable the limiter attack level
	rc |= cs4x_wr(dac, CS43L22_REG_Limit_Ctl_1_Thresholds, 0x00);
	// Adjust Bass and Treble levels
	rc |= cs4x_wr(dac, CS43L22_REG_Tone_Ctl, 0x0f);
	// Adjust PCM volume level
	rc |= cs4x_pcm_volume(dac, 241);

 exit:
	return rc;
}

//-----------------------------------------------------------------------------

int cs4x_start(struct cs4x_drv *dac) {
	// Enable the digital soft ramp
	int rc = cs4x_wr(dac, CS43L22_REG_Misc_Ctl, 0x06);
	// Enable Output device
	rc |= cs4x_mute_off(dac);
	// Power on the Codec
	rc |= cs4x_wr(dac, CS43L22_REG_Power_Ctl_1, 0x9e);
	return rc;
}

int cs4x_stop(struct cs4x_drv *dac) {
	// Mute the output
	int rc = cs4x_mute_on(dac);
	// Disable the digital soft ramp
	rc |= cs4x_wr(dac, CS43L22_REG_Misc_Ctl, 0x04);
	// Power down the DAC and the speaker (PMDAC and PMSPK bits)
	rc |= cs4x_wr(dac, CS43L22_REG_Power_Ctl_1, 0x9f);
	return rc;
}

int cs4x_pause(struct cs4x_drv *dac) {
	// Mute the output
	int rc = cs4x_mute_on(dac);
	// Put the Codec in Power save mode
	rc |= cs4x_wr(dac, CS43L22_REG_Power_Ctl_1, 0x01);
	return rc;
}

int cs4x_resume(struct cs4x_drv *dac) {
	// Unmute the output
	int rc = cs4x_mute_off(dac);
	// Power on the Codec
	rc |= cs4x_wr(dac, CS43L22_REG_Power_Ctl_1, 0x9e);
	return rc;
}

//-----------------------------------------------------------------------------
