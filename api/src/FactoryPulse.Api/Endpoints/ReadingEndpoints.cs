using FactoryPulse.Api.Contracts;
using FactoryPulse.Api.Storage;
using Microsoft.AspNetCore.Http.HttpResults;

namespace FactoryPulse.Api.Endpoints;

public static class ReadingEndpoints
{
    public static IEndpointRouteBuilder MapReadingEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/readings").WithTags("Readings");

        group.MapPost("", IngestAsync);

        return app;
    }

    /// <summary>Accepts a batch of readings from an agent.</summary>
    internal static async Task<Results<Ok<IngestResponse>, ValidationProblem>> IngestAsync(
        List<ReadingDto>? batch,
        IReadingStore store,
        TimeProvider clock,
        CancellationToken cancellationToken)
    {
        var errors = ReadingValidator.Validate(batch);
        if (errors.Count > 0)
        {
            return TypedResults.ValidationProblem(errors);
        }

        var receivedAt = clock.GetUtcNow();
        var readings = batch!.Select(dto => dto.ToReading(receivedAt)).ToList();

        await store.AddRangeAsync(readings, cancellationToken);

        return TypedResults.Ok(new IngestResponse(readings.Count));
    }
}
