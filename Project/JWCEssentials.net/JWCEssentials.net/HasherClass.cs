using System.Buffers;
using System.Runtime.InteropServices;
using System.Text.Unicode;

namespace JWCEssentials.net;

public class HasherClass
{
    /*

       _EXPORT_ utf8_string_struct escapeStringForCommandLine(utf8_string_struct string);

       _EXPORT_ void Random_Generator_SetSeed(P_INSTANCE(Random_Generator) This, uint32_t seed);
       _EXPORT_ uint32_t Random_Generator_Get_uint32_t(P_INSTANCE(Random_Generator) This);
       _EXPORT_ uint64_t Random_Generator_Get_uint64_t(P_INSTANCE(Random_Generator) This);
       _EXPORT_ utf8_string_struct Random_Generator_cstyle_identifier(P_INSTANCE(Random_Generator) This, int length);
       _EXPORT_ void Random_Generator_Reset(P_INSTANCE(Random_Generator) This);
       _EXPORT_ struct_array_struct<uint8_t> Random_Generator_get_state(P_INSTANCE(Random_Generator) This);
       _EXPORT_ void Random_Generator_set_state(P_INSTANCE(Random_Generator) This, struct_array_struct<uint8_t> state);
       _EXPORT_ uint8_t Random_Generator_GetByte(P_INSTANCE(Random_Generator) This);

       _EXPORT_ P_INSTANCE(HasherClass) HasherFactory_Get(utf8_string_struct Name);

       _EXPORT_ P_INSTANCE(Random_MT19937) Random_MT19937_Create(uint32_t seed);

       _EXPORT_ utf8_string_struct_array feffect_list();
       _EXPORT_ utf8_string_struct feffect_code(utf8_string_struct name);
       _EXPORT_ utf8_string_struct feffect_name(utf8_string_struct code);
       _EXPORT_ utf8_string_struct feffect(utf8_string_struct command, utf8_string_struct escape=nullptr);
       _EXPORT_ utf8_string_struct PlatformLineEnding();

     */

    protected class Imports
    {
        /*
        _EXPORT_ uint32_t HasherClass_get_bits(P_INSTANCE(HasherClass) ctx);
        _EXPORT_ utf8_string_struct HasherClass_get_identifier(P_INSTANCE(HasherClass) ctx);
        _EXPORT_ void HasherClass_Compute(P_INSTANCE(HasherClass) ctx, P_ELEMENTS(uint8_t) m, size_t element_size, size_t element_count);
        _EXPORT_ void HasherClass_Compute_Raw(P_INSTANCE(HasherClass) ctx, P_ELEMENTS(uint8_t) m, size_t element_size, size_t element_count);
        _EXPORT_ void HasherClass_Compute_Rev(P_INSTANCE(HasherClass) ctx, P_ELEMENTS(uint8_t) m, size_t element_size, size_t element_count);
        _EXPORT_ void HasherClass_set_endian(P_INSTANCE(HasherClass) ctx, bool big);
        _EXPORT_ bool HasherClass_get_endian(P_INSTANCE(HasherClass) ctx);
        _EXPORT_ void HasherClass_set_Swap(P_INSTANCE(HasherClass) ctx, bool swap);
        _EXPORT_ bool HasherClass_get_Swap(P_INSTANCE(HasherClass) ctx);
        _EXPORT_ void HasherClass_Hash_Begin(P_INSTANCE(HasherClass) ctx);
        _EXPORT_ void HasherClass_Hash_End(P_INSTANCE(HasherClass) ctx);
        _EXPORT_ void HasherClass_delete(P_INSTANCE(HasherClass) ctx);
        _EXPORT_ P_INSTANCE(HasherClass) HasherFactory_Get(utf8_string_struct Name);
        */

        [DllImport("JWCEssentials")]
        public static extern uint HasherClass_get_bits(IntPtr ctx);

