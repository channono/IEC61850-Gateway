#include "influx_writer.h"
#include "core/logger.h"
#include <chrono>

namespace gateway {
namespace storage {

InfluxDBWriter::InfluxDBWriter(const std::string &url, const std::string &org,
                               const std::string &bucket,
                               const std::string &token)
    : url_(url), org_(org), bucket_(bucket), token_(token) {}

InfluxDBWriter::~InfluxDBWriter() { stop(); }

void InfluxDBWriter::start() {
  if (running_)
    return;
  running_ = true;
  worker_ = std::thread(&InfluxDBWriter::workerLoop, this);
  LOG_INFO("InfluxDB Writer started. Target: {}/{}", url_, bucket_);
}

void InfluxDBWriter::stop() {
  if (running_) {
    running_ = false;
    if (worker_.joinable()) {
      worker_.join();
    }
    LOG_INFO("InfluxDB Writer stopped");
  }
}

void InfluxDBWriter::write(const DataPoint &point) {
  std::lock_guard<std::mutex> lock(mutex_);
  buffer_.push(point);
}

void InfluxDBWriter::workerLoop() {
  while (running_) {
    std::vector<DataPoint> batch;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      while (!buffer_.empty() && batch.size() < 100) { // Batch size 100
        batch.push_back(buffer_.front());
        buffer_.pop();
      }
    }

    if (!batch.empty()) {
      flush(batch);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void InfluxDBWriter::flush(const std::vector<DataPoint> &points) {
  // TODO: Implement actual HTTP POST to InfluxDB API
  // For Phase 4 prototype, we just log the batch write
  // LOG_DEBUG("Flushing {} points to InfluxDB", points.size());

  // Example Line Protocol:
  // measurement,tag_device=IED1 field_name=123.45 timestamp
}

} // namespace storage
} // namespace gateway
