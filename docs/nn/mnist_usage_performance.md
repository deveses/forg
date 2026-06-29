# MNIST Example Usage And Performance Notes

This document records how to run the current MNIST example and what performance
to expect from the scalar autograd and dense matrix implementations. Use it as
a baseline when optimizing the neural-network module later.

## Build

The MNIST trainer and inference demo are optional example targets. Build them
with examples enabled:

```sh
cmake -S . -B build/examples -DFORG_BUILD_EXAMPLES=ON -DBUILD_TESTING=OFF
cmake --build build/examples --target forg_mnist forg_mnist_infer
```

The example binaries are expected at:

```sh
build/examples/examples/mnist/forg_mnist
build/examples/examples/mnist/forg_mnist_infer
```

## Run

Usage:

```sh
forg_mnist \
  <train-images> <train-labels> <test-images> <test-labels> \
  [epochs] [train-limit] [test-limit] [learning-rate] [batch-size] \
  [checkpoint-path|backend] [backend]
```

Example using local MNIST IDX files:

```sh
/usr/bin/time -p build/examples/examples/mnist/forg_mnist \
  data/mnist_dataset/train-images.idx3-ubyte \
  data/mnist_dataset/train-labels.idx1-ubyte \
  data/mnist_dataset/t10k-images.idx3-ubyte \
  data/mnist_dataset/t10k-labels.idx1-ubyte \
  3 1000 1000 0.01 16 data/mnist_dataset/mnist.nnparams
```

Arguments:

- `epochs`: Number of passes over the selected training subset.
- `train-limit`: Maximum number of training samples used per epoch.
- `test-limit`: Maximum number of test samples used for accuracy reporting.
- `learning-rate`: SGD learning rate.
- `batch-size`: Number of samples whose gradients are accumulated before one
  averaged optimizer update.
- `checkpoint-path`: Optional parameter file. If it exists, the example loads
  it before training. After training, the example saves current weights there.
- `backend`: Optional `scalar` or `matrix`. `scalar` preserves the educational
  autograd path. `matrix` uses the dense matrix backend.

## Inference

Use `forg_mnist_infer` to classify data from a saved matrix checkpoint without
training:

```sh
build/examples/examples/mnist/forg_mnist_infer \
  mnist_matrix.mnparams \
  data/mnist_dataset/t10k-images.idx3-ubyte \
  data/mnist_dataset/t10k-labels.idx1-ubyte \
  0
```

The optional final argument is the sample index. When it is present, the demo
prints the predicted digit, expected label, match result, and an ASCII rendering
of the image. Omit the index to classify the whole IDX dataset and print
accuracy:

```sh
build/examples/examples/mnist/forg_mnist_infer \
  mnist_matrix.mnparams \
  data/mnist_dataset/t10k-images.idx3-ubyte \
  data/mnist_dataset/t10k-labels.idx1-ubyte
```

## Recommended Starting Points

Use small subsets first. The current implementation is useful for correctness
and educational experiments, not fast full-dataset training.

