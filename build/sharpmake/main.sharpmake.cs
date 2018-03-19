using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

[module: Sharpmake.Include("libraries.sharpmake.cs")]

// The executable that consumes the library.
[Generate]
class WinAppLibraryProject : BaseLibraryProject
{
    public WinAppLibraryProject()
    {
        Name = "winapp";
        SourceRootPath = Path.Combine(ForgScrPath, "winapp");
    }

    public override void ConfigureAll(Project.Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Defines.Add("FORG_STATIC");
        conf.Defines.Add("EMFC_STATIC");

        conf.LibraryFiles.Add(@"Comctl32.lib");

        // This line tells Sharpmake that this project has a dependency on the
        // library project. This will cause all exported include paths and
        // exported defines to be automatically added to this project.
        conf.AddPrivateDependency<EmfcLibraryProject>(target);
        conf.AddPrivateDependency<ForgLibraryProject>(target);        
    }
}

// Represents the solution that will be generated and that will contain the
// project with the sample code.
[Generate]
class ForgSolution : BaseSolution
{
    public ForgSolution()
    {
        // The name of the solution.
        Name = "ForgDemo";
    }

    // Configure for all 4 generated targets. Note that the type of the
    // configuration object is of type Solution.Configuration this time.
    // (Instead of Project.Configuration.)
    public override void ConfigureAll(Solution.Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        // Adds the project described by BasicsProject into the solution.
        // Note that this is done in the configuration, so you can generate
        // solutions that contain different projects based on their target.
        //
        // You could, for example, exclude a project that only supports 64-bit
        // from the 32-bit targets.
        conf.AddProject<EmfcLibraryProject>(target);
        conf.AddProject<ForgLibraryProject>(target);      
        conf.AddProject<GLRendererLibraryProject>(target);      
        conf.AddProject<WinAppLibraryProject>(target);      
     }

    [Main]
    public static void SharpmakeMain(Arguments sharpmakeArgs)
    {
        // Tells Sharpmake to generate the solution described by
        // ForgSolution.
        sharpmakeArgs.Generate<ForgSolution>();
    }
};

