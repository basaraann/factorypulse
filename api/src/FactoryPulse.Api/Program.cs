using FactoryPulse.Api.Endpoints;
using FactoryPulse.Api.Storage;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddSingleton(TimeProvider.System);
builder.Services.AddSingleton<IReadingStore, InMemoryReadingStore>();
builder.Services.AddProblemDetails();

var app = builder.Build();

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

app.Run();

// Lets the integration tests start the app with WebApplicationFactory<Program>.
public partial class Program;
