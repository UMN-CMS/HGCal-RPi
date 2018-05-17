#!/usr/bin/python

import subprocess

for num in range(1000, 10000):
  fout = open('logs/rdout_'+str(num)+'.log', 'w+')
  subprocess.call(['sudo', './bin/new_rdout.exe', str(num), '1000', '0'], stderr=fout, stdout=fout)