        [DllImport("JWCEssentials")]
        public static extern utf8_string_struct HasherClass_get_identifier(IntPtr ctx);

        [DllImport("JWCEssentials")]
        public static extern void HasherClass_Compute(IntPtr ctx, IntPtr m, IntPtr element_size, IntPtr element_count);

        [DllImport("JWCEssentials")]
        public static extern void HasherClass_Compute_Raw(IntPtr ctx, IntPtr m, IntPtr element_size,
            IntPtr element_count);

        [DllImport("JWCEssentials")]
        public static extern void HasherClass_Compute_Rev(IntPtr ctx, IntPtr m, IntPtr element_size,
            IntPtr element_count);

        [DllImport("JWCEssentials")]
        public static extern void HasherClass_set_endian(IntPtr ctx, bool big);

        [DllImport("JWCEssentials")]
        public static extern bool HasherClass_get_endian(IntPtr ctx);

        [DllImport("JWCEssentials")]
        public static extern void HasherClass_set_Swap(IntPtr ctx, bool swap);

        [DllImport("JWCEssentials")]
        public static extern bool HasherClass_get_Swap(IntPtr ctx);

        [DllImport("JWCEssentials")]
        public static extern void HasherClass_Hash_Begin(IntPtr ctx);

        [DllImport("JWCEssentials")]
        public static extern void HasherClass_Hash_End(IntPtr ctx);

        [DllImport("JWCEssentials")]
        public static extern void HasherClass_delete(IntPtr ctx);

        [DllImport("JWCEssentials")]
        public static extern IntPtr HasherFactory_Get(utf8_string_struct Name);
        
        [DllImport("JWCEssentials", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr HasherFactory_Get(utf8_string_struct_test Name);

        public delegate void ComputeDelegate(IntPtr ctx, IntPtr m, IntPtr element_size, IntPtr element_count);

        public static Imports.ComputeDelegate[] dels = new ComputeDelegate[]
            { HasherClass_Compute, HasherClass_Compute_Raw, HasherClass_Compute_Rev };
    }

    public IntPtr This;

    [StructLayout(LayoutKind.Sequential)]
    public struct utf8_string_struct_test
    {
        public ulong c_str;
        public ulong length;
        public ulong free_cstr;
    }

    public static HasherClass FromFactory(string name)
    {
        utf8_string_struct st = name;

        utf8_string_struct_test t;
        t.c_str = st.c_str;
        t.length = st.length;
        t.free_cstr = st.free_cstr;
        
        IntPtr This = Imports.HasherFactory_Get(t);
        if (This == null) throw new AggregateException("HasherClass::FrinFactory unable to get \"" + name + "\"");

        int bits = (int)Imports.HasherClass_get_bits(This);

        if (bits == 32) return new HasherClass32(This);
        if (bits == 64) return new HasherClass64(This);

        throw new Exception(bits + " unknown for hasher");

    }


    public HasherClass(IntPtr This)
    {
        this.This = This;
    }

    public uint bits => Imports.HasherClass_get_bits(This);
    public string identifier => Imports.HasherClass_get_identifier(This);

    public bool big_endian
    {
        get => Imports.HasherClass_get_endian(This);
        set => Imports.HasherClass_set_endian(This, value);
    }

    public bool swap
    {
        get => Imports.HasherClass_get_Swap(This);
        set => Imports.HasherClass_set_Swap(This, value);
    }

    public void Begin() => Imports.HasherClass_Hash_Begin(This);
    public void End() => Imports.HasherClass_Hash_End(This);

    ~HasherClass()
    {
        if (This != IntPtr.Zero) Imports.HasherClass_delete(This);
    }

    public enum mode
    {
        Normal,
        Rev,
        Raw
    }

    public void Compute(object o, mode mode = mode.Normal)
    {
        Imports.ComputeDelegate del = mode == mode.Normal ? Imports.dels[0] :
            mode == mode.Raw ? Imports.dels[1] : Imports.dels[2];

        if (o == null) o = "&%^@#NULL&%^@#";

        Type t = o.GetType();
        bool array = t.IsArray;
        int count = 1;
        if (t.IsArray)
        {
            t = t.GetElementType();
            count = ((Array)o).Length;
        }

        if (t == typeof(string))
        {
            if (array)
            {
                Array A = o as Array;
                int[] index = new int[A.Rank];

                bool c;


                int d;

                do
                {
                    d = A.Rank - 1;

                    for (int i = 0; i < index.Length; i++)
                    {
                        Console.Write(index[i] + ", ");

                    }

                    Console.WriteLine();

                    Compute(A.GetValue(index));

                    do
                    {
                        c = false;

                        index[d]++;
                        if (index[d] >= A.GetLength(d))
                        {
                            c = true;
                            for (int i = d; i < A.Rank; i++) index[i] = 0;
                            d--;

                        }
                    } while (d >= 0 && c);
                } while (d >= 0);

                return;
            }
            else
            {
                ArrayBufferWriter<byte> bw = new ArrayBufferWriter<byte>();

                int charsRead;
                int bytesWritten;

                Utf8.FromUtf16((string)o, bw.GetSpan(), out charsRead, out bytesWritten);

                Compute(bw.GetSpan().ToArray());

                return;

            }
        }

        if (!t.IsPrimitive)
            throw new ArgumentException("HasherClass is designed to work with primitives string and arrays");

        int esize = Marshal.SizeOf(t);

        GCHandle H = GCHandle.Alloc(o, GCHandleType.Pinned);
        del(This, H.AddrOfPinnedObject(), count, esize);
    }
}

public class HasherClass32 : HasherClass
{
    protected class Imports
    {
        /*
           _EXPORT_ void HasherClass32_set_seed(P_INSTANCE(HasherClass32) ctx, uint32_t seed);
           _EXPORT_ uint32_t HasherClass32_get_seed(P_INSTANCE(HasherClass32) ctx);
           _EXPORT_ uint32_t HasherClass32_get_value(P_INSTANCE(HasherClass32) ctx);
         */

