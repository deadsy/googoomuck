#!/usr/bin/python

import math
import sys

def gen_table(name, n, func):
  print('static const uint16_t %s[%s] = {' % (name, n))
  for i in range(n):
    if i == 0:
      sys.stdout.write('\t')
    if i != 0 and i % 16 == 0:
      sys.stdout.write('\r\n\t')
    sys.stdout.write('0x%x, ' % func(i))
  sys.stdout.write('\r\n};\r\n')


def f_exp0(i, n):
  x = float(i)/float(n)
  y = math.pow(2.0, x)
  return int(round(y * (1 << 15), 0))

def f_exp1(i, n):
  x = float(i)/float(n * n)
  y = math.pow(2.0, x)
  return int(round(y * (1 << 15), 0))

def main():
  gen_table('exp0_table', 64, lambda i: f_exp0(i, 64))
  gen_table('exp1_table', 64, lambda i: f_exp1(i, 64))

main()
