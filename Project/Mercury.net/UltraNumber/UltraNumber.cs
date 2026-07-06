using System;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Security.Permissions;
using System.Text;

namespace Mercury.net
{
	/*
[Serializable]
public class MyObject : ISerializable 
{
  public int n1;
  public int n2;
  public String str;

  public MyObject()
  {
  }

  protected MyObject(SerializationInfo info, StreamingContext context)
  {
    n1 = info.GetInt32("i");
    n2 = info.GetInt32("j");
    str = info.GetString("k");
  }
[SecurityPermissionAttribute(SecurityAction.Demand, 
SerializationFormatter =true)]
public virtual void GetObjectData(SerializationInfo info, StreamingContext context)
  {
    info.AddValue("i", n1);
    info.AddValue("j", n2);
    info.AddValue("k", str);
  }
}

	 */

	public class UltraException : Exception
	{
		public UltraException(String message) : base(message)
		{
		}
	}

	[Serializable]
	public class UltraNumber : ISerializable
	{
		[ThreadStatic]
		public static bool InvasiveDebug = false;

		public enum TextMode
		{
			Decimal = 0,
			Hexadecimal = 1,
			Binary = 2
		};
		public class Imports
		{
			[ThreadStatic] public static int Precision = 8;
			public static int GetPrecision()
			{
				return Precision;
			}
			
			public static void SetPrecision(int Precision)
			{
				Imports.Precision = Precision;
			}
			
			[ThreadStatic] public static IntPtr Stack = IntPtr.Zero;

			[DllImport("Mercury")]
			public static extern int mercuryHexStringGetPrecision(IntPtr stack, String Input);

			[DllImport("Mercury")]
			public static extern IntPtr mercuryThreadedStackGet(int thread, IntPtr master);
			
			[DllImport("Mercury")]
			public static extern IntPtr mercuryStackAlloc(IntPtr stack, int size);
			[DllImport("Mercury")]
			public static extern void mercuryStackFree(IntPtr stack, int size);
			
			[DllImport("Mercury")]
			public static extern IntPtr mercuryAllocateStack(long Size);
			[DllImport("Mercury")]
			public static extern void mercuryDeallocateStack(IntPtr stack);

			
			
			
			
			[DllImport("Mercury")]
			public static extern long mercuryStackOffset(IntPtr stack);
			