        [DllImport("JWCEssentials")]
        public static extern void HasherClass32_set_seed(IntPtr This, uint seed);

        [DllImport("JWCEssentials")]
        public static extern uint HasherClass32_get_seed(IntPtr This);

        [DllImport("JWCEssentials")]
        public static extern uint HasherClass32_get_value(IntPtr This);

    }

    public HasherClass32(IntPtr This) : base(This)
    {
    }

    public uint seed
    {
        get => Imports.HasherClass32_get_seed(This);
        set => Imports.HasherClass32_set_seed(This, value);
    }

    public uint value
    {
        get => Imports.HasherClass32_get_value(This);
    }
}

public class HasherClass64 : HasherClass
{
    protected class Imports
    {
        /*
           _EXPORT_ uint64_t HasherClass64_get_seed(P_INSTANCE(HasherClass64) ctx);
           _EXPORT_ void HasherClass64_set_seed(P_INSTANCE(HasherClass64) ctx, uint64_t seed);
           _EXPORT_ uint64_t HasherClass64_get_value(P_INSTANCE(HasherClass64) ctx);
        */

        [DllImport("JWCEssentials")]
        public static extern void HasherClass64_set_seed(IntPtr This, uint seed);

        [DllImport("JWCEssentials")]
        public static extern uint HasherClass64_get_seed(IntPtr This);

        [DllImport("JWCEssentials")]
        public static extern uint HasherClass64_get_value(IntPtr This);

    }

    public HasherClass64(IntPtr This) : base(This)
    {
    }

    public uint seed
    {
        get => Imports.HasherClass64_get_seed(This);
        set => Imports.HasherClass64_set_seed(This, value);
    }

    public uint value
    {
        get => Imports.HasherClass64_get_value(This);
    }
}

    
