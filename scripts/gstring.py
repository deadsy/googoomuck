#!/usr/bin/python

import math

GRAMS = 0.001
MM = 0.001
MM_PER_INCH = 25.4
INCHES = MM_PER_INCH * MM
GRAV = 9.807
LBS_PER_KG = 2.20462

def midi_to_freq(m):
  return 440.0 * math.pow(2.0, float(m - 69) / 12.0)

def note_to_midi(n):
  """convert an SPN note name to a midi note"""
  n = n.upper()
  x = list(n)
  if len(x) == 2:
    name = x[0]
    accidental = None
    octave = x[1]
  elif len(x) == 3:
    name = x[0]
    accidental = x[1]
    octave = x[2]
  else:
    assert False, 'bad note length'
  # octave
  if octave in ('0','1','2','3','4','5','6','7','8'):
    # note: midi octave numbering is +1 on SPN octave numbering
    midi = (ord(octave) - ord('0') + 1) * 12
  else:
    assert False, 'bad octave number'
  # note
  if name in ('A','B','C','D','E','F','G'):
    idx = ord(name) - ord('A')
    midi += (9,11,0,2,4,5,7)[idx]
  else:
    assert False, 'bad note name'
  # accidental
  if accidental == '#':
    midi += 1
  elif accidental == 'B':
    midi -= 1
  elif accidental is not None:
    assert False, 'bad accidental name'
  return midi

def note_to_freq(n):
  return midi_to_freq(note_to_midi(n))

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

  def f_string(self, l, t):
    """return the frequency (Hz) of a plucked string"""
    # l = string length (m)
    # t = string tension (N)
    return math.sqrt(t / self.ldensity) / (2.0 * l)

  def t_string(self, l, f):
    """return the tension (N) of a plucked string"""
    # l = string length (m)
    # f = string frequency (Hz)
    return 4.0 * self.ldensity * l * l * f * f

  def __str__(self):
    return '%s (%d)' % (self.name, self.gauge)


def string_setup(s, l, n):
  f = note_to_freq(n)
  x = []
  x.append(str(s))
  x.append(n)
  x.append('%.2fHz' % f)
  x.append('%.2fN' % s.t_string(l, f))
  return ' '.join(x)

def main():

  s0 = gstring('EB1146', 46, (7.306 - 0.166) * GRAMS, 1.0) # E2
  s1 = gstring('EB1136', 36, (4.570 - 0.166) * GRAMS, 1.0) # A2
  s2 = gstring('EB1126', 26, (2.539 - 0.166) * GRAMS, 1.0) # D3
  s3 = gstring('EB1017', 17, (1.340 - 0.166) * GRAMS, 1.0) # G3
  s4 = gstring('EB1013', 13, (0.859 - 0.166) * GRAMS, 1.0) # B3
  s5 = gstring('EB1010', 10, (0.571 - 0.166) * GRAMS, 1.0) # E4

  d = 25.0 * INCHES

  print 'standard 6-string tuning'
  print string_setup(s0, d, 'e2')
  print string_setup(s1, d, 'a2')
  print string_setup(s2, d, 'd3')
  print string_setup(s3, d, 'g3')
  print string_setup(s4, d, 'b3')
  print string_setup(s5, d, 'e4')


  print 'open G 3-string options'
  print string_setup(s1, d, 'g2')
  print string_setup(s3, d, 'g3')
  print string_setup(s5, d, 'g4')
  print string_setup(s2, d, 'd3')
  print string_setup(s5, d, 'd4')


main()


