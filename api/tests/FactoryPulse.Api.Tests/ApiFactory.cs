using FactoryPulse.Api.Storage;
using FactoryPulse.Api.Tests.Fakes;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.AspNetCore.TestHost;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;

namespace FactoryPulse.Api.Tests;

/// <summary>
/// Starts the API in memory. Without a connection string the readings go to an
/// <see cref="InMemoryReadingStore"/>; with one they go to that PostgreSQL database.
/// </summary>
public sealed class ApiFactory(string? connectionString = null) : WebApplicationFactory<Program>
{
    protected override void ConfigureWebHost(IWebHostBuilder builder)
    {
        builder.ConfigureAppConfiguration(config => config.AddInMemoryCollection(new Dictionary<string, string?>
        {
            ["ConnectionStrings:FactoryPulse"] = connectionString ?? "Host=not-used",
            ["Database:MigrateOnStartup"] = connectionString is null ? "false" : "true",
        }));

        if (connectionString is null)
        {
            builder.ConfigureTestServices(services =>
            {
                services.RemoveAll<IReadingStore>();
                services.AddSingleton<IReadingStore, InMemoryReadingStore>();
            });
        }
    }
}
