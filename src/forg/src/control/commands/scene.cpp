#include <sstream>
#include <string>

#include "control/commands/Commands.h"

namespace forg
{
namespace control
{

using forg::net::Command;
using forg::net::TryGetFloat;

namespace
{

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
      << "\"position\":[" << p.X << "," << p.Y << "," << p.Z << "],"
      << "\"target\":[" << t.X << "," << t.Y << "," << t.Z << "],"
      << "\"fov\":" << ctx.camera->get_FOV() << "},"
      << "\"light\":{"
      << "\"position\":[" << l.Position.X << "," << l.Position.Y << ","
      << l.Position.Z << "],"
      << "\"diffuse\":[" << l.Diffuse.r << "," << l.Diffuse.g << ","
      << l.Diffuse.b << "]},"
      << "\"clearColor\":[" << c.r << "," << c.g << "," << c.b << "],"
      << "\"mesh\":{\"vertices\":" << vertices << ",\"faces\":" << faces
      << "}}";
    return o.str();
}

} // namespace

std::string DispatchScene(SceneControlContext& ctx, const Command& cmd)
{
    const std::string& v = cmd.verb;

    if (v == "light.set")
    {
        float value;
        if (TryGetFloat(cmd, "px", value))
            ctx.light->Position.X = value;
        if (TryGetFloat(cmd, "py", value))
            ctx.light->Position.Y = value;
        if (TryGetFloat(cmd, "pz", value))
            ctx.light->Position.Z = value;
        if (TryGetFloat(cmd, "dr", value))
            ctx.light->Diffuse.r = value;
        if (TryGetFloat(cmd, "dg", value))
            ctx.light->Diffuse.g = value;
        if (TryGetFloat(cmd, "db", value))
            ctx.light->Diffuse.b = value;
        if (TryGetFloat(cmd, "range", value))
            ctx.light->Range = value;
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
    if (v == "state")
    {
        return buildState(ctx);
    }

    return fail("unknown");
}

} // namespace control
} // namespace forg