```sh
# Fast sanity check
/usr/bin/time -p build/examples/examples/mnist/forg_mnist ... 1 100 100 0.01

# First meaningful run with checkpoint save/resume
/usr/bin/time -p build/examples/examples/mnist/forg_mnist ... 3 1000 1000 0.01 16 mnist.nnparams

# Larger scalar-autograd experiment
/usr/bin/time -p build/examples/examples/mnist/forg_mnist ... 3 5000 1000 0.01 32

# Faster dense matrix backend
/usr/bin/time -p build/examples/examples/mnist/forg_mnist ... 3 5000 1000 0.05 64 matrix
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
Training uses `CrossEntropyLoss(output, label)` over raw logits and mini-batch
SGD. Inference predicts the digit with `ArgMax()` over the 10 raw output
scores.

The NN module also includes scalar `Conv2d`, `MaxPool2d`, `Dropout`, and
`BatchNorm` helpers. They compose with `Sequential`, but they still allocate
scalar autograd graph nodes, so a CNN built from them is primarily useful for
small correctness experiments until a tensor backend exists.

The `matrix` backend uses `MatrixMLP`, a dense `Linear -> ReLU -> Linear`
classifier trained with batched softmax cross-entropy and manual
backpropagation. It does not build scalar autograd graphs per sample and is the
recommended path for larger MNIST subsets with the current codebase.

## Scalar Baseline Timing

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

The current example reuses the input `Value` storage across samples and reuses
the scratch containers used by `Backward()`. Scalar operations with literal
constants also avoid allocating temporary constant nodes. The remaining large
allocation cost is the per-sample forward/loss graph itself.

Batching is implemented as scalar mini-batch gradient accumulation: each sample
still runs its own forward and backward pass, parameter gradients accumulate
across the batch, then `SGD::Step(1.0 / batch_size)` applies one averaged
update. This can improve optimization behavior and reduces optimizer-update
frequency, but it is not tensor-vectorized batching.

The built-in `profile_us` line reports per-epoch timing in microseconds:

- `epoch`: Whole epoch including training and evaluation.
- `input`: Pixel flattening.
- `forward`: Model forward pass.
- `loss`: Scalar loss graph creation.
- `zero_grad`: Gradient clearing before backpropagation.
- `backward`: Reverse-mode autograd traversal.
- `update`: SGD parameter update.
- `eval`: Accuracy pass over the selected test subset.

The `profile_avg_us_per_sample` line divides the training phases by the number
of successfully trained samples in that epoch.

## Matrix Backend Timing

Measured locally on 2026-06-29 with:

```sh
/usr/bin/time -p /private/tmp/forg-example-build/examples/mnist/forg_mnist \
  data/mnist_dataset/train-images.idx3-ubyte \
  data/mnist_dataset/train-labels.idx1-ubyte \
  data/mnist_dataset/t10k-images.idx3-ubyte \
  data/mnist_dataset/t10k-labels.idx1-ubyte \
  3 5000 1000 0.05 64 matrix
```

Output:

```text
epoch 1/3 backend=matrix loss=1.30615 accuracy=0.797
profile_us epoch=2114188 input=32267 train_batch=1900666 eval=181149
profile_avg_us_per_sample input=6.4534 train_batch=380.133
epoch 2/3 backend=matrix loss=0.598185 accuracy=0.851
profile_us epoch=2094492 input=31814 train_batch=1887203 eval=175385
profile_avg_us_per_sample input=6.3628 train_batch=377.441
epoch 3/3 backend=matrix loss=0.449604 accuracy=0.864
profile_us epoch=2092788 input=31859 train_batch=1885076 eval=175749
profile_avg_us_per_sample input=6.3718 train_batch=377.015
real 7.59
user 7.50
sys 0.06
```

Approximate timing from that run:

| Backend | Epochs | Train samples | Test samples | Batch size | Total real time | Accuracy |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| matrix | 3 | 5,000 | 1,000 | 64 | 7.59 seconds | 0.864 |

The matrix backend's per-sample `train_batch` time was about 377 to 380 us in
this run. The scalar baseline above was about 15 seconds for one epoch over 100
training samples, while this matrix run completed three epochs over 5,000
training samples plus evaluation in under 8 seconds.

## Full MNIST Matrix Run

Measured locally on 2026-06-29 with the full 60,000-sample training set and
10,000-sample test set:

```sh
/usr/bin/time -p /private/tmp/forg-example-build/examples/mnist/forg_mnist \
  data/mnist_dataset/train-images.idx3-ubyte \
  data/mnist_dataset/train-labels.idx1-ubyte \
  data/mnist_dataset/t10k-images.idx3-ubyte \
  data/mnist_dataset/t10k-labels.idx1-ubyte \
  10 60000 10000 0.05 64 mnist_matrix.mnparams matrix
