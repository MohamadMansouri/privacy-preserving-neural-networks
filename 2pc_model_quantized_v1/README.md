# 2PC quantized model with shifters (V1)

This is an implementation of the quantized NN model using the 2PC protocol to preserve the privacy of the server parameters and client input. The model is built with arithmatic and boolean circuits. The inputs and intermediate outputs of the layers are truncated using shifters (shifting the bits of the integers).


**Note**: 
- To change the dimension of the input and number of neurons in each layer, modify the global variables.
- SIMD (single instruction multiple data) optimization is used to increase the performance.

## Compiling the model:
``` bash
mkdir build
cd build
cmake ..
make
```
If you want to build the pca/max layer under 2pc use:
``` bash
cmake -DWITH_PCA=ON .. 
```
**Note**: 
- If you choose to build the pca layer the client input should be a signal of 180 elements.


## Usage:

- use `./server -p 9999` to start the server and listen on port 9999
- use `./client -p 9999 -s ../input/signal_pca` to start the NN prediction process
- `load_signal.py` is a script that can load a random signal from the dataset. use `./load_signal.py -h` to see how to use it
- `plot_signal.py` takes an ECG signal in its arguments and visualize it.