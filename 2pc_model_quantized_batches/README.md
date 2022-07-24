# 2PC quantized model for batches of signals

This is an implementation of the quantized NN model using the 2PC protocol to preserve the privacy of the server parameters and client input. This implementation accepts batches with arbitrary number of signals from the clients. SIMD optimization is applied to profit from using single gates to deal with multiple number of signals at a time.

**Note**: 
- To change the dimension of the input and number of neurons in each layer, modify the global variables.

## Compiling the model:
``` bash
mkdir build
cd build
cmake ..
make
```
If you wish to build the pca layer under 2pc use:
``` bash
cmake -DWITH_PCA=ON .. 
```
You can also use the V1 of the model (with shifters) by:
``` bash
cmake -DUSE_V1=ON .. 
```
**Note**: 
- If you choose to build the pca layer the client input should be a signal of 180 elements.


## Usage:

- use `./server -p 9999` to start the server and listen on port 9999
- use `./client -p 9999 -i N` to start the NN prediction process. `N` is the size of the batch. By executing the program it will try to predict all the test signals in the dataset. The results will be stored in the `resutls/2pc` directory. 
- `load_all_signal.py` is a script that can load all the test signals from the dataset in two forms, with and without pca applied. The signals will be stored in input/signals and input/signal_pca respectively.