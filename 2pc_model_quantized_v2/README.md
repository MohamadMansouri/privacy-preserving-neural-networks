# 2PC quantized model with input truncations (V2)

This is an implementation of the quantized NN model using the 2PC protocol to preserve the privacy of the server parameters and client input. The model is built with arithmatic circuits and optionally boolean circuits. Only the inputs of the layers are truncated by integer approximations.


**Note**: 
- To change the dimension of the input and number of neurons in each layer, modify the global variables.
- SIMD (single instruction multiple data) optimization is used to increase the performance.

## Compiling the model:
``` bash
mkdir build
cd build
cmake -DWITH_PCA=ON ..
make
```
If you want to build the pca/max layer under 2pc use:
``` bash
cmake -DWITH_PCA=ON -DWITH_MAX=ON .. 
```
**Note**: 
- If you choose to **NOT** build the max layer the output will be a vector containing the score of each layer.
- If you choose to build the pca layer the client input should be a signal of 180 elements.


## Usage:

- use `./server -p 9999` to start the server and listen on port 9999
- use `./client -p 9999 -s ../input/signal_pca` to start the NN prediction process
- `load_signal.py` is a script that can load a random signal from the dataset. use `./load_signal.py -h` to see how to use it
- `plot_signal.py` takes an ECG signal in its arguments and visualize it.