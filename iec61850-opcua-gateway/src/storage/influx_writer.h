#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace gateway {
namespace storage {

struct DataPoint {
  std::string measurement;
  std::string tag_device;
  std::string field_name;
  double value;
  uint64_t timestamp;
};

class InfluxDBWriter {
public:
  InfluxDBWriter(const std::string &url, const std::string &org,
                 const std::string &bucket, const std::string &token);
  ~InfluxDBWriter();

  void start();
  void stop();

  void write(const DataPoint &point);

private:
  std::string url_;
  std::string org_;
  std::string bucket_;
  std::string token_;

  std::queue<DataPoint> buffer_;
  std::mutex mutex_;
  std::atomic<bool> running_{false};
  std::thread worker_;

  void workerLoop();
  void flush(const std::vector<DataPoint> &points);
};

} // namespace storage
} // namespace gateway
