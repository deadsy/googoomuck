#!/usr/bin/python

"""

Given a target audio sampling rate work out the parameters used to setup:
 * the I2S subsystem clock
 * the I2S configuration to generate SCK and MCK to the codec.

 Specifically:
 Input: Fs, MCKOE, CHLEN
 Output: PLLI2SN, PLLI2SR, I2SDIV, I2SODD

"""

import math

HSE_VALUE = 8000000.0
PLLM = 8.0

def i2s_clk(sn, sr):
  if sn < 2 or sn > 432:
    return -1 
  if sr < 2 or sr > 7:
    return -1
  return ((HSE_VALUE / PLLM) * float(sn)) / float(sr)

def fsample(clk, chlen, mckoe, div, odd):
  if chlen not in (16, 32):
    return -1
  if mckoe not in (0, 1):
    return -1
  if div < 2 or div > 255 :
    return -1
  if odd not in (0, 1):
    return -1
  x = (2.0 * float(div)) + float(odd)
  if mckoe:
      return clk / (256.0 * x)
  else:
    if chlen == 16:
      return clk / (32.0 * x)
    else:
      return clk / (64.0 * x)
  return -1

def fs_tolerance(a, b):
  return (math.fabs(a - b) / a) < 0.001  

def build_results():
  """vary the parameters to get a list of all possible results"""
  results = []
  for sn in range(2, 433):
    for sr in range(2, 8):
      clk =  i2s_clk(sn, sr)
      if clk > 0:
        for chlen in (16,32):
          for mckoe in (0,1):
            for div in range(2, 256):
              for odd in (0,1):
                fs = fsample(clk, chlen, mckoe, div, odd)
                if fs > 0:
                  results.append((fs, sn, sr, chlen, mckoe, div, odd))
  # sort results by fs
  results.sort()
  return results

def filter_results(results, fs, chlen, mckoe):
  """filter results by fs, mckoe, chlen"""
  filtered = []
  for r in results:
    if chlen == r[3] and mckoe == r[4] and fs_tolerance(fs, r[0]):
      filtered.append(r)
  return filtered

def main():
  results = build_results()
  results = filter_results(results, 35156.25, 16, 1)
  for (fs, sn, sr, chlen, mckoe, div, odd) in results:
    print('%f: sn %d sr %d div %d odd %d mckoe %d chlen %d' % (fs, sn, sr, div, odd, mckoe, chlen))

main()
