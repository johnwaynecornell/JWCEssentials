namespace JWCFarm.Metrics;

public static class MathFunctions
{
    [Metric]
    [MetricHelp("Get the absolute value of an input.")]
    public static double abs(object sample, double value)
    {
        return Math.Abs(value);
    }

    [Metric]
    [MetricHelp("Negate an input value.")]
    public static double negate(object sample, double value)
    {
        return -value;
    }

    [Metric]
    [MetricHelp("Square an input value.")]
    public static double square(object sample, double value)
    {
        return value * value;
    }

    [Metric]
    [MetricHelp("Get the square root of an input.")]
    public static double sqrt(object sample, double value)
    {
        return Math.Sqrt(value);
    }

    [Metric]
    [MetricHelp("Get the natural logarithm of an input.")]
    public static double ln(object sample, double value)
    {
        return Math.Log(value);
    }

    [Metric]
    [MetricHelp("Add two values together.")]
    public static double add(object sample, double a, double b)
    {
        return a + b;
    }

    [Metric]
    [MetricHelp("Subtract one value from another.")]
    public static double sub(object sample, double a, double b)
    {
        return a - b;
    }

    [Metric]
    [MetricHelp("Multiply two values together.")]
    public static double mul(object sample, double a, double b)
    {
        return a * b;
    }

    [Metric]
    [MetricHelp("Divide one value by another.")]
    public static double div(object sample, double a, double b)
    {
        return a / b;
    }

    [Metric]
    [MetricHelp("Calculate the nth root of a value using reciprocal power.")]
    public static double root(object sample, double value, double n)
    {
        return Math.Pow(value, 1.0 / n);
    }

    [Metric]
    [MetricHelp("Raise a value to a power.")]
    public static double pow(object sample, double value, double exponent)
    {
        return Math.Pow(value, exponent);
    }

    [Metric]
    [MetricHelp("Take the logarithm of a value at a base.")]
    public static double log(object sample, double value, double baseValue)
    {
        return Math.Log(value, baseValue);
    }

    [Metric]
    [MetricHelp("Add an offset to an input value.")]
    public static double offset(object sample, double value, double amount)
    {
        return value + amount;
    }

    [Metric]
    [MetricHelp("Measure an input relative to the 50 percent baseline.")]
    public static double offset50(object sample, double value)
    {
        return value - 50.0;
    }

    [Metric]
    [MetricHelp("Multiply an input by a scale factor.")]
    public static double scale(object sample, double value, double factor)
    {
        return value * factor;
    }

    [Metric]
    [MetricHelp("Divide one input by another.")]
    public static double ratio(object sample, double numerator, double denominator)
    {
        return numerator / denominator;
    }

    [Metric]
    [MetricHelp("Take the minimum of two values.")]
    public static double smaller(object sample, double a, double b)
    {
        return Math.Min(a, b);
    }

    [Metric]
    [MetricHelp("Take the maximum of two values.")]
    public static double larger(object sample, double a, double b)
    {
        return Math.Max(a, b);
    }

    [Metric]
    [MetricHelp("Limit an input to the inclusive minimum and maximum.")]
    public static double clamp(object sample, double value, double min, double max)
    {
        return Math.Clamp(value, min, max);
    }

    [Metric]
    [MetricHelp("Linearly interpolate between two values.")]
    public static double lerp(object sample, double a, double b, double amount)
    {
        return a + ((b - a) * amount);
    }
}

public static class AggregateFunctions
{
    [Metric]
    [MetricHelp("Find the number of values in an input population.")]
    public static double count(object sample, List<double> values)
    {
        return values.Count;
    }

    [Metric]
    [MetricHelp("Find the sum of an input population.")]
    public static double sum(object sample, List<double> values)
    {
        double sum = 0;

        foreach (double value in values)
            sum += value;

        return sum;
    }

    [Metric]
    [MetricHelp("Find the arithmetic mean of an input population.")]
    public static double mean(object sample, List<double> values)
    {
        if (values.Count == 0)
            return double.NaN;

        double sum = 0;

        foreach (double value in values)
            sum += value;

        return sum / values.Count;
    }

    [Metric]
    [MetricHelp("Find the minimum value in an input population.")]
    public static double min(object sample, List<double> values)
    {
        if (values.Count == 0)
            return double.NaN;

        double min = values[0];

        for (int i = 1; i < values.Count; i++)
            if (values[i] < min)
                min = values[i];

        return min;
    }

    [Metric]
    [MetricHelp("Find the maximum value in an input population.")]
    public static double max(object sample, List<double> values)
    {
        if (values.Count == 0)
            return double.NaN;

        double max = values[0];

        for (int i = 1; i < values.Count; i++)
            if (values[i] > max)
                max = values[i];

        return max;
    }

