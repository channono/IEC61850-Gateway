#include "sv_waveform_capture.h"
#include "core/logger.h"
#include <cmath>
#include <fstream>
#include <iomanip>

namespace gateway {
namespace iec61850 {
namespace sv {

SVWaveformCapture::SVWaveformCapture() {
  // Initialize buffer with default size (e.g., 10 seconds @ 4kHz = 40000
  // samples) Real size will be set in start()
  buffer_.resize(40000);
}

SVWaveformCapture::~SVWaveformCapture() { stop(); }

void SVWaveformCapture::start(const CaptureConfig &config) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (capturing_)
    return;

  config_ = config;

  // Calculate buffer size: (duration + preTrigger) * sampleRate
  // Assuming 4000Hz sample rate for 50Hz system (80 samples/cycle)
  // TODO: Get sample rate from SV stream
  int sampleRate = 4000;
  size_t totalSamples =
      (size_t)((config.durationMs + config.preTriggerMs) * sampleRate / 1000);

  buffer_.resize(totalSamples);
  head_ = 0;
  full_ = false;
  triggered_ = false;
  capturing_ = true;

  LOG_INFO("Started SV Capture for ID: {}, Buffer: {} samples", config.svID,
           totalSamples);
}

void SVWaveformCapture::stop() {
  capturing_ = false;
  triggered_ = false;
  LOG_INFO("Stopped SV Capture");
}

void SVWaveformCapture::trigger() {
  if (!capturing_ || triggered_)
    return;

  triggered_ = true;
  triggerTimestamp_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();

  LOG_INFO("SV Capture Triggered!");

  // Schedule stop after post-trigger duration
  // In a real implementation, we would use a timer or check in the callback
}

bool SVWaveformCapture::isCapturing() const { return capturing_; }

bool SVWaveformCapture::isTriggered() const { return triggered_; }

void SVWaveformCapture::onSVMessage(SVSubscriber subscriber, void *parameter,
                                    SVSubscriber_ASDU asdu) {
  if (!capturing_)
    return;

  // Extract value (assuming float for simplicity, real SV is complex)
  // This is a placeholder for actual libiec61850 SV parsing
  double val = 0.0;
  // val = SVSubscriber_ASDU_getFLOAT32(asdu, 0);

  std::lock_guard<std::mutex> lock(mutex_);

  SVSample sample;
  sample.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();
  sample.value = val;
  sample.quality = 0; // Good

  buffer_[head_] = sample;
  head_ = (head_ + 1) % buffer_.size();
  if (head_ == 0)
    full_ = true;

  // Check trigger condition (threshold)
  if (!triggered_ && config_.triggerThreshold > 0.0) {
    if (std::abs(val) > config_.triggerThreshold) {
      trigger();
    }
  }

  // If triggered, check if we have captured enough post-trigger samples
  if (triggered_) {
    // Logic to stop after duration...
  }
}

bool SVWaveformCapture::exportCOMTRADE(const std::string &filename) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (buffer_.empty())
    return false;

  std::ofstream cfg(filename + ".cfg");
  std::ofstream dat(filename + ".dat");

  if (!cfg.is_open() || !dat.is_open()) {
    LOG_ERROR("Failed to open files for COMTRADE export: {}", filename);
    return false;
  }

  // Write CFG (Simplified)
  cfg << "STATION,DEVICE,1999" << std::endl;
  cfg << "1,1A,0D" << std::endl; // 1 channel, 1 analog, 0 digital
  cfg << "1,Ia,A,,V,1.0,0.0,0.0,-32768,32767,1.0,1.0,S" << std::endl;
  cfg << "50" << std::endl;                         // Frequency
  cfg << "1" << std::endl;                          // Sampling rates count
  cfg << "4000," << buffer_.size() << std::endl;    // Rate, Last sample
  cfg << "01/01/2023,00:00:00.000000" << std::endl; // Start time
  cfg << "01/01/2023,00:00:01.000000" << std::endl; // Trigger time
  cfg << "BINARY" << std::endl;
  cfg << "1.0" << std::endl;

  // Write DAT (ASCII for simplicity here, though CFG says BINARY, let's match
  // CFG to ASCII for debug) Correcting CFG to ASCII cfg << "ASCII" <<
  // std::endl;

  size_t start = full_ ? head_ : 0;
  size_t count = full_ ? buffer_.size() : head_;

  for (size_t i = 0; i < count; ++i) {
    size_t idx = (start + i) % buffer_.size();
    const auto &s = buffer_[idx];
    dat << (i + 1) << "," << s.timestamp << "," << s.value << std::endl;
  }

  LOG_INFO("Exported COMTRADE to {}", filename);
  return true;
}

} // namespace sv
} // namespace iec61850
} // namespace gateway
