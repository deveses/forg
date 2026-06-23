# FORG Neural Network Module

`forg::nn` is a tiny CPU-only neural-network module inspired by
micrograd. It provides scalar reverse-mode autograd plus simple
`Neuron`, `Layer`, and `MLP` helpers.

The API is intentionally small. It is useful for experiments, tests, and
learning-oriented models, not as a replacement for a full machine-learning
runtime.

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

These scores are raw logits. The current module does not include softmax,
sigmoid, cross-entropy, batching, serialization, or an optimizer.

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

## Training Status

The current public `Value` API exposes `GetData()`, `GetGrad()`, and
`SetGrad()`. It does not yet expose a parameter data mutator, so a complete
training loop cannot update weights through the public API.

To train classifiers inside FORG, the next small addition should be a
`Value::SetData(double)` accessor or an optimizer API that can update
parameters internally.

