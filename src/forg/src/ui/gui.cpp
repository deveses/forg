#include "forg_pch.h"
#include "gui.h"
#include "math/Math.h"
#include "script/xml/XMLSerializer.h"

namespace forg{ namespace ui{

//#define FORG_UI_BUTTON_RECT {0, 0, 136, 54}
#define FORG_UI_BUTTON_RECT_BG {128, 0, 128+50, 17}
#define FORG_UI_BUTTON_RECT_FG {178, 0, 178+50, 17}
#define FORG_UI_SLIDER_BG_RECT {10, 191, 10+92, 191+41}
#define FORG_UI_SLIDER_FG_RECT {161, 197, 161+41, 197+41}
#define FORG_UI_KNOB_BG_RECT {128, 19, 128+35, 19+35}
#define FORG_UI_KNOB_FG_RECT {163, 19, 163+35, 19+35}
#define FORG_UI_RECT_COMBOBOX_BG {15, 85, 15+240, 85+42}
#define FORG_UI_RECT_COMBOBOX_BUTTON {106, 193, 106+53, 193+49}
#define FORG_UI_RECT_COMBOBOX_DROPDOWN {21, 127, 21+228, 127+37}
#define FORG_UI_RECT_COMBOBOX_SELECTION {20, 167, 20+227, 167+20}
#define FORG_UI_RECT_SCROLLBAR_TRACK {204, 195+21, 204+22, 195+32}
#define FORG_UI_RECT_SCROLLBAR_UP {204, 195+1, 204+22, 195+21}
#define FORG_UI_RECT_SCROLLBAR_DOWN {204, 195+32, 204+22, 195+53}
#define FORG_UI_RECT_SCROLLBAR_BUTTON {228, 196, 228+18, 196+42}

///////////////////////////////////////////////////////////////////////////////
// CUIControl
///////////////////////////////////////////////////////////////////////////////

CUIControl::CUIControl(CUIDialog* _dialog)
{
    m_dialog = _dialog;

    m_id = 0;
    m_type = EControlType::Custom;

    m_bounds.left = m_bounds.top = 0;
    m_bounds.bottom = m_bounds.right = 10;
}

void CUIControl::SetId(int _id)
{
    m_id = _id;
}

void CUIControl::SetLocation(int _x, int _y)
{
    int w = m_bounds.right - m_bounds.bottom;
    int h = m_bounds.bottom - m_bounds.top;

    m_bounds.left = _x;
    m_bounds.top = _y;
    m_bounds.right = m_bounds.left + w;
    m_bounds.bottom = m_bounds.top + h;
}

void CUIControl::SetSize(int _width, int _height)
{
    m_bounds.right = m_bounds.left + _width;
    m_bounds.bottom = m_bounds.top + _height;
}

///////////////////////////////////////////////////////////////////////////////
// CUIButton
///////////////////////////////////////////////////////////////////////////////
class CUIButton : public CUIControl
{
public:
    CUIButton(CUIDialog* _dialog);

    virtual void Render();
};

CUIButton::CUIButton(CUIDialog* _dialog)
    : CUIControl(_dialog)
{
}

void CUIButton::Render()
{
    forg::Rectangle rect_tex = FORG_UI_BUTTON_RECT_BG;

    SUIElement el;

    el.tex_coords = rect_tex;

    m_dialog->DrawSprite(el, m_bounds);

    forg::Rectangle rect_tex2 = FORG_UI_BUTTON_RECT_FG;
    el.tex_coords = rect_tex2;

    m_dialog->DrawSprite(el, m_bounds);
}

///////////////////////////////////////////////////////////////////////////////
// CUIComboBox
///////////////////////////////////////////////////////////////////////////////
class CUIComboBox : public CUIControl
{
public:
    CUIComboBox(CUIDialog* _dialog);

    virtual void Render();
};

CUIComboBox::CUIComboBox(CUIDialog* _dialog)
    : CUIControl(_dialog)
{
}

void CUIComboBox::Render()
{
    forg::Rectangle text_rect = m_bounds;
    forg::Rectangle text_tex = FORG_UI_RECT_COMBOBOX_BG;

    text_rect.right -= text_rect.Height();

    forg::Rectangle button_rect = m_bounds;
    forg::Rectangle button_tex = FORG_UI_RECT_COMBOBOX_BUTTON;

    button_rect.left = text_rect.right;

    SUIElement el;
    el.tex_coords = text_tex;
    m_dialog->DrawSprite(el, text_rect);

    el.tex_coords = button_tex;
    m_dialog->DrawSprite(el, button_rect);

    /*
    forg::Rectangle rect_tex2 = FORG_UI_BUTTON_RECT_FG;
    el.tex_coords = rect_tex2;

    m_dialog->DrawSprite(el, m_bounds);
    */
}

///////////////////////////////////////////////////////////////////////////////
// CUISlider
///////////////////////////////////////////////////////////////////////////////
class CUISlider : public CUIControl
{
private:
    int m_Min;
    int m_Max;
    int m_Value;

public:
    CUISlider(CUIDialog* _dialog);

