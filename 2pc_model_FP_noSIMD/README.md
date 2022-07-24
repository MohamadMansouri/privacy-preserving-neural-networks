# 2PC Model with Floating-Point and without SIMD

This is an implementation of the trained NN model (with pca dim 16) using the 2PC protocol to preserve the privacy of the server parameters and client input. The model is built with arithmatic and boolean floating point circuits.

## Compiling the model:
```
cd build
cmake ..
make
```

## Usage:

- use `./server -p 9999` to start the server and listen on port 9999
- use `./client -p 9999 -i ../input/signal_pca` to start the NN prediction process
- `load_signal.py` is a script that can load a random signal from the dataset. use `./load_signal.py -h` to see how to use it
- `plot_signal.py` takes an ECG signal in its arguments and visualize it.