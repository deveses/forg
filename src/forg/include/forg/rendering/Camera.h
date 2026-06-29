/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2005  Slawomir Strumecki

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

#ifndef FORG_RENDERING_CAMERA_H
#define FORG_RENDERING_CAMERA_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "enums.h"
#include "math/Matrix4.h"
#include "math/Vector3.h"

namespace forg {

using namespace forg::math;

/// Camera class
/**
 * Camera
 * @author eses
 * @version 1.0
 * @date 07-2005
 * @todo
 * @bug
 * @warning
 */
class Camera
{
    //////////////////////////////////////////////////////////////////////////
    // Nested
    //////////////////////////////////////////////////////////////////////////
  public:
  public:
    FORG_API Camera(CameraView view_type = Perspective);
    virtual FORG_API ~Camera(void);

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    //////////////////////////////////////////////////////////////////////////
  private:
    static const float minimal_distance;

    [[maybe_unused]] float m_roll;
    float m_aspect; ///< pixel aspect ratio
    float m_fovy;   ///< vertical field of view
    float m_near;   ///< near range
    float m_far;    ///< far range
    [[maybe_unused]] float m_unitm_size;
    [[maybe_unused]] float m_tanm_halfm_fov;

    Vector3 m_target;   ///< camera target point
    Vector3 m_position; ///< camera position
    Vector3 m_up;       ///< camera up vector
    Vector3 m_dir;

    Matrix4 m_matProjection;
    Matrix4 m_matView;

    CameraView m_camera_view;

    float m_screen_width;
    float m_screen_height;

    //////////////////////////////////////////////////////////////////////////
    // Properties
    //////////////////////////////////////////////////////////////////////////
  public:
    FORG_API const Vector3& get_Target() const;
    FORG_API void set_Target(const Vector3& value);

    FORG_API const Vector3& get_Position() const;
    FORG_API void set_Position(const Vector3& value);

    FORG_API float get_ScreenWidth() const;

    FORG_API float get_ScreenHeight() const;

    FORG_API float get_NearRange() const;

    FORG_API float get_FarRange() const;

    FORG_API float get_Aspect() const;

    /// Gets vertical field of view (in radians)
    FORG_API float get_FOV() const;

    /*
            FORG_API const Vector3& get_Up() const;
            FORG_API void set_Up(const Vector3& value);

            FORG_API void set_FOV(float value);

            FORG_API void set_Aspect(float value);

            FORG_API void set_NearRange(float value);

            FORG_API void set_FarRange(float value);*/

    FORG_API void set_View(CameraView value);
    FORG_API CameraView get_View() const;

    FORG_API void set_ScreenSize(float width, float height);

    //////////////////////////////////////////////////////////////////////////
    // Public methods
    //////////////////////////////////////////////////////////////////////////
  public:
    // Dolly Camera
    //	Przemieszcza kamer� wzd�u� lokalnej osi Z, tj. wzd�u� osi sto�ka
    // widzenia. Dolly Camera + Target 	Przemieszcza kamer� oraz jej cel, wzd�u�
    // osi sto�ka widzenia. Dolly Target 	Przemieszcza cel w kierunku do
    // lub od kamery, wzd�u� osi sto�ka projekcji. Perspective 	Przemieszcza
    // kamer� wzd�u� osi sto�ka widzenia i jednocze�nie zmienia szeroko�� k�ta
    // widzenia.
    // Roll camera
    //	Obraca kamer� wok� osi sto�ka widzenia.
    // Truck Camera
    //	Przemieszcza kamer� oraz jej cel r�wnolegle do p�aszczyzny okna
    // widokowego. Orbit Camera 	Obraca kamer� wok� celu.

    // TODO: zoom
    // 90 degrees fovh = 25mm equivalent

    FORG_API void FieldOfView(float fov_change);

    /// Dolly
    /**
     * Dolly camera with target
     * Przemieszcza kamere oraz jej cel, wzdluz osi stozka widzenia.
     */
    FORG_API void Dolly(float camera, float target);

    FORG_API void Roll(float angle);

    /// Truck Camera
    /**
     * Przemieszcza kamer� oraz jej cel r�wnolegle do p�aszczyzny okna
     * widokowego.
     */
    FORG_API void Truck(float x, float y);

    FORG_API void Orbit(float x, float y);

    /// Panorama / Tilt
    /**
     * Panorama / Tilt
     * Obraca kamere wokol miejsca jej zaczepienia
     */
    FORG_API void Pan(float x, float y);

    FORG_API void GetProjectionMatrix(Matrix4& projection);
    FORG_API void GetViewMatrix(Matrix4& view);

    //////////////////////////////////////////////////////////////////////////
    // Protected Methods
    //////////////////////////////////////////////////////////////////////////
  protected:
    void SetAspect(float value);
    void SetPosition(const Vector3& value);
    void SetTarget(const Vector3& value);
    void SetUp(const Vector3& value);
    void SetFOVY(float value);

    void UpdateViewMatrix();
    void UpdateProjectionMatrix();
};

} // namespace forg

#endif // FORG_RENDERING_CAMERA_H
