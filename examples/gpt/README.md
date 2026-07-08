# Tiny Shakespeare GPT Example

This example trains a very small character-level GPT-style model with the scalar
autograd NN modules. It is intentionally tiny by default because scalar
attention is useful as a reference implementation, not a fast production
backend.

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
```

Fast smoke test:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt 1 4 8 2 1 16 0.001 80 "First"
```

Useful settings:

The smoke test only proves that the full read/train/generate path works. For
output that starts to look like text, use more training steps, a larger context
window, and a slightly larger model.

First run that may start making sense:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt 2000 16 32 4 2 64 0.0005 500 "First Citizen:"
```

Lighter run if the first one is too slow:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt 1000 12 24 3 1 48 0.001 400 "First Citizen:"
```

Better output if you can wait longer:

```sh
./build/examples/gpt/forg_gpt data/gpt_dataset/tiny_shakespeare_dataset.txt 10000 24 48 4 2 96 0.0003 800 "ROMEO:"
```

Argument meaning after the dataset path:

```text
steps block-size model-size heads layers feed-forward-size learning-rate generate-count seed-text
```

Watch `loss` while training:

- `~4.0+`: mostly random characters
- `~3.0`: starts learning spacing and punctuation
- `~2.5`: starts looking text-like
- `<2.0`: much more coherent character-level output

This example uses the scalar autograd backend, so it is educational and slow.
For genuinely good Tiny Shakespeare output, the next steps are batching,
checkpoint saving, and eventually a matrix/tensor backend.
