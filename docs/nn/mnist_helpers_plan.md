# Save And Implement MNIST NN Helpers Plan

## Summary
Save this implementation plan, then add a practical v1 for MNIST training using
the current scalar autograd engine. Keep the API small and PyTorch-inspired,
with helpers for dense layers, sequential composition, flattening, loss,
optimization, and MNIST IDX loading.

## Key Changes
- Extend `forg::nn` with lightweight helpers:
  - `using Values = std::vector<ValuePtr>`.
  - `Linear` as a dense affine layer, reusing existing `Layer` behavior.
  - `ReLU` for vector activation.
  - `Flatten` for converting numeric image data into flat `Values`.
  - `Sequential` for ordered module composition.
  - `MSELoss`, `Softmax`, `CrossEntropyLoss`, `OneHot`, and `ArgMax`.
  - `SGD`, `MomentumSGD`, and `Adam` with `ZeroGrad()` and `Step()`.
  - Mini-batch gradient accumulation through scaled `SGD::Step()`.
  - Model parameter serialization with `SaveParameters()` and
    `LoadParameters()`.
  - Scalar image-model helpers: `Dropout`, `Conv2d`, `MaxPool2d`, and
    `BatchNorm`.
  - Dense matrix backend through `Matrix` and `MatrixMLP` for practical
    batched MNIST training.
- Add MNIST IDX utilities:
  - Read image and label IDX files from user-provided paths.
  - Validate magic numbers and count/dimension consistency.
  - Normalize pixels to `[0.0, 1.0]`.
  - Return samples as flat `std::vector<double>` plus label.
- Add an optional MNIST example executable:
  - Model: `Sequential{Linear(784, 64), ReLU, Linear(64, 10)}`.
  - Loss: cross-entropy over raw logits.
  - Optimizer: SGD.
  - CLI args for train images, train labels, test images, test labels, epochs,
    subset size, learning rate, batch size, and an optional checkpoint path.
  - Print loss and accuracy.
  - Keep full MNIST training manual, not part of default CI.

## Tests
- Extend NN tests for `Linear`, `Sequential`, `Flatten`, `MSELoss`,
  `CrossEntropyLoss`, `SGD`, `OneHot`, and `ArgMax`.
- Add loader tests for valid data, invalid magic numbers, and mismatched
  image/label counts using tiny synthetic IDX files.
- Run:
  - `cmake --build build/debug`
  - `ctest --test-dir build/debug --output-on-failure`

## Future Steps
- Add multithreaded CPU execution for the matrix backend, such as row-parallel
  batch processing with a small thread pool.
- Add SIMD or BLAS-backed matrix kernels for dense layers.
- Add fused log-softmax cross-entropy for lower graph overhead.
- Add an inference-only demo for classifying a single image.
- Document the difference between the educational scalar backend and future
  tensor-backed APIs.

## Assumptions
- v1 remains CPU-only and scalar-autograd-based.
- Existing `Neuron`, `Layer`, and `MLP` APIs remain source-compatible.
- No tensor backend in v1.
- Full MNIST data is not committed to the repo.
