#!/usr/bin/python

HSE_VALUE = 8000000
PLLM = 8

def i2s_clk(plli2sn, plli2sr):
  return (HSE_VALUE / PLLM) * plli2sn / plli2sr

def fsample(clk, chlen, mckoe, div, odd):
  if mckoe is True:
    if chlen == 16:
      return clk / (32*((2*div)+odd)*8)
    else:
      return clk / (64*((2*div)+odd)*4)
  else:
    if chlen == 16:
      return clk / (32*((2*div)+odd))
    else:
      return clk / (64*((2*div)+odd))

def main():
  clk = i2s_clk(290, 2)
  print("i2s_clk %d" % clk)
  chlen = 16
  mckoe = True
  odd = 1
  div = 6
  print("fs %d" % fsample(clk, chlen, mckoe, div, odd))

  clk = i2s_clk(192, 2)
  print("fs %d" % fsample(clk, 16, False, 187, 1))

  clk = i2s_clk(271, 2)
  print("fs %d" % fsample(clk, 16, True, 6, 0))



main()

