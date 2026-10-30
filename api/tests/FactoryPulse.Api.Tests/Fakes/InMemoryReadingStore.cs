using FactoryPulse.Api.Models;
using FactoryPulse.Api.Storage;

namespace FactoryPulse.Api.Tests.Fakes;

/// <summary>Thread-safe in-memory store, used to test the endpoints without a database.</summary>
public sealed class InMemoryReadingStore : IReadingStore
{
    private readonly List<Reading> _readings = [];
    private readonly Lock _lock = new();
    private long _nextId = 1;

    public Task AddRangeAsync(IReadOnlyCollection<Reading> readings, CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(readings);

        lock (_lock)
        {
            foreach (var reading in readings)
            {
                reading.Id = _nextId++;
                _readings.Add(reading);
            }
        }

        return Task.CompletedTask;
    }

    /// <summary>Returns a copy of everything stored so far.</summary>
    public IReadOnlyList<Reading> Snapshot()
    {
        lock (_lock)
        {
            return _readings.ToList();
        }
    }
}