    [Metric]
    [MetricHelp("Find the median of an input population.")]
    public static double median(object sample, List<double> values)
    {
        if (values.Count == 0)
            return double.NaN;

        double[] sorted = values.ToArray();
        Array.Sort(sorted);

        int middle = sorted.Length / 2;

        if ((sorted.Length & 1) != 0)
            return sorted[middle];

        return (sorted[middle - 1] + sorted[middle]) / 2.0;
    }

    [Metric]
    [MetricHelp("Find the population variance of an input population.")]
    public static double variance_population(object sample, List<double> values)
    {
        if (values.Count == 0)
            return double.NaN;

        double average = mean(sample, values);
        double sumSquares = 0;

        foreach (double value in values)
        {
            double delta = value - average;
            sumSquares += delta * delta;
        }

        return sumSquares / values.Count;
    }

    [Metric]
    [MetricHelp("Find the sample variance of an input population.")]
    public static double variance_sample(object sample, List<double> values)
    {
        if (values.Count < 2)
            return double.NaN;

        double average = mean(sample, values);
        double sumSquares = 0;

        foreach (double value in values)
        {
            double delta = value - average;
            sumSquares += delta * delta;
        }

        return sumSquares / (values.Count - 1);
    }

    [Metric]
    [MetricHelp("Find the population standard deviation of an input population.")]
    public static double stddev_population(object sample, List<double> values)
    {
        return Math.Sqrt(variance_population(sample, values));
    }

    [Metric]
    [MetricHelp("Find the sample standard deviation of an input population.")]
    public static double stddev_sample(object sample, List<double> values)
    {
        return Math.Sqrt(variance_sample(sample, values));
    }

    [Metric]
    [MetricHelp("Find the root mean square of an input population.")]
    public static double rms(object sample, List<double> values)
    {
        if (values.Count == 0)
            return double.NaN;

        double sumSquares = 0;

        foreach (double value in values)
            sumSquares += value * value;

        return Math.Sqrt(sumSquares / values.Count);
    }

    [Metric]
    [MetricHelp("Find the mean absolute value of an input population.")]
    public static double mean_abs(object sample, List<double> values)
    {
        if (values.Count == 0)
            return double.NaN;

        double sum = 0;

        foreach (double value in values)
            sum += Math.Abs(value);

        return sum / values.Count;
    }

    [Metric]
    [MetricHelp("Find the population covariance between two input populations.")]
    public static double covariance_population(object sample, 
        List<double> x_values,
        List<double> y_values)
    {
        if (x_values.Count == 0 || x_values.Count != y_values.Count)
            return double.NaN;

        double xMean = mean(sample, x_values);
        double yMean = mean(sample, y_values);
        double sum = 0;

        for (int i = 0; i < x_values.Count; i++)
            sum += (x_values[i] - xMean) * (y_values[i] - yMean);

        return sum / x_values.Count;
    }

    [Metric]
    [MetricHelp("Find the sample covariance between two input populations.")]
    public static double covariance_sample(object sample, 
        List<double> x_values,
        List<double> y_values)
    {
        if (x_values.Count < 2 || x_values.Count != y_values.Count)
            return double.NaN;

        double xMean = mean(sample, x_values);
        double yMean = mean(sample, y_values);
        double sum = 0;

        for (int i = 0; i < x_values.Count; i++)
            sum += (x_values[i] - xMean) * (y_values[i] - yMean);

        return sum / (x_values.Count - 1);
    }

    [Metric]
    [MetricHelp("Find the Pearson correlation coefficient between two input populations.")]
    public static double pearson(object sample, 
        List<double> x_values,
        List<double> y_values)
    {
        if (x_values.Count < 2 || x_values.Count != y_values.Count)
            return double.NaN;

        double xMean = mean(sample, x_values);
        double yMean = mean(sample, y_values);

        double covariance = 0;
        double xSquares = 0;
        double ySquares = 0;

        for (int i = 0; i < x_values.Count; i++)
        {
            double xDelta = x_values[i] - xMean;
            double yDelta = y_values[i] - yMean;

            covariance += xDelta * yDelta;
            xSquares += xDelta * xDelta;
            ySquares += yDelta * yDelta;
        }

        double denominator = Math.Sqrt(xSquares * ySquares);

        if (denominator == 0)
            return double.NaN;

        return Math.Clamp(covariance / denominator, -1.0, 1.0);
    }
}

