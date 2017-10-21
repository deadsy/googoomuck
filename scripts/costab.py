#!/usr/bin/python

"""

Generate a table of cosine value.
The table has 32 bit floats in hex representation.

"""

import struct
import math
import sys

def float2hex(x):
  """convert a 32 bit to it's hex value"""
  buf = [ord(c) for c in struct.pack('!f', x)]
  return ''.join(['%02x' % b for b in buf])

def main():
  n = 64
  print('#define COS_TABLE_SIZE %dU' % n)
  print('static const uint32_t cos_table[COS_TABLE_SIZE] = {')
  for i in range(n):
    if i == 0:
      sys.stdout.write('\t')
    if i != 0 and i % 4 == 0:
      sys.stdout.write('\r\n\t')
    y = math.cos(float(i) * 2.0 * math.pi / float(n))
    sys.stdout.write('0x%sU, ' % float2hex(y))
  sys.stdout.write('\r\n};\r\n')

main()

