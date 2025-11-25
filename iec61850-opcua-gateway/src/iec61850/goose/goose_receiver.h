#pragma once

#include <functional>
#include <libiec61850/goose_receiver.h>
#include <libiec61850/goose_subscriber.h>
#include <memory>
#include <string>

namespace gateway {
namespace iec61850 {
namespace goose {

class GOOSEReceiver {
public:
  GOOSEReceiver(const std::string &interfaceName);
  ~GOOSEReceiver();

  void start();
  void stop();
  bool isRunning() const;

  void addSubscriber(GooseSubscriber subscriber);
  void removeSubscriber(GooseSubscriber subscriber);

private:
  std::string interfaceName_;
  GooseReceiver receiver_{nullptr};
  bool running_{false};
};

} // namespace goose
} // namespace iec61850
} // namespace gateway
