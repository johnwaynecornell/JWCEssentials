namespace JWCFarm.Metrics;

public class MetricAttribute : Attribute
{
    public List<string>? SourceExpressions;
    public String? Name { get; }
    
    public MetricAttribute(string[]? sourceExpressions = null) 
    {
        if (sourceExpressions is not null) SourceExpressions = new List<string>(sourceExpressions);
    }
}

public class MetricHelpAttribute : Attribute
{
    public string Description;

    public MetricHelpAttribute(string description)
    {
        Description = description;
    }
}