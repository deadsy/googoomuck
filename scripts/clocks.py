#!/usr/bin/python

MHz = 1000000.0

HSE = 8.0 * MHz

pll_m_set = range(2, 63 + 1)
pll_n_set = range(50, 432 + 1)
pll_p_set = (2,4,6,8)
pll_q_set = range(2, 15 + 1)
ahb_presc_set = (1, 2, 4, 8, 16, 64, 128, 256, 512)

def pll_vco_input(hse, pll_m):
  return float(hse) / float(pll_m)

def pll(hse, pll_m, pll_n):
  return pll_vco_input(hse, pll_m) * float(pll_n)

def pllclk(hse, pll_m, pll_n, pll_p):
  return pll(hse, pll_m, pll_n) / float(pll_p)

def pll48ck(hse, pll_m, pll_n, pll_q):
  return pll(hse, pll_m, pll_n) / float(pll_q)

def sysclk(hse, pll_m, pll_n, pll_p, ahb_presc):
  return pllclk(hse, pll_m, pll_n, pll_p) / float(ahb_presc)

def main():
  max_sysclk = 0.0
  for pll_m in pll_m_set:
    for pll_n in pll_n_set:
      for pll_p in pll_p_set:
        for pll_q in pll_q_set:
          for ahb_presc in ahb_presc_set:

            x = pll48ck(HSE, pll_m, pll_n, pll_q)
            if x != 48.0 * MHz:
              continue

            x = pll_vco_input(HSE, pll_m)
            if x < 1.0 * MHz or x > 2.0 * MHz:
              continue

            x = pll(HSE, pll_m, pll_n)
            if x < 100.0 * MHz or x > 432.0 * MHz:
              continue

            x = sysclk(HSE, pll_m, pll_n, pll_p, ahb_presc)
            if x > 180.0 * MHz:
              continue

            if x >= max_sysclk:
              print('sysclk %d pll_m %d pll_n %d pll_p %d pll_q %d ahb_presc %d' % (x, pll_m, pll_n, pll_p, pll_q, ahb_presc))
              max_sysclk = x

main()
