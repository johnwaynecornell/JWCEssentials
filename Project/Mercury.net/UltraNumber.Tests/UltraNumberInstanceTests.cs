using Mercury.net;
using Xunit;

namespace Mercury.net.Tests;

public class UltraNumberInstanceTests
{
    public UltraNumberInstanceTests()
    {
        // Ensure stack is initialized for the current thread
        if (UltraNumber.Imports.Stack == IntPtr.Zero)
        {
            UltraNumber.SetPrecisionAndAllocateStack(32, 1024 * 1024);
        }
    }

    [Fact]
    public void TestInstanceUnaryOps()
    {
        UltraNumber a = -5.0;
        Assert.Equal(5.0, (double)a.Abs());
        Assert.Equal(5.0, (double)a.Neg());
        
        UltraNumber b = 1.7;
        Assert.Equal(1.0, (double)b.Trunc());
        Assert.Equal(1.0, (double)b.Floor());
        Assert.Equal(2.0, (double)b.Ceil());
        Assert.Equal(0.7, (double)b.Frac(), 5);
    }

    [Fact]
    public void TestInstanceBinaryOps()
    {
        UltraNumber a = 2.0;
        UltraNumber b = 3.0;
        
        Assert.Equal(5.0, (double)a.Add(b));
        Assert.Equal(-1.0, (double)a.Sub(b));
        Assert.Equal(6.0, (double)a.Mul(b));
        Assert.Equal(8.0, (double)a.Pow(b));
        
        Assert.Equal(2.0, (double)a.Min(b));
        Assert.Equal(3.0, (double)a.Max(b));
    }

    [Fact]
    public void TestInstanceChecks()
    {
        UltraNumber zero = UltraNumber.Zero();
        UltraNumber onePointFive = 1.5;
        
        Assert.True(zero.IsZero());
        Assert.True(onePointFive.HasFraction());
        Assert.False(onePointFive.IsInteger());
    }

    [Fact]
    public void TestInstanceLog()
    {
        UltraNumber a = 100.0;
        UltraNumber b = 10.0;
        Assert.Equal(2.0, (double)a.LogSlow(b), 5);
    }

    [Fact]
    public void TestInstanceShift()
    {
        UltraNumber a = 1.0;
        Assert.Equal(1024.0, (double)a.Shift(10));
    }

    [Fact]
    public void TestInstanceCompare()
    {
        UltraNumber a = 10.0;
        UltraNumber b = 20.0;
        Assert.Equal(-1, a.Compare(b));
        Assert.Equal(1, b.Compare(a));
        Assert.Equal(0, a.Compare(10.0));
    }
}
