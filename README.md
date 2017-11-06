# GooGooMuck
*When the sun goes down, and the moon comes up...*

*I turn into a teenage goo goo muck.*

## What Is It?

GGM is a software based sound synthesizer that runs on the STM32F4 Discovery Board.

http://www.st.com/en/evaluation-tools/stm32f4discovery.html

The synth and platfom code have been partitioned for ease of portability.
The synth code is written using floating point, so a target CPU should
have a 32 bit (single precision) FPU. E.g. ARM Cortex M4 or better.

## Specifications
 * 44100 samples/second (per channel)
 * 16 bits/sample 
 * stereo output
 * 32-bit floats for internal operations

## Source Layout
 * common - common souces (target/SoC independent)
 * drivers - device drivers (non SoC)
 * ggm - GooGooMuck synthesizer (target/SoC independent)
 * mk - make scripts
 * scripts - python scripts 
 * soc - SoC drivers and definitions
 * target - target specific code
 * usb - tinyusb stack (ported to the STM32F4)

## Inspirations
* https://www.quinapalus.com/goom.html
* https://github.com/MrBlueXav/Dekrispator_v2
* https://github.com/hotchk155/Voici-Le-Strum


