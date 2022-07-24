#!/usr/bin/python3
import wfdb
import os
import matplotlib.pyplot as plt
from wfdb import processing
import pickle



def save_obj(obj, name ):
    with open('obj/'+ name + '.pkl', 'wb') as f:
        pickle.dump(obj, f, pickle.HIGHEST_PROTOCOL)


# read dataset from the internet
dataset = dict()
for record in wfdb.get_record_list('mitdb')	:
	path = "../mitdb/" + record
	signals = dict()
	annotation = wfdb.rdann(path, 'atr')
	for i in range(3,len(annotation.__dict__['sample'])-3):
		sig, field = wfdb.rdsamp(path, channels=[0],sampfrom=annotation.__dict__['sample'][i]-90 , sampto = annotation.__dict__['sample'][i]+90)  
		if annotation.__dict__['symbol'][i] in signals:
			signals[annotation.__dict__['symbol'][i]].append(sig)
		else:
			signals[annotation.__dict__['symbol'][i]]=[sig] 
	
	print(record)
	dataset[record] = signals
save_obj(dataset,"data")
