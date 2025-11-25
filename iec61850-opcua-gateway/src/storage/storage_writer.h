#pragma once

#include <string>
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

class IStorageWriter {
public:
  virtual ~IStorageWriter() = default;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void write(const DataPoint &point) = 0;
};

} // namespace storage
} // namespace gateway
