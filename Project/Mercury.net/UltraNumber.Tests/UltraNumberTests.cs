using Mercury.net;
using Xunit;

namespace Mercury.net.Tests;

public class UltraNumberTests
{
    public UltraNumberTests()
    {
        // Ensure stack is initialized for the current thread
        if (UltraNumber.Imports.Stack == IntPtr.Zero)
        {
            UltraNumber.SetPrecisionAndAllocateStack(32, 1024 * 1024);
        }
    }

    [Fact]
    public void TestAddition()
    {
        UltraNumber a = 5;
        UltraNumber b = 10;
        UltraNumber c = a + b;
        Assert.Equal(15.0, (double)c);
    }

    [Fact]
    public void TestSubtraction()
    {
        UltraNumber a = 20;
        UltraNumber b = 7;
        UltraNumber c = a - b;
        Assert.Equal(13.0, (double)c);
    }

    [Fact]
    public void TestMultiplication()
    {
        UltraNumber a = 6;
        UltraNumber b = 7;
        UltraNumber c = a * b;
        Assert.Equal(42.0, (double)c);
    }

    [Fact]
    public void TestDivision()
    {
        UltraNumber a = 100;
        UltraNumber b = 4;
        UltraNumber c = a / b;
        Assert.Equal(25.0, (double)c);
    }

    [Fact]
    public void TestSqr()
    {
        UltraNumber a = 9;
        UltraNumber b = UltraNumber.Sqr(a);
        Assert.Equal(81.0, (double)b);
    }

    [Fact]
    public void TestSqrt()
    {
        UltraNumber a = 64;
        UltraNumber b = UltraNumber.Sqrt(a);
        Assert.Equal(8.0, (double)b);
    }

    [Fact]
    public void TestIsZero()
    {
        UltraNumber zero = UltraNumber.Zero();
        UltraNumber ten = 10;
        Assert.True(UltraNumber.IsZero(zero));
        Assert.False(UltraNumber.IsZero(ten));
    }

    [Fact]
    public void TestHasFraction()
    {
        UltraNumber a = 1.5;
        UltraNumber b = 2;
        Assert.True(UltraNumber.HasFraction(a));
        Assert.False(UltraNumber.HasFraction(b));
    }

    [Fact]
    public void TestIsInteger()
    {
        UltraNumber a = 1.5;
        UltraNumber b = 3;
        Assert.False(UltraNumber.IsInteger(a));
        Assert.True(UltraNumber.IsInteger(b));
    }

    [Fact]
    public void TestTrunc()
    {
        UltraNumber a = 1.7;
        UltraNumber b = -1.7;
        Assert.Equal(1.0, (double)UltraNumber.Trunc(a));
        Assert.Equal(-1.0, (double)UltraNumber.Trunc(b));
    }

    [Fact]
    public void TestFloor()
    {
        UltraNumber a = 1.7;
        UltraNumber b = -1.7;
        Assert.Equal(1.0, (double)UltraNumber.Floor(a));
        Assert.Equal(-2.0, (double)UltraNumber.Floor(b));
    }

    [Fact]
    public void TestCeil()
    {
        UltraNumber a = 1.7;
        UltraNumber b = -1.7;
        Assert.Equal(2.0, (double)UltraNumber.Ceil(a));
        Assert.Equal(-1.0, (double)UltraNumber.Ceil(b));
    }

    [Fact]
    public void TestFrac()
    {
        UltraNumber a = 1.75;
        Assert.Equal(0.75, (double)UltraNumber.Frac(a));
    }

    [Fact]
    public void TestAbs()
    {
        UltraNumber a = -5.0;
        Assert.Equal(5.0, (double)UltraNumber.Abs(a));
    }

    [Fact]
    public void TestNeg()
    {
        UltraNumber a = 5.0;
        Assert.Equal(-5.0, (double)UltraNumber.Neg(a));
    }

    [Fact]
    public void TestMinMax()
    {
        UltraNumber a = 3.0;
        UltraNumber b = 7.0;
        Assert.Equal(3.0, (double)UltraNumber.Min(a, b));
        Assert.Equal(7.0, (double)UltraNumber.Max(a, b));
    }

    [Fact]
    public void TestTwoPow()
    {
        Assert.Equal(1024.0, (double)UltraNumber.TwoPow(10));
    }

    [Fact]
    public void TestCompare()
    {
        UltraNumber a = 10.0;
        UltraNumber b = 20.0;
        Assert.True(a < b);
        Assert.True(b > a);
        Assert.True(a == 10.0);
        Assert.False(a == b);
    }

    [Fact]
    public void TestPow()
    {
        UltraNumber a = 2.0;
        UltraNumber b = 3.0;
        UltraNumber c = UltraNumber.Pow(a, b);
        Assert.Equal(8.0, (double)c);
    }

    [Fact]
    public void TestRoot()
    {
        UltraNumber a = 27.0;
        UltraNumber b = 3.0;
        UltraNumber c = UltraNumber.Root(a, b);
        Assert.Equal(3.0, (double)c, 5); // 5 digits precision
    }

    [Fact]
    public void TestAbsArithmetic()
    {
        UltraNumber a = -10.0;
        UltraNumber b = -5.0;
        Assert.Equal(15.0, (double)UltraNumber.AbsAdd(a, b));
        Assert.Equal(5.0, (double)UltraNumber.AbsSub(a, b));
        Assert.Equal(50.0, (double)UltraNumber.AbsMul(a, b));
        Assert.Equal(2.0, (double)UltraNumber.AbsDiv(a, b));
    }

    [Fact]
    public void TestAbsCompare()
    {
        UltraNumber a = -10.0;
        UltraNumber b = 5.0;
        Assert.Equal(1, UltraNumber.AbsCompare(a, b)); // |-10| > |5|
    }
    [Fact]
    public void TestLog()
    {
        UltraNumber a = 100.0;
        UltraNumber b = 10.0;
        UltraNumber c = UltraNumber.Log(a, b);
        Assert.Equal(2.0, (double)c, 5);
    }

    [Fact]
    public void TestLogSlow()
    {
        UltraNumber a = 100.0;
        UltraNumber b = 10.0;
        UltraNumber c = UltraNumber.LogSlow(a, b);
        Assert.Equal(2.0, (double)c, 5);
    }
}
