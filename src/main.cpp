/*
MIT License

Copyright (c) 2022-2026 Magnus

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
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <changedetection.hpp>
#include <display.hpp>
#include <event_logger.hpp>
#include <kegconfig.hpp>
#include <kegpush.hpp>
#include <kegwebhandler.hpp>
#include <looptimer.hpp>
#include <main.hpp>
#include <ota.hpp>
#include <scale.hpp>
#include <sdcard_mmc.hpp>
#include <sdcard_sd.hpp>
#include <serialws.hpp>
#include <temp_mgr.hpp>
#include <uptime.hpp>
#include <utils.hpp>
#include <weightvolume.hpp>
#include <wificonnection.hpp>
#if defined(ESP32S3)
#include <esp32s3/rom/rtc.h>
#endif
#include <esp_core_dump.h>

#include <cstdio>

void checkCoreDump();
void dumpDataToInfluxDB();

SerialDebug mySerial(115200L);
KegConfig myConfig(CFG_MDNSNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_APPNAME, "password", CFG_MDNSNAME);
OtaUpdate myOta(&myConfig, CFG_APPVER);
KegWebHandler myWebHandler(&myConfig);
KegPushHandler myPush(&myConfig);
Display myDisplay;
TempSensorManager myTemp;
SerialWebSocket mySerialWebSocket;
Scale myScale;
ChangeDetection myChangeDetection;
LoopTimer sdTimer(10 * 1000);
LoopTimer displayTimer(100);
LoopTimer loopTimer(2000);
RunMode runMode = RunMode::normalMode;
#if defined(ENABLE_MMC)
SdCardMMC mySdStorage;
#elif defined(ENABLE_SD)
SdCardSD mySdStorage;
#endif

/**
 * Background task for scale reading and change detection updates
 * Runs on Core 1:
 *  - Reads all scales continuously and updates change detection state machine
 *  - Runs as fast as possible (limited only by HX711 sensor readiness)
 */