public static class StatisticalFunctions
{
    [Metric]
    [MetricHelp("Find the arithmetic mean of an input population.")]
    public static double mean(object sample, List<double> values)
        => AggregateFunctions.mean(sample, values);

    [Metric]
    [MetricHelp("Find the population variance of an input population.")]
    public static double variance_population(object sample, List<double> values)
        => AggregateFunctions.variance_population(sample, values);

    [Metric]
    [MetricHelp("Find the sample variance of an input population.")]
    public static double variance_sample(object sample, List<double> values)
        => AggregateFunctions.variance_sample(sample, values);

    [Metric]
    [MetricHelp("Find the population standard deviation of an input population.")]
    public static double stddev_population(object sample, List<double> values)
        => AggregateFunctions.stddev_population(sample, values);

    [Metric]
    [MetricHelp("Find the sample standard deviation of an input population.")]
    public static double stddev_sample(object sample, List<double> values)
        => AggregateFunctions.stddev_sample(sample, values);

    [Metric]
    [MetricHelp("Find the root mean square of an input population.")]
    public static double rms(object sample, List<double> values)
        => AggregateFunctions.rms(sample, values);

    [Metric]
    [MetricHelp("Find the mean absolute value of an input population.")]
    public static double mean_abs(object sample, List<double> values)
        => AggregateFunctions.mean_abs(sample, values);

    [Metric]
    [MetricHelp("Find the population covariance between two input populations.")]
    public static double covariance_population(object sample, List<double> x_values, List<double> y_values)
        => AggregateFunctions.covariance_population(sample, x_values, y_values);

    [Metric]
    [MetricHelp("Find the sample covariance between two input populations.")]
    public static double covariance_sample(object sample, List<double> x_values, List<double> y_values)
        => AggregateFunctions.covariance_sample(sample, x_values, y_values);

    [Metric]
    [MetricHelp("Find the Pearson correlation coefficient between two input populations.")]
    public static double pearson(object sample, List<double> x_values, List<double> y_values)
        => AggregateFunctions.pearson(sample, x_values, y_values);
}

public abstract class MetricFunctions
{
    [Metric]
    [MetricHelp("Get the absolute value of an input.")]
    public static double abs(object sample, double value) => MathFunctions.abs(sample, value);

    [Metric]
    [MetricHelp("Negate an input value.")]
    public static double negate(object sample, double value) => MathFunctions.negate(sample, value);

    [Metric]
    [MetricHelp("Square an input value.")]
    public static double square(object sample, double value) => MathFunctions.square(sample, value);

    [Metric]
    [MetricHelp("Get the square root of an input.")]
    public static double sqrt(object sample, double value) => MathFunctions.sqrt(sample, value);

    [Metric]
    [MetricHelp("Get the natural logarithm of an input.")]
    public static double ln(object sample, double value) => MathFunctions.ln(sample, value);

    [Metric]
    [MetricHelp("Add two values together.")]
    public static double add(object sample, double a, double b) => MathFunctions.add(sample, a, b);

    [Metric]
    [MetricHelp("Subtract one value from another.")]
    public static double sub(object sample, double a, double b) => MathFunctions.sub(sample, a, b);

    [Metric]
    [MetricHelp("Multiply two values together.")]
    public static double mul(object sample, double a, double b) => MathFunctions.mul(sample, a, b);

    [Metric]
    [MetricHelp("Divide one value by another.")]
    public static double div(object sample, double a, double b) => MathFunctions.div(sample, a, b);

    [Metric]
    [MetricHelp("Calculate the nth root of a value using reciprocal power.")]
    public static double root(object sample, double value, double n) => MathFunctions.root(sample, value, n);

    [Metric]
    [MetricHelp("Raise a value to a power.")]
    public static double pow(object sample, double value, double exponent) => MathFunctions.pow(sample, value, exponent);

    [Metric]
    [MetricHelp("Take the logarithm of a value at a base.")]
    public static double log(object sample, double value, double baseValue) => MathFunctions.log(sample, value, baseValue);

    [Metric]
    [MetricHelp("Add an offset to an input value.")]
    public static double offset(object sample, double value, double amount) => MathFunctions.offset(sample, value, amount);

    [Metric]
    [MetricHelp("Measure an input relative to the 50 percent baseline.")]
    public static double offset50(object sample, double value) => MathFunctions.offset50(sample, value);

    [Metric]
    [MetricHelp("Multiply an input by a scale factor.")]
    public static double scale(object sample, double value, double factor) => MathFunctions.scale(sample, value, factor);

    [Metric]
    [MetricHelp("Divide one input by another.")]
    public static double ratio(object sample, double numerator, double denominator) => MathFunctions.ratio(sample, numerator, denominator);

