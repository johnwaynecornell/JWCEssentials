// MIT License
// Copyright (c) 2024 John W. Cornell
// See LICENSE file in the project root for full license information.
using System.Xml.Linq;

namespace VSXml;

public class CSProj
{
    public string filename;

    public XDocument Doc;

    protected CSProj()
    {
    }

    public static CSProj Open(string fileName)
    {
        if (!File.Exists(fileName))
        {
            Console.WriteLine("CSharp project file \"{0}\" not found", fileName);
            return null;
        }

        XDocument Doc = XDocument.Load(fileName);

        if (Doc.Root.Name.LocalName != "Project")
        {
            Console.WriteLine("file \"{0}\" is not a CSharp project \"{1}\"", fileName, Doc.Root.Name.LocalName);
            return null;
        }

        CSProj Ret = new CSProj();
        Ret.filename = fileName;
        Ret.Doc = Doc;

        return Ret;
    }

    public bool Backup()
    {
        return true;
    }

    public bool BackupWithTimestamp()
    {
        string stamp = DateTime.Now.ToString("yyyyMMdd_HHmmss");
        string dest = filename + "." + stamp + ".bak";
        File.Copy(filename, dest, overwrite: false);
        return true;
    }

    public bool Save()
    {
        Backup();
        Doc.Save(filename);
        return true;
    }

    public IEnumerable<string> ListReferences()
    {
        foreach (XElement group in Doc.Root.Elements())
        {
            if (group.Name.LocalName != "ItemGroup")
            {
                continue;
            }

            foreach (XElement reference in group.Elements())
            {
                if (reference.Name.LocalName == "Reference")
                {
                    yield return GetSimpleName(reference.Attribute("Include").Value).Split("\\").Last();
                }
                else if (reference.Name.LocalName == "ProjectReference")
                {
                    string assemblyPath = GetStagedProjectReferenceAssemblyPath(reference);
                    if (assemblyPath != null)
                    {
                        yield return assemblyPath;
                    }
                }
            }
        }
    }

    public static string[] strip = { ".dll", ".exe", ".csproj" };

    public string GetSimpleName(string path)
    {
        string s = Path.GetFileName(path);

        foreach (string st in strip)
        {
            if (s.EndsWith(st)) return s.Substring(0, s.Length - st.Length);
        }

        return s;
    }

    private string GetStagedProjectReferenceAssemblyPath(XElement reference)
    {
        XAttribute include = reference.Attribute("Include");
        if (include == null)
        {
            return null;
        }

        string projectDirectory = Path.GetDirectoryName(Path.GetFullPath(filename));
        string referencedProjectFile = Path.GetFullPath(Path.Combine(projectDirectory, include.Value)).Replace("\\", "/");

        if (!File.Exists(referencedProjectFile))
        {
            return null;
        }

        CSProj referencedProject = Open(referencedProjectFile);
        if (referencedProject == null)
        {
            return null;
        }

        if (!referencedProject.HasPostBuildCommand("NewAge_stage"))
        {
            return null;
        }

        string myReferencePath = referencedProject.GetPropertyValue("MyReferencePath");
        if (string.IsNullOrEmpty(myReferencePath))
        {
            return null;
        }

        string assemblyName = referencedProject.GetPropertyValue("AssemblyName");
        if (string.IsNullOrEmpty(assemblyName))
        {
            assemblyName = GetSimpleName(referencedProjectFile);
        }

        return assemblyName;
    }

    private bool HasPostBuildCommand(string text)
    {
        foreach (XElement target in Doc.Root.Elements())
        {
            if (target.Name.LocalName != "Target")
            {
                continue;
            }

            XAttribute name = target.Attribute("Name");
            if (name == null || name.Value != "PostBuild")
            {
                continue;
            }

            foreach (XElement child in target.Descendants())
            {
                XAttribute command = child.Attribute("Command");
                if (command != null && command.Value.Contains(text))
                {
                    return true;
                }

                if (child.Value.Contains(text))
                {
                    return true;
                }
            }
        }

        return false;
    }

    public string GetPropertyValue(string propertyName)
    {
        foreach (XElement group in Doc.Root.Elements())
        {
            if (group.Name.LocalName != "PropertyGroup")
            {
                continue;
            }

            foreach (XElement property in group.Elements())
            {
                if (property.Name.LocalName == propertyName)
                {
                    return property.Value;
                }
            }
        }

        return null;
    }

    // Ensures a property exists in the second PropertyGroup (user properties).
    // Creates the group if needed. Updates existing value when update=true.
    // Returns true if the document was modified.
    public bool EnsureProperty(string propertyName, string value, bool update = false)
    {
        XElement firstGroup = null;
        XElement secondGroup = null;

        foreach (XElement x in Doc.Root.Elements())
        {
            if (x.Name.LocalName == "PropertyGroup" && x.Attribute("Condition") == null)
            {
                if (firstGroup == null)
                    firstGroup = x;
                else
                {
                    secondGroup = x;
                    break;
                }
            }
        }

        if (firstGroup == null)
        {
            Console.WriteLine("Damaged project file: no PropertyGroup found");
            return false;
        }

        if (secondGroup == null)
        {
            secondGroup = new XElement(Doc.Root.Name.Namespace + "PropertyGroup");
            firstGroup.AddAfterSelf(secondGroup);
        }

        XElement existing = null;
        foreach (XElement x in secondGroup.Elements())
        {
            if (x.Name.LocalName == propertyName)
            {
                existing = x;
                break;
            }
        }

        if (existing == null)
        {
            var elem = new XElement(Doc.Root.Name.Namespace + propertyName) { Value = value };
            secondGroup.Add(elem);
            return true;
        }
        else if (update)
        {
            existing.Value = value;
            return true;
        }

        return false;
    }

    // Adds <Reference Include="$(MyReferencePath)\assemblyName.dll" />.
    // Updates the existing entry when update=true.
    // Returns true if the document was modified.
    public bool AddMyReference(string assemblyName, bool update = false)
    {
        string includePath = @"$(MyReferencePath)\" + assemblyName + ".dll";

        XElement toAdd = new XElement(Doc.Root.Name.Namespace + "Reference");
        toAdd.SetAttributeValue("Include", includePath);

        XElement References = GetReferences();
        if (References == null)
        {
            References = new XElement(Doc.Root.Name.Namespace + "ItemGroup");

            XElement cur = null;
            foreach (XElement x in Doc.Root.Elements())
            {
                if (!(x.Name.LocalName == "PropertyGroup" || x.Name.LocalName == "ItemGroup")) break;
                cur = x;
            }

            cur.AddAfterSelf(References);
        }

        XElement existing = null;
        foreach (XElement x in References.Elements())
        {
            if (x.Name.LocalName == "Reference" &&
                x.Attribute("Include")?.Value == includePath)
            {
                existing = x;
                break;
            }
        }

        if (existing == null)
        {
            References.Add(toAdd);
            return true;
        }
        else if (update)
        {
            existing.ReplaceWith(toAdd);
            return true;
        }

        return false;
    }

    public static bool Contains(XElement parent, string childElement)
    {
        foreach (XElement child in parent.Elements())
        {
            if (child.Name.LocalName == childElement)
            {
                return true;
            }
        }

        return false;
    }

    public XElement GetItemGroupFor(string ElementType)
    {
        foreach (XElement group in Doc.Root.Elements())
        {
            if (group.Name.LocalName == "ItemGroup")
            {
                if (Contains(group, ElementType)) return group;
            }
        }

        return null;
    }

    public XElement GetReferences()
    {
        return GetItemGroupFor("Reference");
    }
}
