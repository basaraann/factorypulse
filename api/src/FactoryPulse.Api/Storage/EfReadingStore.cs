using FactoryPulse.Api.Data;
using FactoryPulse.Api.Models;

namespace FactoryPulse.Api.Storage;

/// <summary>Saves readings to PostgreSQL through EF Core. A batch is saved in one transaction.</summary>
public sealed class EfReadingStore(FactoryPulseDbContext db) : IReadingStore
{
    public async Task AddRangeAsync(IReadOnlyCollection<Reading> readings, CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(readings);

        db.Readings.AddRange(readings);
        await db.SaveChangesAsync(cancellationToken);
    }
}
