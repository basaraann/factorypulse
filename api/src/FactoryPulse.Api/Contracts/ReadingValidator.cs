namespace FactoryPulse.Api.Contracts;

/// <summary>Checks an incoming batch and collects errors in the ProblemDetails format.</summary>
public static class ReadingValidator
{
    /// <summary>Same limit as the agent's offline buffer, so a full buffer fits in one request.</summary>
    public const int MaxBatchSize = 10_000;

    public const int MaxTextLength = 64;

    /// <summary>Stops reporting after this many bad readings to keep the response small.</summary>
    public const int MaxReportedReadings = 20;

    public static Dictionary<string, string[]> Validate(IReadOnlyList<ReadingDto>? batch)
    {
        var errors = new Dictionary<string, string[]>();

        if (batch is null || batch.Count == 0)
        {
            errors["batch"] = ["At least one reading is required."];
            return errors;
        }

        if (batch.Count > MaxBatchSize)
        {
            errors["batch"] = [$"A batch can contain at most {MaxBatchSize} readings."];
            return errors;
        }

        for (var i = 0; i < batch.Count && errors.Count < MaxReportedReadings; i++)
        {
            var problems = Check(batch[i]).ToArray();
            if (problems.Length > 0)
            {
                errors[$"[{i}]"] = problems;
            }
        }

        return errors;
    }

    private static IEnumerable<string> Check(ReadingDto? reading)
    {
        if (reading is null)
        {
            yield return "Reading cannot be null.";
            yield break;
        }

        foreach (var problem in CheckText("machineId", reading.MachineId)) yield return problem;
        foreach (var problem in CheckText("sensor", reading.Sensor)) yield return problem;
        foreach (var problem in CheckText("unit", reading.Unit)) yield return problem;

        if (reading.Value is not { } value)
        {
            yield return "value is required.";
        }
        else if (!double.IsFinite(value))
        {
            yield return "value must be a finite number.";
        }

        if (reading.Timestamp is null)
        {
            yield return "timestamp is required.";
        }
    }

    private static IEnumerable<string> CheckText(string field, string? text)
    {
        if (string.IsNullOrWhiteSpace(text))
        {
            yield return $"{field} is required.";
        }
        else if (text.Trim().Length > MaxTextLength)
        {
            yield return $"{field} can be at most {MaxTextLength} characters.";
        }
    }
}
