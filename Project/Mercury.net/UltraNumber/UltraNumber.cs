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
			public static extern void mercuryLoadRawRound(IntPtr stack, int Precision, uint[] val, bool Negative, int exp,
				uint[] digits, int digitsLen);

			[DllImport("Mercury")]
			public static extern void mercuryRoundPlaces(IntPtr stack, int Precision, uint[] a, int places, uint[] val);


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
			public static extern bool mercuryHasFraction(IntPtr stack, int Precision, uint[] a);

			[DllImport("Mercury")]
			public static extern bool mercuryIsInteger(IntPtr stack, int Precision, uint[] a);

			[DllImport("Mercury")]
			public static extern uint mercuryGetAt(int Precision, uint[] a, int Place);
			
			[DllImport("Mercury")]
			public static extern bool mercurySetAt(int Precision, uint [] a, int Place, uint value);
			
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
			public static extern void mercuryAbs(IntPtr stack, int Precision, uint[] a, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryNeg(IntPtr stack, int Precision, uint[] a, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryTrunc(IntPtr stack, int Precision, uint[] a, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryRound(IntPtr stack, int Precision, uint[] a, uint[] val);
			
			[DllImport("Mercury")]
			public static extern void mercuryFloor(IntPtr stack, int Precision, uint[] a, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryCeil(IntPtr stack, int Precision, uint[] a, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryFrac(IntPtr stack, int Precision, uint[] a, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryCopySign(IntPtr stack, int Precision, uint[] mag, uint[] sign, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryMin(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

			[DllImport("Mercury")]
			public static extern void mercuryMax(IntPtr stack, int Precision, uint[] a, uint[] b, uint[] val);

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
			public static extern bool mercuryHasFraction(IntPtr stack, int Precision, IntPtr a);

			[DllImport("Mercury")]
			public static extern bool mercuryIsInteger(IntPtr stack, int Precision, IntPtr a);

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
			public static extern void mercuryAbs(IntPtr stack, int Precision, IntPtr a, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryNeg(IntPtr stack, int Precision, IntPtr a, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryTrunc(IntPtr stack, int Precision, IntPtr a, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryFloor(IntPtr stack, int Precision, IntPtr a, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryCeil(IntPtr stack, int Precision, IntPtr a, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryFrac(IntPtr stack, int Precision, IntPtr a, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryCopySign(IntPtr stack, int Precision, IntPtr mag, IntPtr sign, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryMin(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

			[DllImport("Mercury")]
			public static extern void mercuryMax(IntPtr stack, int Precision, IntPtr a, IntPtr b, IntPtr val);

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

		public static UltraNumber Zero()
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryLoadZero(Imports.Stack, Imports.GetPrecision(), r.Elements);
			return r;
		}

		public static UltraNumber FromRaw(bool Negative, int exp, uint[] digits)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryLoadRaw(Imports.Stack, Imports.GetPrecision(), r.Elements, Negative, exp, digits, digits.Length);
			return r;
		}
		
		public static UltraNumber FromRawRound(bool Negative, int exp, uint[] digits)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryLoadRawRound(Imports.Stack, Imports.GetPrecision(), r.Elements, Negative, exp, digits, digits.Length);
			return r;
		}

		public void Load(UltraNumber a)
		{
			Imports.mercuryLoadMercury(Imports.Stack, Imports.GetPrecision(), a.Elements, this.Elements);
		}
		
		public UltraNumber RoundPlaces(int places)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryRoundPlaces(Imports.Stack, Imports.GetPrecision(), this.Elements, places, r.Elements);
			return r;
		}

		public UltraNumber Abs() => Abs(this);
		public UltraNumber Neg() => Neg(this);
		public UltraNumber Round() => Round(this);
		public UltraNumber Trunc() => Trunc(this);
		public UltraNumber Floor() => Floor(this);
		public UltraNumber Ceil() => Ceil(this);
		public UltraNumber Frac() => Frac(this);
		public UltraNumber Sqr() => Sqr(this);
		public UltraNumber Sqrt() => Sqrt(this);
		
		public UltraNumber Add(UltraNumber b) => this + b;
		public UltraNumber Sub(UltraNumber b) => this - b;
		public UltraNumber Mul(UltraNumber b) => this * b;
		public UltraNumber Div(UltraNumber b) => this / b;
		
		public UltraNumber Pow(UltraNumber b) => Pow(this, b);
		public UltraNumber Root(UltraNumber b) => Root(this, b);
		public UltraNumber Log(UltraNumber b) => Log(this, b);
		public UltraNumber LogSlow(UltraNumber b) => LogSlow(this, b);
		public UltraNumber Min(UltraNumber b) => Min(this, b);
		public UltraNumber Max(UltraNumber b) => Max(this, b);
		public UltraNumber CopySign(UltraNumber sign) => CopySign(this, sign);
		public UltraNumber Interpolate(UltraNumber b, UltraNumber x) => Interpolate(this, b, x);
		public UltraNumber Shift(int places) => Shift(this, places);
		
		public bool IsZero() => IsZero(this);
		public bool HasFraction() => HasFraction(this);
		public bool IsInteger() => IsInteger(this);
		
		public int GetBit(int Place) => GetBit(this, Place);
		public uint GetAt(int Place) => GetAt(this, Place);
		public bool SetAt(int Place, uint value) => SetAt(this, Place, value);
		
		public int Compare(UltraNumber b) => Compare(this, b);
		public int AbsCompare(UltraNumber b) => AbsCompare(this, b);
		
		public UltraNumber AbsAdd(UltraNumber b) => AbsAdd(this, b);
		public UltraNumber AbsSub(UltraNumber b) => AbsSub(this, b);
		public UltraNumber AbsMul(UltraNumber b) => AbsMul(this, b);
		public UltraNumber AbsDiv(UltraNumber b) => AbsDiv(this, b);

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

		public static UltraNumber operator +(UltraNumber a)
		{
			return new UltraNumber(a);
		}
		
		public static UltraNumber operator -(UltraNumber a)
		{
			return UltraNumber.Neg(a);
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

		public static UltraNumber Shift(UltraNumber a, int places)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryShift(Imports.Stack, Imports.GetPrecision(), a.Elements, places, r.Elements);
			return r;
		}

		public static UltraNumber operator <<(UltraNumber a, int places)
		{
			return Shift(a, places);
		}

		public static UltraNumber operator >>(UltraNumber a, int places)
		{
			return Shift(a, -places);
		}

		public static UltraNumber Abs(UltraNumber a)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryAbs(Imports.Stack, Imports.GetPrecision(), a.Elements, r.Elements);
			return r;
		}

		public static UltraNumber Neg(UltraNumber a)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryNeg(Imports.Stack, Imports.GetPrecision(), a.Elements, r.Elements);
			return r;
		}

		public static UltraNumber Round(UltraNumber a)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryRound(Imports.Stack, Imports.GetPrecision(), a.Elements, r.Elements);
			return r;
		}

		public static UltraNumber Trunc(UltraNumber a)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryTrunc(Imports.Stack, Imports.GetPrecision(), a.Elements, r.Elements);
			return r;
		}

		public static UltraNumber Floor(UltraNumber a)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryFloor(Imports.Stack, Imports.GetPrecision(), a.Elements, r.Elements);
			return r;
		}

		public static UltraNumber Ceil(UltraNumber a)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryCeil(Imports.Stack, Imports.GetPrecision(), a.Elements, r.Elements);
			return r;
		}

		public static UltraNumber Frac(UltraNumber a)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryFrac(Imports.Stack, Imports.GetPrecision(), a.Elements, r.Elements);
			return r;
		}

		public static UltraNumber CopySign(UltraNumber mag, UltraNumber sign)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryCopySign(Imports.Stack, Imports.GetPrecision(), mag.Elements, sign.Elements, r.Elements);
			return r;
		}

		public static UltraNumber Min(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryMin(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);
			return r;
		}

		public static UltraNumber Max(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryMax(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);
			return r;
		}

		public static UltraNumber TwoPow(int Place)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercury_2Pow(Imports.Stack, Imports.GetPrecision(), Place, r.Elements);
			return r;
			
		}

		public static UltraNumber Sqr(UltraNumber a)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercurySqr(Imports.Stack, Imports.GetPrecision(), a.Elements, r.Elements);
			return r;
		}

		public static bool IsZero(UltraNumber a)
		{
			return Imports.mercuryIsZero(Imports.GetPrecision(), a.Elements);
		}

		public static bool HasFraction(UltraNumber a)
		{
			return Imports.mercuryHasFraction(Imports.Stack, Imports.GetPrecision(), a.Elements);
		}

		public static bool IsInteger(UltraNumber a)
		{
			return Imports.mercuryIsInteger(Imports.Stack, Imports.GetPrecision(), a.Elements);
		}

		public static int GetBit(UltraNumber a, int Place)
		{
			return Imports.mercuryGetBit(Imports.Stack, Imports.GetPrecision(), Place, a.Elements);
		}

		public static uint GetAt(UltraNumber a, int Place)
		{
			return Imports.mercuryGetAt(Imports.GetPrecision(), a.Elements, Place);
		}

		public static bool SetAt(UltraNumber a, int Place, uint value)
		{
			return Imports.mercurySetAt(Imports.GetPrecision(), a.Elements, Place, value);
		}
		
		public static int AbsCompare(UltraNumber a, UltraNumber b)
		{
			return Imports.mercuryAbsCmp(Imports.GetPrecision(), a.Elements, b.Elements);
		}

		public static UltraNumber AbsAdd(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryAbsAdd(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);
			return r;
		}

		public static UltraNumber AbsSub(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryAbsSub(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);
			return r;
		}

		public static UltraNumber AbsMul(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryAbsMul(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);
			return r;
		}

		public static UltraNumber AbsDiv(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryAbsDiv(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);
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

		public static UltraNumber LogSlow(UltraNumber a, UltraNumber b)
		{
			UltraNumber r = new UltraNumber();
			Imports.mercuryLogSlow(Imports.Stack, Imports.GetPrecision(), a.Elements, b.Elements, r.Elements);
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