    [Metric]
    [MetricHelp("Take the minimum of two values.")]
    public static double smaller(object sample, double a, double b) => MathFunctions.smaller(sample, a, b);

    [Metric]
    [MetricHelp("Take the maximum of two values.")]
    public static double larger(object sample, double a, double b) => MathFunctions.larger(sample, a, b);

    [Metric]
    [MetricHelp("Limit an input to the inclusive minimum and maximum.")]
    public static double clamp(object sample, double value, double min, double max) => MathFunctions.clamp(sample, value, min, max);

    [Metric]
    [MetricHelp("Linearly interpolate between two values.")]
    public static double lerp(object sample, double a, double b, double amount) => MathFunctions.lerp(sample, a, b, amount);

    public static MetricCatalog LoadMath(MetricCatalog catalog)
        => MetricCatalogs.MetricLoadStaticFromType(typeof(MathFunctions), catalog);

    public static MetricCatalog LoadAggregate(MetricCatalog catalog)
        => MetricCatalogs.MetricLoadStaticFromType(typeof(AggregateFunctions), catalog);

    public static MetricCatalog LoadStatistical(MetricCatalog catalog)
        => MetricCatalogs.MetricLoadStaticFromType(typeof(StatisticalFunctions), catalog);

    public static MetricCatalog LoadAll(MetricCatalog catalog)
    {
        MetricCatalogs.MetricLoadStaticFromType(typeof(MathFunctions), catalog);
        MetricCatalogs.MetricLoadStaticFromType(typeof(AggregateFunctions), catalog);
        return catalog;
    }

    public static MetricCatalog CreateAllCatalog()
        => LoadAll(new MetricCatalog());
}

public abstract class MetricFunctionsAggregate : MetricFunctions
{
    [Metric]
    [MetricHelp("Find the number of values in an input population.")]
    public static double count(object sample, List<double> values) => AggregateFunctions.count(sample, values);

    [Metric]
    [MetricHelp("Find the sum of an input population.")]
    public static double sum(object sample, List<double> values) => AggregateFunctions.sum(sample, values);

    [Metric]
    [MetricHelp("Find the arithmetic mean of an input population.")]
    public static double mean(object sample, List<double> values) => AggregateFunctions.mean(sample, values);

    [Metric]
    [MetricHelp("Find the minimum value in an input population.")]
    public static double min(object sample, List<double> values) => AggregateFunctions.min(sample, values);

    [Metric]
    [MetricHelp("Find the maximum value in an input population.")]
    public static double max(object sample, List<double> values) => AggregateFunctions.max(sample, values);

    [Metric]
    [MetricHelp("Find the median of an input population.")]
    public static double median(object sample, List<double> values) => AggregateFunctions.median(sample, values);

    [Metric]
    [MetricHelp("Find the population variance of an input population.")]
    public static double variance_population(object sample, List<double> values) => AggregateFunctions.variance_population(sample, values);

    [Metric]
    [MetricHelp("Find the sample variance of an input population.")]
    public static double variance_sample(object sample, List<double> values) => AggregateFunctions.variance_sample(sample, values);

    [Metric]
    [MetricHelp("Find the population standard deviation of an input population.")]
    public static double stddev_population(object sample, List<double> values) => AggregateFunctions.stddev_population(sample, values);

    [Metric]
    [MetricHelp("Find the sample standard deviation of an input population.")]
    public static double stddev_sample(object sample, List<double> values) => AggregateFunctions.stddev_sample(sample, values);

    [Metric]
    [MetricHelp("Find the root mean square of an input population.")]
    public static double rms(object sample, List<double> values) => AggregateFunctions.rms(sample, values);

    [Metric]
    [MetricHelp("Find the mean absolute value of an input population.")]
    public static double mean_abs(object sample, List<double> values) => AggregateFunctions.mean_abs(sample, values);

    [Metric]
    [MetricHelp("Find the population covariance between two input populations.")]
    public static double covariance_population(object sample, List<double> x_values, List<double> y_values) => AggregateFunctions.covariance_population(sample, x_values, y_values);

    [Metric]
    [MetricHelp("Find the sample covariance between two input populations.")]
    public static double covariance_sample(object sample, List<double> x_values, List<double> y_values) => AggregateFunctions.covariance_sample(sample, x_values, y_values);

    [Metric]
    [MetricHelp("Find the Pearson correlation coefficient between two input populations.")]
    public static double pearson(object sample, List<double> x_values, List<double> y_values) => AggregateFunctions.pearson(sample, x_values, y_values);
}
