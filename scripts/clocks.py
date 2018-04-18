#!/usr/bin/python


HSE = 8000000.0

pll_m_set = range(2, 63 + 1)
pll_n_set = range(50, 432 + 1)
pll_p_set = (2,4,6,8)
pll_q_set = range(2, 15 + 1)
ahb_presc_set = (1, 2, 4, 8, 16, 64, 128, 256, 512)

def pll(hse, pll_m, pll_n):
  return (float(hse) / float(pll_m)) * float(pll_n)

def pllclk(hse, pll_m, pll_n, pll_p):
  return pll(hse, pll_m, pll_n) / float(pll_p)

def pll48ck(hse, pll_m, pll_n, pll_q):
  return pll(hse, pll_m, pll_n) / float(pll_q)

def sysclk(hse, pll_m, pll_n, pll_p, ahb_presc):
  return pllclk(hse, pll_m, pll_n, pll_p) / float(ahb_presc)

def main():

  for pll_m in pll_m_set:
    for pll_n in pll_n_set:
      for pll_p in pll_p_set:
        for pll_q in pll_q_set:
          for ahb_presc in ahb_presc_set:
            x = sysclk(HSE, pll_m, pll_n, pll_p, ahb_presc)
            y = pll48ck(HSE, pll_m, pll_n, pll_q)
            if x == 180000000.0 and y == 48000000.0:
              print('pll_m %d pll_n %d pll_p %d pll_q %d ahb_presc %d' % (pll_m, pll_n, pll_p, pll_q, ahb_presc))


main()