    virtual void Render();
};

CUISlider::CUISlider(CUIDialog* _dialog)
    : CUIControl(_dialog)
{
    m_Min = 0;
    m_Max = 100;
    m_Value = 0;
}

void CUISlider::Render()
{
    int range = m_Max - m_Min;
    float pos = float(m_Value-m_Min)/float(range);    
    forg::Rectangle rect_tex = FORG_UI_SLIDER_BG_RECT;
    
    SUIElement el;
    el.tex_coords = rect_tex;
    m_dialog->DrawSprite(el, m_bounds);

    forg::Rectangle rect_tex2 = FORG_UI_SLIDER_FG_RECT;
    
    SUIElement el2;
    el2.tex_coords = rect_tex2;
    forg::Rectangle rect_button = m_bounds;
    rect_button.right = rect_button.left + rect_button.Height();
    rect_button.Offset(-rect_button.Width()/2, 0);
    rect_button.Offset(pos*m_bounds.Width(), 0);
    m_dialog->DrawSprite(el2, rect_button);
}

///////////////////////////////////////////////////////////////////////////////
// CUIKnob
///////////////////////////////////////////////////////////////////////////////

CUIKnob::CUIKnob(CUIDialog* _dialog)
    : CUIControl(_dialog)
{
    m_type = EControlType::Knob;

    m_Min = 0;
    m_Max = 100;
    m_Value = 0;
}

void CUIKnob::SetValue(int _value)
{
    m_Value = min(_value, m_Max);
    m_Value = max(m_Value, m_Min);
}

void CUIKnob::SetRange(int _min, int _max)
{
    m_Min = _min;
    m_Max = _max;
}

void CUIKnob::Render()
{
    int range = m_Max - m_Min;
    int half_range = range/2;
    float angle = 0.5f*1.5f*Math::PI*float(m_Value-half_range)/float(half_range);    
    forg::Rectangle rect_tex = FORG_UI_KNOB_BG_RECT;

    SUIElement el;

    el.tex_coords = rect_tex;

    m_dialog->DrawSprite(el, m_bounds);

    forg::Rectangle rect_tex2 = FORG_UI_KNOB_FG_RECT;

    SUIElement el2;

    el2.tex_coords = rect_tex2;
    el2.angle = angle;

    m_dialog->DrawSprite(el2, m_bounds);
}


///////////////////////////////////////////////////////////////////////////////
// CUIDialog
///////////////////////////////////////////////////////////////////////////////

CUIDialog::CUIDialog()
{
    m_Sprite = 0;
    m_Texture = 0;
}

CUIDialog::~CUIDialog()
{
    Close();
}

void CUIDialog::Close()
{
    for (uint i=0; i<m_controls.size(); i++)
    {
        delete m_controls[i];
    }

    m_controls.clear();

    if (m_Sprite)
    {
        delete m_Sprite;
        m_Sprite = 0;
    }

    if (m_Texture)
    {
        m_Texture->Release();
        m_Texture = 0;
    }

}

bool CUIDialog::Init(IRenderDevice* _device, const char* _filename)
{
    m_Texture = forg::ITexture::FromFile(_device, _filename);
    m_Sprite = forg::Sprite::CreateSprite(_device);

    return true;
}

bool CUIDialog::Load(const char* _filename)
{
    forg::io::XMLSerializer xmlfile;

    if (xmlfile.Open(_filename))
    {
        Serialize(&xmlfile);

        return true;
    }

    return false;
}

void CUIDialog::Serialize(forg::io::ISerializer* _serializer)
{

}

void CUIDialog::Render()
{
    /*
    Matrix4 tm;
    float angle = 0.5f*1.5f*Math::PI*float(d_value-50)/50.0f;
    
    tm.Scale(0.5f, 0.5f, 0.5f);
    m_Sprite->SetTransform(&tm);

    forg::Rectangle rect = {3, 252, 3+121, 252+121};
    //forg::Rectangle rect = {3, 376, 3+121, 376+121};
    forg::Vector3 translation(0.0f, 15.0f, 0.0f);
    forg::Vector3 center((rect.right - rect.left)/2.0f, (rect.bottom - rect.top)/2.0f, 0.0f);
    m_Sprite->Begin(forg::SpriteFlags::AlphaBlend);
    m_Sprite->Draw(m_Texture, &rect, 0, &translation, forg::Color4b(0xff, 0xff, 0xff, 0xff));
    m_Sprite->End();

    tm.RotateZ(angle);
    m_Sprite->SetTransform(&tm);

    forg::Rectangle rect2 = {3, 376, 3+121, 376+121};
    rect = rect2;
    center = Vector3((rect.right - rect.left)/2.0f, (rect.bottom - rect.top)/2.0f, 0.0f);
    m_Sprite->Begin(forg::SpriteFlags::AlphaBlend);
    m_Sprite->Draw(m_Texture, &rect, &center, &translation, forg::Color4b(0xff, 0xff, 0xff, 0xff));
    m_Sprite->End();
    */
    /*
    m_Sprite->SetTransform(0);

    forg::Rectangle rectButton = {0, 0, 136, 54};
    translation.X = 100.0f;
    m_Sprite->Begin(forg::SpriteFlags::AlphaBlend);
    m_Sprite->Draw(m_Texture, &rectButton, 0, &translation, forg::Color4b(0xff, 0xff, 0xff, 0xff));
    m_Sprite->End();
    */

    for (uint i=0; i<m_controls.size(); i++)
    {
        m_controls[i]->Render();
    }
    
}

void CUIDialog::DrawSprite(const SUIElement& _element, Rectangle& _rect)
{
    Vector3 translation;
    Vector3 center;

    translation.X = _rect.left;
    translation.Y = _rect.top;
    translation.Z = 0.0f;

    float sx = float(_rect.right - _rect.left)/float(_element.tex_coords.right - _element.tex_coords.left);
    float sy = float(_rect.bottom - _rect.top)/float(_element.tex_coords.bottom - _element.tex_coords.top);

    Matrix4 tm;

    Matrix4::Scaling(tm, sx, sy, 1.0f);
    if (_element.angle != 0.0f)
    {
        center.X = float(_element.tex_coords.right - _element.tex_coords.left)/2.0f;
        center.Y = float(_element.tex_coords.bottom - _element.tex_coords.top)/2.0f;
        center.Z = 0.0f;

        tm.RotateZ(_element.angle);
    }

    m_Sprite->SetTransform(&tm);

    m_Sprite->Begin(forg::SpriteFlags::AlphaBlend);
    m_Sprite->Draw(m_Texture, &_element.tex_coords, &center, &translation, forg::Color4b(0xff, 0xff, 0xff, 0xff));
    m_Sprite->End();
}

CUIControl* CUIDialog::GetControl(int _id)
{
    for(uint i=0; i<m_controls.size(); i++)
    {
        if (m_controls[i]->GetId() == _id)
        {
            return m_controls[i];
        }
    }

    return 0;
}

CUIControl* CUIDialog::GetControl(int _id, int _type)
{
    for(uint i=0; i<m_controls.size(); i++)
    {
        if (m_controls[i]->GetType() == _type && m_controls[i]->GetId() == _id)
        {
            return m_controls[i];
        }
    }

    return 0;
}

CUIControl* CUIDialog::GetControlAtPoint(const Point& _point)
{
    for(uint i=0; i<m_controls.size(); i++)
    {
        if (m_controls[i]->ContainsPoint(_point))
        {
            return m_controls[i];
        }
    }

    return 0;
}

bool CUIDialog::HandleMouse(int _event_id, Point _point, int _buttons, int _delta)
{
    CUIControl* c = GetControlAtPoint(_point);

    if (c!=0)
    {
    }

    return false;
}

int CUIDialog::InitControl(CUIControl* _control)
{
    return FORG_OK;
}

int CUIDialog::AddControl(CUIControl* _control)
{
    InitControl(_control);

    m_controls.push_back(_control);

    return FORG_OK;
}

int CUIDialog::AddButton(int _id, int x, int y, int width, int height)
{
    CUIButton* c = new CUIButton(this);

    AddControl(c);

    // TODO: set control's properies
    c->SetId(_id);
    c->SetLocation(x, y);
    c->SetSize(width, height);

    return FORG_OK;
}

int CUIDialog::AddSlider(int _id, int x, int y, int width, int height)
{
    CUISlider* c = new CUISlider(this);

    AddControl(c);

    // TODO: set control's properies
    c->SetId(_id);
    c->SetLocation(x, y);
    c->SetSize(width, height);

    return FORG_OK;
}

int CUIDialog::AddKnob(int _id, int x, int y, int width, int height)
{
    CUIKnob* c = new CUIKnob(this);

    AddControl(c);

    // TODO: set control's properies
    c->SetId(_id);
    c->SetLocation(x, y);
    c->SetSize(width, height);

    return FORG_OK;
}

int CUIDialog::AddComboBox(int _id, int x, int y, int width, int height)
{
    CUIComboBox* c = new CUIComboBox(this);

    AddControl(c);

    // TODO: set control's properies
    c->SetId(_id);
    c->SetLocation(x, y);
    c->SetSize(width, height);

    return FORG_OK;
}


}}

