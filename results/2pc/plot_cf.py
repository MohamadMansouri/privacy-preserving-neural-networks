#!/usr/bin/python3
#############################################
# Author: Mohamad Mansouri
# Email: Mohamad.Mansouri@eurecom.fr
# Date: 30/06/2019
##############################################
import sys

if len(sys.argv) != 2:
    print("Usage: ./plot_cf.py <file>")
    exit(-1)

import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix, precision_score, accuracy_score
import numpy as np
import itertools


classes = {'N': 0, 'L': 9, 'R': 12, 'V': 2, '/': 3, 'A': 1, '+': 5, 'f': 4,\
 'F': 7, '!': 13, 'j': 8, 'x': 6, 'a': 10, 'E': 14, 'J': 11, 'e': 15}


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
    fig_cm.savefig(sys.argv[1] + '.png',dpi=700)
    plt.show()    




OK = False
truth = []
predicted = []

with open(sys.argv[1]) as f:
    for line in f: 
        l = line.split()
        truth.append(classes[l[0]])         
        predicted.append(classes[l[1]])	

    classes_inv = {}
    for c in classes:
        classes_inv[classes[c]] = c

    max_class = []
    for i in predicted:
      max_class.append(list(classes.keys()).index(classes_inv[i]))


    Y_test_ordered = []
    for y in truth:
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
        print("\t {} : {:.2f} %".format(list(classes.keys())[i],precision[i]*100  ))

    OK = True

if not OK:
    print("file is not valid")
    exit(-1)

exit(0)
