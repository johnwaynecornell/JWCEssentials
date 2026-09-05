using System.Reflection;
namespace JWCFarm.Metrics;

public class MetricCatalog
{
    public Dictionary<string, MetricDescriptor> Metrics { get; set; } = new Dictionary<string, MetricDescriptor>();
    
    public void Add(MetricDescriptor metric)
    {
        Metrics[metric.Name] = metric;
    }
    
}