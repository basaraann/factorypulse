using FactoryPulse.Api.Models;

namespace FactoryPulse.Api.Storage;

/// <summary>Where readings are saved. The endpoints only depend on this interface.</summary>
public interface IReadingStore
{
    Task AddRangeAsync(IReadOnlyCollection<Reading> readings, CancellationToken cancellationToken = default);
}
