#!/usr/bin/python3
import pickle
import wfdb
from random import sample
import numpy as np


def load_obj(name ):
    with open('obj/' + name + '.pkl', 'rb') as f:
        return pickle.load(f)


def save_obj(obj, name ):
    with open('obj/'+ name + '.pkl', 'wb') as f:
        pickle.dump(obj, f, pickle.HIGHEST_PROTOCOL)


def makedir(path):
    try:
        os.stat(path)
    except:
        os.mkdir(path)


def unison_shuffled_copies(a, b):
    assert len(a) == len(b)
    p = np.random.permutation(len(a))
    return a[p], b[p]


def remove_list(a):
	return [x[0] for x in a]


def save_plain(obj, name):
	makedir('plain_dataset')
	with open('plain_dataset/' + name ,'w') as f:
		for i in obj:
			f.write('{} '.format(i))

dataset = load_obj("data")


x_train = []
x_test = []
y_train = []
y_test = []

# split dataset to train and test portions
count = 0 
for individual in dataset:
	for symbol in dataset[individual]:
		count=0
		if symbol == 'N':
			samples = sample(range(len(dataset[individual]['N'])),
				int(len(dataset[individual]['N'])/4))
			for i in samples :
				if count < len(samples)*0.80:
					x_train.append(dataset[individual]['N'][i])
					y_train.append('N')
				else:
					x_test.append(dataset[individual]['N'][i])
					y_test.append('N')
				count+=1
		elif symbol == '"' or symbol =="[" or symbol =="]" or symbol =="S" or 
		symbol == "Q" or symbol == "~" or symbol == "|":
			continue
		elif symbol == 'e':
			samples = sample(range(len(dataset[individual][symbol])),
				len(dataset[individual][symbol]))
			for i in samples:
				if count < len(samples)*0.80:
					x_train.append(dataset[individual][symbol][i])
					x_train.append(dataset[individual][symbol][i])
					y_train.append(symbol)
					y_train.append(symbol)
				else:
					x_test.append(dataset[individual][symbol][i])
					x_test.append(dataset[individual][symbol][i])
					y_test.append(symbol)
					y_test.append(symbol)
				count+=1

		else:
		samples = sample(range(len(dataset[individual][symbol])),
			len(dataset[individual][symbol]))
		for i in samples:
			if count < len(samples)*0.80:
				x_train.append(dataset[individual][symbol][i])
				y_train.append(symbol)
			else:
				x_test.append(dataset[individual][symbol][i])
				y_test.append(symbol)
			count+=1

X_train=[]
X_test=[]

for i in x_train:
	X_train.append(remove_list(i))

for i in x_test:
	X_test.append(remove_list(i))


X_train,Y_train = unison_shuffled_copies(np.array(X_train),np.array(y_train))
X_test,Y_test = unison_shuffled_copies(np.array(X_test),np.array(y_test))


save_obj(X_train,"x_train")
save_obj(X_test,"x_test")
save_obj(Y_train,"y_train")
save_obj(Y_test,"y_test")

count = defaultdict(int)

# save dataset in plain text format
for x,y in zip(X_test,Y_test):
	count[y] +=1
	if(y == '/'):
		symbol = ''
	else: 
		symbol = y

	save_plain(x,symbol + '_{}'.format(count[y]))