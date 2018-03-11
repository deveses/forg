using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

[module: Sharpmake.Include("base.sharpmake.cs")]

// The library project.
[Generate]
class EmfcLibraryProject : BaseLibraryProject
{
    public EmfcLibraryProject()
    {
        Name = "emfc";
        SourceRootPath = Path.Combine(_ForgScrPath, "emfc");
    }

    public override void ConfigureAll(Project.Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        // Setup the precompiled headers for the project. Just assigning a
        // value to those fields is enough for Sharpmake to understand that
        // the project has precompiled headers.
        conf.PrecompHeader = "StdAfx.h";
        conf.PrecompSource = "StdAfx.cpp";

        // Sets the include path of the library. Those will be shared with any
        // project that adds this one as a dependency. (The executable here.)
        conf.IncludePaths.Add(Path.Combine(SourceRootPath, "inc"));

        // The library wants LIBRARY_COMPILE defined when it compiles the
        // library, so that it knows whether it must use dllexport or
        // dllimport.
        conf.Defines.Add("LIBRARY_COMPILE");

        if (target.OutputType == OutputType.Dll)
        {
            // We want this to output a shared library. (DLL)
            conf.Output = Configuration.OutputType.Dll;

            // This library project expects LIBRARY_DLL symbol to be defined
            // when used as a DLL. While we could define it in the executable,
            // it is better to put it as an exported define. That way, any
            // projects with a dependency on this one will have LIBRARY_DLL
            // automatically defined by Sharpmake.
            conf.ExportDefines.Add("LIBRARY_DLL");

            // Exported defines are not necessarily defines as well, so we need
            // to add LIBRARY_DLL as an ordinary define too.
            conf.Defines.Add("LIBRARY_DLL");
        }
        else if (target.OutputType == OutputType.Lib)
        {
            // We want this to output a static library. (LIB)
            conf.Output = Configuration.OutputType.Lib;
        }
    }
}

[Generate]
class ForgLibraryProject : BaseLibraryProject
{
    public ForgLibraryProject()
    {
        Name = "forg";
        SourceRootPath = Path.Combine(_ForgScrPath, "forg");
    }

    public override void ConfigureAll(Project.Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        // Setup the precompiled headers for the project. Just assigning a
        // value to those fields is enough for Sharpmake to understand that
        // the project has precompiled headers.
        conf.PrecompHeader = "forg_pch.h";
        conf.PrecompSource = "forg_pch.cpp";

        // Sets the include path of the library. Those will be shared with any
        // project that adds this one as a dependency. (The executable here.)
        //conf.IncludePaths.Add(Path.Combine(SourceRootPath, "inc"));

        // The library wants LIBRARY_COMPILE defined when it compiles the
        // library, so that it knows whether it must use dllexport or
        // dllimport.
        conf.Defines.Add("LIBRARY_COMPILE");

        if (target.OutputType == OutputType.Dll)
        {
            // We want this to output a shared library. (DLL)
            conf.Output = Configuration.OutputType.Dll;

            // This library project expects LIBRARY_DLL symbol to be defined
            // when used as a DLL. While we could define it in the executable,
            // it is better to put it as an exported define. That way, any
            // projects with a dependency on this one will have LIBRARY_DLL
            // automatically defined by Sharpmake.
            conf.ExportDefines.Add("LIBRARY_DLL");

            // Exported defines are not necessarily defines as well, so we need
            // to add LIBRARY_DLL as an ordinary define too.
            conf.Defines.Add("LIBRARY_DLL");
        }
        else if (target.OutputType == OutputType.Lib)
        {
            // We want this to output a static library. (LIB)
            conf.Output = Configuration.OutputType.Lib;
        }
    }
}

[Generate]
class GLRendererLibraryProject : BaseLibraryProject
{
    public GLRendererLibraryProject()
    {
        Name = "glrenderer";
        SourceRootPath = Path.Combine(_ForgScrPath, "glrenderer");
    }

    public override void ConfigureAll(Project.Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        
        // Sets the include path of the library. Those will be shared with any
        // project that adds this one as a dependency. (The executable here.)
        //conf.IncludePaths.Add(Path.Combine(SourceRootPath, "inc"));

        // The library wants LIBRARY_COMPILE defined when it compiles the
        // library, so that it knows whether it must use dllexport or
        // dllimport.
        conf.Defines.Add("LIBRARY_COMPILE");

        if (target.OutputType == OutputType.Dll)
        {
            // We want this to output a shared library. (DLL)
            conf.Output = Configuration.OutputType.Dll;

            // This library project expects LIBRARY_DLL symbol to be defined
            // when used as a DLL. While we could define it in the executable,
            // it is better to put it as an exported define. That way, any
            // projects with a dependency on this one will have LIBRARY_DLL
            // automatically defined by Sharpmake.
            conf.ExportDefines.Add("LIBRARY_DLL");

            // Exported defines are not necessarily defines as well, so we need
            // to add LIBRARY_DLL as an ordinary define too.
            conf.Defines.Add("LIBRARY_DLL");
        }
        else if (target.OutputType == OutputType.Lib)
        {
            // We want this to output a static library. (LIB)
            conf.Output = Configuration.OutputType.Lib;
        }
    }
}