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
import argparse as ap
from collections import defaultdict, OrderedDict

def load_obj(name):
    with open('../training/obj/' + name + '.pkl', 'rb') as f:
        return pickle.load(f)

def remove_list(a):
	return [x[0] for x in a]

def change_dim(X,N,Z ):
    pca = PCA(N)
    pca.fit(X)
    return pca.transform(X),pca.transform(Z)

def put_to_file(sig,name):
    with open("./input/" + name ,'w') as file:
        for samp in sig:
            file.write("%f " % samp)                


def main(args):

    dataset = load_obj("data")
    if args.show_data:
        ind_size = len(dataset)
        annot_size = defaultdict(int)
        print("Individual ID\tAnnotations")
        for individual in dataset:
            print("{}\t\t".format(individual),end='')
            for annotation in dataset[individual]:
                print("{} ".format(annotation),end='')
                annot_size[annotation] += len(dataset[individual][annotation])
            print("")

        print("\nStatistics:")
        print("Total number of individuals is {}".format(ind_size))
        print("Annotation\tCount")
        sorted_ann = sorted(annot_size.items(), key=lambda kv: kv[1], reverse=True)
        annot_size = OrderedDict(sorted_ann)
        for ann in annot_size:
            print("{}\t\t{}".format(ann,annot_size[ann]))
    else:
        if args.individual == None or args.annotation == None:
            print("ERROR: specify individual and annotation")
            sys.exit(-1)

        if args.individual not in dataset:
            print("ERROR: invalid individual id (use -s to see all valid individual ids)")
            sys.exit(-1)
        
        if args.annotation not in dataset[args.individual]:
            print("ERROR: invalid annotation (use -s to see all valid annotations for each individuals)")
            sys.exit(-1)
        
        annotation = args.annotation
        individual = args.individual

        length = len(dataset[individual][annotation])
        sig = dataset[individual][annotation][random.randint(0,length-1)]
        signal = remove_list(sig)
        if args.output == None:
            name = "signal"
        else:
            name = args.output
        pca_name = name + '_pca_{}_{}'.format(individual,annotation)
        name += "_{}_{}".format(individual,annotation)

        put_to_file(signal,name)

        print("[-] A random signal from individual {} and of annotation {} ".format(individual, annotation) + \
            "is written to ./input/{}".format(name) )

        pca = load_obj("PCA")
        signal_pca = pca.transform(np.array(signal).reshape(1,-1))[0]


        # signal_pca = signal_pca.reshape(1,-1)[0]
        put_to_file(signal_pca,pca_name)

        print("[-] The pca of the signal is written in ./input/{}".format(pca_name))



    sys.exit(0)



if(__name__ == "__main__"):
    parser = ap.ArgumentParser(description='Load a signal from the dataset')
    parser.add_argument('individual', nargs='?', help='the individual id')
    parser.add_argument('annotation', nargs='?', help='the class of the signal')
    parser.add_argument('-s', '--show-data', action='store_true',
                        help='only show details about the data' )
    parser.add_argument('-o', '--output', action='store', dest='output',
                        help='the name of the output file' )
    args = parser.parse_args()
    main(args)