void scaleDetectionTask(void *parameter) {
  Log.notice(F("ScaleTask: Started on core %d" CR), xPortGetCoreID());

  // Fire system startup event to mark the beginning of a monitoring session
  myChangeDetection.fireStartupEvent(millis());

  while (true) {
    uint32_t currentTimeMs = millis();

    for (int i = 0; i < MAX_SCALES; i++) {
        UnitIndex idx = static_cast<UnitIndex>(i);
        // Only read and update if the ADC hardware was found during setup
        if (myScale.isConnected(idx)) {
            ScaleReadingResult res = myScale.read(idx);
            myChangeDetection.update(idx, res, currentTimeMs);
        }
    }

    // Process any scheduled tare or calibration operations
    myScale.loop();

    dumpDataToInfluxDB();

    // Yield to other tasks (display, etc.)
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

TaskHandle_t scaleTaskHandle = nullptr;

void setup() {

  delay(2000);  // Wait for stable power

  // see: rtc.h for reset reasons
  Log.notice(F("Main: Reset reason %d." CR), rtc_get_reset_reason(0));
  Log.notice(F("Core dump check %d." CR), esp_core_dump_image_check());

  char cbuf[30];
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  snprintf(&cbuf[0], sizeof(cbuf), "%6x", chipId);
  Log.notice(F("Main: Started setup for %s." CR), &cbuf[0]);
  Log.notice(F("Main: Build options: %s (%s) LOGLEVEL %d " CR), CFG_APPVER,
             CFG_GITREV, LOG_LEVEL);

  myConfig.checkFileSystem();
  myConfig.loadFile();
  myConfig.setWifiScanAP(true);

  myDisplay.setup();
  myDisplay.setFont(FontSize::FONT_12);
  myDisplay.printLineCentered(1, "Kegmon v2");
  myDisplay.printLineCentered(3, "Starting");
  myWifi.init();
  delay(200);
  myScale.setup();

  // If ADC hardware is missing, permanently disable the scale unit in change detection
  for (int i = 0; i < MAX_SCALES; i++) {
    if (!myScale.isConnected(static_cast<UnitIndex>(i))) {
      myChangeDetection.disableUnit(static_cast<UnitIndex>(i));
    }
  }

#if defined(ENABLE_MMC)
  myDisplay.printLineCentered(3, "Mounting SD (SD_MMC) card");
  Log.notice(F("Main: MMC_CLK %d." CR), MMC_CLK);
  Log.notice(F("Main: MMC_CMD %d." CR), MMC_CMD);
  Log.notice(F("Main: MMC_D0 %d." CR), MMC_D0);
  mySdStorage.begin(MMC_CLK, MMC_CMD, MMC_D0);
#endif

#if defined(ENABLE_SD)
  myDisplay.printLineCentered(3, "Mounting SD (SD) card");
#if defined(ENABLE_TFT)
  mySdStorage.begin(SD_CS, myDisplay.getSPI());
#else
  mySdStorage.begin(SD_CS, SPI);
#endif  // ENABLE_TFT
#endif  // ENABLE_SD

  // Initialize event logger
  myEventLogger.init();

  // No stored config, move to portal
  if (!myWifi.hasConfig() || myWifi.isDoubleResetDetected()) {
    Log.notice(
        F("Main: Missing wifi config or double reset detected, entering wifi "
          "setup." CR));
    myDisplay.printLineCentered(3, "Entering WIFI Setup");
    myWifi.enableImprov(true);
    myWifi.startAP();
    runMode = RunMode::wifiSetupMode;
  }

  switch (runMode) {
    case RunMode::normalMode:
      myDisplay.printLineCentered(3, "Connecting to WIFI");
      myWifi.connect();
      myWifi.timeSync();
      break;

    case RunMode::wifiSetupMode:
      break;
  }

  checkCoreDump();

  myWebHandler.setupWebServer();
  mySerialWebSocket.begin(myWebHandler.getWebServer(), &EspSerial);
  mySerial.begin(&mySerialWebSocket);
  myTemp.setup();

  Log.notice(F("Main: Setup completed." CR));

  switch (runMode) {
    case RunMode::normalMode: {
      // Show the connected sensors on the display
      char buf[40];
      snprintf(&buf[0], sizeof(buf), "Temperature sensors: %d",
               myTemp.getSensorCount());
      myDisplay.printLineCentered(3, &buf[0]);

      for (int i = 0; i < MAX_SCALES; i++) {
        snprintf(&buf[0], sizeof(buf), "Scale %d: Connected=%s", i + 1,
                 myScale.isConnected((UnitIndex)i) ? "yes" : "no");
        myDisplay.printLineCentered(4 + i, &buf[0]);
      }

      // Launch background task for scale reading and change detection on Core 1
      Log.notice(F("Main: Launching scale detection task on core 1" CR));
      xTaskCreatePinnedToCore(scaleDetectionTask,    // Task function
                              "ScaleDetectionTask",  // Task name
                              4096,                  // Stack size (4KB)
                              nullptr,               // Task parameter
                              1,  // Priority (0=idle, higher=more important)
                              &scaleTaskHandle,  // Task handle output
                              1);                // Core ID (0=PRO, 1=APP)

      myTemp.read();
      delay(3000);

      // Choose display layout based on number of connected scales
      myDisplay.createUI(myScale.getConnectedScaleCount() > 2 ? 0 : 1);
    } break;

    case RunMode::wifiSetupMode:
      break;
  }

#if defined(INJECT_TEST_EVENT)
  Log.notice(F("Main: Injecting test events." CR));
  delay(2000);
  myChangeDetection.injectTestEvents();
#endif // INJECT_TEST_EVENT
}

void loop() {
  if (!myWifi.isConnected() && runMode == RunMode::normalMode) myWifi.connect();

  myUptime.calculate();
  myWebHandler.loop();
  myWifi.loop();
  mySerialWebSocket.loop();

  // Consume events from change detection and queue for web status publishing
  ChangeDetectionEvent event;
  if (myChangeDetection.getNextEvent(event)) {
  // while (myChangeDetection.getNextEvent(event)) {
    // Log event to SD card
    myEventLogger.logEvent(event);

    // Send the event to the web server for publishing
    myWebHandler.queueEvent(event);

    // Send the event to the TFT display for processing
    myDisplay.setScaleEvent(event.unitIndex, event.type);

    // Process the events if needed
    switch (event.type) {
      case ChangeDetectionEventType::SYSTEM_STARTUP:
        break;

      case ChangeDetectionEventType::STABLE_LEVEL: {
        // Fetch temperature for this scale
        float tempC = myTemp.getLastTempByIdC(myConfig.getTempSensorId(event.unitIndex));
        
        // Calculate glasses from stable volume
        WeightVolumeConverter volumeConverter(event.unitIndex);
        float stableVolume = volumeConverter.weightToVolume(event.stable.stableWeightKg);
        float glasses = stableVolume / myConfig.getGlassVolume(event.unitIndex);
        
        // Push keg information to integrations
        myPush.pushKegInformation(event.unitIndex, stableVolume,
                                  0.0f, glasses, tempC);
        
        // Push event state to Home Assistant
        myPush.getHomeAssist()->sendEventState(event.unitIndex, event.type);
        break;
      }

      case ChangeDetectionEventType::POURING:
        // Push event state to Home Assistant
        myPush.getHomeAssist()->sendEventState(event.unitIndex, event.type);
        break;

      case ChangeDetectionEventType::POUR_COMPLETED: {
        // Fetch temperature for this scale
        float tempC = myTemp.getLastTempByIdC(myConfig.getTempSensorId(event.unitIndex));
        
        // Push pour information to integrations
        myPush.pushPourInformation(event.unitIndex, event.pour.prePourWeightKg,
                                   event.pour.pourVolumeL, tempC);
        
        // Push event state to Home Assistant
        myPush.getHomeAssist()->sendEventState(event.unitIndex, event.type);
        break;
      }

      case ChangeDetectionEventType::KEG_REMOVED:
        // Clear Brewspy tap data
        myPush.getBrewspy()->clearKegInformation(event.unitIndex);
        
        // Push event state to Home Assistant
        myPush.getHomeAssist()->sendEventState(event.unitIndex, event.type);
        break;

      case ChangeDetectionEventType::KEG_REPLACED:
        // Push event state to Home Assistant
        myPush.getHomeAssist()->sendEventState(event.unitIndex, event.type);
        break;

      case ChangeDetectionEventType::INVALID_WEIGHT:
        // Push event state to Home Assistant
        myPush.getHomeAssist()->sendEventState(event.unitIndex, event.type);
        break;

      case ChangeDetectionEventType::LOAD_CELL_ERROR:
        // Push event state to Home Assistant
        myPush.getHomeAssist()->sendEventState(event.unitIndex, event.type);
        break;

      case ChangeDetectionEventType::LOAD_CELL_RECOVERED:
        Log.notice(
            F("Loop: Load cell recovered, reinitializing temperature "
              "sensors." CR));
        myTemp.setup();
        
        // Push event state to Home Assistant
        myPush.getHomeAssist()->sendEventState(event.unitIndex, event.type);
        break;

      case ChangeDetectionEventType::CALIBRATION_NEEDED:
        // Push event state to Home Assistant
        myPush.getHomeAssist()->sendEventState(event.unitIndex, event.type);
        break;

      case ChangeDetectionEventType::SETTLING_STARTED:
      case ChangeDetectionEventType::WEIGHT_CHANGE_DETECTED:
      case ChangeDetectionEventType::KEG_ABSENT_TIMEOUT:
      case ChangeDetectionEventType::SENSOR_RECOVERED:
      case ChangeDetectionEventType::CALIBRATION_COMPLETE:
        // These events are informational; push state to Home Assistant
        myPush.getHomeAssist()->sendEventState(event.unitIndex, event.type);
        break;
    }
  }

  // Handle SD card mount retries
  if (sdTimer.hasExpired()) {
    sdTimer.reset();
#if defined(ENABLE_MMC)
    if (!mySdStorage.hasCard()) {
      Log.notice(F("Loop: SD card not mounted, retry mounting." CR));
      mySdStorage.end();
      mySdStorage.begin(MMC_CLK, MMC_CMD, MMC_D0);
    }
#endif

#if defined(ENABLE_SD)
    if (!mySdStorage.hasCard()) {
      Log.notice(F("Loop: SD card not mounted, retry mounting." CR));
      mySdStorage.end();
#if defined(ENABLE_TFT)
      mySdStorage.begin(SD_CS, myDisplay.getSPI());
#else
      mySdStorage.begin(SD_CS, SPI);
#endif  // ENABLE_TFT
    }
#endif  // ENABLE_SD
  }

  // Update data for the display
  if (displayTimer.hasExpired()) {
    displayTimer.reset();

    char info[100];

#if defined(ENABLE_SD) && defined(ENABLE_LVGL)
    snprintf(info, sizeof(info), "ssid: %s ip: %s rssi: %d %s",
             WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(),
             WiFi.RSSI(), mySdStorage.hasCard() ? LV_SYMBOL_SD_CARD : "");
#else
    snprintf(info, sizeof(info), "ssid: %s ip: %s rssi: %d",
             WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(),
             WiFi.RSSI());
#endif
             myDisplay.setStatus(info);

    // Set format and theme once (global display settings)
    myDisplay.setDisplayUnit(myConfig.getVolumeUnit(),
                             String(myConfig.getTempUnit()).c_str());
    myDisplay.setTheme(myConfig.getDarkMode());

    for (int i = 0; i < MAX_SCALES; i++) {
      UnitIndex idx = static_cast<UnitIndex>(i);

      float t = myTemp.getLastTempByIdC(myConfig.getTempSensorId(idx));
      if (!isnan(t)) t = myConfig.isTempUnitC() ? t : convertCtoF(t);

      float w = myChangeDetection.getStableWeight(idx);
      w = isnan(w) ? 0 : (myConfig.isWeightUnitKG() ? w : convertKGtoLBS(w));

      float v = myChangeDetection.getStableVolume(idx);
      v = isnan(v) ? 0 : (v * 100.0f);  // Convert from liters to centiliters
      v = myConfig.isVolumeUnitCL()
              ? v
              : (myConfig.isVolumeUnitUKOZ() ? convertCLtoUKOZ(v)
                                             : convertCLtoUSOZ(v));

      float p = myChangeDetection.getLastPourVolume(idx);
      p = p * 100.0f;  // Convert from liters to centiliters
      p = myConfig.isVolumeUnitCL()
              ? p
              : (myConfig.isVolumeUnitUKOZ() ? convertCLtoUKOZ(p)
                                             : convertCLtoUSOZ(p));

      myDisplay.setScaleData(i, w, v, p, t, myScale.isConnected(idx));
      float keg_vol = myConfig.getKegVolume(idx) *
                      100.0f;  // Convert from liters to centiliters
      myDisplay.setKegInfo(idx, keg_vol);
      myDisplay.setBeerInfo(idx, myConfig.getBeerName(idx),
                            myConfig.getBeerABV(idx), myConfig.getBeerEBC(idx),
                            myConfig.getBeerIBU(idx));
    }
  }

  // Handle peridoic updadates like pushing data and reading temperature sensors
  if (loopTimer.hasExpired()) {
    loopTimer.reset();

    // Read temperature at interval.
    if (!(loopTimer.getLoopCounter() % 15)) {
      Log.notice(F("LOOP: Reading temperature sensors." CR));
      myTemp.read();
    }
  }
}

void checkCoreDump() {
#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
  esp_core_dump_summary_t *summary = static_cast<esp_core_dump_summary_t *>(
      malloc(sizeof(esp_core_dump_summary_t)));

  if (summary) {
    memset(summary, 0, sizeof(esp_core_dump_summary_t));

    if (esp_core_dump_get_summary(summary) == ESP_OK) {
      Log.notice(F("Exception cause %d." CR), summary->ex_info.exc_cause);
      Log.notice(F("PC 0x%x." CR), summary->exc_pc);

      int depth = (summary->exc_bt_info.depth > 16) ? 16 : summary->exc_bt_info.depth;
      for (int i = 0; i < depth; i++) {
        Log.notice(F("PC(%d) 0x%x." CR), i, summary->exc_bt_info.bt[i]);
      }
    }

    free(summary);
  }
#endif
}

LoopTimer myInfluxTimer(1000);

void dumpDataToInfluxDB() {
  // Send raw measurement data to InfluxDB periodically (every 1 second)
  if (myInfluxTimer.hasExpired() && myConfig.hasTargetInfluxDb2()) {
    myInfluxTimer.reset();

    char buf[200];
    float v;
    String s;
    s.reserve(512);

    snprintf(&buf[0], sizeof(buf), "scale,host=%s,device=%s ",
             myConfig.getMDNS(), myConfig.getID());
    s = &buf[0];

    // Temperature data
    for (int i = 0; i < myTemp.getSensorCount(); i++) {
      v = myTemp.getLastTempC(i);
      if (!isnan(v)) {
        snprintf(&buf[0], sizeof(buf), "tempC%i=%f,", i + 1, v);
        s = s + &buf[0];
      }
    }

    // Scale filter data - all computed values
    for (int i = 0; i < MAX_SCALES; i++) {
      ScaleReadingResult res = myScale.getLastResult(static_cast<UnitIndex>(i));

      if (!isnan(res.raw)) {
        snprintf(&buf[0], sizeof(buf), "raw%i=%f,", i + 1, res.raw);
        s = s + &buf[0];
      }
      if (!isnan(res.moving_average)) {
        snprintf(&buf[0], sizeof(buf), "ma%i=%f,", i + 1, res.moving_average);
        s = s + &buf[0];
      }
      if (!isnan(res.ema)) {
        snprintf(&buf[0], sizeof(buf), "ema%i=%f,", i + 1, res.ema);
        s = s + &buf[0];
      }
      if (!isnan(res.weighted_ma)) {
        snprintf(&buf[0], sizeof(buf), "wma%i=%f,", i + 1, res.weighted_ma);
        s = s + &buf[0];
      }
      if (!isnan(res.median)) {
        snprintf(&buf[0], sizeof(buf), "median%i=%f,", i + 1, res.median);
        s = s + &buf[0];
      }
      if (!isnan(res.zscore)) {
        snprintf(&buf[0], sizeof(buf), "zscore%i=%f,", i + 1, res.zscore);
        s = s + &buf[0];
      }
      if (!isnan(res.hampel)) {
        snprintf(&buf[0], sizeof(buf), "hampel%i=%f,", i + 1, res.hampel);
        s = s + &buf[0];
      }
      if (!isnan(res.complementary)) {
        snprintf(&buf[0], sizeof(buf), "comp%i=%f,", i + 1, res.complementary);
        s = s + &buf[0];
      }
      if (!isnan(res.alphabeta)) {
        snprintf(&buf[0], sizeof(buf), "ab%i=%f,", i + 1, res.alphabeta);
        s = s + &buf[0];
      }
      if (!isnan(res.butterworth)) {
        snprintf(&buf[0], sizeof(buf), "butter%i=%f,", i + 1, res.butterworth);
        s = s + &buf[0];
      }
      if (!isnan(res.chebyshev)) {
        snprintf(&buf[0], sizeof(buf), "cheby%i=%f,", i + 1, res.chebyshev);
        s = s + &buf[0];
      }
      if (!isnan(res.kalman)) {
        snprintf(&buf[0], sizeof(buf), "kalman%i=%f,", i + 1, res.kalman);
        s = s + &buf[0];
      }

      // Change detection state data
      float stableWeight = myChangeDetection.getStableWeight((UnitIndex)i);
      float pourVolume = myChangeDetection.getPouringVolume((UnitIndex)i);
      float stableVolume = myChangeDetection.getStableVolume((UnitIndex)i);
      float lastPourVolume = myChangeDetection.getLastPourVolume((UnitIndex)i);
      ChangeDetectionState state = myChangeDetection.getState((UnitIndex)i);
      float confidence = myChangeDetection.getConfidence((UnitIndex)i);
      uint8_t signalQuality = myChangeDetection.getSignalQuality((UnitIndex)i);
      float avgSlope = myChangeDetection.getAverageSlope((UnitIndex)i);

      snprintf(&buf[0], sizeof(buf), "state%i=\"%s\",", i + 1,
               myChangeDetection.getStateString((UnitIndex)i));
      s = s + &buf[0];
      if (!isnan(stableWeight)) {
        snprintf(&buf[0], sizeof(buf), "stable_wgt%i=%f,", i + 1, stableWeight);
        s = s + &buf[0];
      }
      if (!isnan(stableVolume)) {
        snprintf(&buf[0], sizeof(buf), "stable_vol%i=%f,", i + 1, stableVolume);
        s = s + &buf[0];
      }
      // Only export current pouring volume while actively pouring
      if (state == ChangeDetectionState::Pouring && !isnan(pourVolume)) {
        snprintf(&buf[0], sizeof(buf), "pour%i=%f,", i + 1, pourVolume);
        s = s + &buf[0];
      }
      // Always export last completed pour volume
      if (lastPourVolume > 0.0f) {
        snprintf(&buf[0], sizeof(buf), "last_pour_vol%i=%f,", i + 1,
                 lastPourVolume);
        s = s + &buf[0];
      }
      snprintf(&buf[0], sizeof(buf), "signal_quality%i=%d,", i + 1,
               signalQuality);
      s = s + &buf[0];
      if (!isnan(avgSlope)) {
        snprintf(&buf[0], sizeof(buf), "avg_slope%i=%f,", i + 1, avgSlope);
        s = s + &buf[0];
      }
      snprintf(&buf[0], sizeof(buf), "conf%i=%f,", i + 1, confidence);
      s = s + &buf[0];
    }

    // Remove trailing comma if present
    if (s.endsWith(",")) {
      s = s.substring(0, s.length() - 1);
    }

    // Serial.printf(F("InfluxDB2 Data: %s" CR), s.c_str());

    myPush.sendInfluxDb2(
        s, myConfig.getTargetInfluxDB2(), myConfig.getOrgInfluxDB2(),
        myConfig.getBucketInfluxDB2(), myConfig.getTokenInfluxDB2());
  }
}

// EOF
