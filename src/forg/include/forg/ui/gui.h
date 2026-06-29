/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef _FORG_UI_GUI_H_
#define _FORG_UI_GUI_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "core/RefPtr.h"
#include "core/string.hpp"
#include "rendering/Sprite.h"
#include "scene/SceneNode.h"

#include <memory>

namespace forg::ui {

enum class GuiControlType
{
    Container,
    Button,
    Slider,
    Knob,
    ComboBox
};

class FORG_API GuiNode : public scene::SceneNode
{
  public:
    GuiNode();
    ~GuiNode() override;

    const char* TypeName() const override;
    bool Save(io::ISerializer& serializer) const override;
    bool Load(io::ISerializer& serializer) override;
    void Render(IRenderDevice* device) override;

    bool LoadResources(IRenderDevice* device);
    void CloseResources();

    void SetId(int id);
    int Id() const;

    void SetControlType(GuiControlType type);
    GuiControlType ControlType() const;

    void SetBounds(int x, int y, int width, int height);
    void SetLocation(int x, int y);
    void SetSize(int width, int height);
    const Rectangle& Bounds() const;
    Rectangle AbsoluteBounds() const;

    void SetTexturePath(const char* path);
    const core::string& TexturePath() const;

    void SetRange(int min, int max);
    int Min() const;
    int Max() const;
    void SetValue(int value);
    int Value() const;

    bool ContainsPoint(const Point& point) const;
    GuiNode* FindById(int id);
    const GuiNode* FindById(int id) const;
    GuiNode* FindAtPoint(const Point& point);
    const GuiNode* FindAtPoint(const Point& point) const;

  private:
    struct UIElement
    {
        Rectangle texCoords;
        float angle = 0.0f;
    };

    const GuiNode* ResourceOwner() const;
    GuiNode* ResourceOwner();
    void RenderControl(IRenderDevice* device);
    void DrawSprite(const UIElement& element, const Rectangle& rect);

    int m_id = 0;
    GuiControlType m_controlType = GuiControlType::Container;
    Rectangle m_bounds = {0, 0, 0, 0};
    int m_min = 0;
    int m_max = 100;
    int m_value = 0;
    core::string m_texturePath;
    std::unique_ptr<Sprite> m_sprite;
    core::RefPtr<ITexture> m_texture;
};

const char* GuiControlTypeName(GuiControlType type);
bool GuiControlTypeFromName(const core::string& name, GuiControlType& type);

} // namespace forg::ui

#endif
