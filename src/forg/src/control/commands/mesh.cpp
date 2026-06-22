#include <string>
#include <utility>

#include "control/commands/Commands.h"

namespace forg::control {

using forg::net::Command;
using forg::net::TryGetFloat;
using forg::net::TryGetInt;
using forg::net::TryGetString;

namespace {
// Keep untrusted request params inside sane bounds so a single request cannot
// trigger an absurd allocation or degenerate geometry.
int ClampSegments(int n) { return (n < 3) ? 3 : (n > 512 ? 512 : n); }
float PositiveOr(float v, float fallback) { return (v > 0.0f) ? v : fallback; }
float NonNegativeOr(float v, float fallback)
{
    return (v >= 0.0f) ? v : fallback;
}
} // namespace

std::string DispatchMesh(SceneControlContext& ctx, const Command& cmd)
{
    const std::string& v = cmd.verb;

    if (v == "mesh.load")
    {
        std::string path;
        if (!TryGetString(cmd, "path", path))
            return fail("badparam");

        forg::geometry::Mesh::MeshPtr loaded =
            forg::geometry::Mesh::FromFile(path.c_str(), 0, ctx.device);
        // FromFile returns an empty (non-null) mesh on failure; treat a mesh
        // with no geometry as a failed load and keep the current mesh.
        if (!loaded || loaded->GetNumVertices() == 0)
            return fail("loadfailed");

        *ctx.mesh = std::move(loaded);
        return ok();
    }
    if (v == "mesh.box")
    {
        float w = 1.0f, h = 1.0f, d = 1.0f;
        TryGetFloat(cmd, "w", w);
        TryGetFloat(cmd, "h", h);
        TryGetFloat(cmd, "d", d);
        w = PositiveOr(w, 1.0f);
        h = PositiveOr(h, 1.0f);
        d = PositiveOr(d, 1.0f);
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
        radius = PositiveOr(radius, 1.0f);
        slices = ClampSegments(slices);
        stacks = ClampSegments(stacks);
        *ctx.mesh =
            forg::geometry::Mesh::Sphere(ctx.device, radius, slices, stacks);
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
        r1 = NonNegativeOr(r1, 1.0f); // a cone tip (r == 0) is valid
        r2 = NonNegativeOr(r2, 1.0f);
        length = PositiveOr(length, 2.0f);
        slices = ClampSegments(slices);
        stacks = ClampSegments(stacks);
        *ctx.mesh = forg::geometry::Mesh::Cylinder(ctx.device, r1, r2, length,
                                                   slices, stacks);
        return ok();
    }
    if (v == "mesh.transform")
    {
        forg::Matrix4 m = forg::Matrix4::Identity;

        float sx = 1.0f, sy = 1.0f, sz = 1.0f;
        if (TryGetFloat(cmd, "sx", sx) && TryGetFloat(cmd, "sy", sy) &&
            TryGetFloat(cmd, "sz", sz))
        {
            m.Scale(sx, sy, sz);
        }

        float rx = 0.0f, ry = 0.0f, rz = 0.0f;
        if (TryGetFloat(cmd, "rx", rx))
            m.RotateX(rx);
        if (TryGetFloat(cmd, "ry", ry))
            m.RotateY(ry);
        if (TryGetFloat(cmd, "rz", rz))
            m.RotateZ(rz);

        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
        TryGetFloat(cmd, "tx", tx);
        TryGetFloat(cmd, "ty", ty);
        TryGetFloat(cmd, "tz", tz);
        m.SetPosition(tx, ty, tz);

        *ctx.meshTm = m;
        return ok();
    }

    return fail("unknown");
}

} // namespace forg::control
