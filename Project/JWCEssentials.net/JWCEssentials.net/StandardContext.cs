namespace JWCEssentials.net;

public class StandardContext
{
    public TextReader Input { get; set; } = Console.In;
    public TextWriter Output { get; set; } = Console.Out;
    public TextWriter ErrorOutput { get; set; } = Console.Error;

    public int Status { get; set; } = 0;
}