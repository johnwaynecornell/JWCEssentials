using System.Runtime.InteropServices;

namespace JWCEssentials.net;

public class TLS
{
    public delegate IntPtr TLS_INITIALIZE_VALUE();
    public delegate void TLS_DESTRUCTOR(IntPtr value);
    
    protected class Imports
    {
        [DllImport("JWCEssentials")]
        public static extern IntPtr TLS_Alloc(TLS_INITIALIZE_VALUE initialize_value, TLS_DESTRUCTOR destructor);
        [DllImport("JWCEssentials")]
        public static extern void TLS_Free(IntPtr tls);
        [DllImport("JWCEssentials")]
        public static extern IntPtr TLS_get(IntPtr tls);
    }

    public IntPtr This;
    public TLS_INITIALIZE_VALUE tls_initialize_value;
    public TLS_DESTRUCTOR tls_destructor;

    public TLS(TLS_INITIALIZE_VALUE initialize_value, TLS_DESTRUCTOR destructor)
    {
        this.tls_initialize_value = initialize_value;
        this.tls_destructor = destructor;

        This = Imports.TLS_Alloc(initialize_value, destructor);
    }

    ~TLS()
    {
        if (This != 0) Imports.TLS_Free(This);
        This = 0;
    }

    public void Free()
    {
        if (This != 0) Imports.TLS_Free(This);
        This = 0;
    }

    public IntPtr get()
    {
        return Imports.TLS_get(This);
    }
}
