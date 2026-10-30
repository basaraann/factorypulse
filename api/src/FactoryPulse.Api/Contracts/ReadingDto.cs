using FactoryPulse.Api.Models;

namespace FactoryPulse.Api.Contracts;

/// <summary>
/// A reading as the C++ agent sends it. Every field is nullable so that a missing
/// field is reported as a validation error instead of silently becoming 0 or empty.
/// </summary>
public sealed record ReadingDto(
    string? MachineId,
    string? Sensor,
    string? Unit,
    double? Value,
    DateTimeOffset? Timestamp)
{
    /// <summary>Converts a validated DTO into a <see cref="Reading"/>.</summary>
    public Reading ToReading(DateTimeOffset receivedAt) => new()
    {
        MachineId = MachineId!.Trim(),
        Sensor = Sensor!.Trim(),
        Unit = Unit!.Trim(),
        Value = Value!.Value,
        Timestamp = Timestamp!.Value.ToUniversalTime(),
        ReceivedAt = receivedAt.ToUniversalTime(),
    };
}

public sealed record IngestResponse(int Accepted);
