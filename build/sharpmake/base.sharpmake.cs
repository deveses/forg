using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

static class ForgConstants
{
	public static readonly string ProjectRootPath = @"[project.SharpmakeCsPath]/../../";  
	public static readonly string SolutionRootPath = @"[solution.SharpmakeCsPath]/../../";  
}

abstract class BaseForgProject : Project
{
    protected string _ForgRootPath;
    protected string _ForgScrPath;

    public BaseForgProject()
    {
        _ForgRootPath = ForgConstants.ProjectRootPath; 
        _ForgScrPath = Path.Combine(_ForgRootPath, "src");        
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
            DevEnv.vs2015,
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
        conf.Name = @"[target.Optimization] [target.OutputType]";

        // Gives a unique path for the project because Visual Studio does not
        // like shared intermediate directories.
        conf.ProjectPath =  Path.Combine(_ForgRootPath, "output");
    }
}

// Represents the solution that will be generated and that will contain the
// project with the sample code.
abstract class BaseSolution : Solution
{
    protected string _ForgRootPath;

    public BaseSolution()
    {
        _ForgRootPath = ForgConstants.SolutionRootPath; 

        // As with the project, define which target this solution builds for.
        // It's usually the same thing.
        AddTargets(new Target(
            //Platform.win32 | Platform.win64,
            Platform.win64,
            DevEnv.vs2015,
            Optimization.Debug | Optimization.Release));
    }

    // Configure for all 4 generated targets. Note that the type of the
    // configuration object is of type Solution.Configuration this time.
    // (Instead of Project.Configuration.)
    [Configure]
    public virtual void ConfigureAll(Solution.Configuration conf, Target target)
    {
        // Puts the generated solution in the /generated folder too.
        conf.SolutionPath = Path.Combine(_ForgRootPath, "output");
     }
};