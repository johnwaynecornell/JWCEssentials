using System.Reflection;

namespace JWCFarm.Metrics;

public class MetricCatalogs
{
    public Dictionary<Type, MetricCatalog?> Catalogs { get; set; } = new Dictionary<Type, MetricCatalog>();
    public Func<Type, MetricCatalog?>? Reflect { get; set; } = (T) => null;

    public bool TryGet(Type type, out MetricCatalog? catalog)
    {
        if (Catalogs.TryGetValue(type, out catalog)) return catalog != null;
        catalog = Catalogs[type] = Reflect(type);
        return catalog != null;
        
    }
    
    private static bool IsConcreteGenericList(Type type)
    {
        return type.IsGenericType && type.GetGenericTypeDefinition() == typeof(List<>);
    }
    
    public static MetricDescriptor? MetricLoadStaticFromMethod(MethodInfo methodInfo, bool autoProp = false)
    {
        MetricAttribute? metricAttribute =
            methodInfo.GetCustomAttributes(typeof(MetricAttribute), true).FirstOrDefault() as MetricAttribute;
        
        if (metricAttribute == null)
            return null;

        ParameterInfo[] parameters = methodInfo.GetParameters();

        List<MetricParameterDescriptor> _p = new List<MetricParameterDescriptor>();

        int ii = ((parameters.Length > 0) && (parameters[0].ParameterType == typeof(MetricEvaluationContext))) ? 1 : 0;
        ii++;
        for (int i = ii; i < parameters.Length; i++)
        {
            if (IsConcreteGenericList(parameters[i].ParameterType))
                _p.Add(new MetricParameterDescriptor()
                {
                    Name = parameters[i].Name,
                    Type = MetricParameterType.Aggregate,
                    ReflectedType = parameters[i].ParameterType
                });
            else
                _p.Add(new MetricParameterDescriptor()
                {
                    Name = parameters[i].Name,
                    Type = MetricParameterType.Scalar,
                    ReflectedType = parameters[i].ParameterType
                });
        }

        if (!autoProp || _p.Count != 0)
        {
            return new MetricDescriptor
            {
                Type = MetricDescriptor.EType.Method,
                Name = metricAttribute.Name ??  methodInfo.Name,
                ValueType = methodInfo.ReturnType,
                Help = (methodInfo.GetCustomAttributes(typeof(MetricHelpAttribute), true)
                    .FirstOrDefault() as MetricHelpAttribute).Description,
                Invoke = (ctx, instance, args) =>
                {
                    object?[] invokeArgs = new object?[args.Length + ii];
                    if (ii == 1)
                        invokeArgs[0] = instance;
                    else
                    {
                        invokeArgs[0] = ctx;
                        invokeArgs[1] = instance;
                    }

                    Array.Copy(args, 0, invokeArgs, ii, args.Length);
                    return methodInfo.Invoke(null, invokeArgs);
                },
                Parameters = _p,
                SourceExpressions = metricAttribute?.SourceExpressions
            };
        }
        else
        {
            return new MetricDescriptor
            {
                Type = MetricDescriptor.EType.Property,
                Name = metricAttribute.Name ??  methodInfo.Name,
                ValueType = methodInfo.ReturnType,
                Help = (methodInfo.GetCustomAttributes(typeof(MetricHelpAttribute), true)
                    .FirstOrDefault() as MetricHelpAttribute).Description,
                Getter = (ctx, instance) =>
                {
                    object?[] invokeArgs = new object?[ii];
                    if (ii == 1)
                        invokeArgs[0] = instance;
                    else
                    {
                        invokeArgs[0] = ctx;
                        invokeArgs[1] = instance;
                    }
                    
                    return methodInfo.Invoke(null, invokeArgs);
                },
                SourceExpressions = metricAttribute?.SourceExpressions
            };
            
        }
    }

    public MetricCatalogs LoadStaticFromMethodBySampleType(MethodInfo methodInfo, bool autoProp = true)
    {
        MetricDescriptor? descriptor = MetricLoadStaticFromMethod(methodInfo, autoProp);
        if (descriptor == null) return this;
        
        ParameterInfo[] parameters = methodInfo.GetParameters();
        int ii = ((parameters.Length > 0) && (parameters[0].ParameterType == typeof(MetricEvaluationContext))) ? 1 : 0;
        Type t = parameters[ii].ParameterType;
        if (!TryGet(t, out var catalog)) throw new ArgumentException($"No catalog found for type {t}");
        
        catalog?.Add(descriptor);
        
        return this;
    }

    public MetricCatalogs MetricLoadStaticFromTypeBySampleType(Type type, bool autoProp = true)
    {
        foreach (var member in type.GetMethods(BindingFlags.Public | BindingFlags.Static | BindingFlags.FlattenHierarchy))
        {
            LoadStaticFromMethodBySampleType(member, autoProp);
        }
        
        return this;
    }

    public static MetricCatalog MetricLoadStaticFromType(Type type, MetricCatalog catalog)
    {
        catalog.Metrics ??= new Dictionary<string, MetricDescriptor>();
        foreach (var member in type.GetMembers(BindingFlags.Public | BindingFlags.Static | BindingFlags.FlattenHierarchy))
        {
            if (member is MethodInfo methodInfo)
            {
                var descriptor = MetricLoadStaticFromMethod(methodInfo);
                if (descriptor != null)
                {
                    catalog.Metrics[descriptor.Name] = descriptor;
                }
            }
        }
        return catalog;
    }
    
