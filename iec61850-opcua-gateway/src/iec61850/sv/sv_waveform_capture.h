#pragma once

#include <atomic>
#include <libiec61850/sv_subscriber.h>
#include <mutex>
#include <string>
#include <vector>

namespace gateway {
namespace iec61850 {
namespace sv {

struct SVSample {
  uint64_t timestamp;
  double value;
  int quality;
};

struct CaptureConfig {
  std::string svID;
  int durationMs = 1000;         // Capture duration
  int preTriggerMs = 200;        // Pre-trigger buffer
  double triggerThreshold = 0.0; // 0 = manual trigger
};

class SVWaveformCapture {
public:
  SVWaveformCapture();
  ~SVWaveformCapture();

  void start(const CaptureConfig &config);
  void stop();
  void trigger(); // Manual trigger

  bool isCapturing() const;
  bool isTriggered() const;

  // Export captured data to COMTRADE (CFG + DAT)
  bool exportCOMTRADE(const std::string &filename);

  // Callback for libiec61850
  void onSVMessage(SVSubscriber subscriber, void *parameter,
                   SVSubscriber_ASDU asdu);

private:
  CaptureConfig config_;
  std::vector<SVSample> buffer_;
  size_t head_ = 0;
  bool full_ = false;

  std::atomic<bool> capturing_{false};
  std::atomic<bool> triggered_{false};
  uint64_t triggerTimestamp_ = 0;

  mutable std::mutex mutex_;
  SVSubscriber subscriber_{nullptr};
  SVReceiver receiver_{nullptr};
};

} // namespace sv
} // namespace iec61850
} // namespace gateway
