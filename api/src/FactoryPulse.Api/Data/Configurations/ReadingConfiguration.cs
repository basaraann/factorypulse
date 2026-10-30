using FactoryPulse.Api.Contracts;
using FactoryPulse.Api.Models;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;

namespace FactoryPulse.Api.Data.Configurations;

internal sealed class ReadingConfiguration : IEntityTypeConfiguration<Reading>
{
    public void Configure(EntityTypeBuilder<Reading> reading)
    {
        reading.ToTable("readings");
        reading.HasKey(r => r.Id);

        reading.Property(r => r.Id).HasColumnName("id");
        reading.Property(r => r.MachineId).HasColumnName("machine_id").HasMaxLength(ReadingValidator.MaxTextLength);
        reading.Property(r => r.Sensor).HasColumnName("sensor").HasMaxLength(ReadingValidator.MaxTextLength);
        reading.Property(r => r.Unit).HasColumnName("unit").HasMaxLength(ReadingValidator.MaxTextLength);
        reading.Property(r => r.Value).HasColumnName("value");
        reading.Property(r => r.Timestamp).HasColumnName("timestamp");
        reading.Property(r => r.ReceivedAt).HasColumnName("received_at");

        // Dashboards and alarm rules look up the latest readings of one sensor on one machine.
        reading.HasIndex(r => new { r.MachineId, r.Sensor, r.Timestamp })
            .HasDatabaseName("ix_readings_machine_sensor_time");
    }
}
