// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#ifndef ENABLE_METRICS_PREVIEW
#  include <gtest/gtest.h>
#  include "opentelemetry/sdk/metrics/meter.h"
#  include "opentelemetry/sdk/metrics/meter_provider.h"

using namespace opentelemetry::sdk::metrics;

class MockMetricExporter : public MetricExporter
{

public:
  MockMetricExporter() = default;
  opentelemetry::sdk::common::ExportResult Export(
      const opentelemetry::nostd::span<std::unique_ptr<Recordable>> &spans) noexcept override
  {
    return opentelemetry::sdk::common::ExportResult::kSuccess;
  }

  bool ForceFlush(
      std::chrono::microseconds timeout = (std::chrono::microseconds::max)()) noexcept override
  {
    return true;
  }

  bool Shutdown() noexcept override { return true; }
};

class MockMetricReader : public MetricReader
{
public:
  bool Collect() noexcept override { return true; }

  bool Shutdown() noexcept override { return true; }
};

class MockView : public View
{};

TEST(MeterProvider, GetMeter)
{
  std::vector<std::unique_ptr<MetricExporter>> exporters;
  std::vector<std::unique_ptr<MetricReader>> readers;
  std::vector<std::unique_ptr<View>> views;
  MeterProvider mp1(std::move(exporters), std::move(readers), std::move(views));
  auto m1 = mp1.GetMeter("test");
  auto m2 = mp1.GetMeter("test");
  auto m3 = mp1.GetMeter("different", "1.0.0");
  auto m4 = mp1.GetMeter("");
  auto m5 = mp1.GetMeter(opentelemetry::nostd::string_view{});
  auto m6 = mp1.GetMeter("different", "1.0.0", "https://opentelemetry.io/schemas/1.2.0");
  ASSERT_NE(nullptr, m1);
  ASSERT_NE(nullptr, m2);
  ASSERT_NE(nullptr, m3);
  ASSERT_NE(nullptr, m6);

  // Should return the same instance each time.
  ASSERT_EQ(m1, m2);
  ASSERT_NE(m1, m3);
  ASSERT_EQ(m4, m5);
  ASSERT_NE(m3, m6);

  // Should be an sdk::trace::Tracer with the processor attached.
#  ifdef RTTI_ENABLED
  auto sdkMeter1 = dynamic_cast<Meter *>(m1.get());
#  else
  auto sdkMeter1 = static_cast<Meter *>(m1.get());
#  endif
  ASSERT_NE(nullptr, sdkMeter1);

  std::unique_ptr<MetricExporter> exporter{new MockMetricExporter()};
  ASSERT_NO_THROW(mp1.AddMetricExporter(std::move(exporter)));

  std::unique_ptr<MetricReader> reader{new MockMetricReader()};
  ASSERT_NO_THROW(mp1.AddMetricReader(std::move(reader)));

  std::unique_ptr<View> view{new MockView()};
  ASSERT_NO_THROW(mp1.AddView(std::move(view)));
}
#endif
