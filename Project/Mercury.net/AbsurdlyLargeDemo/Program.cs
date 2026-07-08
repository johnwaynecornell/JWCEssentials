using Mercury.net;

UltraNumber.SetPrecisionAndAllocateStack(16,1024*1024);

Console.WriteLine(@"1. Exact giant powers of two

This is the cleanest first demo:

UltraNumber x = UltraNumber.TwoPow(1_000_000);
Console.WriteLine(x);
Console.WriteLine(UltraNumber.Log(x, 2));");

UltraNumber x = UltraNumber.TwoPow(1_000_000);
Console.WriteLine(x);
UltraNumber t = UltraNumber.Log(x, 2);
Console.WriteLine($"{t} // {(double)t}");

PageBreak();

Console.WriteLine(@"2. Square root of absurd powers
UltraNumber x = UltraNumber.TwoPow(1_000_000);
UltraNumber r = UltraNumber.Sqrt(x);

Console.WriteLine(r);
Console.WriteLine(r * r);

Conceptually:

sqrt(2^1,000,000) = 2^500,000");

OutputEnter();

x = UltraNumber.TwoPow(1_000_000);
UltraNumber r = UltraNumber.Sqrt(x);

Console.WriteLine($"x = 2^1,000,000 = {x}");
Console.WriteLine($"sqrt(x) = 2^500,000 = {r}");

PageBreak();

Console.WriteLine(@"Then test an odd exponent:

UltraNumber x = UltraNumber.TwoPow(1_000_001);
UltraNumber r = UltraNumber.Sqrt(x);
Console.WriteLine(r);

Conceptually:

sqrt(2^1,000,001) = 2^500000.5

That one shows the fractional hinge: it should contain the sqrt(2) signature scaled way out into absurd space.");
OutputEnter();

x = UltraNumber.TwoPow(1_000_001);
r = UltraNumber.Sqrt(x);
Console.WriteLine($"sqrt(2^1,000,001) = sqrt(2) * 2^500,000 = {r}");

PageBreak();

Console.WriteLine(@"3. Giant root/log round trip
UltraNumber x = UltraNumber.TwoPow(1234567);
UltraNumber l = UltraNumber.Log(x, 2);
UltraNumber y = UltraNumber.Pow(2, l);

Console.WriteLine(l);
Console.WriteLine(y);

The conceptual loop:

x → log₂(x) → pow(2, result) → x");
OutputEnter();

x = UltraNumber.TwoPow(1234567);
UltraNumber l = UltraNumber.Log(x, 2);
UltraNumber y = UltraNumber.Pow(2, l);

Console.WriteLine($"{l} // {(double)l}");
Console.WriteLine(y);

PageBreak();

Console.WriteLine(@"4. “Impossible looking” decimal powers
UltraNumber x = UltraNumber.Pow(10, 100000);
UltraNumber l = UltraNumber.Log(x, 10);

Console.WriteLine(x);
Console.WriteLine(l);

Expected:

log_10(10^100000) ≈ 100000

This is fun because humans understand 10^100000 as “a 1 with 100,000 zeroes,” but Mercury does not need to print or store every zero as ordinary text to reason about the magnitude.");
OutputEnter();

x = UltraNumber.Pow(10, 100000);
l = UltraNumber.Log(x, 10);

Console.WriteLine(x);
Console.WriteLine($"{l} // {(double) l}");

PageBreak();

Console.WriteLine(@"5. The “telescope” demo

This one feels aligned with your visual intuition:

UltraNumber a = 3;
UltraNumber p = a;

for (int i = 0; i < 20; i++)
{
    Console.WriteLine($""step {i}: {p}"");
    p = UltraNumber.Sqrt(p);
}");
OutputEnter();

UltraNumber a = 3;
UltraNumber p = a;

for (int i = 0; i < 20; i++)
{
    Console.WriteLine($"step {i}: {p}");
    p = UltraNumber.Sqrt(p);
}

PageBreak();

Console.WriteLine(@"Then reverse it:

for (int i = 0; i < 20; i++)
{
    p = p * p;
    Console.WriteLine($""reverse {i}: {p}"");
}

That demonstrates the telescope opening and closing around the identity.
Reverse telescope returns near 3; the final tail shows accumulated finite-precision loss.");
OutputEnter();

for (int i = 0; i < 20; i++)
{
    p = UltraNumber.Sqr(p);
    Console.WriteLine($"reverse {i}: {p}");
}

PageBreak();
Console.WriteLine(@"micro round p.
trim off the last 2 base 2 pow 32 digits.
Console.WriteLine($""micro rounded : {p.RoundPlaces(2)}"");");
OutputEnter();
Console.WriteLine($"micro rounded : {p.RoundPlaces(2)}");
PageBreak();
    

Console.WriteLine(@"6. Log against absurd exponent bit positions

This is probably the most Mercury-flavored:

int place = 1_000_000;

UltraNumber x = UltraNumber.TwoPow(place);
UltraNumber l = UltraNumber.Log(x, 2);

Console.WriteLine($""x = 2^{place}"");
Console.WriteLine($""log_2(x) = {l}"");

If that works cleanly, it is a gorgeous proof that log is not intimidated by the value’s scale. It just walks the geometric ladder back to the exponent.");
OutputEnter();

int place = 1_000_000;

x = UltraNumber.TwoPow(place);
l = UltraNumber.Log(x, 2);

Console.WriteLine($"x = 2^{place}");
Console.WriteLine($"log_2(x) = {l} // {(double)l}");

PageBreak();

UltraNumber MicroRound(UltraNumber p)
{
    bool rnd = (p.Elements[2] >= 0x80000000);
    p.Elements[2] = 0;
    if (rnd) p += UltraNumber.FromRaw(false, (((int)p.Elements[1])- (p.Precision - 2)), new[] { (uint) 1 });
    return p;
}

void OutputEnter()
{
    Console.WriteLine($"\n{new string('/', 100)}\n");
}

void PageBreak()
{
    Console.WriteLine($"\n{new string('_', 100)}\n");
}