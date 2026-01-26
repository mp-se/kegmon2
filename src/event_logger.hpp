/*
MIT License

Copyright (c) 2026 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#ifndef SRC_EVENT_LOGGER_HPP_
#define SRC_EVENT_LOGGER_HPP_

#include <changedetection.hpp>
#include <cstdint>
#include <sdcard_mmc.hpp>
#include <sdcard_sd.hpp>
#include <weightvolume.hpp>

// CSV log versioning for schema evolution
#define LOG_VERSION 1

// File rotation configuration
#define MAX_LOG_FILE_SIZE (16 * 1024)  // 16KB
#define MAX_LOG_FILES 4
#define LOG_BASE_FILENAME "/data"
#define LOG_FILE_EXTENSION ".csv"

/**
 * SD Card Event Logger
 * Logs all change detection events to CSV format on SD card
 * Each row includes a version number for schema compatibility
 * File rotation: data.csv -> data1.csv -> data2.csv -> data3.csv (drops oldest)
 */
class EventLogger {
 public:
  EventLogger() = default;
  ~EventLogger() = default;

  /**
   * Initialize the logger
   * Sets up baseline timestamp from system startup
   */
  void init();

  /**
   * Log a single event to CSV
   * Formats event with version, timestamp, and all event-specific data
   * Performs file rotation if threshold exceeded
   */
  void logEvent(const ChangeDetectionEvent& event);

 private:
  // Timestamp baseline (milliseconds from SYSTEM_STARTUP event)
  uint64_t _systemStartupMs = 0;

  // Track if we've written the baseline SYSTEM_STARTUP event
  bool _startupLogged = false;

  /**
   * Check if SD card is present and accessible
   */
  bool hasCard() const;

  /**
   * Get full path to current log file
   */
  const char* getCurrentLogPath() const;

  /**
   * Get full path to log file by index (0 = data.csv, 1 = data1.csv, etc.)
   */
  void getLogPathByIndex(int index, char* buffer, size_t bufferSize) const;

  /**
   * Format timestamp from event
   * Uses RTC time with millisecond offset from SYSTEM_STARTUP baseline
   */
  void formatTimestamp(const ChangeDetectionEvent& event, char* buffer,
                       size_t bufferSize);

  /**
   * Get event type as string
   */
  const char* getEventTypeString(ChangeDetectionEventType type) const;

  /**
   * Format event data as CSV row
   * Includes version number as first column
   */
  void formatEventRow(const ChangeDetectionEvent& event, char* buffer,
                      size_t bufferSize);

  /**
   * Check if current log file should be rotated
   */
  bool shouldRotate() const;

  /**
   * Perform file rotation
   * data3.csv deleted, data2->data3, data1->data2, data->data1
   */
  void rotateFiles();

  /**
   * Write row to current log file
   * Creates file if it doesn't exist
   */
  void writeToFile(const char* row);
};

extern EventLogger myEventLogger;

#endif  // SRC_EVENT_LOGGER_HPP_
