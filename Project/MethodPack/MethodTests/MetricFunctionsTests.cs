using JWCFarm;
using JWCFarm.Metrics;
using Xunit;

namespace MethodTests;

public class MetricFunctionsTests
{
    private sealed class MockProcess : FarmProcess
    {
        private readonly Type _statType;
        private readonly Type _inputType;
        private readonly IEnumerable<object>? _items;

        public MockProcess(Type statType, Type inputType, IEnumerable<object>? items = null)
        {
            _statType = statType;
            _inputType = inputType;
            _items = items;
        }

        public override Type StatType => _statType;
        public override Type InputType => _inputType;

        protected override IEnumerable<object> EnumerateItems(FarmContext context)
            => _items ?? Enumerable.Empty<object>();
    }

    public sealed class DataItem
    {
        [Metric]
        public double X { get; init; }

        [Metric]
        public double Y { get; init; }
    }

    [Fact]
    public void MetricLoadStaticFromType_MathFunctions_LoadsExpectedMetrics()
    {
        var catalog = MetricCatalogs.MetricLoadStaticFromType(typeof(MathFunctions));
        Assert.NotNull(catalog);
        Assert.Equal(20, catalog.Metrics.Count);
        Assert.True(catalog.Metrics.ContainsKey("abs"));
        Assert.True(catalog.Metrics.ContainsKey("negate"));
        Assert.True(catalog.Metrics.ContainsKey("square"));
        Assert.True(catalog.Metrics.ContainsKey("sqrt"));
        Assert.True(catalog.Metrics.ContainsKey("ln"));
        Assert.True(catalog.Metrics.ContainsKey("add"));
        Assert.True(catalog.Metrics.ContainsKey("sub"));
        Assert.True(catalog.Metrics.ContainsKey("mul"));
        Assert.True(catalog.Metrics.ContainsKey("div"));
        Assert.True(catalog.Metrics.ContainsKey("root"));
        Assert.True(catalog.Metrics.ContainsKey("pow"));
        Assert.True(catalog.Metrics.ContainsKey("log"));
        Assert.True(catalog.Metrics.ContainsKey("offset"));
        Assert.True(catalog.Metrics.ContainsKey("offset50"));
        Assert.True(catalog.Metrics.ContainsKey("scale"));
        Assert.True(catalog.Metrics.ContainsKey("ratio"));
        Assert.True(catalog.Metrics.ContainsKey("smaller"));
        Assert.True(catalog.Metrics.ContainsKey("larger"));
        Assert.True(catalog.Metrics.ContainsKey("clamp"));
        Assert.True(catalog.Metrics.ContainsKey("lerp"));
    }

    [Fact]
    public void MetricLoadStaticFromType_AggregateFunctions_LoadsExpectedMetrics()
    {
        var catalog = MetricCatalogs.MetricLoadStaticFromType(typeof(AggregateFunctions));
        Assert.NotNull(catalog);
        Assert.Equal(15, catalog.Metrics.Count);
        Assert.True(catalog.Metrics.ContainsKey("count"));
        Assert.True(catalog.Metrics.ContainsKey("sum"));
        Assert.True(catalog.Metrics.ContainsKey("mean"));
        Assert.True(catalog.Metrics.ContainsKey("min"));
        Assert.True(catalog.Metrics.ContainsKey("max"));
        Assert.True(catalog.Metrics.ContainsKey("median"));
        Assert.True(catalog.Metrics.ContainsKey("variance_population"));
        Assert.True(catalog.Metrics.ContainsKey("variance_sample"));
        Assert.True(catalog.Metrics.ContainsKey("stddev_population"));
        Assert.True(catalog.Metrics.ContainsKey("stddev_sample"));
        Assert.True(catalog.Metrics.ContainsKey("rms"));
        Assert.True(catalog.Metrics.ContainsKey("mean_abs"));
        Assert.True(catalog.Metrics.ContainsKey("covariance_population"));
        Assert.True(catalog.Metrics.ContainsKey("covariance_sample"));
        Assert.True(catalog.Metrics.ContainsKey("pearson"));
    }

