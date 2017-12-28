#!/usr/bin/python
# Format the project source code per the standard (linux style)

import glob
import subprocess
import os

src_dirs = (
  'target/mb997',
  'soc/st/stm32f4/lib',
  'common',
  'drivers',
  'ggm',
  'ui',
  'usb/files/tinyusb/host',
  'usb/files/tinyusb/hal',
  'usb/files/tinyusb/device',
  'usb/files/boards/st/mb997',
)

src_filter_out = (
  'target/mb997/stm32f4xx_hal_conf.h',
  'target/mb997/system_stm32f4xx.c',
  'usb/files/boards/st/mb997/stm32f4xx_hal_conf.h',
)

indent_exec = '/usr/bin/indent'

def exec_cmd(cmd):
  """execute a command, return the output and return code"""
  output = ''
  rc = 0
  try:
    output = subprocess.check_output(cmd, shell=True)
  except subprocess.CalledProcessError as x:
    rc = x.returncode
  return output, rc

def get_files(dirs, filter_out):
  files = []
  for d in dirs:
    files.extend(glob.glob('%s/*.h' % d))
    files.extend(glob.glob('%s/*.c' % d))
  return [f for f in files if f not in filter_out]

def format(f):
  exec_cmd('%s -brf -linux -l10000 %s' % (indent_exec, f))
  os.unlink('%s~' % f)

def main():
  files = get_files(src_dirs, src_filter_out)
  for f in files:
    format(f)

main()
