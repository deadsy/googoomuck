#!/usr/bin/python

import math

G = 9.80665
MM_PER_INCH = 25.4

def f_string(t, u, l):
  """return the frequency (Hz) of a plucked string"""
  # t = string tension (N)
  # u = linear density of string (kg/m)
  # l = string length (m)
  # wave velocity
  v = math.sqrt(t / u)
  # standing wave with nodes at the ends
  f = v / (2.0 * l)
  return f

def frets(l):
  k = math.pow(2.0, 1.0/12.0)
  l_nut = l
  for i in range(25):
    print('%d: %.3f mm' % (i, l_nut - l))
    l = l/k

def main():

  # high e
  u = 0.401e-3 # kg/m
  l = 640.0e-3 # m
  t = 7.28 * G # N 
  print f_string(t, u, l)

  # low e
  u = 6.79e-3 # kg/m
  l = 640.0e-3 # m
  t = 7.71 * G # N 
  print f_string(t, u, l)

  frets(25.5 * MM_PER_INCH)

main()

