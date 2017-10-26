#!/usr/bin/python

"""

Generate a table of midi note to frequency values.
The table has 32 bit floats in hex representation.

"""

import struct
import math
import sys

def float2hex(x):
  """convert a 32 bit to it's hex value"""
  buf = [ord(c) for c in struct.pack('!f', x)]
  return ''.join(['%02x' % b for b in buf])

def midi2freq(x):
  return 440.0 * math.pow(2.0, float(x - 69) / 12.0)


def main():
  n = 128
  print('static const uint32_t m2f_table[%d] = {' % n)
  for i in range(n):
    if i == 0:
      sys.stdout.write('\t')
    if i != 0 and i % 4 == 0:
      sys.stdout.write('\r\n\t')
    f = midi2freq(i)
    sys.stdout.write('0x%sU, ' % float2hex(f))
  sys.stdout.write('\r\n};\r\n')

main()