    [Fact]
    public void MetricLoadStaticFromType_StatisticalFunctions_LoadsExpectedMetrics()
    {
        var catalog = MetricCatalogs.MetricLoadStaticFromType(typeof(StatisticalFunctions));
        Assert.NotNull(catalog);
        Assert.Equal(10, catalog.Metrics.Count);
        Assert.True(catalog.Metrics.ContainsKey("mean"));
        Assert.True(catalog.Metrics.ContainsKey("variance_population"));
        Assert.True(catalog.Metrics.ContainsKey("variance_sample"));
        Assert.True(catalog.Metrics.ContainsKey("stddev_population"));
        Assert.True(catalog.Metrics.ContainsKey("stddev_sample"));
        Assert.True(catalog.Metrics.ContainsKey("rms"));
        Assert.True(catalog.Metrics.ContainsKey("mean_abs"));
        Assert.True(catalog.Metrics.ContainsKey("covariance_population"));
        Assert.True(catalog.Metrics.ContainsKey("covariance_sample"));
        Assert.True(catalog.Metrics.ContainsKey("pearson"));
    }

    [Fact]
    public void MetricLoadStaticFromType_AppendsToExistingCatalog()
    {
        var catalog = new MetricCatalog();
        MetricCatalogs.MetricLoadStaticFromType(typeof(MathFunctions), catalog);
        Assert.Equal(20, catalog.Metrics.Count);

        MetricCatalogs.MetricLoadStaticFromType(typeof(AggregateFunctions), catalog);
        Assert.Equal(35, catalog.Metrics.Count);
    }

    [Fact]
    public void MathFunctions_DirectInvocations_MatchExpectedCalculations()
    {
        object sample = new();
        Assert.Equal(5.0, MathFunctions.abs(sample, -5.0));
        Assert.Equal(-5.0, MathFunctions.negate(sample, 5.0));
        Assert.Equal(25.0, MathFunctions.square(sample, 5.0));
        Assert.Equal(3.0, MathFunctions.sqrt(sample, 9.0));
        Assert.Equal(0.0, MathFunctions.ln(sample, 1.0));
        Assert.Equal(7.0, MathFunctions.add(sample, 3.0, 4.0));
        Assert.Equal(2.0, MathFunctions.sub(sample, 6.0, 4.0));
        Assert.Equal(12.0, MathFunctions.mul(sample, 3.0, 4.0));
        Assert.Equal(2.5, MathFunctions.div(sample, 5.0, 2.0));
        Assert.Equal(2.0, MathFunctions.root(sample, 8.0, 3.0));
        Assert.Equal(8.0, MathFunctions.pow(sample, 2.0, 3.0));
        Assert.Equal(3.0, MathFunctions.log(sample, 8.0, 2.0));
        Assert.Equal(15.0, MathFunctions.offset(sample, 10.0, 5.0));
        Assert.Equal(25.0, MathFunctions.offset50(sample, 75.0));
        Assert.Equal(20.0, MathFunctions.scale(sample, 10.0, 2.0));
        Assert.Equal(0.5, MathFunctions.ratio(sample, 1.0, 2.0));
        Assert.Equal(3.0, MathFunctions.smaller(sample, 3.0, 5.0));
        Assert.Equal(5.0, MathFunctions.larger(sample, 3.0, 5.0));
        Assert.Equal(5.0, MathFunctions.clamp(sample, 10.0, 0.0, 5.0));
        Assert.Equal(15.0, MathFunctions.lerp(sample, 10.0, 20.0, 0.5));
    }

    [Fact]
    public void AggregateFunctions_DirectInvocations_MatchExpectedCalculations()
    {
        object sample = new();
        List<double> values = [2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0];

        Assert.Equal(8.0, AggregateFunctions.count(sample, values));
        Assert.Equal(40.0, AggregateFunctions.sum(sample, values));
        Assert.Equal(5.0, AggregateFunctions.mean(sample, values));
        Assert.Equal(2.0, AggregateFunctions.min(sample, values));
        Assert.Equal(9.0, AggregateFunctions.max(sample, values));
        Assert.Equal(4.5, AggregateFunctions.median(sample, values));
        Assert.Equal(3.5, AggregateFunctions.median(sample, [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]));
        Assert.Equal(3.0, AggregateFunctions.median(sample, [1.0, 2.0, 3.0, 4.0, 5.0]));

        Assert.Equal(4.0, AggregateFunctions.variance_population(sample, values), 5);
        Assert.Equal(4.571428571428571, AggregateFunctions.variance_sample(sample, values), 5);
        Assert.Equal(2.0, AggregateFunctions.stddev_population(sample, values), 5);
        Assert.Equal(2.138089935299395, AggregateFunctions.stddev_sample(sample, values), 5);

        List<double> negValues = [-2.0, 4.0, -6.0];
        Assert.Equal(4.0, AggregateFunctions.mean_abs(sample, negValues));

        List<double> rmsValues = [3.0, 4.0];
        Assert.Equal(3.5355339059327378, AggregateFunctions.rms(sample, rmsValues), 5);

        List<double> x = [1.0, 2.0, 3.0, 4.0, 5.0];
        List<double> y = [2.0, 4.0, 6.0, 8.0, 10.0];
        Assert.Equal(4.0, AggregateFunctions.covariance_population(sample, x, y), 5);
        Assert.Equal(5.0, AggregateFunctions.covariance_sample(sample, x, y), 5);
        Assert.Equal(1.0, AggregateFunctions.pearson(sample, x, y), 5);
    }

