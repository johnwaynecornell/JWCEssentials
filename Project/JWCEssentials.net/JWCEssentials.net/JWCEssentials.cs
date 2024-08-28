using System.Runtime.InteropServices;

namespace JWCEssentials.net;

public class Essentials
{
    /*
     
         _EXPORT_ utf8_string_struct escapeStringForCommandLine(utf8_string_struct string);
        
         _EXPORT_ utf8_string_struct_array feffect_list();
         _EXPORT_ utf8_string_struct feffect_code(utf8_string_struct name);
         _EXPORT_ utf8_string_struct feffect_name(utf8_string_struct code);
         _EXPORT_ utf8_string_struct feffect(utf8_string_struct command, utf8_string_struct escape=nullptr);
         _EXPORT_ utf8_string_struct PlatformLineEnding();
     */

    class Imports
    {
        [DllImport("JWCEssentials")]
        public static extern utf8_string_struct escapeStringForCommandLine(ref utf8_string_struct text);
        
        [DllImport("JWCEssentials")]
        public static extern struct_array_struct<utf8_string_struct> feffect_list();
        [DllImport("JWCEssentials")]
        public static extern utf8_string_struct feffect_code(ref utf8_string_struct name);
        [DllImport("JWCEssentials")]
                public static extern utf8_string_struct feffect_name(ref utf8_string_struct code);
        [DllImport("JWCEssentials")]
        public static extern utf8_string_struct feffect(ref utf8_string_struct command, ref utf8_string_struct escape);
        [DllImport("JWCEssentials")]
        public static extern utf8_string_struct PlatformLineEnding();
    }

    public string escapeStringForCommandLine(string text)
    {
        utf8_string_struct p1 = text;
        return Imports.escapeStringForCommandLine(ref p1);
    }
        
    public static string[] feffect_list()
    {
        utf8_string_struct[] raw = Imports.feffect_list();
        string[] R = new string[raw.Length];
        for (int i = 0; i < raw.Length; i++) R[i] = raw[i];
        return R;
    }

    public static string feffect_code(string name)
    {
        utf8_string_struct p1 = name;
        return Imports.feffect_code(ref p1);
    }

    public static string feffect_name(string code)
    {
        utf8_string_struct p1 = code;
        return Imports.feffect_name(ref p1);
    }

    public static string feffect(string command, string escape = null)
    {
        utf8_string_struct p1 = command;
        utf8_string_struct p2 = escape;
        
        return Imports.feffect(ref p1, ref p2);
    }

    public static string PlatformLineEnding()
    {
        return Imports.PlatformLineEnding();
    }
}