using System;
using System.Buffers;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Unicode;

namespace JWCEssentials.net
{
    [StructLayout(LayoutKind.Sequential)]
    public struct utf8_string_struct : IDisposable
    {
        private IntPtr c_str;
        private IntPtr length;
        private IntPtr free_cstr;

        delegate void delegate_free_c_str(IntPtr c_str);
        
        private static void my_free(IntPtr c_str)
        {
            Marshal.FreeCoTaskMem(c_str);
        }
        
        private static delegate_free_c_str d_my_free = my_free;
        private static IntPtr p_my_free = Marshal.GetFunctionPointerForDelegate(d_my_free);
        
        public utf8_string_struct()
        {
            c_str = IntPtr.Zero;
            length = IntPtr.Zero;
        }

        public static implicit operator string(utf8_string_struct handle)
        {
            return Marshal.PtrToStringUTF8(handle.c_str);
        }

        public static implicit operator utf8_string_struct(string str)
        {
            ArrayBufferWriter<byte> bw = new ArrayBufferWriter<byte>();

            int charsRead;
            int bytesWritten;

            Utf8.FromUtf16(str, bw.GetSpan(), out charsRead, out bytesWritten);

            utf8_string_struct handle;
            handle.c_str = Marshal.AllocCoTaskMem(bytesWritten);
            Marshal.Copy(bw.WrittenSpan.ToArray(), 0, handle.c_str, bytesWritten);
            handle.length = (IntPtr)bytesWritten;
            handle.free_cstr = p_my_free; 

            return handle;
        }

        public void Release()
        {
            length = IntPtr.Zero;
            if (c_str != IntPtr.Zero)
            {
                if (free_cstr == p_my_free) d_my_free(c_str);
                else
                {
                    delegate_free_c_str free = Marshal.GetDelegateForFunctionPointer<delegate_free_c_str>(free_cstr);
                    free(c_str);
                }
                
                c_str = IntPtr.Zero;
            }
        }

        public void Dispose()
        {
            Release();
        }
    }
}