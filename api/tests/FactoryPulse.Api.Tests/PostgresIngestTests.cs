using System.Net;
using System.Text;
using FactoryPulse.Api.Data;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.DependencyInjection;
using Testcontainers.PostgreSql;

namespace FactoryPulse.Api.Tests;

/// <summary>Starts one throwaway PostgreSQL container (needs Docker) for all tests in the class.</summary>
public sealed class PostgresFixture : IAsyncLifetime
{
    private readonly PostgreSqlContainer _container = new PostgreSqlBuilder()
        .WithImage("postgres:17-alpine")
        .Build();

    public string ConnectionString => _container.GetConnectionString();

    public Task InitializeAsync() => _container.StartAsync();

    public Task DisposeAsync() => _container.DisposeAsync().AsTask();
}

public sealed class PostgresIngestTests(PostgresFixture postgres) : IClassFixture<PostgresFixture>
{
    private static StringContent Json(string json) => new(json, Encoding.UTF8, "application/json");

    [Fact]
    public async Task SavesTheBatchToPostgres()
    {
        await using var factory = new ApiFactory(postgres.ConnectionString);
        using var client = factory.CreateClient();

        var response = await client.PostAsync("/api/readings", Json("""
            [
              {"machineId":"pg-save","sensor":"temperature","unit":"C","value":66.4,"timestamp":"2026-10-30T09:15:02.250+03:00"},
              {"machineId":"pg-save","sensor":"vibration","unit":"mm/s","value":2.61,"timestamp":"2026-10-30T09:15:02.250+03:00"}
            ]
            """));

        Assert.Equal(HttpStatusCode.OK, response.StatusCode);

        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<FactoryPulseDbContext>();
        var saved = await db.Readings.Where(r => r.MachineId == "pg-save").OrderBy(r => r.Id).ToListAsync();

        Assert.Equal(2, saved.Count);
        Assert.Equal(new[] { "temperature", "vibration" }, saved.Select(r => r.Sensor));
        Assert.Equal(66.4, saved[0].Value);
        // Stored in UTC: 09:15 at +03:00 is 06:15 UTC.
        Assert.Equal(new DateTimeOffset(2026, 10, 30, 6, 15, 2, 250, TimeSpan.Zero), saved[0].Timestamp);
        Assert.Equal(TimeSpan.Zero, saved[0].Timestamp.Offset);
    }

    [Fact]
    public async Task SavesNothingWhenTheBatchIsInvalid()
    {
        await using var factory = new ApiFactory(postgres.ConnectionString);
        using var client = factory.CreateClient();

        var response = await client.PostAsync("/api/readings", Json("""
            [
              {"machineId":"pg-invalid","sensor":"temperature","unit":"C","value":65.0,"timestamp":"2026-10-30T09:15:02Z"},
              {"machineId":"pg-invalid","sensor":"pressure","unit":"bar"}
            ]
            """));

        Assert.Equal(HttpStatusCode.BadRequest, response.StatusCode);

        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<FactoryPulseDbContext>();
        Assert.False(await db.Readings.AnyAsync(r => r.MachineId == "pg-invalid"));
    }
}
