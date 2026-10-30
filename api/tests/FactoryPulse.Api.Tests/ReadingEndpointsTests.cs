using System.Net;
using System.Net.Http.Json;
using System.Text;
using System.Text.Json;
using FactoryPulse.Api.Storage;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.Extensions.DependencyInjection;

namespace FactoryPulse.Api.Tests;

public sealed class ReadingEndpointsTests : IDisposable
{
    // A fresh app per test, so each test starts with an empty store.
    private readonly WebApplicationFactory<Program> _factory = new();
    private readonly HttpClient _client;

    public ReadingEndpointsTests() => _client = _factory.CreateClient();

    public void Dispose()
    {
        _client.Dispose();
        _factory.Dispose();
    }

    private InMemoryReadingStore Store => (InMemoryReadingStore)_factory.Services.GetRequiredService<IReadingStore>();

    private Task<HttpResponseMessage> PostJsonAsync(string json) =>
        _client.PostAsync("/api/readings", new StringContent(json, Encoding.UTF8, "application/json"));

    [Fact]
    public async Task HealthReturnsOk()
    {
        var response = await _client.GetAsync("/health");

        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
    }

    [Fact]
    public async Task StoresABatchInTheFormatTheAgentSends()
    {
        // Exactly what fp_agent's HttpPublisher posts.
        const string json = """
            [
              {"machineId":"press-1","sensor":"temperature","unit":"C","value":66.4,"timestamp":"2026-10-30T09:15:02.250Z"},
              {"machineId":"press-1","sensor":"pressure","unit":"bar","value":5.97,"timestamp":"2026-10-30T09:15:02.250Z"}
            ]
            """;

        var response = await PostJsonAsync(json);

        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        var body = await response.Content.ReadFromJsonAsync<JsonElement>();
        Assert.Equal(2, body.GetProperty("accepted").GetInt32());

        var stored = Store.Snapshot();
        Assert.Equal(2, stored.Count);
        Assert.Equal("press-1", stored[0].MachineId);
        Assert.Equal("temperature", stored[0].Sensor);
        Assert.Equal(66.4, stored[0].Value);
        Assert.Equal(new DateTimeOffset(2026, 10, 30, 9, 15, 2, 250, TimeSpan.Zero), stored[0].Timestamp);
        Assert.Equal(new[] { 1L, 2L }, stored.Select(r => r.Id));
    }

    [Fact]
    public async Task RejectsAnEmptyBatch()
    {
        var response = await PostJsonAsync("[]");

        Assert.Equal(HttpStatusCode.BadRequest, response.StatusCode);
        var errors = await ReadErrorsAsync(response);
        Assert.True(errors.TryGetProperty("batch", out _));
        Assert.Empty(Store.Snapshot());
    }

    [Fact]
    public async Task RejectsTheWholeBatchIfOneReadingIsInvalid()
    {
        const string json = """
            [
              {"machineId":"press-1","sensor":"temperature","unit":"C","value":65.0,"timestamp":"2026-10-30T09:15:02Z"},
              {"machineId":"","sensor":"pressure","unit":"bar"}
            ]
            """;

        var response = await PostJsonAsync(json);

        Assert.Equal(HttpStatusCode.BadRequest, response.StatusCode);
        var errors = await ReadErrorsAsync(response);
        var messages = errors.GetProperty("[1]").EnumerateArray().Select(e => e.GetString()).ToList();
        Assert.Contains("machineId is required.", messages);
        Assert.Contains("value is required.", messages);
        Assert.Contains("timestamp is required.", messages);
        Assert.Empty(Store.Snapshot());
    }

    [Fact]
    public async Task RejectsMalformedJsonWith400()
    {
        var response = await PostJsonAsync("{not json");

        Assert.Equal(HttpStatusCode.BadRequest, response.StatusCode);
    }

    private static async Task<JsonElement> ReadErrorsAsync(HttpResponseMessage response)
    {
        var problem = await response.Content.ReadFromJsonAsync<JsonElement>();
        return problem.GetProperty("errors");
    }
}
