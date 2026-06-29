# MNIST Example Usage And Performance Notes

This document records how to run the current MNIST example and what performance
to expect from the scalar autograd implementation. Use it as a baseline when
optimizing the neural-network module later.

## Build

The MNIST trainer is an optional example target. Build it with examples enabled:

```sh
cmake -S . -B build/examples -DFORG_BUILD_EXAMPLES=ON -DBUILD_TESTING=OFF
cmake --build build/examples --target forg_mnist
```

The example binary is expected at:

```sh
build/examples/examples/mnist/forg_mnist
```

## Run

Usage:

```sh
forg_mnist \
  <train-images> <train-labels> <test-images> <test-labels> \
  [epochs] [train-limit] [test-limit] [learning-rate]
```

Example using local MNIST IDX files:

```sh
/usr/bin/time -p build/examples/examples/mnist/forg_mnist \
  data/mnist_dataset/train-images.idx3-ubyte \
  data/mnist_dataset/train-labels.idx1-ubyte \
  data/mnist_dataset/t10k-images.idx3-ubyte \
  data/mnist_dataset/t10k-labels.idx1-ubyte \
  3 1000 1000 0.01
```

Arguments:

- `epochs`: Number of passes over the selected training subset.
- `train-limit`: Maximum number of training samples used per epoch.
- `test-limit`: Maximum number of test samples used for accuracy reporting.
- `learning-rate`: SGD learning rate.

## Recommended Starting Points

Use small subsets first. The current implementation is useful for correctness
and educational experiments, not fast full-dataset training.

```sh
# Fast sanity check
/usr/bin/time -p build/examples/examples/mnist/forg_mnist ... 1 100 100 0.01

# First meaningful run
/usr/bin/time -p build/examples/examples/mnist/forg_mnist ... 3 1000 1000 0.01

# Larger scalar-autograd experiment
/usr/bin/time -p build/examples/examples/mnist/forg_mnist ... 3 5000 1000 0.01
```

Start with `0.01` learning rate. If loss explodes or accuracy becomes unstable,
try `0.005` or `0.001`.

## Current Model

The example trains this model:

```cpp
Sequential{
    Linear(784, 64),
    ReLU(),
    Linear(64, 10),
}
```

Input images are flattened 28x28 MNIST pixels normalized to `[0.0, 1.0]`.
Labels are converted with `OneHot(10, label)`. Training uses `MSELoss` and
`SGD`. Inference predicts the digit with `ArgMax()` over the 10 raw output
scores.

## Baseline Timing

Measured locally on 2026-06-29 with:

```sh
/usr/bin/time -p /private/tmp/forg-example-build/examples/mnist/forg_mnist \
  data/mnist_dataset/train-images.idx3-ubyte \
  data/mnist_dataset/train-labels.idx1-ubyte \
  data/mnist_dataset/t10k-images.idx3-ubyte \
  data/mnist_dataset/t10k-labels.idx1-ubyte \
  1 100 100 0.01
```

Output:

```text
epoch 1/1 loss=41.0573 accuracy=0.16
profile_us epoch=14650000 input=... forward=... loss=... zero_grad=... backward=... update=... eval=...
profile_avg_us_per_sample input=... forward=... loss=... zero_grad=... backward=... update=...
real 14.65
user 14.36
sys 0.05
```

Approximate extrapolation from that run:

| Training samples | Approx. time per epoch |
| ---: | ---: |
| 100 | 15 seconds |
| 1,000 | 2.5 minutes |
| 10,000 | 25 minutes |
| 60,000 | 2.4 hours |

These numbers are rough. Runtime depends on compiler, CPU, build type, test
limit, and whether the loss graph allocation pattern changes.

The built-in `profile_us` line reports per-epoch timing in microseconds:

- `epoch`: Whole epoch including training and evaluation.
- `input`: Pixel flattening and one-hot target creation.
- `forward`: Model forward pass.
- `loss`: Scalar loss graph creation.
- `zero_grad`: Gradient clearing before backpropagation.
- `backward`: Reverse-mode autograd traversal.
- `update`: SGD parameter update.
- `eval`: Accuracy pass over the selected test subset.

The `profile_avg_us_per_sample` line divides the training phases by the number
of successfully trained samples in that epoch.

## Accuracy Expectations

Do not treat the current example as a competitive MNIST classifier.

Main limitations:

- Scalar autograd creates many small heap-allocated `Value` nodes per sample.
- Training is single-sample SGD with no batching.
- Dense layers are implemented through scalar operations, not matrix kernels.
- Classification uses one-hot MSE, not softmax cross-entropy.
- There is no model serialization yet, so trained weights are not saved.
- The model is an MLP, not a convolutional network.

Expected behavior:

- Small runs such as `1 100 100 0.01` mainly verify that the pipeline works.
- Accuracy may be low and noisy on very small subsets.
- Multiple epochs over `1000` to `5000` samples should be more informative.
- Full MNIST training is possible but slow with this implementation.

## Optimization Reference Points

Useful future optimization targets:

- Reduce per-sample graph allocation overhead.
- Add batched training.
- Add tensor or matrix primitives for dense layers.
- Add softmax cross-entropy.
- Add model serialization so inference does not require retraining.
- Use the built-in profile output to target forward, backward, and update
  bottlenecks.

When optimizing, keep this document updated with:

- Command used.
- Dataset subset sizes.
- Epoch count.
- Learning rate.
- Model architecture.
- Build type and platform.
- `real`, `user`, and `sys` timing.
- `profile_us` and `profile_avg_us_per_sample` output.
- Final loss and accuracy.
