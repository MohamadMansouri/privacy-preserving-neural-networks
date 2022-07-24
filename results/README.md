# Results

This directory contains the results of the training and the predictions under 2-PC:
- **variable_dim**: Directory containing the results of the training of the model to find the best pca reduced dimension of the inputs
- **variable_neurons**: Directory containing the results of the training of the model to find the best number of neurons in the hidden layer
- **2pc**: Directory containing the results of the prediction of the quantized 2pc model (the results are stored in 2 columns, the first represents the truth value and the second represents the predicted value). Use the script `./plot_cf.py <file>` to plot the confusion matrix of the results. 
