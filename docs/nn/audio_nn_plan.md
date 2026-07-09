# Neural Audio Ideas

This note collects practical additions for using `forg::nn` with the engine
audio system. The goal is lightweight real-time audio intelligence, not a large
music or speech model.

## Best First Direction

The most practical first feature is NN-driven DSP parameter control. A small
model can read game state and output normal audio parameters:

```text
player speed
room size
enemy distance
health
danger level
        -> small NN
cutoff, gain, pan, pitch, distortion, reverb send
        -> normal DSP/audio engine
```

This keeps the neural network small, predictable, and useful during gameplay.

## Useful Additions

### Audio Feature Extraction

Add basic signal-analysis helpers:

- RMS and peak level
- zero-crossing rate
- FFT or STFT
- mel spectrogram
- MFCC
- spectral centroid, brightness, and rolloff

These make it possible to build sound classifiers, beat/onset detectors, voice
activity detection, adaptive mixing, and simple material or impact recognition.

### NeuralAudioEffect

Add an `IAudioSource` wrapper that reads from another source, transforms blocks,
and produces PCM output:

```cpp
class NeuralAudioEffect : public IAudioSource {
    std::shared_ptr<IAudioSource> m_input;
    // model, ring buffer, feature buffer, output buffer
};
```

Possible effects:

- denoising
- learned EQ
- learned compressor or limiter
- distortion and tone shaping
- lo-fi, radio, or helmet-comms voice processing
- robotized or creature voice effects

### NeuralAudioSource

Add a model-driven source similar to `AudioGenerator`, but backed by a small
neural model.

Good early targets:

- procedural ambient drones
- creature vocalizations
- engine hum variants
- sci-fi UI sounds
- wind or noise texture generation
- footstep and surface variation

Avoid starting with full music generation. Short looping textures and one-shot
sounds are a better fit for the current framework.

### 1D Audio Layers

The neural module would benefit from audio-shaped layers:

- `Conv1d`
- `CausalConv1d`
- `DilatedConv1d`
- simple upsampling or `Resample1d`
- `LayerNorm`
- output shaping helpers such as `Tanh` or `Softsign`

Small causal convolution networks are more realistic for audio than GPT-style
raw sample generation.

### Spectrogram Models

For many manipulation tasks, operate on spectrogram chunks instead of raw
44.1 kHz samples:

```text
PCM -> STFT/mel -> small NN -> inverse STFT -> PCM
```

This is useful for denoising, filtering, voice-ish effects, and texture
morphing.

### Inference-Only Runtime Path

Audio rendering needs predictable timing. Add a runtime path with:

- fixed sample rate
- fixed block size
- `float` buffers internally
- no heap allocation during audio callbacks
- model loaded once and reused
- simple parameter serialization
- deterministic CPU cost per block

Training can remain offline or in examples; gameplay should use inference only.

## Example Programs

Potential examples:

- `examples/audio_nn/classify_sound.cpp`
  Classifies short WAV clips such as footsteps, hits, explosions, voices, and
  ambience.

- `examples/audio_nn/neural_tone.cpp`
  Uses a small MLP or recurrent model to generate oscillator parameters over
  time.

- `examples/audio_nn/denoise.cpp`
  Reads a noisy WAV and writes a cleaned WAV.

- `examples/audio_nn/neural_effect.cpp`
  Applies a learned distortion, EQ, or dynamics effect to an input WAV.

- `examples/audio_nn/realtime_source.cpp`
  Runs a `NeuralAudioSource` through `AudioMixer`.

## Practical Roadmap

1. Add audio feature extraction helpers and tests.
2. Add `NeuralDspController` for game-state-to-DSP parameters.
3. Add `NeuralAudioEffect` as an `IAudioSource` wrapper.
4. Add simple inference-only dense model support for audio blocks.
5. Add `Conv1d` and causal block processing.
6. Add spectrogram utilities and offline WAV examples.
7. Try short neural sources for ambience and creature sounds.

