// MIT License
// Copyright (c) 2024 John W. Cornell
// See LICENSE file in the project root for full license information.
using System.Xml.Linq;
using VSXml;

namespace csproj_add_reference;

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
            Console.WriteLine("usage: csproj_add_reference File.csproj xml_reference_as_string");
        }

        CSProj proj = CSProj.Open(args[0]);
        if (proj == null) return 1;

        XElement toAdd = XElement.Parse(args[1]);

        toAdd.Name = proj.Doc.Root.Name.Namespace + toAdd.Name.LocalName;

        XElement cur;
        XElement References = proj.GetReferences();
        if (References == null)
        {
            References = new XElement(proj.Doc.Root.Name.Namespace + "ItemGroup");

            cur = null;
            foreach (XElement x in proj.Doc.Root.Elements())
            {
                if (!(x.Name.LocalName == "PropertyGroup" || x.Name.LocalName == "ItemGroup")) break;
                cur = x;
            }

            Mod = true;
            cur.AddAfterSelf(References);
        }

        cur = null;
        foreach (XElement x in References.Elements())
        {
            if (x.Name.LocalName == "Reference" &&
                x.Attribute("Include").Value == toAdd.Attribute("Include").Value)
            {
                cur = x;
                break;
            }
        }

        if (cur == null)
        {
            Mod = true;
            References.Add(toAdd);
        }
        else if (switches.ContainsKey("-u"))
        {
            Mod = true;
            cur.ReplaceWith(toAdd);
        }

        if (Mod && !proj.Save()) return 1;
        return 0;
    }
}
