// MIT License
// Copyright (c) 2024 John W. Cornell
// See LICENSE file in the project root for full license information.
using VSXml;

namespace csproj_add_my_references;

public class Program
{
    const string MyReferencePathValue =
        @"$(NewAge)\DotNet\Libs\lib\$(Configuration)\$(TargetFramework)";

    public static int Main(string[] args)
    {
        bool update = false;
        var positional = new List<string>();

        foreach (string arg in args)
        {
            if (arg == "-u")
                update = true;
            else if (arg.StartsWith("-"))
                Console.Error.WriteLine($"Warning: unknown switch '{arg}' ignored");
            else
                positional.Add(arg);
        }

        if (positional.Count < 2)
        {
            Console.Error.WriteLine("usage: csproj_add_my_references [-u] File.csproj Assembly1 [Assembly2 ...]");
            Console.Error.WriteLine();
            Console.Error.WriteLine("  Ensures MyReferencePath is set and adds one <Reference> per assembly name.");
            Console.Error.WriteLine("  -u  Update existing MyReferencePath / Reference entries.");
            return 1;
        }

        string csprojFile = positional[0];
        List<string> assemblies = positional.GetRange(1, positional.Count - 1);

        CSProj proj = CSProj.Open(csprojFile);
        if (proj == null) return 1;

        proj.BackupWithTimestamp();

        bool mod = false;

        mod |= proj.EnsureProperty("MyReferencePath", MyReferencePathValue, update);

        foreach (string assembly in assemblies)
        {
            mod |= proj.AddMyReference(assembly, update);
        }

        if (mod && !proj.Save()) return 1;

        return 0;
    }
}
