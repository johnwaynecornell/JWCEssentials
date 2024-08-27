// See https://aka.ms/new-console-template for more information

using JWCEssentials.net;

Console.WriteLine("Hello, World!");

string[,] a = new string[,] { { "a", "b", "c" }, { "1", "2", "3" }, { "x", "y", "z" } };

HasherClass32 H = (HasherClass32) HasherClass.FromFactory("Buffered:FarmHash:32");

H.Begin();
H.Compute(a);
H.End();

Console.WriteLine(H.value);
