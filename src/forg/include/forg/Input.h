#pragma once
namespace forg {

enum class InputEventType
{
    PointerDrag,
    Scroll
};

enum class InputButton
{
    None,
    Left,
    Right,
    Middle
};

struct InputEvent
{
    InputEventType Type;
    InputButton Button;
    float DeltaX;
    float DeltaY;
    float ScrollDelta;
};

} // namespace forg
