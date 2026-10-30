namespace FactoryPulse.Api.Models;

/// <summary>One sensor measurement taken on a machine.</summary>
public sealed class Reading
{
    public long Id { get; set; }

    public required string MachineId { get; init; }

    public required string Sensor { get; init; }

    public required string Unit { get; init; }

    public double Value { get; init; }

    /// <summary>When the agent took the measurement (UTC).</summary>
    public DateTimeOffset Timestamp { get; init; }

    /// <summary>When the API received it (UTC).</summary>
    public DateTimeOffset ReceivedAt { get; init; }
}
