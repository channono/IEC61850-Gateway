#pragma once

#include "storage_writer.h"
#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// Forward declaration for libpq or similar if used directly
// For prototype, we'll simulate the connection

namespace gateway {
namespace storage {

class TimescaleDBWriter : public IStorageWriter {
public:
  TimescaleDBWriter(const std::string &connectionString);
  ~TimescaleDBWriter() override;

  void start() override;
  void stop() override;
  void write(const DataPoint &point) override;

private:
  std::string connectionString_;
  std::queue<DataPoint> buffer_;
  std::mutex mutex_;
  std::atomic<bool> running_{false};
  std::thread worker_;

  void workerLoop();
  void flush(const std::vector<DataPoint> &points);
};

} // namespace storage
} // namespace gateway
