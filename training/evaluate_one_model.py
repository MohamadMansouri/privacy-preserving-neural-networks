#!/usr/bin/python3
#############################################
# Author: Mohamad Mansouri
# Email: Mohamad.Mansouri@eurecom.fr
# Date: 11/09/2018
##############################################
import pickle
import numpy as np
import wfdb
import os
import matplotlib.pyplot as plt
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation
import keras
from sklearn.metrics import confusion_matrix, precision_score, accuracy_score
import itertools
from sklearn.decomposition import PCA
import argparse as ap




classes = {'N': 0, 'L': 9, 'R': 12, 'V': 2, '/': 3, 'A': 1, '+': 5, 'f': 4,\
 'F': 7, '!': 13, 'j': 8, 'x': 6, 'a': 10, 'E': 14, 'J': 11, 'e': 15}

path = '../results/withpca/'

hidden = None
output = None

X_train = None 
X_test = None
Y_train = None
Y_test = None

def number_to_class(n):
    for c in classes:
        if classes[c]== n:
            return c

class X2(Activation):
    
    def __init__(self, activation, **kwargs):
        super(X2, self).__init__(activation, **kwargs)
        self.__name__ = 'x2'


def square_act(x):
    return x*x

def change_dim(X,N,Z ):
    pca = PCA(N)
    pca.fit(X)
    return pca.transform(X),pca.transform(Z)

def load_obj(name ):
    with open('obj/' + name + '.pkl', 'rb') as f:
        return pickle.load(f)


def save_obj(obj, name ):
    with open('obj/'+ name + '.pkl', 'wb') as f:
        pickle.dump(obj, f, pickle.HIGHEST_PROTOCOL)

def read_weights(hidden, output):
    hidden_w = []
    with open(hidden) as f:
        for line in f:
            hidden_w.append([float(x) for x in line.split()])
    output_w = []
    with open(output) as f:
        for line in f:
            output_w.append([float(x) for x in line.split()])
    return np.array(hidden_w), np.array(output_w)



def read_biases(hidden, output):
    hidden_b = []
    with open(hidden) as f:
        line = f.readline()
        hidden_b = [float(x) for x in line.split()]
    output_b = []
    with open(output) as f:
        line = f.readline()
        output_b = [float(x) for x in line.split()]
    return np.array(hidden_b), np.array(output_b)
    


def save_model(w,b,name):
    with open(path_to_models + name + "_weight" ,'w') as file:
        for row in w:
            for column in row:
                file.write("%f " % column)                
            file.write("\n")
    with open(path_to_models + name + "_bias",'w') as file:
        for x in b:
            file.write("%f " % x)                


def NN_model(act,neurons,vald,batch,epochs, input_d):

    model = Sequential()

    model.add(Dense(activation=act, input_dim=input_d,output_dim=neurons,
        init=keras.initializers.glorot_normal(seed=None)))

    model.add(Dense(units=16,activation='softmax',
        init=keras.initializers.glorot_normal(seed=None)))

    model.compile(loss='sparse_categorical_crossentropy',
                  optimizer=keras.optimizers.Adam(lr=0.002),
                  metrics=['accuracy'])

    model.layers[0].set_weights(hidden)
    model.layers[1].set_weights(output)

    predicted = model.predict(X_test, batch_size=batch, verbose=0, steps=None)

    return  predicted


def plot_confusion_matrix(cm, classes, normalize=True,
                          title='Confusion matrix',
                          cmap=plt.cm.Blues):
    """
    This function prints and plots the confusion matrix.
    Normalization can be applied by setting `normalize=True`.
    """
    if normalize:
        cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
        print("Normalized confusion matrix")
    else:
        print('Confusion matrix, without normalization')


    fig_cm = plt.figure(figsize=(9,8))
    plt.imshow(cm, interpolation='nearest', cmap=cmap)
    plt.title(title)
    plt.colorbar()
    tick_marks = np.arange(len(classes))
    plt.xticks(tick_marks, classes, rotation=45)
    plt.yticks(tick_marks, classes)

    fmt = '.2f' if normalize else 'd'
    thresh = cm.max() / 2.
    for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
        plt.text(j, i, format(cm[i, j], fmt),
                 horizontalalignment="center",
                 color="white" if cm[i, j] > thresh else "black")

    plt.tight_layout()
    plt.ylabel('True label')
    plt.xlabel('Predicted label')
    fig_cm.savefig('/tmp/cm.png',dpi=700)
    plt.show()    




def main(args):
    global hidden
    global output

    global X_test
    global X_train
    global Y_test
    global Y_train

    hidden_w, output_w = read_weights(args.hw, args.ow)
    hidden_b, output_b = read_biases(args.hb, args.ob)

    hidden = np.array([hidden_w, hidden_b])
    output = np.array([output_w, output_b])

    act = 'x2'
    neurons = hidden_w.shape[1]
    vald = 0.20
    batch = 64
    epochs = 100
    input_d = hidden_w.shape[0]

    X_train=np.array(load_obj("x_train"))                                   
    X_test=np.array(load_obj("x_test"))                                     
    Y_train=np.array([ classes[x] for x in load_obj("y_train")])            
    Y_test=np.array([classes[x] for x in load_obj("y_test")])               
                                                                            
                                                                            
    if input_d != 180:
        X_train,X_test = change_dim(X_train,input_d,X_test)                     


    keras.utils.generic_utils.get_custom_objects().update(
        {'x2':keras.layers.Activation(square_act)})


    predicted = NN_model(act,neurons,vald,batch,epochs,input_d)

    classes_inv = {}
    for c in classes:
        classes_inv[classes[c]] = c

    max_class = []
    for i in predicted:
      max_class.append(list(classes.keys()).index(classes_inv[np.argmax(i)]))


    Y_test_ordered = []
    for y in Y_test:
        Y_test_ordered.append(list(classes.keys()).index(classes_inv[y]))

    cnf_matrix=confusion_matrix(np.array(Y_test_ordered), np.array(max_class), labels=None,
     sample_weight=None)

    plot_confusion_matrix(cnf_matrix,classes,True)

    accuracy = accuracy_score(np.array(Y_test_ordered), np.array(max_class))
    print("Accuracy: {:.2f} %".format(accuracy * 100))

    precision = precision_score(np.array(Y_test_ordered), np.array(max_class), labels=None,
     sample_weight=None, average=None)

    print("Precision:")
    for i in range(len(precision)):
        print("\t {} : {:.2f}".format(list(classes.keys())[i],precision[i]*100  ))



if(__name__ == "__main__"):
    parser = ap.ArgumentParser(description='Evaluate the performance of a given model ')
    parser.add_argument('-hw', '--hidden-weight', action='store', dest='hw', required=True,
                        help='the file containing the hidden weights')
    parser.add_argument('-hb', '--hidden-bias', action='store', dest='hb', required=True,
                        help='the file containing the hidden bias' )
    parser.add_argument('-ow', '--output-weight', action='store', dest='ow', required=True,
                        help='the file containing the output weights' )
    parser.add_argument('-ob', '--output-bias', action='store', dest='ob', required=True,
                        help='the file containing the output bias' )
    args = parser.parse_args()
    main(args)