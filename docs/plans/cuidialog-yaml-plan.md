# Restore CUIDialog Using YAML

## Summary

Replace the planned `dialog.xml` recovery with `dialog.yml` and load it
through `forg::io::YAMLSerializer`. Keep the `CUIDialog` public API mostly
intact, but make `Load()` read YAML instead of XML.

## Key Changes

- Add `data/ui/dialog.yml` using the serializer's existing array format:

  ```yaml
  dialog:
    controls:
      item_0:
        type: "button"
        id: "0"
        x: "100"
        y: "15"
        width: "50"
        height: "30"
      item_1:
        type: "slider"
        id: "1"
        x: "180"
        y: "15"
        width: "80"
        height: "30"
      item_2:
        type: "knob"
        id: "2"
        x: "0"
        y: "15"
        width: "30"
        height: "30"
      item_3:
        type: "combobox"
        id: "3"
        x: "300"
        y: "15"
        width: "100"
        height: "30"
  ```

- Update `CUIDialog::Load()` to use `YAMLSerializer::OpenRead()`.
- Implement dialog deserialization with:
  - `BeginObject("dialog")`
  - `BeginArray("controls", count)`
  - loop over array items with `BeginObject("control")`
  - read `type`, `id`, `x`, `y`, `width`, `height`
  - dispatch to `AddButton`, `AddSlider`, `AddKnob`, or `AddComboBox`
- Rename app references from `data/ui/dialog.xml` to `data/ui/dialog.yml`.
- Keep the recreated `data/ui/debug_texture2.dds` plan unchanged.
- Use `forg::Font` for overlay/debug text only when `FORG_USE_FREETYPE` is
  enabled. The app code should attempt `Font::CreateIndirect()` with the
  bundled `data/fonts/Roboto-Regular.ttf` path and skip text drawing if it
  returns `nullptr`.
- Keep the CMake resource-copy changes for both `winapp` and `macapp`.
- Keep FreeType opt-in through CMake:
  - configure with `-DFORG_USE_FREETYPE=ON` to download/build FreeType and
    enable `Font.cpp`'s FreeType-backed path;
  - configure with the default `OFF` value to build without text rendering
    while still rendering `CUIDialog` sprites.

## Tests

- Add a CUIDialog YAML load test that verifies `dialog.yml` creates controls
  `0..3`.
- Verify `GetKnob(2)` returns a valid knob.
- Keep the hit-test regression test for `ContainsPoint()`.
- Build `forg`, `winapp`, and `macapp`.
- Verify both `FORG_USE_FREETYPE=OFF` and `FORG_USE_FREETYPE=ON` builds. In
  the enabled build, confirm the bundled font path is copied with app
  resources and FPS/debug text renders; in the disabled build, confirm the
  apps still run and the dialog sprites render without text.

## Assumptions

- Use the existing `YAMLSerializer` object/array format instead of introducing
  generic YAML list syntax.
- `dialog.xml` is removed or left unused; `dialog.yml` becomes the source of
  truth.
- FreeType remains optional. `CUIDialog` itself must not depend on FreeType;
  only optional overlay/debug text uses the font path.
