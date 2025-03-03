// See https://aka.ms/new-console-template for more information

using JWCEssentials.net;

Console.WriteLine("Hello, World!");

string[,] a = new string[,] { { "a", "b", "c" }, { "1", "2", "3" }, { "x", "y", "z" } };

HasherClass32 H = (HasherClass32) HasherClass.FromFactory("Buffered:FarmHash:32");

H.Begin();
H.Compute(a);
H.End();

Console.WriteLine(H.value);

RandomGenerator r;

string[] codes = Essentials.feffect_list();

r = RandomGenerator.MT19937_Create(32);

for (int i = 0; i < 128; i++)
{
    if (r.Get_uint32_t() % 2 == 1) r.GetByte();
}

byte[] state = r.get_state();

Console.WriteLine("_______________");
Console.WriteLine(r.GetDouble());
Console.WriteLine(r.Get_uint32_t());
Console.WriteLine(Essentials.feffect($"fg_green('{r.cstyle_identifier()}')"));

r = RandomGenerator.MT19937_Create(902834);
r.set_state(state);

Console.WriteLine("_______________");
Console.WriteLine(r.GetDouble());
Console.WriteLine(r.Get_uint32_t());
Console.WriteLine(Essentials.feffect($"fg_green('{r.cstyle_identifier()}')"));

double d = 0.0;
int c = 0;

for (int i = 0; i < 1000; i++) 
{
    d += r.GetDouble();
    c++; 
}

Console.WriteLine(d / c);


