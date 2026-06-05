// MIT License
// Copyright (c) 2024 John W. Cornell
// See LICENSE file in the project root for full license information.
using System.Xml.Linq;
using VSXml;

namespace csproj_add_property;

public class Program
{
    public static int Main(string[] args)
    {
        bool Mod = false;

        Dictionary<string, string> switches = new Dictionary<string, string>();

        for (int i = 0; i < args.Length; i++)
        {
            if (args[i][0] == '-')
            {
                string[] s = args[i].Split("=");

                string cmd = "";
                for (int i2 = 1; i2 < s.Length; i2++)
                {
                    if (i2 != 1) cmd += "=";
                    cmd += s[i2];
                }

                switches[s[0]] = cmd;
            }
        }

        string[] t = new string[args.Length - switches.Count];

        int ii = 0;

        for (int i = 0; i < args.Length; i++)
        {
            if (args[i][0] != '-')
            {
                t[ii] = args[i];
                ii++;
            }
        }

        args = t;

        if (args.Length != 2)
        {
            Console.WriteLine("usage: csproj_add_property File.csproj xml_property_as_string");
        }

        CSProj proj = CSProj.Open(args[0]);
        if (proj == null) return 1;

        XElement toAdd = XElement.Parse(args[1]);

        toAdd.Name = proj.Doc.Root.Name.Namespace + toAdd.Name.LocalName;

        bool update = switches.ContainsKey("-u");
        Mod = proj.EnsureProperty(toAdd.Name.LocalName, toAdd.Value, update);

        if (Mod && !proj.Save()) return 1;
        return 0;
    }
}