    [Fact]
    public void AggregateFunctions_EmptyAndSingleElement_HandleEdgeCases()
    {
        object sample = new();
        List<double> empty = [];

        Assert.True(double.IsNaN(AggregateFunctions.mean(sample, empty)));
        Assert.True(double.IsNaN(AggregateFunctions.min(sample, empty)));
        Assert.True(double.IsNaN(AggregateFunctions.max(sample, empty)));
        Assert.True(double.IsNaN(AggregateFunctions.median(sample, empty)));
        Assert.True(double.IsNaN(AggregateFunctions.variance_population(sample, empty)));
        Assert.True(double.IsNaN(AggregateFunctions.variance_sample(sample, empty)));
        Assert.True(double.IsNaN(AggregateFunctions.variance_sample(sample, [5.0])));
        Assert.True(double.IsNaN(AggregateFunctions.rms(sample, empty)));
        Assert.True(double.IsNaN(AggregateFunctions.mean_abs(sample, empty)));

        Assert.True(double.IsNaN(AggregateFunctions.covariance_population(sample, empty, empty)));
        Assert.True(double.IsNaN(AggregateFunctions.covariance_sample(sample, [1.0], [2.0])));
        Assert.True(double.IsNaN(AggregateFunctions.pearson(sample, [1.0], [2.0])));
        Assert.True(double.IsNaN(AggregateFunctions.pearson(sample, [1.0, 1.0], [2.0, 3.0])));
    }

    [Fact]
    public void BinderAndSession_CanBindAndEvaluateMathAndAggregateFunctions()
    {
        var catalogs = new MetricCatalogs();
        catalogs.Catalogs[typeof(DataItem)] = MetricCatalogs.DefaultReflect(typeof(DataItem));

        var mathCat = MetricCatalogs.MetricLoadStaticFromType(typeof(MathFunctions));
        var aggCat = MetricCatalogs.MetricLoadStaticFromType(typeof(AggregateFunctions));

        var combinedStatCat = new MetricCatalog();
        foreach (var kvp in mathCat.Metrics) combinedStatCat.Metrics[kvp.Key] = kvp.Value;
        foreach (var kvp in aggCat.Metrics) combinedStatCat.Metrics[kvp.Key] = kvp.Value;

        catalogs.Catalogs[typeof(MetricFunctions)] = combinedStatCat;

        var items = new List<DataItem>
        {
            new() { X = 10.0, Y = 20.0 },
            new() { X = 30.0, Y = 40.0 }
        };

        var process = new MockProcess(typeof(MetricFunctions), typeof(DataItem), items);

        string[] fields = [
            "add#5.0,15.0",
            "count#X",
            "sum#X",
            "mean#X",
            "max#Y",
            "pearson#X,Y"
        ];

        bool bound = process.BindFields(catalogs, fields, out var error);
        Assert.True(bound, error?.FormatDiagnostic());
        Assert.NotNull(process.Projection);

        var session = new MetricEvaluationSession(process.Projection);
        object dummy = new();
        foreach (var item in items)
        {
            session.Inspect(process, dummy, item);
        }

        Assert.Equal(20.0, (double)process.Projection.Fields[0].Get(session, dummy));
        Assert.Equal(2.0, (double)process.Projection.Fields[1].Get(session, dummy));
        Assert.Equal(40.0, (double)process.Projection.Fields[2].Get(session, dummy));
        Assert.Equal(20.0, (double)process.Projection.Fields[3].Get(session, dummy));
        Assert.Equal(40.0, (double)process.Projection.Fields[4].Get(session, dummy));
        Assert.Equal(1.0, (double)process.Projection.Fields[5].Get(session, dummy), 5);
    }
}
