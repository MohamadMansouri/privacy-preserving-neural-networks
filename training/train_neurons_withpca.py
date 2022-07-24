#!/usr/bin/python3
import pickle
import numpy as np
import wfdb
import matplotlib.pyplot as plt
from keras.models import Sequential, load_model
from keras.layers import Dense, Dropout, Activation
from keras.callbacks import ReduceLROnPlateau, ModelCheckpoint
import keras
from keras import backend as K
from sklearn.metrics import confusion_matrix
from sklearn.decomposition import PCA
import itertools
import os
from pathlib import Path
from sklearn.model_selection import train_test_split

classes = {'N': 0, 'L': 9, 'R': 12, 'V': 2, '/': 3, 'A': 1, '+': 5, 'f': 4,\
 'F': 7, '!': 13, 'j': 8, 'x': 6, 'a': 10, 'E': 14, 'J': 11, 'e': 15}

path = '../results/variable_neurons/withpca/'

X_train = None 
X_valid = None
Y_train = None
Y_valid = None

X_train_pca = None 
Y_train_split = None

path_to_accuracy = None
path_to_cm = None
path_to_models  = None



activations = ['x2']
vald =0.2
batch = 64
epochs= 100
dim = 16

path += "{}_dimension/".format(dim)

def number_to_classe(n):
    for c in classes:
        if classes[c]== n:
            return c


def change_dim(X,N):
    pca = PCA(N)
    pca.fit(X)
    return pca.transform(X)


class X2(Activation):
    
    def __init__(self, activation, **kwargs):
        super(X2, self).__init__(activation, **kwargs)
        self.__name__ = 'x2'


def square_act(x):
    return x*x


def load_obj(name ):
    with open('obj/' + name + '.pkl', 'rb') as f:
        return pickle.load(f)


def save_obj(obj, name ):
    with open('obj/'+ name + '.pkl', 'wb') as f:
        pickle.dump(obj, f, pickle.HIGHEST_PROTOCOL)


def makedir(path):
    p = Path(path)
    p.mkdir(parents=True, exist_ok=True)

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

    reduce_lr = ReduceLROnPlateau(monitor='val_acc', factor=0.2,
                              patience=10, min_lr=0.001)

    checkpointer = ModelCheckpoint(filepath= path + 'best_model',
         save_weights_only=True, monitor='val_acc', verbose=0,
         save_best_only=True, mode='max')
  
    history = model.fit(X_train_pca, Y_train_split, epochs=epochs, shuffle=True,
     validation_data=(X_valid, Y_valid), batch_size=batch, verbose=1,
     callbacks=[checkpointer, reduce_lr], class_weight='auto')

    model.load_weights(path + 'best_model')
    
    weight = []
    bias = []
    weight.append(model.layers[0].get_weights()[0])
    bias.append(model.layers[0].get_weights()[1])
    weight.append(model.layers[1].get_weights()[0])
    bias.append(model.layers[1].get_weights()[1])
    
    eval_valid = model.evaluate(X_valid,Y_valid,batch_size=batch)[1]

    predicted = model.predict(X_valid, batch_size=batch, verbose=1, steps=None)

    return history, eval_valid, predicted, weight, bias


def plot_confusion_matrix(cm, classes, name, path,
                          normalize=False,
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
    fig_cm.savefig(path + '{}.png'.format(name),dpi=500)
    plt.close(fig_cm)

def main():
    
    global X_train
    global X_valid
    global Y_train
    global Y_valid

    global X_train_pca
    global Y_train_split
    
    global path_to_accuracy
    global path_to_cm
    global path_to_models 

    # Load dataset 
    X_train=np.array(load_obj("x_train"))
    # X_test=np.array(load_obj("x_test"))
    Y_train=np.array([classes[x] for x in load_obj("y_train")])
    # Y_test=np.array([classes[x] for x in load_obj("y_test")])
    
    classes_inv = {}
    for c in classes:
        classes_inv[classes[c]] = c

    makedir(path)
    # add x2 method for the activation function
    keras.utils.generic_utils.get_custom_objects().update({'x2':X2(square_act)})

    for act in activations:


        path_to_accuracy = path + 'accuracy_{}_{}_{}/'.format(act,vald,batch)
        path_to_cm = path + 'cm_{}_{}_{}/'.format(act,vald,batch)
        path_to_models = path + 'models_{}_{}_{}/'.format(act,vald,batch)

        makedir(path_to_accuracy)
        makedir(path_to_cm)
        makedir(path_to_models)
            
        X_train_pca = change_dim(X_train,dim)

        X_train_pca, X_valid, Y_train_split, Y_valid = train_test_split(X_train_pca, Y_train,
         test_size=vald)
        
        evaluation= {}
        for neurons in range(2,102,2):
            
            history ,eval_valid, predicted, weight, bias = NN_model(act,neurons,
                vald,batch,epochs,dim)

            # store the accuracy of this test
            evaluation[neurons] = eval_valid

            # draw the accuracy trace of this test
            fig_acc = plt.figure()
            plt.plot(history.history['acc'])
            plt.plot(history.history['val_acc'])
            plt.title("Accuracy\neval={}".format(eval_valid))
            plt.ylabel('accuracy')
            plt.xlabel('epoch')
            plt.legend(['train', 'valid'], loc='upper left')
            fig_acc.savefig(path_to_accuracy + '{}.png'.format(neurons))
            plt.close(fig_acc)

            #draw the confusion matrix of this test
            max_class = []
            for i in predicted:
              max_class.append(list(classes.keys()).index(classes_inv[np.argmax(i)]))


            Y_valid_ordered = []
            for y in Y_valid:
                Y_valid_ordered.append(list(classes.keys()).index(classes_inv[y]))

            cnf_matrix=confusion_matrix(np.array(Y_valid_ordered), np.array(max_class),
             labels=None, sample_weight=None)
            plot_confusion_matrix(cnf_matrix,classes, neurons, path_to_cm, True)

            # save the model parameters
            save_model(weight[0],bias[0], '{}_hidden'.format(neurons))    
            save_model(weight[1],bias[1], '{}_out'.format(neurons))    
            
        # draw the accuracy versus number of neurons
        fig = plt.figure(figsize=(14,10))
        ax = fig.add_subplot(1, 1, 1)
        x_ticks = np.arange(2, 102, 4)
        y_ticks = np.arange(0.6, 1.0, 0.02)
        ax.set_xticks(x_ticks)
        ax.set_yticks(y_ticks)
        plt.title("Changing # neurons")
        plt.ylabel('Accuracy')
        plt.xlabel('Neurons')
        plt.grid(b=True, which='major', color='#666666', linestyle='-')
        plt.minorticks_on()
        plt.grid(b=True, which='minor', color='#999999', linestyle='-',
         alpha=0.2)
        ax.plot(evaluation.keys(), evaluation.values(),  marker='o')
        fig.savefig(path + 'results_{}_{}_{}.png'.format(act,vald,
            batch),dpi=600)
        plt.close(fig)

        with open(path + 'results_{}_{}_{}.txt'.format(act,vald,
            batch), 'w') as f:
            for i in evaluation:
                f.write('{} {}\n'.format(i, evaluation[i]))

if __name__ == '__main__':

    main()