```

Output:

```text
epoch 1/10 backend=matrix loss=0.473042 accuracy=0.9148
profile_us epoch=24736320 input=381763 train_batch=22573033 eval=1780368
profile_avg_us_per_sample input=6.36272 train_batch=376.217
epoch 2/10 backend=matrix loss=0.268993 accuracy=0.9284
profile_us epoch=24792460 input=382783 train_batch=22639622 eval=1768853
profile_avg_us_per_sample input=6.37972 train_batch=377.327
epoch 3/10 backend=matrix loss=0.223326 accuracy=0.9391
profile_us epoch=24703422 input=380800 train_batch=22548907 eval=1772559
profile_avg_us_per_sample input=6.34667 train_batch=375.815
epoch 4/10 backend=matrix loss=0.193755 accuracy=0.9437
profile_us epoch=24706584 input=381069 train_batch=22558280 eval=1766064
profile_avg_us_per_sample input=6.35115 train_batch=375.971
epoch 5/10 backend=matrix loss=0.172001 accuracy=0.9495
profile_us epoch=24732123 input=381502 train_batch=22579413 eval=1770043
profile_avg_us_per_sample input=6.35837 train_batch=376.324
epoch 6/10 backend=matrix loss=0.154819 accuracy=0.9542
profile_us epoch=24775368 input=381545 train_batch=22624821 eval=1767803
profile_avg_us_per_sample input=6.35908 train_batch=377.08
epoch 7/10 backend=matrix loss=0.140941 accuracy=0.9569
profile_us epoch=24737488 input=381625 train_batch=22583886 eval=1770818
profile_avg_us_per_sample input=6.36042 train_batch=376.398
epoch 8/10 backend=matrix loss=0.129512 accuracy=0.9601
profile_us epoch=24720094 input=381258 train_batch=22569564 eval=1768097
profile_avg_us_per_sample input=6.3543 train_batch=376.159
epoch 9/10 backend=matrix loss=0.119795 accuracy=0.963
profile_us epoch=24799864 input=381896 train_batch=22632860 eval=1783935
profile_avg_us_per_sample input=6.36493 train_batch=377.214
epoch 10/10 backend=matrix loss=0.111567 accuracy=0.9658
profile_us epoch=24727168 input=381301 train_batch=22575643 eval=1769040
profile_avg_us_per_sample input=6.35502 train_batch=376.261
saved_checkpoint=mnist_matrix.mnparams
real 248.72
user 248.16
sys 0.33
```

Summary:

| Backend | Epochs | Train samples | Test samples | Batch size | Total real time | Final accuracy |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| matrix | 10 | 60,000 | 10,000 | 64 | 248.72 seconds | 0.9658 |

Per-epoch time was consistently about 24.7 seconds, with `train_batch` around
376 us per sample and evaluation around 1.77 seconds over the full test set.

## Accuracy Expectations

Do not treat the current example as a competitive MNIST classifier.

Main limitations:

- Scalar autograd creates many small heap-allocated `Value` nodes per sample.
- Batching is gradient accumulation, not vectorized matrix/tensor execution.
- Dense layers are implemented through scalar operations, not matrix kernels.
- Convolution and pooling helpers are scalar modules, not tensor kernels.
- Cross-entropy is implemented through scalar softmax/log operations, not a
  fused log-softmax kernel.
- Checkpoints store parameter values only; callers must recreate the matching
  model architecture before loading.
- The model is an MLP, not a convolutional network.

These limitations apply to the scalar backend. The matrix backend avoids the
largest scalar graph costs for dense MLP training, but it is still a small CPU
implementation rather than a general tensor library.

Expected behavior:

- Small runs such as `1 100 100 0.01` mainly verify that the pipeline works.
- Accuracy may be low and noisy on very small subsets.
- Multiple epochs over `1000` to `5000` samples should be more informative.
- Full MNIST training is possible but slow with this implementation.

## Optimization Reference Points

Useful future optimization targets:

- Further reduce per-sample graph allocation overhead with a graph arena or
  fused dense-layer operations.
- Add tensor-vectorized batched training.
- Add tensor primitives beyond the current dense `MatrixMLP` backend.
- Add fused log-softmax cross-entropy.
- Extend inference to load standalone image files in addition to IDX datasets.
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
