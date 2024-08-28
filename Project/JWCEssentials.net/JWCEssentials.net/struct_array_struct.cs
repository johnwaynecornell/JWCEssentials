using System;
using System.Buffers;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Unicode;

namespace JWCEssentials.net
{
    class _98732
    {
        public delegate void delegate_free_c_str(IntPtr c_str);
        public static delegate_free_c_str d_my_free = (mem)=>Marshal.FreeCoTaskMem(mem);
    }
    
    
    [StructLayout(LayoutKind.Sequential)]
    
    public struct struct_array_struct<T> : IDisposable
    {
        public IntPtr memory;
        public IntPtr length;
        public IntPtr free_cstr;

        
        private static void my_free(IntPtr c_str)
        {
            Marshal.FreeCoTaskMem(c_str);
        }
        
        private static IntPtr p_my_free = Marshal.GetFunctionPointerForDelegate(_98732.d_my_free);
        
        public struct_array_struct()
        {
            memory = 0;
            length = 0;
            free_cstr = 0;
        }

        public static implicit operator T[](struct_array_struct<T> handle)
        {
            T[] R = new T[handle.length];
            byte[] buffer = new byte[handle.length * Marshal.SizeOf(typeof(T))];
            Marshal.Copy(handle.memory, buffer, 0, buffer.Length);
            
            GCHandle h = GCHandle.Alloc(R, GCHandleType.Pinned);
            Marshal.Copy(buffer, 0, h.AddrOfPinnedObject(), buffer.Length);
            
            h.Free();
            
            return R;

        }

        public static implicit operator struct_array_struct<T>(T[] arr)
        {
            //ArrayBufferWriter<byte> bw = new ArrayBufferWriter<byte>();
            
            int bytesWritten = Marshal.SizeOf(typeof(T)) * arr.Length;
            
            struct_array_struct<T> handle;
            
            handle.memory = Marshal.AllocCoTaskMem(bytesWritten);
            
            MethodInfo Copy = typeof(Marshal).GetMethod("Copy", new Type[] { typeof(T[]),  typeof(int), typeof(IntPtr), typeof(int)});
            
            Copy.Invoke(null, new object?[]{arr, 0, (IntPtr) handle.memory, bytesWritten});
            
            handle.length = arr.Length;
            handle.free_cstr = p_my_free; 

            return handle;
        }

        public void Release()
        {
            Console.WriteLine("RELEASE");
            length = 0;
            if (memory != 0)
            {
                if (free_cstr ==  p_my_free) _98732.d_my_free((IntPtr)memory);
                else
                {
                    _98732.delegate_free_c_str free = Marshal.GetDelegateForFunctionPointer<_98732.delegate_free_c_str>((IntPtr) free_cstr);
                    free((IntPtr) memory);
                }
                
                memory = 0;
            }
        }

        public void Dispose()
        {
            Release();
        }
    }
}