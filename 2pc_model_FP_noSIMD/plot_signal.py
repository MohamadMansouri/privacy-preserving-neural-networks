#!/usr/bin/python3
#############################################
# Author: Mohamad Mansouri
# Email: Mohamad.Mansouri@eurecom.fr
# Date: 11/09/2018
##############################################

import wfdb
import numpy as np
import sys

if len(sys.argv) != 2:
	print("Usage: ./plot_signal.py <file>")
	exit(-1)

line = open(sys.argv[1]).readline()
signal = [float(x) for x in line.split()]
wfdb.plot_items(np.array(signal))