    public static MetricCatalog MetricLoadStaticFromType(Type type)
    {
        return MetricLoadStaticFromType(type, new MetricCatalog());
    }
    
    public static MetricCatalog? DefaultReflect(Type arg)
    {
        List<MetricDescriptor> l = new List<MetricDescriptor>();

        foreach (var member in arg.GetMembers(BindingFlags.Public | BindingFlags.Instance))
        {
            MetricAttribute? metricAttribute = member.GetCustomAttributes(typeof(MetricAttribute), true).FirstOrDefault() as MetricAttribute;
            
            if (metricAttribute is not null)
            {
                if (member is FieldInfo fieldInfo)
                    l.Add(new MetricDescriptor
                    {
                        Type = MetricDescriptor.EType.Property,
                        Name = metricAttribute.Name ?? member.Name,
                        ValueType = fieldInfo.FieldType,
                        Help =
                            (member.GetCustomAttributes(typeof(MetricHelpAttribute), true).FirstOrDefault() as
                                MetricHelpAttribute)?.Description ?? member.Name,
                        Getter = (ctx, obj) => ((FieldInfo)member).GetValue(obj)
                    });
                else if (member is PropertyInfo propertyInfo)
                    l.Add(new MetricDescriptor
                    {
                        Type = MetricDescriptor.EType.Property,
                        Name = metricAttribute.Name ?? member.Name,
                        ValueType = propertyInfo.PropertyType,
                        Help =
                            (member.GetCustomAttributes(typeof(MetricHelpAttribute), true).FirstOrDefault() as
                                MetricHelpAttribute)?.Description ?? member.Name,
                        Getter = (ctx, obj) => ((PropertyInfo)member).GetValue(obj)
                    });
                else if (member is MethodInfo methodInfo)
                {
                    ParameterInfo[] parameters = methodInfo.GetParameters();
                    bool wantsContext = parameters.Length > 0 && parameters[0].ParameterType == typeof(MetricEvaluationContext);
                    
                    List<MetricParameterDescriptor> _p = new List<MetricParameterDescriptor>();
                    
                    for (int i = wantsContext ? 1 : 0; i < parameters.Length; i++)
                    {
                        if (IsConcreteGenericList(parameters[i].ParameterType))
                            _p.Add(new MetricParameterDescriptor()
                            {
                                Name = parameters[i].Name,
                                Type = MetricParameterType.Aggregate,
                                ReflectedType = parameters[i].ParameterType
                            });
                        else
                            _p.Add(new MetricParameterDescriptor()
                            {
                                Name = parameters[i].Name,
                                Type = MetricParameterType.Scalar,
                                ReflectedType = parameters[i].ParameterType
                            });
                    }

                    if (!wantsContext)
                    {
                        l.Add(new MetricDescriptor
                        {
                            Type = MetricDescriptor.EType.Method,
                            Name = metricAttribute.Name ?? member.Name,
                            ValueType = methodInfo.ReturnType,
                            Help = (member.GetCustomAttributes(typeof(MetricHelpAttribute), true)
                                .FirstOrDefault() as MetricHelpAttribute)?.Description ?? member.Name,
                            Invoke = (ctx, instance, args) =>
                                ((MethodInfo)member).Invoke(instance, args),
                            Parameters = _p,
                            SourceExpressions = metricAttribute?.SourceExpressions,
                        });
                    }
                    else //wantsContext
                    {
                        l.Add(new MetricDescriptor
                        {
                            Type = MetricDescriptor.EType.Method,
                            Name = metricAttribute.Name ?? member.Name,
                            ValueType = methodInfo.ReturnType,
                            Help = (member.GetCustomAttributes(typeof(MetricHelpAttribute), true)
                                .FirstOrDefault() as MetricHelpAttribute)?.Description ?? member.Name,
                            Invoke = (ctx, instance, args) =>
                            {
                                object?[] invokeArgs = new object?[args.Length + 1];
                                invokeArgs[0] = ctx;
                                Array.Copy(args, 0, invokeArgs, 1, args.Length);
                                return ((MethodInfo)member).Invoke(instance, invokeArgs);
                            },
                            
                            Parameters = _p,
                            SourceExpressions = metricAttribute?.SourceExpressions
                        });
                    }

                }
            }
        }
        
        foreach (var member in arg.GetMembers(BindingFlags.Public | BindingFlags.Static | BindingFlags.FlattenHierarchy))
        {
            MetricAttribute? metricAttribute = member.GetCustomAttributes(typeof(MetricAttribute), true).FirstOrDefault() as MetricAttribute;
            
            if (metricAttribute is not null)
            {
                if (member is MethodInfo methodInfo)
                {
                    var m = MetricLoadStaticFromMethod(methodInfo);
                    if (m == null) throw new NullReferenceException($"Failed to load metric from method {methodInfo.Name}");
                    l.Add(m);
                }
            }
        }

        if (l.Count == 0) return null;
        MetricCatalog R = new MetricCatalog();
        R.Metrics = l.ToDictionary(x => x.Name, x => x);
        return R;
    }

    
}