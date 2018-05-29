#!/usr/bin/python

import math

GRAMS = 0.001
MM = 0.001
MM_PER_INCH = 25.4

class gstring(object):

  def __init__(self, name, gauge, weight, length):
    self.name = name
    self.gauge = gauge

    # linear density
    self.ldensity = float(weight) / float(length)

    # volume density
    d = (MM_PER_INCH * (gauge / 1000.0)) * MM
    r = 0.5 * d
    a = math.pi * r * r
    self.vdensity = float(weight) / (float(length) * a)

  def __str__(self):
    return '%s l %f v %f' % (self.name, self.ldensity, self.vdensity)


def main():

  s = gstring('EB1146', 46, (7.306 - 0.166) * GRAMS, 1.0) # E2
  print s
  s = gstring('EB1136', 36, (4.570 - 0.166) * GRAMS, 1.0) # A2
  print s
  s = gstring('EB1126', 26, (2.539 - 0.166) * GRAMS, 1.0) # D3 
  print s
  s = gstring('EB1017', 17, (1.340 - 0.166) * GRAMS, 1.0) # G3 
  print s
  s = gstring('EB1013', 13, (0.859 - 0.166) * GRAMS, 1.0) # B3 
  print s
  s = gstring('EB1010', 10, (0.571 - 0.166) * GRAMS, 1.0) # E4 
  print s


main()


