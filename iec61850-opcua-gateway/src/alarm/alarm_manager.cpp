#include "alarm_manager.h"
#include "core/logger.h"
#include <algorithm>
#include <random>
#include <sstream>

namespace gateway {
namespace alarm {

AlarmManager::AlarmManager() {}

AlarmManager::~AlarmManager() {}

void AlarmManager::raiseAlarm(const std::string &source,
                              const std::string &message,
                              AlarmSeverity severity) {
  std::lock_guard<std::mutex> lock(mutex_);

  // Check if alarm already active for this source
  auto it =
      std::find_if(alarms_.begin(), alarms_.end(), [&source](const Alarm &a) {
        return a.source == source && a.active;
      });

  if (it != alarms_.end()) {
    // Update existing alarm
    it->message = message;
    it->severity = severity;
    it->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();
    LOG_INFO("Updated alarm for {}: {}", source, message);
  } else {
    // Create new alarm
    Alarm alarm;
    alarm.id = generateId();
    alarm.source = source;
    alarm.message = message;
    alarm.severity = severity;
    alarm.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
    alarm.active = true;
    alarm.acknowledged = false;

    alarms_.push_back(alarm);
    LOG_WARN("Raised alarm for {}: {}", source, message);
  }
}

void AlarmManager::clearAlarm(const std::string &source) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it =
      std::find_if(alarms_.begin(), alarms_.end(), [&source](const Alarm &a) {
        return a.source == source && a.active;
      });

  if (it != alarms_.end()) {
    it->active = false;
    LOG_INFO("Cleared alarm for {}", source);
  }
}

void AlarmManager::acknowledgeAlarm(const std::string &alarmId) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it =
      std::find_if(alarms_.begin(), alarms_.end(),
                   [&alarmId](const Alarm &a) { return a.id == alarmId; });

  if (it != alarms_.end()) {
    it->acknowledged = true;
    LOG_INFO("Acknowledged alarm {}", alarmId);
  }
}

std::vector<Alarm> AlarmManager::getActiveAlarms() const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<Alarm> active;
  std::copy_if(alarms_.begin(), alarms_.end(), std::back_inserter(active),
               [](const Alarm &a) { return a.active; });
  return active;
}

std::string AlarmManager::generateId() {
  // Simple UUID-like generation for prototype
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 15);

  std::stringstream ss;
  for (int i = 0; i < 8; i++)
    ss << std::hex << dis(gen);
  return ss.str();
}

} // namespace alarm
} // namespace gateway
