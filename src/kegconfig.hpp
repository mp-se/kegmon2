/*
 * KegMon
 * Copyright (c) 2022-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef SRC_KEGCONFIG_HPP_
#define SRC_KEGCONFIG_HPP_

#include <baseconfig.hpp>
#include <main.hpp>

constexpr auto PARAM_BREWLOGGER_URL = "brewlogger_url";
constexpr auto PARAM_BREWFATHER_USERKEY = "brewfather_userkey";
constexpr auto PARAM_BREWFATHER_APIKEY = "brewfather_apikey";
constexpr auto PARAM_BARHELPER_APIKEY = "barhelper_apikey";
constexpr auto PARAM_DISPLAY_LAYOUT = "display_layout";
// UNUSED: Currently not persisted in config
// constexpr auto PARAM_TEMP_SENSOR = "temp_sensor";
constexpr auto PARAM_WEIGHT_UNIT = "weight_unit";
constexpr auto PARAM_VOLUME_UNIT = "volume_unit";

// JSON array field names for per-scale configuration
constexpr auto PARAM_BREWSPY_TOKENS = "brewspy_tokens";
constexpr auto PARAM_BARHELPER_MONITORS = "barhelper_monitors";
constexpr auto PARAM_SCALES = "scales";
constexpr auto PARAM_BEERS = "beers";

// Nested object keys (used within the above arrays)
constexpr auto PARAM_BREWSPY_TOKEN = "brewspy_token";
constexpr auto PARAM_BARHELPER_MONITOR = "barhelper_monitor";
constexpr auto PARAM_SCALE_FACTOR = "scale_factor";
constexpr auto PARAM_SCALE_OFFSET = "scale_offset";

// Scale calibration constants
constexpr float FACTOR_UNCALIBRATED =
    0.0f;  // Sentinel value: scale needs calibration

constexpr auto PARAM_KEG_WEIGHT = "keg_weight";
constexpr auto PARAM_KEG_VOLUME = "keg_volume";
constexpr auto PARAM_GLASS_VOLUME = "glass_volume";
constexpr auto PARAM_TEMP_SENSOR_ID = "temp_sensor_id";
constexpr auto PARAM_BEER_NAME = "beer_name";
constexpr auto PARAM_BEER_ID = "beer_id";
constexpr auto PARAM_BEER_ABV = "beer_abv";
constexpr auto PARAM_BEER_FG = "beer_fg";
constexpr auto PARAM_BEER_EBC = "beer_ebc";
constexpr auto PARAM_BEER_IBU = "beer_ibu";

struct BeerInfo {
  String _name = "";
  float _abv = 0.0;
  int _ebc = 0;
  int _ibu = 0;
  float _fg = 1;
  String _id = "";
};

struct KegInfo {
  float scaleFactor =
      FACTOR_UNCALIBRATED;  // Must call findFactor() to calibrate
  int32_t scaleOffset = 0;
  float kegWeight = 4.0f;     // Weight in kg
  float kegVolume = 19.0f;    // Volume in liters
  float glassVolume = 0.40f;  // Volume in liters
  String tempSensorId = "";   // OneWire sensor ID for this keg
};

constexpr auto WEIGHT_KG = "kg";
constexpr auto WEIGHT_LBS = "lbs";

constexpr auto VOLUME_CL = "cl";
constexpr auto VOLUME_US = "us-oz";
constexpr auto VOLUME_UK = "uk-oz";

enum DisplayLayoutType { Default = 0 };
enum TempSensorType { SensorDS18B20 = 0 };

class KegConfig : public BaseConfig {
 private:
  String _weightUnit = WEIGHT_KG;
  String _volumeUnit = VOLUME_CL;

  String _brewfatherUserKey = "";
  String _brewfatherApiKey = "";

  String _brewspyToken[MAX_SCALES] = {"", "", "", ""};

  String _barhelperApiKey = "";
  String _barhelperMonitor[MAX_SCALES] = {"Kegmon TAP 1", "Kegmon TAP 2",
                                          "Kegmon TAP 3", "Kegmon TAP 4"};

  String _brewLoggerUrl = "";

  DisplayLayoutType _displayLayout = DisplayLayoutType::Default;
  TempSensorType _tempSensor = TempSensorType::SensorDS18B20;

  KegInfo _kegs[MAX_SCALES];
  BeerInfo _beer[MAX_SCALES];

  int _scaleReadCount = 7;  // Default in HX711 library, should be odd number
  int _scaleReadCountCalibration = 29;

  // Change detection parameters
  int _stabilityDetectionFilterIndex = 12;  // FILTER_KALMAN
  int _pourDetectionFilterIndex = 3;        // FILTER_WEIGHTED_MA (better balance for slow pours)
  uint32_t _levelStabilizationDurationSeconds =
      6;                                     // Increased from 3 to 6 seconds for filter convergence
  uint32_t _pourDurationSeconds = 60;         // Increased from 2 to 60 seconds (prevents early locking on slow flow or noise)
  uint32_t _kegAbsenceDurationSeconds = 30;  // Reduced from 60
  uint32_t _kegReplacementDurationSeconds = 30;  // Reduced from 60
  float _weightAbsentThreshold = 0.1f;           // kg
  float _levelIncreaseThreshold = 0.4f;  // kg (keg replacement, ignore drift)
  float _levelDecreaseThreshold = 0.1f;  // kg (pour detection, 100-150g pours)
  float _pourSlopeThreshold =
      -0.001f;  // kg/sec minimum downward slope (for slow pours) - lowered from -0.002 to catch gradual weight loss
  float _minPourSlope = -0.15f;  // kg/sec maximum flow rate (fastest realistic pour)
  float _maxPourSlope = -0.008f;  // kg/sec minimum flow rate (slowest realistic pour)

 public:
  KegConfig(String baseMDNS, String fileName);

  void createJson(JsonObject& doc) const;
  void parseJson(JsonObject& doc);

  const char* getBrewfatherUserKey() const {
    return _brewfatherUserKey.c_str();
  }
  void setBrewfatherUserKey(String s) {
    _brewfatherUserKey = s;
    _saveNeeded = true;
  }
  const char* getBrewfatherApiKey() const { return _brewfatherApiKey.c_str(); }
  void setBrewfatherApiKey(String s) {
    _brewfatherApiKey = s;
    _saveNeeded = true;
  }

  const char* getBarhelperApiKey() const { return _barhelperApiKey.c_str(); }
  void setBarhelperApiKey(String s) {
    _barhelperApiKey = s;
    _saveNeeded = true;
  }
  const char* getBarhelperMonitor(UnitIndex idx) const {
    return _barhelperMonitor[idx].c_str();
  }
  void setBarhelperMonitor(UnitIndex idx, String s) {
    _barhelperMonitor[idx] = s;
    _saveNeeded = true;
  }

  const char* getBrewLoggerUrl() const { return _brewLoggerUrl.c_str(); }
  void setBrewLoggerUrl(String s) {
    _brewLoggerUrl = s;
    _saveNeeded = true;
  }

  const char* getBrewspyToken(UnitIndex idx) const {
    return _brewspyToken[idx].c_str();
  }
  void setBrewspyToken(UnitIndex idx, String s) {
    _brewspyToken[idx] = s;
    _saveNeeded = true;
  }

  const char* getBeerName(UnitIndex idx) const {
    return _beer[idx]._name.c_str();
  }
  void setBeerName(UnitIndex idx, String s) {
    _beer[idx]._name = s;
    _saveNeeded = true;
  }
  const char* getBeerId(UnitIndex idx) const { return _beer[idx]._id.c_str(); }
  void setBeerId(UnitIndex idx, String s) {
    _beer[idx]._id = s;
    _saveNeeded = true;
  }
  float getBeerABV(UnitIndex idx) const { return _beer[idx]._abv; }
  void setBeerABV(UnitIndex idx, float f) {
    _beer[idx]._abv = f;
    _saveNeeded = true;
  }
  float getBeerFG(UnitIndex idx) const { return _beer[idx]._fg; }
  void setBeerFG(UnitIndex idx, float f) {
    _beer[idx]._fg = f;
    _saveNeeded = true;
  }
  int getBeerEBC(UnitIndex idx) const { return _beer[idx]._ebc; }
  void setBeerEBC(UnitIndex idx, int i) {
    _beer[idx]._ebc = i;
    _saveNeeded = true;
  }
  int getBeerIBU(UnitIndex idx) const { return _beer[idx]._ibu; }
  void setBeerIBU(UnitIndex idx, int i) {
    _beer[idx]._ibu = i;
    _saveNeeded = true;
  }

  float getKegWeight(UnitIndex idx) const { return _kegs[idx].kegWeight; }
  void setKegWeight(UnitIndex idx, float f) {
    if (!isnan(f)) {  // Ignore NAN input, keep previous value
      _kegs[idx].kegWeight = f;
      _saveNeeded = true;
    }
  }

  float getKegVolume(UnitIndex idx) const { return _kegs[idx].kegVolume; }
  void setKegVolume(UnitIndex idx, float f) {
    if (!isnan(f)) {  // Ignore NAN input, keep previous value
      _kegs[idx].kegVolume = f;
      _saveNeeded = true;
    }
  }

  float getGlassVolume(UnitIndex idx) const { return _kegs[idx].glassVolume; }
  void setGlassVolume(UnitIndex idx, float f) {
    if (!isnan(f)) {  // Ignore NAN input, keep previous value
      _kegs[idx].glassVolume = f;
      _saveNeeded = true;
    }
  }

  const char* getTempSensorId(UnitIndex idx) const {
    return _kegs[idx].tempSensorId.c_str();
  }
  void setTempSensorId(UnitIndex idx, String s) {
    _kegs[idx].tempSensorId = s;
    _saveNeeded = true;
  }

  const char* getWeightUnit() const { return _weightUnit.c_str(); }
  bool isWeightUnitKG() const { return _weightUnit.equals(WEIGHT_KG); }
  bool isWeightUnitLBS() const { return _weightUnit.equals(WEIGHT_LBS); }
  void setWeightUnit(String s) {
    if (!s.compareTo(WEIGHT_KG) || !s.compareTo(WEIGHT_LBS)) {
      _weightUnit = s;
      _saveNeeded = true;
    }
  }

  const char* getVolumeUnit() const { return _volumeUnit.c_str(); }
  bool isVolumeUnitCL() const { return _volumeUnit.equals(VOLUME_CL); }
  bool isVolumeUnitUSOZ() const { return _volumeUnit.equals(VOLUME_US); }
  bool isVolumeUnitUKOZ() const { return _volumeUnit.equals(VOLUME_UK); }
  void setVolumeUnit(String s) {
    if (!s.compareTo(VOLUME_CL) || !s.compareTo(VOLUME_UK) ||
        !s.compareTo(VOLUME_US)) {
      _volumeUnit = s;
      _saveNeeded = true;
    }
  }

  int32_t getScaleOffset(UnitIndex idx) const { return _kegs[idx].scaleOffset; }
  void setScaleOffset(UnitIndex idx, int32_t l) {
    _kegs[idx].scaleOffset = l;
    _saveNeeded = true;
  }

  float getScaleFactor(UnitIndex idx) const { return _kegs[idx].scaleFactor; }
  void setScaleFactor(UnitIndex idx, float f) {
    // Protect against NAN values from corrupted config - default to
    // uncalibrated
    _kegs[idx].scaleFactor = isnan(f) ? FACTOR_UNCALIBRATED : f;
    _saveNeeded = true;
  }

  DisplayLayoutType getDisplayLayoutType() const { return _displayLayout; }
  int getDisplayLayoutTypeAsInt() const { return _displayLayout; }
  void setDisplayLayoutType(DisplayLayoutType d) {
    _displayLayout = d;
    _saveNeeded = true;
  }
  void setDisplayLayoutType(int d) {
    _displayLayout = (DisplayLayoutType)d;
    _saveNeeded = true;
  }

  TempSensorType getTempSensorType() const { return _tempSensor; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // int getTempSensorTypeAsInt() const { return _tempSensor; }
  // void setTempSensorType(TempSensorType t) {
  //   _tempSensor = t;
  //   _saveNeeded = true;
  // }
  // void setTempSensorType(int t) {
  //   _tempSensor = (TempSensorType)t;
  //   _saveNeeded = true;
  // }

  int getScaleReadCount() const { return _scaleReadCount; }
  int getScaleReadCountCalibration() const {
    return _scaleReadCountCalibration;
  }

  int getStabilityDetectionFilterIndex() const {
    return _stabilityDetectionFilterIndex;
  }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setStabilityDetectionFilterIndex(int i) {
  //   _stabilityDetectionFilterIndex = i;
  //   _saveNeeded = true;
  // }

  int getPourDetectionFilterIndex() const { return _pourDetectionFilterIndex; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setPourDetectionFilterIndex(int i) {
  //   _pourDetectionFilterIndex = i;
  //   _saveNeeded = true;
  // }

  float getLevelStabilizationDurationSeconds() const {
    return _levelStabilizationDurationSeconds;
  }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setLevelStabilizationDurationSeconds(uint32_t i) {
  //   _levelStabilizationDurationSeconds = i;
  //   _saveNeeded = true;
  // }

  uint32_t getPourDurationSeconds() const { return _pourDurationSeconds; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setPourDurationSeconds(uint32_t i) {
  //   _pourDurationSeconds = i;
  //   _saveNeeded = true;
  // }

  uint32_t getKegAbsenceDurationSeconds() const {
    return _kegAbsenceDurationSeconds;
  }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setKegAbsenceDurationSeconds(uint32_t i) {
  //   _kegAbsenceDurationSeconds = i;
  //   _saveNeeded = true;
  // }

  uint32_t getKegReplacementDurationSeconds() const {
    return _kegReplacementDurationSeconds;
  }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setKegReplacementDurationSeconds(uint32_t i) {
  //   _kegReplacementDurationSeconds = i;
  //   _saveNeeded = true;
  // }

  float getWeightAbsentThreshold() const { return _weightAbsentThreshold; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setWeightAbsentThreshold(float f) {
  //   _weightAbsentThreshold = f;
  //   _saveNeeded = true;
  // }

  float getLevelIncreaseThreshold() const { return _levelIncreaseThreshold; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setLevelIncreaseThreshold(float f) {
  //   _levelIncreaseThreshold = f;
  //   _saveNeeded = true;
  // }

  float getLevelDecreaseThreshold() const { return _levelDecreaseThreshold; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setLevelDecreaseThreshold(float f) {
  //   _levelDecreaseThreshold = f;
  //   _saveNeeded = true;
  // }

  float getPourSlopeThreshold() const { return _pourSlopeThreshold; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setPourSlopeThreshold(float f) {
  //   _pourSlopeThreshold = f;
  //   _saveNeeded = true;
  // }

  float getMinPourSlope() const { return _minPourSlope; }
  float getMaxPourSlope() const { return _maxPourSlope; }

  float getBeerVolumeToWeight(UnitIndex idx) const {
    // Assume beer density of ~1.0 kg/L (slightly less than water)
    // Returns max weight = keg weight + beer weight
    return _kegs[idx].kegWeight + _kegs[idx].kegVolume;
  }

  float getMinValidWeight(UnitIndex idx) const {
    // Minimum valid weight with 15% tolerance for sensor inaccuracy
    // Allows down to 85% of keg weight (empty) - more lenient for empty
    // detection
    return _kegs[idx].kegWeight * 0.85f;
  }

  float getMaxValidWeight(UnitIndex idx) const {
    // Maximum valid weight with 10% tolerance for sensor inaccuracy
    // Allows up to 110% of keg weight + beer volume (full) - stricter for
    // overfill detection
    return (_kegs[idx].kegWeight + _kegs[idx].kegVolume) * 1.10f;
  }

  // These are helper function to assist with formatting of values
  int getWeightPrecision() const { return 2; }  // 2 decimans for kg
  int getVolumePrecision() const { return 0; }  // no decimals for cl
};

extern KegConfig myConfig;

#endif  // SRC_KEGCONFIG_HPP_

// EOF
