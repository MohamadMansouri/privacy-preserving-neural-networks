#!/usr/bin/python3
import os
import subprocess
import re
import time 

model = "../2pc_model_quantized_v2/build"
signal = "../input/signal_101_N"


repeat = 10

rexp = "Total number of gates: (?P<gates>[0-9]+) Total depth: (?P<depth>[0-9]+)\n\
Timings: \n\
Total =		(?P<total_t>[^ ]+) ms\n\
Init =		(?P<init_t>[^ ]+) ms\n\
CircuitGen =	(?P<cgen_t>[^ ]+) ms\n\
Network =	(?P<network_t>[^ ]+) ms\n\
BaseOTs =	(?P<baseot_t>[^ ]+) ms\n\
Setup =		(?P<setup_t>[^ ]+) ms\n\
OTExtension =	(?P<otext_t>[^ ]+) ms\n\
Garbling =	(?P<garbling_t>[^ ]+) ms\n\
Online =	(?P<online_t>[^ ]+) ms\n\
\n\
Communication: \n\
Total Sent / Rcv	(?P<total_bs>[^ ]+)  bytes / (?P<total_br>[^ ]+) bytes\n\
BaseOTs Sent / Rcv	(?P<baseot_bs>[^ ]+)  bytes / (?P<baseot_br>[^ ]+) bytes\n\
Setup Sent / Rcv	(?P<setup_bs>[^ ]+)  bytes / (?P<setup_br>[^ ]+) bytes\n\
OTExtension Sent / Rcv	(?P<otext_bs>[^ ]+)  bytes / (?P<otext_br>[^ ]+) bytes\n\
Garbling Sent / Rcv	(?P<garbling_bs>[^ ]+)  bytes / (?P<garbling_br>[^ ]+) bytes\n\
Online Sent / Rcv	(?P<online_bs>[^ ]+)  bytes / (?P<online_br>[^ ]+) bytes\n\
"

os.chdir(model)

total_t = 0.0
init_t = 0.0
cgen_t = 0.0
network_t = 0.0
baseot_t = 0.0
setup_t = 0.0
otext_t = 0.0
garbling_t = 0.0
online_t = 0.0

total_bs = 0
total_br = 0
baseot_bs = 0
baseot_br = 0
setup_bs = 0
setup_br = 0
otext_bs = 0
otext_br = 0
garbling_bs = 0
garbling_br = 0
online_bs = 0
online_br = 0

for i in range(repeat):
	print("{} / {}".format(i + 1,repeat))
	subprocess.Popen(["./server", "-p", "9999"], stdout=subprocess.PIPE)

	time.sleep(1)
	result = subprocess.check_output(["./client", "-p", "9999", "-s" ,signal])

	txt = []
	for i in result:
		txt.append(chr(i))
	txt = ''.join(txt)

	m = re.search(rexp, txt)

	gates = int(m.group('gates'))
	depth = int(m.group('depth'))

	total_t += float(m.group('total_t'))
	init_t += float(m.group('init_t'))
	cgen_t += float(m.group('cgen_t'))
	network_t += float(m.group('network_t'))
	baseot_t += float(m.group('baseot_t'))
	setup_t += float(m.group('setup_t'))
	otext_t += float(m.group('otext_t'))
	garbling_t += float(m.group('garbling_t'))
	online_t += float(m.group('online_t'))

	total_bs += int(m.group('total_bs'))
	total_br += int(m.group('total_br'))
	baseot_bs += int(m.group('baseot_bs'))
	baseot_br += int(m.group('baseot_br'))
	setup_bs += int(m.group('setup_bs'))
	setup_br += int(m.group('setup_br'))
	otext_bs += int(m.group('otext_bs'))
	otext_br += int(m.group('otext_br'))
	garbling_bs += int(m.group('garbling_bs'))
	garbling_br += int(m.group('garbling_br'))
	online_bs += int(m.group('online_bs'))
	online_br += int(m.group('online_br'))


total_t /= repeat
init_t /= repeat
cgen_t /= repeat
network_t /= repeat
baseot_t /= repeat
setup_t /= repeat
otext_t /= repeat
garbling_t /= repeat
online_t /= repeat

total_bs /= repeat
total_br /= repeat
baseot_bs /= repeat
baseot_br /= repeat
setup_bs /= repeat
setup_br /= repeat
otext_bs /= repeat
otext_br /= repeat
garbling_bs /= repeat
garbling_br /= repeat
online_bs /= repeat
online_br /= repeat



print()
print(gates) 
print(depth) 
print()
print("{:.3f}".format(total_t))
print("{:.3f}".format(init_t))
print("{:.3f}".format(cgen_t))
print("{:.3f}".format(network_t))
print("{:.3f}".format(baseot_t))
print("{:.3f}".format(setup_t))
print("{:.3f}".format(otext_t))
print("{:.3f}".format(garbling_t))
print("{:.3f}".format(online_t))
print()
print(int(total_bs / 1024), end=' / ')
print(int(total_br / 1024))
print(int(baseot_bs / 1024), end=' / ')
print(int(baseot_br / 1024))
print(int(setup_bs / 1024), end=' / ')
print(int(setup_br / 1024))
print(int(otext_bs / 1024), end=' / ')
print(int(otext_br / 1024))
print(int(garbling_bs / 1024), end=' / ')
print(int(garbling_br / 1024))
print(int(online_bs / 1024), end=' / ')
print(int(online_br / 1024))
