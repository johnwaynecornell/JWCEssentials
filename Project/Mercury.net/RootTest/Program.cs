using Mercury.net;

UltraNumber.SetPrecisionAndAllocateStack(16,1024*1024);

UltraNumber a = 3;
UltraNumber b = 3;

UltraNumber c = UltraNumber.Root(a, b);
Console.WriteLine(c);

//1.7137449123EF65CDDE7F16C56E3267C0A1894C2AF56ECD99F4574287D2052CA55FCF9C0DAB2C0AF2873C818C05C18213D62508AE9756B461AA585188
//B504.F333F9DE6484597D89B3754ABE9F1D6F60BA893BA84CED17AC85833399154AFC83043AB8A2C3A8B1FE6FDC83DB390F74A85E439C7B4A780487363DFA

c = UltraNumber.Sqrt((ulong)1 << 31);
Console.WriteLine("1 << 31 : "+c.ToString());

UltraNumber bit;

int place = 0;
for (int i = 1; i < 66; i++)
{
    bit = UltraNumber.TwoPow(place - i);    
    c = UltraNumber.Sqrt(bit);
    Console.WriteLine($"bit {place - i} : " + bit.ToString());
    Console.WriteLine($"bit root 2 : " + c.ToString());
}

Console.WriteLine();

bit = UltraNumber.TwoPow(-65);
UltraNumber root = UltraNumber.Sqrt(bit);
UltraNumber reparsed = UltraNumber.Parse(root.ToString());

Console.WriteLine(root.ToString());
Console.WriteLine(reparsed.ToString());
Console.WriteLine(root == reparsed);

UltraNumber test = UltraNumber.Parse("F@+10");
Console.WriteLine(test.ToString());