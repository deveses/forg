#include "gui.h"
#include "forg_pch.h"
#include "forg/io/ISerializer.h"
#include "math/Math.h"

#include <algorithm>
#include <cstring>

namespace forg::ui {
namespace {

#define FORG_UI_BUTTON_RECT_BG {128, 0, 128 + 50, 17}
#define FORG_UI_BUTTON_RECT_FG {178, 0, 178 + 50, 17}
#define FORG_UI_SLIDER_BG_RECT {10, 191, 10 + 92, 191 + 41}
#define FORG_UI_SLIDER_FG_RECT {161, 197, 161 + 41, 197 + 41}
#define FORG_UI_KNOB_BG_RECT {128, 19, 128 + 35, 19 + 35}
#define FORG_UI_KNOB_FG_RECT {163, 19, 163 + 35, 19 + 35}
#define FORG_UI_RECT_COMBOBOX_BG {15, 85, 15 + 240, 85 + 42}
#define FORG_UI_RECT_COMBOBOX_BUTTON {106, 193, 106 + 53, 193 + 49}

bool StringEquals(const core::string& lhs, const char* rhs)
{
    return std::strcmp(lhs.c_str(), rhs) == 0;
}

float RangePosition(int minValue, int maxValue, int value)
{
    const int range = maxValue - minValue;
    if (range <= 0)
        return 0.0f;

    return float(value - minValue) / float(range);
}

} // namespace

const char* GuiControlTypeName(GuiControlType type)
{
    switch (type)
    {
    case GuiControlType::Container:
        return "container";
    case GuiControlType::Button:
        return "button";
    case GuiControlType::Slider:
        return "slider";
    case GuiControlType::Knob:
        return "knob";
    case GuiControlType::ComboBox:
        return "combobox";
    }
    return "container";
}

bool GuiControlTypeFromName(const core::string& name, GuiControlType& type)
{
    if (StringEquals(name, "container"))
        type = GuiControlType::Container;
    else if (StringEquals(name, "button"))
        type = GuiControlType::Button;
    else if (StringEquals(name, "slider"))
        type = GuiControlType::Slider;
    else if (StringEquals(name, "knob"))
        type = GuiControlType::Knob;
    else if (StringEquals(name, "combobox"))
        type = GuiControlType::ComboBox;
    else
        return false;

    return true;
}

GuiNode::GuiNode() = default;

GuiNode::~GuiNode() { CloseResources(); }

const char* GuiNode::TypeName() const { return "GuiNode"; }

bool GuiNode::Save(io::ISerializer& serializer) const
{
    if (!SceneNode::Save(serializer) || !serializer.BeginObject("gui"))
        return false;

    core::string controlType(GuiControlTypeName(m_controlType));
    core::string texturePath = m_texturePath;

    int id = m_id;
    int x = m_bounds.left;
    int y = m_bounds.top;
    int minValue = m_min;
    int maxValue = m_max;
    int value = m_value;

    if (!serializer.Value("control_type", controlType) ||
        !serializer.Value("id", id) || !serializer.Value("x", x) ||
        !serializer.Value("y", y))
    {
        return false;
    }

    int width = m_bounds.Width();
    int height = m_bounds.Height();
    if (!serializer.Value("width", width) ||
        !serializer.Value("height", height) ||
        !serializer.Value("texture_path", texturePath) ||
        !serializer.Value("min", minValue) ||
        !serializer.Value("max", maxValue) || !serializer.Value("value", value))
    {
        return false;
    }

    return serializer.EndObject();
}

bool GuiNode::Load(io::ISerializer& serializer)
{
    if (!SceneNode::Load(serializer) || !serializer.BeginObject("gui"))
        return false;

    core::string controlType;
    int id = 0;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    if (!serializer.Value("control_type", controlType) ||
        !serializer.Value("id", id) || !serializer.Value("x", x) ||
        !serializer.Value("y", y) || !serializer.Value("width", width) ||
        !serializer.Value("height", height))
    {
        return false;
    }

    GuiControlType parsedType = GuiControlType::Container;
    if (!GuiControlTypeFromName(controlType, parsedType))
        return false;

    core::string texturePath;
    serializer.Value("texture_path", texturePath);

    int minValue = 0;
    int maxValue = 100;
    int value = 0;
    serializer.Value("min", minValue);
    serializer.Value("max", maxValue);
    serializer.Value("value", value);

    if (!serializer.EndObject())
        return false;

    m_controlType = parsedType;
    m_id = id;
    SetBounds(x, y, width, height);
    m_texturePath = texturePath;
    SetRange(minValue, maxValue);
    SetValue(value);
    return true;
}

bool GuiNode::LoadResources(IRenderDevice* device)
{
    CloseResources();

    if (m_texturePath.length() != 0)
    {
        m_texture.reset(ITexture::FromFile(device, m_texturePath.c_str()));
        if (!m_texture)
            return false;

        m_sprite.reset(Sprite::CreateSprite(device));
        if (!m_sprite)
            return false;
    }

    return true;
}

void GuiNode::CloseResources()
{
    m_sprite.reset();
    m_texture.reset();
}

void GuiNode::Render(IRenderDevice* device)
{
    RenderControl(device);
    SceneNode::Render(device);
}

void GuiNode::SetId(int id) { m_id = id; }

int GuiNode::Id() const { return m_id; }

void GuiNode::SetControlType(GuiControlType type) { m_controlType = type; }

GuiControlType GuiNode::ControlType() const { return m_controlType; }

void GuiNode::SetBounds(int x, int y, int width, int height)
{
    m_bounds.left = x;
    m_bounds.top = y;
    m_bounds.right = x + width;
    m_bounds.bottom = y + height;
}

void GuiNode::SetLocation(int x, int y)
{
    SetBounds(x, y, m_bounds.Width(), m_bounds.Height());
}

void GuiNode::SetSize(int width, int height)
{
    m_bounds.right = m_bounds.left + width;
    m_bounds.bottom = m_bounds.top + height;
}

const Rectangle& GuiNode::Bounds() const { return m_bounds; }

Rectangle GuiNode::AbsoluteBounds() const
{
    Rectangle bounds = m_bounds;
    const TreeNode* parent = Parent();
    while (parent != nullptr)
    {
        const GuiNode* parentGui = dynamic_cast<const GuiNode*>(parent);
        if (parentGui != nullptr)
            bounds.Offset(parentGui->m_bounds.left, parentGui->m_bounds.top);
        parent = parent->Parent();
    }
    return bounds;
}

void GuiNode::SetTexturePath(const char* path)
{
    m_texturePath = path != nullptr ? path : "";
}

const core::string& GuiNode::TexturePath() const { return m_texturePath; }

void GuiNode::SetRange(int minValue, int maxValue)
{
    m_min = minValue;
    m_max = std::max(maxValue, minValue);
    SetValue(m_value);
}

int GuiNode::Min() const { return m_min; }

int GuiNode::Max() const { return m_max; }

void GuiNode::SetValue(int value)
{
    m_value = std::min(std::max(value, m_min), m_max);
}

int GuiNode::Value() const { return m_value; }

bool GuiNode::ContainsPoint(const Point& point) const
{
    const Rectangle bounds = AbsoluteBounds();
    return point.x >= bounds.left && point.x <= bounds.right &&
           point.y >= bounds.top && point.y <= bounds.bottom;
}

GuiNode* GuiNode::FindById(int id)
{
    if (m_id == id)
        return this;

    for (TreeNode* child : Children())
    {
        GuiNode* guiNode = dynamic_cast<GuiNode*>(child);
        if (guiNode == nullptr)
            continue;

        GuiNode* found = guiNode->FindById(id);
        if (found != nullptr)
            return found;
    }

    return nullptr;
}

const GuiNode* GuiNode::FindById(int id) const
{
    return const_cast<GuiNode*>(this)->FindById(id);
}

GuiNode* GuiNode::FindAtPoint(const Point& point)
{
    for (auto it = Children().rbegin(); it != Children().rend(); ++it)
    {
        GuiNode* guiNode = dynamic_cast<GuiNode*>(*it);
        if (guiNode == nullptr)
            continue;

        GuiNode* found = guiNode->FindAtPoint(point);
        if (found != nullptr)
            return found;
    }

    return ContainsPoint(point) ? this : nullptr;
}

const GuiNode* GuiNode::FindAtPoint(const Point& point) const
{
    return const_cast<GuiNode*>(this)->FindAtPoint(point);
}

const GuiNode* GuiNode::ResourceOwner() const
{
    const GuiNode* node = this;
    while (node != nullptr)
    {
        if (node->m_sprite && node->m_texture)
            return node;

        node = dynamic_cast<const GuiNode*>(node->Parent());
    }
    return nullptr;
}

GuiNode* GuiNode::ResourceOwner()
{
    return const_cast<GuiNode*>(
        static_cast<const GuiNode*>(this)->ResourceOwner());
}

void GuiNode::RenderControl(IRenderDevice* device)
{
    if (m_controlType == GuiControlType::Container)
        return;

    GuiNode* owner = ResourceOwner();
    if (owner == nullptr)
        return;

    device->SetRenderState(RenderStates_Lighting, false);

    const Rectangle bounds = AbsoluteBounds();
    UIElement element;

    switch (m_controlType)
    {
    case GuiControlType::Button:
    {
        element.texCoords = FORG_UI_BUTTON_RECT_BG;
        owner->DrawSprite(element, bounds);
        element.texCoords = FORG_UI_BUTTON_RECT_FG;
        owner->DrawSprite(element, bounds);
        break;
    }

    case GuiControlType::Slider:
    {
        element.texCoords = FORG_UI_SLIDER_BG_RECT;
        owner->DrawSprite(element, bounds);

        UIElement thumb;
        thumb.texCoords = FORG_UI_SLIDER_FG_RECT;
        Rectangle thumbBounds = bounds;
        thumbBounds.right = thumbBounds.left + thumbBounds.Height();
        thumbBounds.Offset(-thumbBounds.Width() / 2, 0);
        thumbBounds.Offset(
            static_cast<int>(RangePosition(m_min, m_max, m_value) *
                             bounds.Width()),
            0);
        owner->DrawSprite(thumb, thumbBounds);
        break;
    }

    case GuiControlType::Knob:
    {
        element.texCoords = FORG_UI_KNOB_BG_RECT;
        owner->DrawSprite(element, bounds);

        UIElement knob;
        knob.texCoords = FORG_UI_KNOB_FG_RECT;
        knob.angle = 0.5f * 1.5f * Math::PI *
                     (RangePosition(m_min, m_max, m_value) * 2.0f - 1.0f);
        owner->DrawSprite(knob, bounds);
        break;
    }

    case GuiControlType::ComboBox:
    {
        Rectangle textBounds = bounds;
        textBounds.right -= textBounds.Height();

        Rectangle buttonBounds = bounds;
        buttonBounds.left = textBounds.right;

        element.texCoords = FORG_UI_RECT_COMBOBOX_BG;
        owner->DrawSprite(element, textBounds);
        element.texCoords = FORG_UI_RECT_COMBOBOX_BUTTON;
        owner->DrawSprite(element, buttonBounds);
        break;
    }

    case GuiControlType::Container:
        break;
    }
}

void GuiNode::DrawSprite(const UIElement& element, const Rectangle& rect)
{
    if (!m_sprite || !m_texture)
        return;

    Vector3 translation;
    Vector3 center;

    translation.X = rect.left;
    translation.Y = rect.top;
    translation.Z = 0.0f;

    const float sx = float(rect.Width()) / float(element.texCoords.Width());
    const float sy = float(rect.Height()) / float(element.texCoords.Height());

    Matrix4 transform;
    Matrix4::Scaling(transform, sx, sy, 1.0f);
    if (element.angle != 0.0f)
    {
        center.X = float(element.texCoords.Width()) / 2.0f;
        center.Y = float(element.texCoords.Height()) / 2.0f;
        center.Z = 0.0f;

        transform.RotateZ(element.angle);
    }

    m_sprite->SetTransform(&transform);
    m_sprite->Begin(SpriteFlags::AlphaBlend);
    m_sprite->Draw(m_texture.get(), &element.texCoords, &center, &translation,
                   Color4b(0xff, 0xff, 0xff, 0xff));
    m_sprite->End();
}

} // namespace forg::ui
