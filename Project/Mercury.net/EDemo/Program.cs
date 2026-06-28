using Mercury.net;

// Set precision for the calculation:
// 128 elements of 32 bits each = 4096 bits of precision.
UltraNumber.SetPrecisionAndAllocateStack(128, 1024 * 1024);

Console.WriteLine("Mercury.net Demo: Progressively estimating Euler's number (e)");
Console.WriteLine("Using Taylor series: e = sum_{n=0}^{infinity} (1 / n!)");
Console.WriteLine("------------------------------------------------------------");

UltraNumber e = 0;
UltraNumber factorial = 1;

for (int n = 0; n < 100; n++)
{
    // 0! is 1, so we start multiplying from n=1
    if (n > 0)
    {
        factorial *= (double)n;
    }

    // e += 1 / n!
    UltraNumber term = 1.0 / factorial;
    e += term;

    // Show progress at intervals to demonstrate 'progressive estimation'
    if (n < 5 || n % 10 == 0)
    {
        string display = e.ToString();
        if (display.Length > 80) display = display.Substring(0, 80) + "...";
        Console.WriteLine($"n = {n,2} | e approx: {display}");
    }
}

Console.WriteLine("------------------------------------------------------------");
Console.WriteLine("Final estimation of e (in hexadecimal):");
Console.WriteLine(e);
