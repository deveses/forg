#ifndef FORG_INPUT_H
#define FORG_INPUT_H

#if _MSC_VER > 1000
#pragma once
#endif

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

#endif // FORG_INPUT_H
