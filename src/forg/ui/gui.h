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

#include "core/vector.hpp"
#include "rendering/Sprite.h"

namespace forg { namespace ui {

    struct SUIElement
    {
        Rectangle tex_coords;
        float angle;

        SUIElement()
        {
            angle = 0.0f;
        }
    };

    class CUIDialog;

    namespace EControlType { enum { Custom, Knob }; }

    class CUIControl
    {
    protected:
        Rectangle m_bounds;
        int m_id;
        int m_type;
        bool m_focus;
        bool m_mouse_over;
        CUIDialog* m_dialog;

    public:
        CUIControl(CUIDialog* _dialog);
        virtual ~CUIControl() {};

    public:
        void SetId(int _id);
        void SetLocation(int _x, int _y);
        void SetSize(int _width, int _height);

        int GetId() const { return m_id; }
        int GetType() const { return m_type; }

    public:
        virtual void Render() {};

        virtual bool    CanHaveFocus()
        {
            return false;
        }

        virtual void    OnFocusIn()
        {
            m_focus = true;
        }

        virtual void    OnFocusOut()
        {
            m_focus = false;
        }

        virtual void    OnMouseEnter()
        {
            m_mouse_over = true;
        }

        virtual void    OnMouseLeave()
        {
            m_mouse_over = false;
        }

        virtual bool ContainsPoint(const Point& _point) const
        {
            int dx = _point.x - m_bounds.left;
            int dy = _point.y - m_bounds.top;

            if (dx <= m_bounds.Width() || dy <= m_bounds.Height())
            {
                return true;
            }

            return false;
        }
    };

    class FORG_API CUIKnob : public CUIControl
    {
    private:
        int m_Min;
        int m_Max;
        int m_Value;

    public:
        CUIKnob(CUIDialog* _dialog);

        void SetValue(int _value);
        void SetRange(int _min, int _max);

        int GetValue() const { return m_Value; }
        int GetMin() const { return m_Min; }
        int GetMax() const { return m_Max; }

        virtual void Render();
    };

    ///////////////////////////////////////////////////////////////////////////
    // Dialog
    ///////////////////////////////////////////////////////////////////////////
    namespace EMouseEvent
    {
        enum 
        {
            None,
            Move,
        };
    }

    namespace EMouseButton
    {
        enum
        {
            Button0,
            Button1,
            Button2
        };
    }

    class FORG_API CUIDialog
    {
        typedef forg::core::vector<CUIControl*> ControlList;

        Sprite* m_Sprite;
        ITexture* m_Texture;

         ControlList m_controls;

    public:
        CUIDialog();
        ~CUIDialog();

        bool Init(IRenderDevice* _device, const char* _filename);
        void Close();
        void Render();

        int AddButton(int _id, int x, int y, int width, int height);
        int AddSlider(int _id, int x, int y, int width, int height);
        int AddKnob(int _id, int x, int y, int width, int height);
        int AddComboBox(int _id, int x, int y, int width, int height);
        int AddControl(CUIControl* _control);

        CUIControl* GetControl(int _id);
        CUIControl* GetControl(int _id, int _type);
        CUIControl* GetControlAtPoint(const Point& _point);
        
        CUIKnob* GetKnob(int _id) { return (CUIKnob*)GetControl(_id, EControlType::Knob); }
            
        bool HandleMouse(int _event_id, Point _point, int _buttons, int _delta);

        int InitControl(CUIControl* _control);
        void DrawSprite(const SUIElement& _element, Rectangle& _rect);

    private:
    };

}}

#endif