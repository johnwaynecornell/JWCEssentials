using System;
using System.Buffers;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Unicode;

namespace JWCEssentials.net
{
    [StructLayout(LayoutKind.Sequential)]
    public struct utf8_string_struct
    {
        public ulong c_str;
        public ulong length;
        public ulong free_cstr;

        delegate void delegate_free_c_str(IntPtr c_str);
        
        private static void my_free(IntPtr c_str)
        {
            Marshal.FreeCoTaskMem(c_str);
        }
        
        private static delegate_free_c_str d_my_free = my_free;
        private static IntPtr p_my_free = Marshal.GetFunctionPointerForDelegate(d_my_free);
        
        public utf8_string_struct()
        {
            c_str = 0;
            length = 0;
            free_cstr = 0;
        }

        public static implicit operator string(utf8_string_struct handle)
        {
            return Marshal.PtrToStringUTF8((IntPtr) handle.c_str);
        }

        public static implicit operator utf8_string_struct(string str)
        {
            //ArrayBufferWriter<byte> bw = new ArrayBufferWriter<byte>();

            int charsRead;
            int bytesWritten;

            //Utf8.FromUtf16(str, bw.GetSpan(), out charsRead, out bytesWritten);

            byte[] bytes = Encoding.Default.GetBytes(str);
            bytesWritten = bytes.Length;

            byte[] zero = new byte[] { 0 };
            
            Console.WriteLine("sz = " + Marshal.SizeOf<utf8_string_struct>());
            
            
            utf8_string_struct handle;
            /*
            handle.c_str = Marshal.AllocCoTaskMem(bytesWritten+1);
            Marshal.Copy(bytes, 0, handle.c_str, bytesWritten);
            Marshal.Copy(zero, 0, (IntPtr) ((ulong) handle.c_str + (ulong) bytesWritten), 1);
            handle.length = (IntPtr)bytesWritten;
            handle.free_cstr = p_my_free; 
            */

            handle.c_str = 1;
            handle.length = 0;
            handle.free_cstr = 0;
            
            return handle;
        }

        public void Release()
        {
            Console.WriteLine("RELEASE");
            length = 0;
            if (c_str != 0)
            {
                if (free_cstr == (ulong) p_my_free) d_my_free((IntPtr)c_str);
                else
                {
                    delegate_free_c_str free = Marshal.GetDelegateForFunctionPointer<delegate_free_c_str>((IntPtr) free_cstr);
                    free((IntPtr) c_str);
                }
                
                c_str = 0;
            }
        }

        public void Dispose()
        {
            Release();
        }
    }
}