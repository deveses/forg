using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

static class ForgConstants
{
	public static readonly string ProjectRootPath = @"[project.SharpmakeCsPath]/../../";  
	public static readonly string SolutionRootPath = @"[solution.SharpmakeCsPath]/../../";  
}

abstract class BaseForgProject : Project
{
    public string ForgRootPath {get; private set;}
    public string ForgScrPath {get; private set;}
    public string ForgTmpPath {get; private set;}
    public string ForgExternPath {get; private set;}

    public BaseForgProject()
    {
        ForgRootPath = ForgConstants.ProjectRootPath; 
        ForgScrPath = Path.Combine(ForgRootPath, "src");        
        ForgTmpPath = Path.Combine(ForgRootPath, "tmp");        
        ForgExternPath = Path.Combine(ForgRootPath, "extern");        
    }
}

// Both the library and the executable can share these base settings, so create
// a base class for both projects.
abstract class BaseLibraryProject : BaseForgProject
{
    public BaseLibraryProject()
    {
        // Declares the target for which we build the project. This time we add
        // the additional OutputType fragment, which is a prebuilt fragment
        // that help us specify the kind of library output that we want.
        AddTargets(new Target(
            Platform.win64,
            DevEnv.vs2017,
            Optimization.Debug | Optimization.Release,
            OutputType.Dll | OutputType.Lib));
    }

    [Configure]
    public virtual void ConfigureAll(Project.Configuration conf, Target target)
    {
        // This is the name of the configuration. By default, it is set to
        // [target.Optimization] (so Debug or Release), but both the debug and
        // release configurations have both a shared and a static version so
        // that would not create unique configuration names.
        conf.Name = @"[target.Optimization][target.OutputType]";

        // Gives a unique path for the project because Visual Studio does not
        // like shared intermediate directories.
        conf.ProjectPath =  Path.Combine(ForgRootPath, "output");

        conf.TargetPath = Path.Combine("[conf.ProjectPath]", "[target.OutputType]", "[project.Name]", "[target.Optimization]");
        //conf.Options.Add(Sharpmake.Options.Vc.General.WindowsTargetPlatformVersion.v8_1);

		conf.IntermediatePath = Path.Combine("[conf.ProjectPath]", "int", "[project.Name]","[target.Optimization]");
    }
}

// Represents the solution that will be generated and that will contain the
// project with the sample code.
abstract class BaseSolution : Solution
{
    public string ForgRootPath {get; private set;}

    public BaseSolution()
    {
        ForgRootPath = ForgConstants.SolutionRootPath; 

        // As with the project, define which target this solution builds for.
        // It's usually the same thing.
        AddTargets(new Target(
            //Platform.win32 | Platform.win64,
            Platform.win64,
            DevEnv.vs2017,
            Optimization.Debug | Optimization.Release));
    }

    // Configure for all 4 generated targets. Note that the type of the
    // configuration object is of type Solution.Configuration this time.
    // (Instead of Project.Configuration.)
    [Configure]
    public virtual void ConfigureAll(Solution.Configuration conf, Target target)
    {
        // Puts the generated solution in the /generated folder too.
        conf.SolutionPath = Path.Combine(ForgRootPath, "output");
     }
};