			[DllImport("Mercury")]
			public static extern void mercuryLoadZero(IntPtr stack, int Precision, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryLoadRaw(IntPtr stack, int Precision, uint[] val, bool Negative, int exp,
				uint[] digits, int digitsLen);

			[DllImport("Mercury")]
			public static extern void mercuryLoadMercury(IntPtr stack, int Precision, uint[] a, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryLoadExtendMercury(IntPtr stack, int OldPrecision, int Precision, uint[] a,
				uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryLoadInt(IntPtr stack, int Precision, uint[] val, int a);

			[DllImport("Mercury")]
			public static extern void mercuryLoadUint(IntPtr stack, int Precision, uint[] val, uint a);

			[DllImport("Mercury")]
			public static extern void mercuryLoadLong(IntPtr stack, int Precision, uint[] val, long a);

			[DllImport("Mercury")]
			public static extern void mercuryLoadUlong(IntPtr stack, int Precision, uint[] val, ulong a);

			[DllImport("Mercury")]
			public static extern void mercuryLoadDouble(IntPtr stack, int Precision, uint[] val, double a);

			[DllImport("Mercury")]
			public static extern double mercuryToDouble(IntPtr stack, int Precision, uint[] a);
			
			[DllImport("Mercury")]
			public static extern void mercuryFromString(IntPtr stack, int Precision, String Input, uint[] val);

			[DllImport("Mercury")]
			public static extern int mercuryToString(IntPtr stack, int Precision, uint[] val, byte[] buffer, int len);

			[DllImport("Mercury")]
			public static extern bool mercuryIsZero(int Precision, uint[] val);

			[DllImport("Mercury")]
			public static extern uint mercuryGetAt(int Precision, uint[] a, int Place);

			[DllImport("Mercury")]
			public static extern int mercuryAbsCmp(int Precision, uint[] a, uint[] b);

			[DllImport("Mercury")]
			public static extern int mercuryCmp(int Precision, uint[] a, uint[] b);

			[DllImport("Mercury")]
			public static extern void mercuryShift(IntPtr stack, int Precision, uint[] a, int shift, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryAbsAdd(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryAbsSub(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryAbsMul(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryAbsDiv(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryAdd(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercurySub(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryMul(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryDiv(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercury_2Pow(IntPtr stack, int Precision, int Place, uint[] val);

			[DllImport("Mercury")]
			public static extern int mercuryGetBit(IntPtr stack, int Precision, int Place, uint[] a);

			[DllImport("Mercury")]
			public static extern void mercurySqr(IntPtr stack, int Precision, uint[] a, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercurySqrt(IntPtr stack, int Precision, uint[] a, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryPow(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryRoot(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryLogSlow(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryLog(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);
			
			[DllImport("Mercury")]
			public static extern void mercuryLoadZero(IntPtr stack, int Precision, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryLoadRaw(IntPtr stack, int Precision, IntPtr val, bool Negative, int exp,
				IntPtr digits, int digitsLen);

			[DllImport("Mercury")]
			public static extern void mercuryLoadMercury(IntPtr stack, int Precision, IntPtr a, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryLoadExtendMercury(IntPtr stack, int OldPrecision, int Precision, IntPtr a,
				IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryLoadInt(IntPtr stack, int Precision, IntPtr val, int a);

			[DllImport("Mercury")]
			public static extern void mercuryLoadUint(IntPtr stack, int Precision, IntPtr val, uint a);

			[DllImport("Mercury")]
			public static extern void mercuryLoadLong(IntPtr stack, int Precision, IntPtr val, long a);

			[DllImport("Mercury")]
			public static extern void mercuryLoadUlong(IntPtr stack, int Precision, IntPtr val, ulong a);

			[DllImport("Mercury")]
			public static extern void mercuryLoadDouble(IntPtr stack, int Precision, IntPtr val, double a);

			[DllImport("Mercury")]
			public static extern double mercuryToDouble(IntPtr stack, int Precision, IntPtr a);

			
			[DllImport("Mercury")]
			public static extern void mercuryFromString(IntPtr stack, int Precision, String Input, IntPtr val);

			[DllImport("Mercury")]
			public static extern int mercuryToString(IntPtr stack, int Precision, IntPtr val, byte[] buffer, int len);

			[DllImport("Mercury")]
			public static extern bool mercuryIsZero(int Precision, IntPtr val);

			[DllImport("Mercury")]
			public static extern uint mercuryGetAt(int Precision, IntPtr a, int Place);

			[DllImport("Mercury")]
			public static extern int mercuryAbsCmp(int Precision, IntPtr a, IntPtr b);

			[DllImport("Mercury")]
			public static extern int mercuryCmp(int Precision, IntPtr a, IntPtr b);

			[DllImport("Mercury")]
			public static extern void mercuryShift(IntPtr stack, int Precision, IntPtr a, int shift, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryAbsAdd(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryAbsSub(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryAbsMul(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryAbsDiv(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryAdd(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercurySub(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryMul(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryDiv(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercury_2Pow(IntPtr stack, int Precision, int Place, IntPtr val);

			[DllImport("Mercury")]
			public static extern int mercuryGetBit(IntPtr stack, int Precision, int Place, IntPtr a);

			[DllImport("Mercury")]
			public static extern void mercurySqr(IntPtr stack, int Precision, IntPtr a, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercurySqrt(IntPtr stack, int Precision, IntPtr a, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryPow(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryRoot(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);
			
			[DllImport("Mercury")]
			public static extern void mercuryLog(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);
			
		}

		public uint[] _Elements;

		public uint[] Elements
		{
			get
			{
				if (_Elements == null) return null;
				if (Precision != Imports.Precision)
				{
					uint[] New = new uint[Imports.Precision + 2];
					New[0] = _Elements[0];
					New[1] = _Elements[1];

					for (int d = 0; d < Precision; d++)
					{
						if (d<Imports.Precision) New[2 + Imports.Precision - 1 - d] = _Elements[2+Precision-1-d];
					}

					_Elements = New;
					Precision = Imports.Precision;
				}

				return _Elements;
			}

			set
			{
				_Elements = value;
			}
		}
		
		
		
		public IntPtr Handle;
		public int Precision;

		public UltraNumber ()
		{
			Precision = Imports.GetPrecision ();
			Elements = new uint[Precision + 2];
		}

		public UltraNumber(double value)
		{
			Precision = Imports.GetPrecision ();
			Elements = new uint[Precision + 2];
			Imports.mercuryLoadDouble(Imports.Stack, Precision, Elements, value);
		}

		public UltraNumber(int value)
		{
			Precision = Imports.GetPrecision ();
			Elements = new uint[Precision + 2];
			Imports.mercuryLoadInt(Imports.Stack, Precision, Elements, value);
		}

		public UltraNumber(uint value)
		{
			Precision = Imports.GetPrecision ();
			Elements = new uint[Precision + 2];
			Imports.mercuryLoadUint(Imports.Stack, Precision, Elements, value);
		}

		public UltraNumber(long value)
		{
			Precision = Imports.GetPrecision ();
			Elements = new uint[Precision + 2];
			Imports.mercuryLoadLong(Imports.Stack, Precision, Elements, value);
		}

		public UltraNumber(ulong value)
		{
			Precision = Imports.GetPrecision ();
			Elements = new uint[Precision + 2];
			Imports.mercuryLoadUlong(Imports.Stack, Precision, Elements, value);
		}

		public UltraNumber(IntPtr value)
		{
			Precision = Imports.GetPrecision ();
			Handle = value;
		}

		public UltraNumber(UltraNumber a)
		{
			Precision = Imports.GetPrecision();
			Elements = (uint[]) a.Elements.Clone();
		}

		~UltraNumber()
		{
		}

		public static explicit operator double(UltraNumber a)
		{
			if (a.Elements == null) return Imports.mercuryToDouble(Imports.Stack, Imports.GetPrecision(), a.Handle);
			else return Imports.mercuryToDouble(Imports.Stack, Imports.GetPrecision(), a.Elements);
		}

		public static implicit operator UltraNumber(double d)
		{
			return new UltraNumber (d);
		}

		public static explicit operator int(UltraNumber a)
		{
			return (int)(double)a;
		}

		public static implicit operator UltraNumber(int i)
		{
			return new UltraNumber (i);
		}

		public static explicit operator uint(UltraNumber a)
		{
			return (uint)(double)a;
		}

		public static implicit operator UltraNumber(uint i)
		{
			return new UltraNumber (i);
		}

		public static explicit operator long(UltraNumber a)
		{
			return (long)(double)a;
		}

		public static implicit operator UltraNumber(long i)
		{
			return new UltraNumber (i);
		}

		public static explicit operator ulong(UltraNumber a)
		{
			return (ulong)(double)a;
		}

		public static implicit operator UltraNumber(ulong i)
		{
			return new UltraNumber (i);
		}

		public static UltraNumber operator +(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			
			if (a.Elements == null)
			{
				GCHandle H = GCHandle.Alloc(r.Elements, GCHandleType.Pinned);
				Imports.mercuryAdd(Imports.Stack, Imports.GetPrecision(), a.Handle, b.Handle, H.AddrOfPinnedObject());
				H.Free();
			}
			else
			{
				Imports.mercuryAdd(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);

			}

			return r;
		}

		public static UltraNumber operator -(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercurySub(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);
			return r;
		}

		public static UltraNumber operator *(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryMul(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);
			return r;
		}

		public static UltraNumber operator /(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryDiv(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);
			return r;
		}

		public static int Compare(UltraNumber a, UltraNumber b)
		{
			return Imports.mercuryCmp(Imports.GetPrecision(), a.Elements, b.Elements);
		}

		public static UltraNumber operator <<(UltraNumber a, int places)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryShift(Imports.Stack, Imports.GetPrecision(), a.Elements, places, r.Elements);
			return r;
		}

		public static UltraNumber operator >>(UltraNumber a, int places)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryShift(Imports.Stack, Imports.GetPrecision(), a.Elements, -places, r.Elements);
			return r;
		}

		public static UltraNumber Abs(UltraNumber a)
		{
			UltraNumber r = new UltraNumber(a);
			r.Elements[0] &= 0xFFFFFFFE;
			return r;
		}

		public static UltraNumber Pow2(int Place)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercury_2Pow(Imports.Stack, Imports.GetPrecision(), Place, r.Elements);
			return r;
			
		}

		public static UltraNumber Sqrt(UltraNumber a)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercurySqrt(Imports.Stack, Imports.GetPrecision(), a.Elements, r.Elements);
			return r;
		}

		public static UltraNumber Pow(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryPow(Imports.Stack, Imports.GetPrecision(), a.Elements,b.Elements,r.Elements);
			return r;
		}
		
		public static UltraNumber Root(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryRoot(Imports.Stack, Imports.GetPrecision(), a.Elements,b.Elements,r.Elements);
			return r;
		}

		public static UltraNumber Log(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryLog(Imports.Stack, Imports.GetPrecision(), a.Elements,b.Elements,r.Elements);
			return r;
		}


		public static bool operator ==(UltraNumber a, UltraNumber b)
		{
			int s = Compare (a, b);
			return s == 0;
		}

		public static bool operator !=(UltraNumber a, UltraNumber b)
		{
			int s = Compare (a, b);
			return s != 0;
		}

		public static bool operator <=(UltraNumber a, UltraNumber b)
		{
			int s = Compare (a, b);
			return s <= 0;
		}

		public static bool operator >=(UltraNumber a, UltraNumber b)
		{
			int s = Compare (a, b);
			return s >= 0;
		}


		public static bool operator <(UltraNumber a, UltraNumber b)
		{
			int s = Compare (a, b);
			return s < 0;
		}

		public static bool operator >(UltraNumber a, UltraNumber b)
		{
			int s = Compare (a, b);
			return s > 0;
		}

		public override string ToString ()
		{
			int Len = 16384;//Imports.mercuryToString(Imports.Stack, Imports.GetPrecision(), Elements, null, Int32.MaxValue);
			byte[] buffer = new byte[Len];
			Imports.mercuryToString(Imports.Stack, Imports.GetPrecision(), Elements, buffer, Len);

			int l;
			for (l = 0; l < Len && buffer[l] != 0; l++) ;
			
			return ASCIIEncoding.Default.GetString(buffer,0,l);
		}

		public static UltraNumber Parse(String S)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryFromString(Imports.Stack, Imports.GetPrecision(), S, r.Elements);
			return r;
		}

		public static implicit operator UltraNumber(String S)
		{
			return Parse(S);
		}

		public static String GetError()
		{
			return "UNKNOWN";//System.Runtime.InteropServices.Marshal.PtrToStringAnsi (Imports.GetError ());
		}

		protected UltraNumber(SerializationInfo info, StreamingContext context)
		{
			Precision = info.GetInt32("Precision");		
			String str = info.GetString("Hex");

			int oldPrecision = Imports.GetPrecision ();
			Imports.SetPrecision (Precision);
			Elements = new uint[2 + Imports.GetPrecision()];
			
			Imports.mercuryFromString(Imports.Stack, Precision, str, Elements);
			
			Imports.SetPrecision(oldPrecision);
		}

		[SecurityPermissionAttribute(SecurityAction.Demand, 
			SerializationFormatter =true)]
		public virtual void GetObjectData(SerializationInfo info, StreamingContext context)
		{
			info.AddValue("Precision", Precision);
			info.AddValue("Hex", ToString());
		}

		public override bool Equals (object obj)
		{
			if (obj == null)
				return false;
			if (obj is UltraNumber == false)
				return false;

			UltraNumber b = (UltraNumber)obj;

			return this == b;
		}

		public override int GetHashCode ()
		{
			return ToString().GetHashCode ();
		}

		public static UltraNumber Interpolate(UltraNumber a, UltraNumber b, UltraNumber x)
		{
			return a + (b - a) * x;
		}
		
		public static void SetPrecision(int presision)
		{
			Imports.SetPrecision(presision);
		}
		
		public static IntPtr AllocateThreadStack(long size)
		{
			return Imports.Stack = Imports.mercuryAllocateStack(size);
		}
		
		public static void SetPrecisionAndAllocateStack(int presision, long size)
		{
			Imports.SetPrecision(presision);
			Imports.Stack = Imports.mercuryAllocateStack(size);
		}

		public static IntPtr Stack { get; private set; }
	}
}