#!/usr/bin/python

def main():
  n = 128;

  print("void block_mul(float *out, float *buf, size_t n){")

  avars = ['a%d' % i for i in range(n)]
  print('float %s;' % ','.join(avars))

  bvars = ['b%d' % i for i in range(n)]
  print('float %s;' % ','.join(bvars))

  cvars = ['c%d' % i for i in range(n)]
  print('float %s;' % ','.join(cvars))

  loada = ['a%d = out[%d];' % (i,i) for i in range(n)]
  print('%s' % '\n'.join(loada))

  loadb = ['b%d = buf[%d];' % (i,i) for i in range(n)]
  print('%s' % '\n'.join(loadb))

  muls = ['c%d = a%d * b%d;' % (i,i,i) for i in range(n)]
  print('%s' % '\n'.join(muls))

  store = ['out[%d] = c%d;' % (i,i) for i in range(n)]
  print('%s' % '\n'.join(store))

  print("}")


main()

