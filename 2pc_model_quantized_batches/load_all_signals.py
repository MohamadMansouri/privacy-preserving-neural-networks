#!/usr/bin/python3
#############################################
# Author: Mohamad Mansouri
# Email: Mohamad.Mansouri@eurecom.fr
# Date: 11/09/2018
##############################################

import sys
import random
import pickle 
from sklearn.decomposition import PCA
import numpy as np
from collections import defaultdict
from pathlib import Path

def load_obj(name ):
    with open('../training/obj/' + name + '.pkl', 'rb') as f:
        return pickle.load(f)

def remove_list(a):
	return [x[0] for x in a]

def change_dim(X,N,Z ):
    pca = PCA(N)
    pca.fit(X)
    return pca.transform(X),pca.transform(Z)

def put_pca(sig,name):
    makedir("input/signal_pca/")
    with open("input/signal_pca/" + name ,'w') as file:
        for samp in sig:
            file.write("%f " % samp)                

def put_signal(sig,name):
    makedir("input/signals/")
    with open("input/signals/" + name ,'w') as file:
        for samp in sig:
            file.write("%f " % samp)                

def makedir(path):
    p = Path(path)
    p.mkdir(parents=True, exist_ok=True)

x_test = load_obj("x_test").tolist()
y_test = load_obj("y_test").tolist()


data = defaultdict(list)

print(len(y_test))

for i in range(len(y_test)):
    if y_test[i] == '/':
        data['_'].append(x_test[i])
    else:
        data[y_test[i]].append(x_test[i])
        
print("[-] Finished reading test signals from the database...")

pca = load_obj("PCA")

for key,value in data.items():
    i=0
    for signal in value:
        put_signal(signal, key + "_" + str(i))
        signal_pca = pca.transform(np.array(signal).reshape(1,-1))[0]
        put_pca(signal_pca,key + "_" + str(i))
        i +=1
        

print("[-] Finished loading all signals to input/signals/ and input/signal_pca/...")
sys.exit()