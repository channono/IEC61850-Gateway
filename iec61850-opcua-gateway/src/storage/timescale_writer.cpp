#include "timescale_writer.h"
#include "core/logger.h"
#include <chrono>
#include <sstream>

namespace gateway {
namespace storage {

TimescaleDBWriter::TimescaleDBWriter(const std::string &connectionString)
    : connectionString_(connectionString) {}

TimescaleDBWriter::~TimescaleDBWriter() { stop(); }

void TimescaleDBWriter::start() {
  if (running_)
    return;
  running_ = true;
  worker_ = std::thread(&TimescaleDBWriter::workerLoop, this);
  LOG_INFO("TimescaleDB Writer started. Connection: {}", connectionString_);
}

void TimescaleDBWriter::stop() {
  if (running_) {
    running_ = false;
    if (worker_.joinable()) {
      worker_.join();
    }
    LOG_INFO("TimescaleDB Writer stopped");
  }
}

void TimescaleDBWriter::write(const DataPoint &point) {
  std::lock_guard<std::mutex> lock(mutex_);
  buffer_.push(point);
}

void TimescaleDBWriter::workerLoop() {
  while (running_) {
    std::vector<DataPoint> batch;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      while (!buffer_.empty() && batch.size() < 100) {
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

void TimescaleDBWriter::flush(const std::vector<DataPoint> &points) {
  // TODO: Implement actual PostgreSQL INSERT using libpq or pqxx
  // Example SQL: INSERT INTO history (time, device, measurement, value) VALUES
  // ...

  // LOG_DEBUG("Flushing {} points to TimescaleDB", points.size());

  // Simulation of SQL generation
  // std::stringstream sql;
  // sql << "INSERT INTO history (time, device, measurement, value) VALUES ";
  // for (const auto& p : points) {
  //    sql << "('" << p.timestamp << "', '" << p.tag_device << "', '" <<
  //    p.field_name << "', " << p.value << "),";
  // }
}

} // namespace storage
} // namespace gateway
