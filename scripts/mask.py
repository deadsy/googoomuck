#!/usr/bin/python

def mask(name, bits):
  val = 0
  for (n, s) in bits:
    val |= ((1 << n) - 1) << s
  print('#define %s (0x%xU)' % (name, val))

mask('ADC_CCR_MASK', ((5,0),(4,8),(5,13),(2,22)))
mask('ADC_SR_MASK', ((6,0),))
mask('ADC_CR1_MASK', ((16,0),(5,22)))
mask('ADC_CR2_MASK', ((2,0),(4,8),(7,16),(7,24)))
mask('ADC_SMPR1_MASK', ((27,0),))
mask('ADC_SMPR2_MASK', ((30,0),))
mask('ADC_JOFRx_MASK', ((12,0),))
mask('ADC_HTR_MASK', ((12,0),))
mask('ADC_LTR_MASK', ((12,0),))
mask('ADC_SQR1_MASK', ((24,0),))
mask('ADC_SQR2_MASK', ((30,0),))
mask('ADC_SQR3_MASK', ((30,0),))
mask('ADC_JSQR_MASK', ((22,0),))

mask('SPI_I2SCFGR_MASK', ((6,0),(5,7)))
mask('SPI_I2SPR_MASK', ((10, 0),))
mask('SPI_CR2_MASK', ((4,4),(3,0)))


mask('USART_SR_MASK', ((10,0),))
mask('USART_CR1_MASK', ((14,0),(1,15),))
mask('USART_CR2_MASK', ((4,0),(2,5), (7,8),))
mask('USART_CR3_MASK', ((12,0),))
mask('USART_GTPR_MASK', ((16,0),))

mask('RNG_CR_MASK', ((2,2),))
mask('RNG_SR_MASK', ((3,0),(2,5),))

