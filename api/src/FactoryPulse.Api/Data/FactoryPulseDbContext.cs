using FactoryPulse.Api.Models;
using Microsoft.EntityFrameworkCore;

namespace FactoryPulse.Api.Data;

public sealed class FactoryPulseDbContext(DbContextOptions<FactoryPulseDbContext> options) : DbContext(options)
{
    public DbSet<Reading> Readings => Set<Reading>();

    protected override void OnModelCreating(ModelBuilder modelBuilder) =>
        modelBuilder.ApplyConfigurationsFromAssembly(typeof(FactoryPulseDbContext).Assembly);
}
