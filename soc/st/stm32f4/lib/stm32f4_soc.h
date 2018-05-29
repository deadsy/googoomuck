//-----------------------------------------------------------------------------
/*

STM32F4 SoC top-level include file

*/
//-----------------------------------------------------------------------------

#ifndef STM32F4_SOC_H
#define STM32F4_SOC_H

//-----------------------------------------------------------------------------

#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "delay.h"
#include "i2c.h"
#include "spi.h"
#include "dma.h"
#include "irq.h"
#include "adc.h"
#include "usart.h"
#include "rng.h"

#if defined(STM32F427xx)
#include "sai.h"
#endif

//-----------------------------------------------------------------------------

#endif				// STM32F4_SOC_H

//-----------------------------------------------------------------------------
