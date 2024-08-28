using System.Runtime.InteropServices;

namespace JWCEssentials.net;

public class RandomGenerator
{
    /*
       _EXPORT_ void Random_Generator_SetSeed(IntPtr This, uint32_t seed);
       _EXPORT_ uint32_t Random_Generator_Get_uint32_t(IntPtr This);
       _EXPORT_ uint64_t Random_Generator_Get_uint64_t(IntPtr This);
       _EXPORT_ utf8_string_struct Random_Generator_cstyle_identifier(IntPtr This, int length);
       _EXPORT_ void Random_Generator_Reset(IntPtr This);
       _EXPORT_ struct_array_struct<uint8_t> Random_Generator_get_state(IntPtr This);
       _EXPORT_ void Random_Generator_set_state(IntPtr This, struct_array_struct<uint8_t> state);
       _EXPORT_ uint8_t Random_Generator_GetByte(IntPtr This);
       _EXPORT_ P_INSTANCE(Random_MT19937) Random_MT19937_Create(uint32_t seed);

     */

    protected class Imports
    {
        [DllImport("JWCEssentials")]
        public static extern  void Random_Generator_SetSeed(IntPtr This, uint seed);
        [DllImport("JWCEssentials")]
        public static extern  uint Random_Generator_GetSeed(IntPtr This);
        [DllImport("JWCEssentials")]
        public static extern  uint Random_Generator_Get_uint32_t(IntPtr This);
        [DllImport("JWCEssentials")]
        public static extern  ulong Random_Generator_Get_uint64_t(IntPtr This);
        
        [DllImport("JWCEssentials")]
        public static extern  double Random_Generator_Get_double(IntPtr This);
        
        [DllImport("JWCEssentials")]
        public static extern  utf8_string_struct Random_Generator_cstyle_identifier(IntPtr This, int length);
        [DllImport("JWCEssentials")]
        public static extern  void Random_Generator_Reset(IntPtr This);
        [DllImport("JWCEssentials")]
        public static extern  struct_array_struct<byte> Random_Generator_get_state(IntPtr This);
        [DllImport("JWCEssentials")]
        public static extern  void Random_Generator_set_state(IntPtr This, ref struct_array_struct<byte> state);
        [DllImport("JWCEssentials")]
        public static extern byte Random_Generator_GetByte(IntPtr This);

        [DllImport("JWCEssentials")]
        public static extern IntPtr Random_MT19937_Create(uint seed);

        [DllImport("JWCEssentials")]
        public static extern void Random_Destroy(IntPtr This);
    }

    public IntPtr This;
    
    public uint seed
    {
        get => Imports.Random_Generator_GetSeed(This);
        set => Imports.Random_Generator_SetSeed(This, value);
    }

    public uint Get_uint32_t()
    {
        return Imports.Random_Generator_Get_uint32_t(This);
    }
    
    public ulong Get_uint64_t()
    {
        return Imports.Random_Generator_Get_uint64_t(This);
    }

    public string cstyle_identifier(int length = 10)
    {
        return Imports.Random_Generator_cstyle_identifier(This, length);
    }

    public void Reset()
    {
        Imports.Random_Generator_Reset(This);
    }

    public byte[] get_state()
    {
        return Imports.Random_Generator_get_state(This);
    }

    public void set_state(byte[] state)
    {
        struct_array_struct<byte> s = state;
        Imports.Random_Generator_set_state(This, ref s);
    }

    public byte GetByte()
    {
        return Imports.Random_Generator_GetByte(This);
    }

    public double GetDouble()
    {
        return Imports.Random_Generator_Get_double(This);
    }

    public static RandomGenerator MT19937_Create(uint seed)
    {
        RandomGenerator R = new RandomGenerator();
        R.This = Imports.Random_MT19937_Create(seed);
        return R;
    }

    ~RandomGenerator()
    {
        if (This != 0) Imports.Random_Destroy(This);
    }
}