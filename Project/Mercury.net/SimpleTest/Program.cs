using Mercury.net;

UltraNumber.SetPrecisionAndAllocateStack(16,1024*1024);

UltraNumber a = 1;
UltraNumber b = 3;

UltraNumber c = a / b;
Console.WriteLine(c);
