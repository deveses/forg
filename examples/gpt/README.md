# Tiny Shakespeare GPT Example

This example trains a small character-level GPT-style model. It defaults to the
batched `MatrixGPT` backend, which uses plain `double` arrays and hand-written
backpropagation. A slower scalar autograd backend is still available as a
reference implementation.

Build with examples enabled:

```sh
cmake -S . -B build -DFORG_BUILD_EXAMPLES=ON
cmake --build build --target forg_gpt
```

Run the default Tiny Shakespeare setup:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt
```

Arguments:

```text
forg_gpt [dataset-path] [steps] [block-size] [model-size] [heads] [layers]
         [feed-forward-size] [learning-rate] [generate-count] [seed-text]
         [backend] [batch-size] [thread-count] [checkpoint-path] [target-loss]
```

Fast smoke test:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt 2 4 8 2 1 16 0.001 80 "First" matrix 4
```

Useful settings:

The smoke test only proves that the full read/train/generate path works. For
output that starts to look like text, use more training steps, a larger context
window, and a slightly larger model.

First run that may start making sense:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt 2000 16 32 4 2 64 0.0005 500 "First Citizen:" matrix 32
```

Lighter run if the first one is too slow:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt 1000 12 24 3 1 48 0.001 400 "First Citizen:" matrix 16
```

Better output if you can wait longer:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt 10000 24 48 4 2 96 0.0003 800 "ROMEO:" matrix 32
```

Argument meaning after the dataset path:

```text
steps block-size model-size heads layers feed-forward-size learning-rate generate-count seed-text backend batch-size thread-count checkpoint-path target-loss
```

`steps` is the maximum number of training updates. `target-loss` is optional; if
it is greater than zero, training stops early once the current batch loss is less
than or equal to that value.

Backends:

- `matrix`: default, batched `MatrixGPT`, much faster and trainable for demos
- `scalar`: original scalar autograd reference backend, useful for debugging

Checkpoint example:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt 5000 16 32 4 2 64 0.0005 500 "First Citizen:" matrix 32 0 tiny-shakespeare.gpt
```

Early-stop example:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt 10000 16 32 4 2 64 0.0005 500 "First Citizen:" matrix 32 0 tiny-shakespeare.gpt 1.8
```

Watch `loss` while training:

- `~4.0+`: mostly random characters
- `~3.0`: starts learning spacing and punctuation
- `~2.5`: starts looking text-like
- `<2.0`: much more coherent character-level output

The matrix backend is still a small educational implementation, not a GPU-grade
trainer, but it is the path to use for meaningful Tiny Shakespeare experiments.
