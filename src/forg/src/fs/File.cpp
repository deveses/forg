#include "forg_pch.h"

#include "forg/fs/Filesystem.h"

#include <cctype>
#include <utility>

namespace forg::fs {
namespace {

bool IsPathSeparator(char ch) { return ch == '/' || ch == '\\'; }

bool IsWindowsDrivePath(std::string_view path)
{
    return path.size() >= 3 &&
           std::isalpha(static_cast<unsigned char>(path[0])) &&
           path[1] == ':' && IsPathSeparator(path[2]);
}

} // namespace

std::string Filesystem::NormalizeMountName(std::string_view name)
{
    if (!name.empty() && name.back() == ':')
        name.remove_suffix(1);

    return std::string(name);
}

bool Filesystem::SplitMountedPath(std::string_view path, std::string& mount,
                                  std::string_view& relative)
{
    if (IsWindowsDrivePath(path))
        return false;

    const std::string_view::size_type colon = path.find(':');
    if (colon == std::string_view::npos)
        return false;

    if (colon == 0)
        return false;

    mount = NormalizeMountName(path.substr(0, colon + 1));
    relative = path.substr(colon + 1);
    return true;
}

std::filesystem::path Filesystem::StripLeadingSeparators(std::string_view path)
{
    while (!path.empty() && IsPathSeparator(path.front()))
        path.remove_prefix(1);

    return std::filesystem::path(std::string(path));
}

bool Filesystem::IsSafeRelativePath(std::string_view path)
{
    const std::filesystem::path nativePath = StripLeadingSeparators(path);
    if (nativePath.is_absolute())
        return false;

    for (const std::filesystem::path& part : nativePath)
    {
        if (part == "..")
            return false;
    }

    return true;
}

bool Filesystem::Mount(std::string_view name, std::filesystem::path root,
                       MountPermissions permissions)
{
    const std::string normalized = NormalizeMountName(name);
    if (normalized.empty())
        return false;

    MountPoint mount;
    mount.root = std::move(root);
    mount.permissions = permissions;
    m_mounts[normalized] = std::move(mount);
    return true;
}

bool Filesystem::Unmount(std::string_view name)
{
    return m_mounts.erase(NormalizeMountName(name)) > 0;
}

bool Filesystem::HasMount(std::string_view name) const
{
    return m_mounts.find(NormalizeMountName(name)) != m_mounts.end();
}

bool Filesystem::ResolveMountedReadPath(
    std::string_view path, std::filesystem::path& nativePath) const
{
    std::string mountName;
    std::string_view relative;
    if (!SplitMountedPath(path, mountName, relative))
        return false;

    const auto mount = m_mounts.find(mountName);
    if (mount == m_mounts.end())
        return false;

    if (!IsSafeRelativePath(relative))
        return false;

    nativePath = mount->second.root / StripLeadingSeparators(relative);
    return true;
}

bool Filesystem::ResolveReadPath(std::string_view path,
                                 std::filesystem::path& nativePath) const
{
    std::string mountName;
    std::string_view relative;
    if (SplitMountedPath(path, mountName, relative))
        return ResolveMountedReadPath(path, nativePath);

    nativePath = std::filesystem::path(std::string(path));
    return true;
}

bool Filesystem::ResolveReadPathRelative(
    std::string_view basePath, std::string_view childPath,
    std::filesystem::path& nativePath) const
{
    std::string mountName;
    std::string_view baseRelative;
    if (!SplitMountedPath(basePath, mountName, baseRelative))
    {
        std::string childMount;
        std::string_view childRelative;
        if (SplitMountedPath(childPath, childMount, childRelative))
            return ResolveMountedReadPath(childPath, nativePath);

        const std::filesystem::path childNative{std::string(childPath)};
        if (childNative.is_absolute())
        {
            nativePath = childNative;
            return true;
        }

        nativePath = std::filesystem::path(std::string(basePath))
                         .parent_path() /
                     childNative;
        return true;
    }

    std::string childMount;
    std::string_view childRelative;
    if (SplitMountedPath(childPath, childMount, childRelative))
        return ResolveMountedReadPath(childPath, nativePath);

    if (!IsSafeRelativePath(childPath))
        return false;

    std::filesystem::path baseNative;
    if (!ResolveMountedReadPath(basePath, baseNative))
        return false;

    nativePath = baseNative.parent_path() / StripLeadingSeparators(childPath);
    return true;
}

} // namespace forg::fs
