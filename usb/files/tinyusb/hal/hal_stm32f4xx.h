/**************************************************************************/
/*!
    @file     hal_stm32f407.h
    @author   sirmanlypowers@gmail.com

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2017, Jason T. Harris
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    This file is part of the tinyusb stack.
*/
/**************************************************************************/

#ifndef _TUSB_HAL_STM32F407_H_
#define _TUSB_HAL_STM32F407_H_

#include "stm32f407xx.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
	static inline void hal_interrupt_enable(uint8_t coreid) {
		// TODO
		(void)coreid;	// discard compiler's warning
		//NVIC_EnableIRQ(USB_IRQn);
	} static inline void hal_interrupt_disable(uint8_t coreid) {
		// TODO
		(void)coreid;	// discard compiler's warning
		//NVIC_DisableIRQ(USB_IRQn);
	}

#ifdef __cplusplus
}
#endif

#endif				/* _TUSB_HAL_STM32F407_H_ */
