#include <sstream>
#include <string>

#include "control/SceneControl.h"

namespace forg { namespace control {

namespace {

using forg::net::Command;
using forg::net::TryGetFloat;
using forg::net::TryGetInt;
using forg::net::TryGetString;

std::string ok()
{
    return "{\"ok\":true}";
}

std::string fail(const char* error)
{
    return std::string("{\"ok\":false,\"error\":\"") + error + "\"}";
}

std::string buildState(SceneControlContext& ctx)
{
    const forg::math::Vector3& p = ctx.camera->get_Position();
    const forg::math::Vector3& t = ctx.camera->get_Target();
    const forg::Light& l = *ctx.light;
    const forg::Color& c = *ctx.clearColor;

    forg::geometry::Mesh::MeshPtr& mesh = *ctx.mesh;
    forg::uint vertices = mesh.is_null() ? 0 : mesh->GetNumVertices();
    forg::uint faces = mesh.is_null() ? 0 : mesh->GetNumFaces();

    std::ostringstream o;
    o << "{\"ok\":true,"
      << "\"camera\":{"
      <<   "\"position\":[" << p.X << "," << p.Y << "," << p.Z << "],"
      <<   "\"target\":["   << t.X << "," << t.Y << "," << t.Z << "],"
      <<   "\"fov\":" << ctx.camera->get_FOV() << "},"
      << "\"light\":{"
      <<   "\"position\":[" << l.Position.X << "," << l.Position.Y << "," << l.Position.Z << "],"
      <<   "\"diffuse\":["  << l.Diffuse.r  << "," << l.Diffuse.g  << "," << l.Diffuse.b  << "]},"
      << "\"clearColor\":[" << c.r << "," << c.g << "," << c.b << "],"
      << "\"mesh\":{\"vertices\":" << vertices << ",\"faces\":" << faces << "}}";
    return o.str();
}

} // namespace

std::string DispatchCommand(SceneControlContext& ctx, const Command& cmd)
{
    const std::string& v = cmd.verb;

    // --- camera ---------------------------------------------------------
    if (v == "camera.orbit")
    {
        float dx = 0.0f, dy = 0.0f;
        TryGetFloat(cmd, "dx", dx);
        TryGetFloat(cmd, "dy", dy);
        ctx.camera->Orbit(dx, dy);
        return ok();
    }
    if (v == "camera.truck")
    {
        float dx = 0.0f, dy = 0.0f;
        TryGetFloat(cmd, "dx", dx);
        TryGetFloat(cmd, "dy", dy);
        ctx.camera->Truck(dx, dy);
        return ok();
    }
    if (v == "camera.dolly")
    {
        float camera = 0.0f, target = 0.0f;
        TryGetFloat(cmd, "camera", camera);
        TryGetFloat(cmd, "target", target);
        ctx.camera->Dolly(camera, target);
        return ok();
    }
    if (v == "camera.zoom")
    {
        float fov = 0.0f;
        if (!TryGetFloat(cmd, "fov", fov)) return fail("badparam");
        ctx.camera->FieldOfView(fov);
        return ok();
    }
    if (v == "camera.roll")
    {
        float angle = 0.0f;
        TryGetFloat(cmd, "angle", angle);
        ctx.camera->Roll(angle);
        return ok();
    }
    if (v == "camera.pan")
    {
        float x = 0.0f, y = 0.0f;
        TryGetFloat(cmd, "x", x);
        TryGetFloat(cmd, "y", y);
        ctx.camera->Pan(x, y);
        return ok();
    }
    if (v == "camera.place")
    {
        float px, py, pz, tx, ty, tz;
        if (!(TryGetFloat(cmd, "px", px) && TryGetFloat(cmd, "py", py) && TryGetFloat(cmd, "pz", pz) &&
              TryGetFloat(cmd, "tx", tx) && TryGetFloat(cmd, "ty", ty) && TryGetFloat(cmd, "tz", tz)))
        {
            return fail("badparam");
        }
        ctx.camera->set_Position(forg::math::Vector3(px, py, pz));
        ctx.camera->set_Target(forg::math::Vector3(tx, ty, tz));
        return ok();
    }

    // --- mesh -----------------------------------------------------------
    if (v == "mesh.load")
    {
        std::string path;
        if (!TryGetString(cmd, "path", path)) return fail("badparam");

        forg::geometry::Mesh::MeshPtr loaded =
            forg::geometry::Mesh::FromFile(path.c_str(), 0, ctx.device);
        // FromFile returns an empty (non-null) mesh on failure; treat a mesh
        // with no geometry as a failed load and keep the current mesh.
        if (loaded.is_null() || loaded->GetNumVertices() == 0) return fail("loadfailed");

        *ctx.mesh = loaded; // destructive assign frees the previous mesh
        return ok();
    }
    if (v == "mesh.box")
    {
        float w = 1.0f, h = 1.0f, d = 1.0f;
        TryGetFloat(cmd, "w", w);
        TryGetFloat(cmd, "h", h);
        TryGetFloat(cmd, "d", d);
        *ctx.mesh = forg::geometry::Mesh::Box(ctx.device, w, h, d);
        return ok();
    }
    if (v == "mesh.sphere")
    {
        float radius = 1.0f;
        int slices = 16, stacks = 16;
        TryGetFloat(cmd, "radius", radius);
        TryGetInt(cmd, "slices", slices);
        TryGetInt(cmd, "stacks", stacks);
        *ctx.mesh = forg::geometry::Mesh::Sphere(ctx.device, radius, slices, stacks);
        return ok();
    }
    if (v == "mesh.cylinder")
    {
        float r1 = 1.0f, r2 = 1.0f, length = 2.0f;
        int slices = 16, stacks = 16;
        TryGetFloat(cmd, "r1", r1);
        TryGetFloat(cmd, "r2", r2);
        TryGetFloat(cmd, "length", length);
        TryGetInt(cmd, "slices", slices);
        TryGetInt(cmd, "stacks", stacks);
        *ctx.mesh = forg::geometry::Mesh::Cylinder(ctx.device, r1, r2, length, slices, stacks);
        return ok();
    }
    if (v == "mesh.transform")
    {
        forg::Matrix4 m = forg::Matrix4::Identity;

        float sx = 1.0f, sy = 1.0f, sz = 1.0f;
        if (TryGetFloat(cmd, "sx", sx) && TryGetFloat(cmd, "sy", sy) && TryGetFloat(cmd, "sz", sz))
        {
            m.Scale(sx, sy, sz);
        }

        float rx = 0.0f, ry = 0.0f, rz = 0.0f;
        if (TryGetFloat(cmd, "rx", rx)) m.RotateX(rx);
        if (TryGetFloat(cmd, "ry", ry)) m.RotateY(ry);
        if (TryGetFloat(cmd, "rz", rz)) m.RotateZ(rz);

        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
        TryGetFloat(cmd, "tx", tx);
        TryGetFloat(cmd, "ty", ty);
        TryGetFloat(cmd, "tz", tz);
        m.SetPosition(tx, ty, tz);

        *ctx.meshTm = m;
        return ok();
    }

    // --- light / clear --------------------------------------------------
    if (v == "light.set")
    {
        float value;
        if (TryGetFloat(cmd, "px", value)) ctx.light->Position.X = value;
        if (TryGetFloat(cmd, "py", value)) ctx.light->Position.Y = value;
        if (TryGetFloat(cmd, "pz", value)) ctx.light->Position.Z = value;
        if (TryGetFloat(cmd, "dr", value)) ctx.light->Diffuse.r = value;
        if (TryGetFloat(cmd, "dg", value)) ctx.light->Diffuse.g = value;
        if (TryGetFloat(cmd, "db", value)) ctx.light->Diffuse.b = value;
        if (TryGetFloat(cmd, "range", value)) ctx.light->Range = value;
        return ok();
    }
    if (v == "clear.color")
    {
        float r = 0.0f, g = 0.0f, b = 0.0f;
        TryGetFloat(cmd, "r", r);
        TryGetFloat(cmd, "g", g);
        TryGetFloat(cmd, "b", b);
        *ctx.clearColor = forg::Color(r, g, b);
        return ok();
    }

    // --- query ----------------------------------------------------------
    if (v == "state")
    {
        return buildState(ctx);
    }

    return fail("unknown");
}

}}
