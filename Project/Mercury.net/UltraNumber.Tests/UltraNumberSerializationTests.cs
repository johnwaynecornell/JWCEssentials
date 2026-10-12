using Mercury.net;
using Xunit;

namespace Mercury.net.Tests;

public class UltraNumberSerializationTests
{
    public UltraNumberSerializationTests()
    {
        if (UltraNumber.Imports.Stack == IntPtr.Zero)
        {
            UltraNumber.SetPrecisionAndAllocateStack(32, 1024 * 1024);
        }
    }

    [Theory]
    [InlineData("0", "0")]
    [InlineData("0.0", "0")]
    [InlineData("0@+10", "0")]
    [InlineData("1", "1")]
    [InlineData("-1", "-1")]
    [InlineData("A", "A")]
    [InlineData("-F", "-F")]
    [InlineData("123", "123")]
    [InlineData("-456", "-456")]
    [InlineData("DEADBEEF", "DEADBEEF")]
    public void TestBasicFormattingAndParsing(string input, string expectedCanonical)
    {
        UltraNumber val = UltraNumber.Parse(input);
        string formatted = val.ToString();
        Assert.Equal(expectedCanonical, formatted);

        UltraNumber reparsed = UltraNumber.Parse(formatted);
        Assert.Equal(val, reparsed);
    }

    [Theory]
    [InlineData("1@+0")]
    [InlineData("1@+1")]
    [InlineData("1@+2")]
    [InlineData("1@+7")]
    [InlineData("1@+8")]
    [InlineData("1@+F")]
    [InlineData("1@+10")]
    [InlineData("1@+17")]
    [InlineData("1@+18")]
    [InlineData("1@+20")]
    [InlineData("1@+100")]
    [InlineData("1@+1000")]
    [InlineData("1@+10000")]
    [InlineData("1@+100000")]
    [InlineData("1@+1000000")]
    [InlineData("1@+10000000")]
    [InlineData("1@+20000000")]
    [InlineData("1@+100000000")]
    [InlineData("1@+3FFFFFFFF")] // Upper supported boundary (word exp INT32_MAX)
    [InlineData("-1@+3FFFFFFFF")]
    [InlineData("1@-10")]
    [InlineData("1@-20")]
    [InlineData("1@-100")]
    [InlineData("1@-1000")]
    [InlineData("1@-10000")]
    [InlineData("1@-100000")]
    [InlineData("1@-1000000")]
    [InlineData("1@-10000000")]
    [InlineData("1@-100000000")]
    [InlineData("1@-3FFFFFF08")] // Lower supported boundary for P=32 (word exp INT32_MIN + 31)
    [InlineData("-1@-3FFFFFF08")]
    [InlineData("3.243F6A8885A308D@+10")]
    [InlineData("2.B7E151628AED2A6@-20")]
    [InlineData("F.FFFFFFFFFFFFFFF@+100")]
    [InlineData("A.BCDEF0123456789@-50")]
    public void TestScientificExponentRoundTrip(string inputString)
    {
        UltraNumber parsed = UltraNumber.Parse(inputString);
        string formatted = parsed.ToString();

        // Roundtrip invariant: Parse(Format(value)) == value
        UltraNumber reparsed = UltraNumber.Parse(formatted);
        Assert.Equal(parsed, reparsed);

        // Canonical string invariant: Format(Parse(canonicalString)) == canonicalString
        string canonicalFormatted = reparsed.ToString();
        Assert.Equal(formatted, canonicalFormatted);
    }

    [Fact]
    public void TestBoundaryExponentsFromRaw()
    {
        // Test max positive exponent: INT32_MAX
        uint[] limbHigh = { 0x10000000U };
        UltraNumber maxVal = UltraNumber.FromRaw(false, int.MaxValue, limbHigh);
        string maxStr = maxVal.ToString();
        Assert.Contains("@+3FFFFFFFF", maxStr);

        UltraNumber parsedMax = UltraNumber.Parse(maxStr);
        Assert.Equal(maxVal, parsedMax);

        // Test min negative exponent for P=32: INT32_MIN + 31
        uint[] limbLow = { 1U };
        UltraNumber minVal = UltraNumber.FromRaw(false, int.MinValue + 31, limbLow);
        string minStr = minVal.ToString();
        Assert.Contains("@-3FFFFFF08", minStr);

        UltraNumber parsedMin = UltraNumber.Parse(minStr);
        Assert.Equal(minVal, parsedMin);
    }

