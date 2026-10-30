using FactoryPulse.Api.Data;
using FactoryPulse.Api.Endpoints;
using FactoryPulse.Api.Storage;
using Microsoft.EntityFrameworkCore;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddSingleton(TimeProvider.System);
builder.Services.AddDbContext<FactoryPulseDbContext>((services, options) =>
{
    // Read when the context is created, so tests can override the connection string.
    var connectionString = services.GetRequiredService<IConfiguration>().GetConnectionString("FactoryPulse")
        ?? throw new InvalidOperationException("Connection string 'FactoryPulse' is not configured.");
    options.UseNpgsql(connectionString);
});
builder.Services.AddScoped<IReadingStore, EfReadingStore>();
builder.Services.AddProblemDetails();

var app = builder.Build();

if (app.Configuration.GetValue<bool>("Database:MigrateOnStartup"))
{
    await using var scope = app.Services.CreateAsyncScope();
    await scope.ServiceProvider.GetRequiredService<FactoryPulseDbContext>().Database.MigrateAsync();
}

app.UseExceptionHandler(new ExceptionHandlerOptions
{
    // Malformed request bodies are the client's fault: answer 400, not 500.
    StatusCodeSelector = ex => ex is BadHttpRequestException bad
        ? bad.StatusCode
        : StatusCodes.Status500InternalServerError,
    // ...and don't log them as server errors with a stack trace.
    SuppressDiagnosticsCallback = context => context.Exception is BadHttpRequestException,
});

app.MapGet("/health", () => TypedResults.Ok(new { status = "ok" }));
app.MapReadingEndpoints();

await app.RunAsync();

// Lets the integration tests start the app with WebApplicationFactory<Program>.
public partial class Program;
