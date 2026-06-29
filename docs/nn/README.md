# FORG Neural Network Module

`forg::nn` is a tiny CPU-only neural-network module inspired by
micrograd. It provides scalar reverse-mode autograd plus simple
`Neuron`, `Layer`, and `MLP` helpers.

The API is intentionally small. It is useful for experiments, tests, and
learning-oriented models, not as a replacement for a full machine-learning
runtime.

For practical dense MNIST-style training, the module also includes a small
matrix backend. `MatrixMLP` trains a `Linear -> ReLU -> Linear` classifier with
batched softmax cross-entropy and manual backpropagation, avoiding per-sample
scalar graph allocation.

## Basic Forward Pass

Include the umbrella header:

```cpp
#include <forg/nn.h>
```

Create an MLP and run inputs through it:

```cpp
using namespace forg::nn;

MLP model(2, {8, 1}); // 2 inputs, one hidden layer with 8 neurons, 1 output

std::vector<ValuePtr> input = {
    MakeValue(0.5),
    MakeValue(-1.2),
};

std::vector<ValuePtr> output = model.Forward(input);
if (output.empty())
{
    return;
}

double score = output[0]->GetData();
```

`Forward()` returns an empty vector when the input does not match the network
shape or contains null values.

## Classifier Use

Yes, the module can be used as the core of a small classifier.

For a binary classifier, create one output and treat it as a raw score:

```cpp
bool predicted_class = output[0]->GetData() > 0.0;
```

For multiple classes, create one output per class and choose the largest score:

```cpp
std::size_t best_class = 0;
for (std::size_t i = 1; i < output.size(); ++i)
{
    if (output[i]->GetData() > output[best_class]->GetData())
        best_class = i;
}
```

These scores are raw logits. The current module includes `Softmax()` and
`CrossEntropyLoss()` helpers, scalar mini-batch gradient accumulation, and
plain parameter serialization.

For small multi-class experiments, the helper API provides `Linear`, `ReLU`,
`Dropout`, `Conv2d`, `MaxPool2d`, `BatchNorm`, `Sequential`, `Flatten`,
`ArgMax`, `CrossEntropyLoss`, `SGD`, `MomentumSGD`, and `Adam`:

```cpp
using namespace forg::nn;

Sequential model({
    std::make_shared<Linear>(784, 64),
    std::make_shared<ReLU>(),
    std::make_shared<Linear>(64, 10),
});

SGD optimizer(model.Parameters(), 0.01);
Values input = Flatten::From(normalized_pixels);
Values output = model.Forward(input);
ValuePtr loss = CrossEntropyLoss(output, label);
```

Save trained parameters and load them into another model with the same
architecture:

```cpp
std::string error;
if (!SaveParameters(model, "mnist.nnparams", &error))
{
    return;
}

Sequential loaded({
    std::make_shared<Linear>(784, 64),
    std::make_shared<ReLU>(),
    std::make_shared<Linear>(64, 10),
});

if (!LoadParameters(loaded, "mnist.nnparams", &error))
{
    return;
}
```

The checkpoint stores only the ordered parameter values. Recreate the same
module structure before loading.

`Conv2d` and `MaxPool2d` operate on flattened channel-first image data:
`channel * height * width + y * width + x`. They are scalar-autograd modules,
so they are useful for correctness experiments and tiny image models, not fast
full MNIST CNN training.

## Gradients And Losses

You can build scalar losses from `ValuePtr` operations and call `Backward()`:

```cpp
ValuePtr target = MakeValue(1.0);
ValuePtr prediction = output[0];
ValuePtr loss = Pow(prediction - target, 2.0);

model.ZeroGrad();
Backward(loss);

for (const ValuePtr& parameter : model.Parameters())
{
    double gradient = parameter->GetGrad();
}
```

Gradients accumulate until `ZeroGrad()` or `SetGrad(0.0)` is called.

## Minimal Training Loop

`Value::SetData()` lets callers update parameters directly, but most training
loops should use one of the optimizer helpers. A minimal Adam step looks like
this:

```cpp
std::vector<ValuePtr> output = model.Forward(input);
if (output.empty())
{
    return;
}

ValuePtr target = MakeValue(1.0);
ValuePtr loss = Pow(output[0] - target, 2.0);

Adam optimizer(model.Parameters(), 0.001);
optimizer.ZeroGrad();
Backward(loss);
optimizer.Step();
```

`SGD::Step(scale)`, `MomentumSGD::Step(scale)`, and `Adam::Step(scale)` can
scale accumulated gradients before applying an update, which is useful for
averaged mini-batches.

For binary experiments, `Sigmoid()` can turn one raw score into a probability.
For multi-class classification, prefer raw logits with `CrossEntropyLoss()`.

## MNIST Example

An optional MNIST trainer is available when examples are enabled:

```sh
cmake -S . -B build/examples -DFORG_BUILD_EXAMPLES=ON -DBUILD_TESTING=OFF
cmake --build build/examples --target forg_mnist forg_mnist_infer
```

Run it with IDX image and label files:

```sh
build/examples/examples/mnist/forg_mnist \
  train-images.idx3-ubyte train-labels.idx1-ubyte \
  t10k-images.idx3-ubyte t10k-labels.idx1-ubyte \
  1 1000 200 0.01 16 mnist.nnparams
```

This is a scalar-autograd educational example, so use small subsets first.
Pass `matrix` as the backend to use the faster dense matrix path:

```sh
build/examples/examples/mnist/forg_mnist \
  train-images.idx3-ubyte train-labels.idx1-ubyte \
  t10k-images.idx3-ubyte t10k-labels.idx1-ubyte \
  3 5000 1000 0.05 64 matrix
```

After training, classify one test image from a saved matrix checkpoint:

```sh
build/examples/examples/mnist/forg_mnist_infer \
  mnist_matrix.mnparams \
  t10k-images.idx3-ubyte t10k-labels.idx1-ubyte \
  0
```

Omit the final index to classify the whole IDX dataset and print accuracy.

See `docs/nn/mnist_usage_performance.md` for timing notes and optimization
baseline guidance.