    [Theory]
    [InlineData(1, "1@-400000000", "1@-400000001")]
    [InlineData(8, "1@-3FFFFFFC8", "1@-3FFFFFFC9")]
    [InlineData(16, "1@-3FFFFFF88", "1@-3FFFFFF89")]
    [InlineData(32, "1@-3FFFFFF08", "1@-3FFFFFF09")]
    [InlineData(64, "1@-3FFFFFE08", "1@-3FFFFFE09")]
    [InlineData(128, "1@-3FFFFFC08", "1@-3FFFFFC09")]
    public void TestPrecisionDependentUnderflowBoundary(int precision, string validBoundaryStr, string firstInvalidStr)
    {
        int oldPrecision = UltraNumber.Imports.GetPrecision();
        try
        {
            UltraNumber.SetPrecision(precision);

            // 1. Verify valid boundary string parses to non-zero and roundtrips
            UltraNumber validNum = UltraNumber.Parse(validBoundaryStr);
            Assert.False(UltraNumber.IsZero(validNum));
            string formatted = validNum.ToString();
            UltraNumber reparsed = UltraNumber.Parse(formatted);
            Assert.Equal(validNum, reparsed);

            // 2. Verify FromRaw at exact lowest supported exponent (INT32_MIN + precision - 1)
            int minExp = int.MinValue + precision - 1;
            uint[] limbLow = { 1U };
            UltraNumber rawMin = UltraNumber.FromRaw(false, minExp, limbLow);
            string rawFormatted = rawMin.ToString();
            UltraNumber parsedRaw = UltraNumber.Parse(rawFormatted);
            Assert.Equal(rawMin, parsedRaw);

            // 3. Verify first invalid combination below boundary underflows to zero predictably
            UltraNumber invalidNum = UltraNumber.Parse(firstInvalidStr);
            Assert.True(UltraNumber.IsZero(invalidNum));
        }
        finally
        {
            UltraNumber.SetPrecision(oldPrecision);
        }
    }

    [Theory]
    [InlineData(8)]
    [InlineData(16)]
    [InlineData(32)]
    [InlineData(64)]
    [InlineData(128)]
    public void TestMultiplePrecisions(int precision)
    {
        int oldPrecision = UltraNumber.Imports.GetPrecision();
        try
        {
            UltraNumber.SetPrecision(precision);

            // Construct valid lower boundary string for this precision
            int minExp = int.MinValue + precision - 1;
            uint[] limbLow = { 1U };
            UltraNumber minVal = UltraNumber.FromRaw(false, minExp, limbLow);
            string minBoundaryStr = minVal.ToString();

            string[] testValues = {
                "1@+0",
                "1@+100",
                "1@-100",
                "1@+3FFFFFFFF",
                minBoundaryStr,
                "123.456",
                "0"
            };

            foreach (var str in testValues)
            {
                UltraNumber num = UltraNumber.Parse(str);
                string formatted = num.ToString();
                UltraNumber reparsed = UltraNumber.Parse(formatted);
                Assert.Equal(num, reparsed);
            }
        }
        finally
        {
            UltraNumber.SetPrecision(oldPrecision);
        }
    }

    [Fact]
    public void TestMantissaWithZeroLimbs()
    {
        // Mantissa with trailing zero limbs
        uint[] digits = { 0x00000000U, 0x00000000U, 0x12345678U };
        UltraNumber num = UltraNumber.FromRaw(false, 10, digits);
        string formatted = num.ToString();
        UltraNumber reparsed = UltraNumber.Parse(formatted);
        Assert.Equal(num, reparsed);

        // Negative mantissa with zero limbs
        UltraNumber negNum = UltraNumber.FromRaw(true, -10, digits);
        string negFormatted = negNum.ToString();
        UltraNumber negReparsed = UltraNumber.Parse(negFormatted);
        Assert.Equal(negNum, negReparsed);
    }

    [Theory]
    [InlineData("1.5@")]
    [InlineData("1.5@+")]
    [InlineData("1.5@-")]
    [InlineData("1.0@+400000000")] // Out of representable word exp range
    [InlineData("1.0@-400000001")] // Out of representable word exp range
    [InlineData("1.0@+FFFFFFFFFFFFFFFF")]
    [InlineData("1.0@-FFFFFFFFFFFFFFFF")]
    [InlineData("1.0@+999999999999999999999999")]
    public void TestMalformedAndOutOfRangeInputs(string invalidInput)
    {
        UltraNumber num = UltraNumber.Parse(invalidInput);
        Assert.True(UltraNumber.IsZero(num));
    }

    [Fact]
    public void TestPrecisionInference()
    {
        int p1 = UltraNumber.Imports.mercuryHexStringGetPrecision(UltraNumber.Imports.Stack, "1.0");
        Assert.True(p1 >= 1);

        int p2 = UltraNumber.Imports.mercuryHexStringGetPrecision(UltraNumber.Imports.Stack, "1.23456789ABCDEF0123456789ABCDEF0");
        Assert.True(p2 >= 4);

        int p3 = UltraNumber.Imports.mercuryHexStringGetPrecision(UltraNumber.Imports.Stack, "1.23456789ABCDEF0@+3FFFFFFFF");
        Assert.True(p3 >= 2);

        int p4 = UltraNumber.Imports.mercuryHexStringGetPrecision(UltraNumber.Imports.Stack, "1.0@-400000000");
        Assert.True(p4 >= 1);
    }
}
