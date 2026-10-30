using FactoryPulse.Api.Contracts;

namespace FactoryPulse.Api.Tests;

public class ReadingValidatorTests
{
    private static ReadingDto Valid(string machineId = "press-1") =>
        new(machineId, "temperature", "C", 65.0, DateTimeOffset.UnixEpoch);

    [Fact]
    public void AcceptsAValidBatch()
    {
        var errors = ReadingValidator.Validate([Valid(), Valid("press-2")]);

        Assert.Empty(errors);
    }

    [Fact]
    public void RejectsNullOrEmptyBatch()
    {
        Assert.Contains("batch", ReadingValidator.Validate(null).Keys);
        Assert.Contains("batch", ReadingValidator.Validate([]).Keys);
    }

    [Fact]
    public void RejectsBatchOverTheLimit()
    {
        var batch = Enumerable.Repeat(Valid(), ReadingValidator.MaxBatchSize + 1).ToList();

        var errors = ReadingValidator.Validate(batch);

        Assert.Equal(new[] { "batch" }, errors.Keys);
    }

    [Fact]
    public void ReportsEveryProblemOfAReadingUnderItsIndex()
    {
        var bad = new ReadingDto("  ", "pressure", null, double.NaN, null);

        var errors = ReadingValidator.Validate([Valid(), bad]);

        var problems = Assert.Single(errors).Value;
        Assert.Equal("[1]", errors.Keys.Single());
        Assert.Contains("machineId is required.", problems);
        Assert.Contains("unit is required.", problems);
        Assert.Contains("value must be a finite number.", problems);
        Assert.Contains("timestamp is required.", problems);
    }

    [Fact]
    public void RejectsTooLongText()
    {
        var errors = ReadingValidator.Validate([Valid(new string('x', ReadingValidator.MaxTextLength + 1))]);

        Assert.Contains($"machineId can be at most {ReadingValidator.MaxTextLength} characters.", errors["[0]"]);
    }

    [Fact]
    public void StopsAfterTheReportLimit()
    {
        var batch = Enumerable.Repeat(new ReadingDto(null, null, null, null, null), 100).ToList();

        var errors = ReadingValidator.Validate(batch);

        Assert.Equal(ReadingValidator.MaxReportedReadings, errors.Count);
    }
}
