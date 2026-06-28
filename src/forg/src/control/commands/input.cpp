#include <string>

#include "control/commands/Commands.h"

namespace forg::control {

using forg::net::Command;
using forg::net::TryGetFloat;
using forg::net::TryGetString;

namespace {

bool TryGetButton(const Command& cmd, InputButton& button)
{
    std::string value;
    if (!TryGetString(cmd, "button", value))
        return false;

    if (value == "none")
        button = InputButton::None;
    else if (value == "left")
        button = InputButton::Left;
    else if (value == "right")
        button = InputButton::Right;
    else if (value == "middle")
        button = InputButton::Middle;
    else
        return false;

    return true;
}

std::string DispatchInputEvent(SceneControlContext& ctx,
                               const InputEvent& event)
{
    if (ctx.inputHandler == nullptr)
        return fail("unavailable");

    if (!ctx.inputHandler(event, ctx.inputUserData))
        return fail("unsupported");

    return ok();
}

} // namespace

std::string DispatchInput(SceneControlContext& ctx, const Command& cmd)
{
    const std::string& v = cmd.verb;

    if (v == "input.drag")
    {
        InputButton button = InputButton::None;
        float dx = 0.0f;
        float dy = 0.0f;
        if (!(TryGetButton(cmd, button) && TryGetFloat(cmd, "dx", dx) &&
              TryGetFloat(cmd, "dy", dy)))
        {
            return fail("badparam");
        }

        return DispatchInputEvent(
            ctx, {InputEventType::PointerDrag, button, dx, dy, 0.0f});
    }

    if (v == "input.scroll")
    {
        float delta = 0.0f;
        if (!TryGetFloat(cmd, "delta", delta))
            return fail("badparam");

        return DispatchInputEvent(
            ctx, {InputEventType::Scroll, InputButton::None, 0.0f, 0.0f,
                  delta});
    }

    return fail("unknown");
}

} // namespace forg::control
