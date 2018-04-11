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

#define ADAU1361_REG_Clock_Ctl              0x4000
#define ADAU1361_REG_PLL_Ctl                0x4002
#define ADAU1361_REG_Mic_Jack_Detect        0x4008
#define ADAU1361_REG_Rec_Power_Mgmt         0x4009
#define ADAU1361_REG_Rec_Mixer_Left0        0x400A
#define ADAU1361_REG_Rec_Mixer_Left1        0x400B
#define ADAU1361_REG_Rec_Mixer_Right0       0x400C
#define ADAU1361_REG_Rec_Mixer_Right1       0x400D
#define ADAU1361_REG_Left_Diff_Input_Vol    0x400E
#define ADAU1361_REG_Right_Diff_Input_Vol   0x400F
#define ADAU1361_REG_Record_Mic_Bias        0x4010
#define ADAU1361_REG_ALC0                   0x4011
#define ADAU1361_REG_ALC1                   0x4012
#define ADAU1361_REG_ALC2                   0x4013
#define ADAU1361_REG_ALC3                   0x4014
#define ADAU1361_REG_Serial_Port0           0x4015
#define ADAU1361_REG_Serial_Port1           0x4016
#define ADAU1361_REG_Converter0             0x4017
#define ADAU1361_REG_Converter1             0x4018
#define ADAU1361_REG_ADC_Ctl                0x4019
#define ADAU1361_REG_Left_Digital_Vol       0x401A
#define ADAU1361_REG_Right_Digital_Vol      0x401B
#define ADAU1361_REG_Play_Mixer_Left0       0x401C
#define ADAU1361_REG_Play_Mixer_Left1       0x401D
#define ADAU1361_REG_Play_Mixer_Right0      0x401E
#define ADAU1361_REG_Play_Mixer_Right1      0x401F
#define ADAU1361_REG_Play_LR_Mixer_Left     0x4020
#define ADAU1361_REG_Play_LR_Mixer_Right    0x4021
#define ADAU1361_REG_Play_LR_Mixer_Mono     0x4022
#define ADAU1361_REG_Play_HP_Left_Vol       0x4023
#define ADAU1361_REG_Play_HP_Right_Vol      0x4024
#define ADAU1361_REG_Line_Output_Left_Vol   0x4025
#define ADAU1361_REG_Line_Output_Right_Vol  0x4026
#define ADAU1361_REG_Play_Mono_Output       0x4027
#define ADAU1361_REG_Pop_Click_Suppress     0x4028
#define ADAU1361_REG_Play_Power_Mgmt        0x4029
#define ADAU1361_REG_DAC_Ctl0               0x402A
#define ADAU1361_REG_DAC_Ctl1               0x402B
#define ADAU1361_REG_DAC_Ctl2               0x402C
#define ADAU1361_REG_Serial port_Pad        0x402D
#define ADAU1361_REG_Ctl_Port_Pad0          0x402F
#define ADAU1361_REG_Ctl_Port_Pad1          0x4030
#define ADAU1361_REG_Jack_Detect_Pin        0x4031
#define ADAU1361_REG_Dejitter_Ctl           0x4036

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
