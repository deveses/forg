#pragma once
#include "forg/base.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace forg::fs {

enum class MountPermissions
{
    ReadOnly,
    ReadWrite
};

class FORG_API Filesystem
{
  public:
    bool Mount(std::string_view name, std::filesystem::path root,
               MountPermissions permissions = MountPermissions::ReadOnly);
    bool Unmount(std::string_view name);

    bool ResolveReadPath(std::string_view path,
                         std::filesystem::path& nativePath) const;
    bool ResolveReadPathRelative(std::string_view basePath,
                                 std::string_view childPath,
                                 std::filesystem::path& nativePath) const;

    bool HasMount(std::string_view name) const;

  private:
    struct MountPoint
    {
        std::filesystem::path root;
        MountPermissions permissions = MountPermissions::ReadOnly;
    };

    static std::string NormalizeMountName(std::string_view name);
    static bool SplitMountedPath(std::string_view path, std::string& mount,
                                 std::string_view& relative);
    static bool IsSafeRelativePath(std::string_view path);
    static std::filesystem::path StripLeadingSeparators(std::string_view path);

    bool ResolveMountedReadPath(std::string_view path,
                                std::filesystem::path& nativePath) const;

    std::unordered_map<std::string, MountPoint> m_mounts;
};

} // namespace forg::fs
