/*
 * Kegmon
 * Copyright (c) 2024-2026 Magnus
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

export var configData = {
  // Device configuration
  id: "7376ef",
  mdns: "tap2",
  temp_unit: "C",
  weight_unit: "kg",
  volume_unit: "cl",
  dark_mode: false, 
  display_layout: 0,
  // Hardware
  ota_url: "",
  brewlogger_url: "http://192.168.0.24",
  // Wifi
  wifi_portal_timeout: 120,
  wifi_connect_timeout: 20,
  wifi_ssid: "network A",
  wifi_ssid2: "",
  wifi_pass: "password",
  wifi_pass2: "mypass",
  // Push - Generic
  push_timeout: 10,  
  brewfather_apikey: "1",
  brewfather_userkey: "2",
  brewspy_tokens: ["2", "3", "4", "1"],
  barhelper_apikey: 'sdfsdfds',
  barhelper_monitors: ["Monitor 1", "Monitor 2", "Monitor 3", "Monitor 4"],
  // Push - Http Post 1
  http_post_target: "http://192.168.1.10:9090/api/v1/ZYfjlUNeiuyu9N/telemetry",
  http_post_header1: "Auth: Basic T7IF9DD9fF3RDddE=",
  http_post_header2: "Auth: Advanced T7IF9DD9fF3RDddE=",
  // Push - Http Post 2
  http_post2_target: "http://192.168.1.10/ispindel",
  http_post2_header1: "Header: Second",
  http_post2_header2: "Header: First",
  // Push - Http Get
  http_get_target: "http://192.168.1.10/ispindel",
  http_get_header1: "Header: Fourth",
  http_get_header2: "Header: Fifth",
  // Push - Influx
  influxdb2_target: "http://192.168.1.10:8086",
  influxdb2_org: "hello",
  influxdb2_bucket: "spann",
  influxdb2_token: "OijkU((jhfkh",
  // Push - MQTT
  mqtt_target: "192.168.1.20",
  mqtt_port: 1883,
  mqtt_user: "kegmon",
  mqtt_pass: "testpass",
  // Arrays
  scales: [
    {
      scale_factor: -21648.68945,
      scale_offset: 26689,
      keg_weight: 5.2,
      keg_volume: 19,
      glass_volume: 0.4,
      temp_sensor_id: ""
    },
    {
      scale_factor: -23080.98438,
      scale_offset: -108721,
      keg_weight: 5.1,
      keg_volume: 19,
      glass_volume: 0.4,
      temp_sensor_id: ""
    },
    {
      scale_factor: -21648.68945,
      scale_offset: 26689,
      keg_weight: 5.05,
      keg_volume: 19,
      glass_volume: 0.4,
      temp_sensor_id: ""
    },
    {
      scale_factor: -21648.68945,
      scale_offset: 26689,
      keg_weight: 5.2,
      keg_volume: 19,
      glass_volume: 0.4,
      temp_sensor_id: ""
    }
  ],
  beers: [
    {
      beer_name: "West Coast IPA",
      beer_id: "2",
      beer_abv: 7,
      beer_fg: 1.01,
      beer_ebc: 5,
      beer_ibu: 26
    },
    {
      beer_name: "Helles Festbier",
      beer_id: "3",
      beer_abv: 5.5,
      beer_fg: 1.01,
      beer_ebc: 5,
      beer_ibu: 28
    },
    {
      beer_name: "IPA",
      beer_id: "2",
      beer_abv: 7,
      beer_fg: 1.01,
      beer_ebc: 5,
      beer_ibu: 26
    },
    {
      beer_name: "APA",
      beer_id: "2",
      beer_abv: 7,
      beer_fg: 1.01,
      beer_ebc: 5,
      beer_ibu: 26
    }
  ]
}

export var statusData = {
  // Device
  mdns: "Kegmone9d5a0",
  id: "e9d5a0",
  wifi_ssid: "@home",
  weight_unit: "kg",
  volume_unit: "cl",
  temp_unit: "C",
  rssi: -47,
  total_heap: 338292,
  free_heap: 202912,
  ip: "192.168.1.155",
  wifi_setup: false,
  uptime_seconds: 28,
  uptime_minutes: 29,
  uptime_hours: 8,
  uptime_days: 0,
  scale_busy: false,
  sd_mounted: true,
  scales: [
    {
      index: 0,
      connected: true,
      sampling_rate: 10,
      state: "KegAbsent",
      stable_weight: 4.56,
      stable_volume: 44,
      pouring_volume: 0,
      last_pour_volume: 0.4,
      keg_volume: 1900,
      glass: 0.4
    },
    {
      index: 1,
      connected: true,
      sampling_rate: 80,
      state: "KegAbsent",
      stable_weight: 14.23,
      stable_volume: 56,
      pouring_volume: 0,
      last_pour_volume: 0.5,
      keg_volume: 1900,
      glass: 0.4
    },
    {
      index: 2,
      connected: true,
      sampling_rate: 80,
      state: "KegAbsent",
      stable_weight: 10.2,
      stable_volume: 20,
      pouring_volume: 0,
      last_pour_volume: 0.3,
      keg_volume: 1900,
      glass: 0.4
    },
    {
      index: 3,
      connected: true,
      sampling_rate: 80,
      state: "KegAbsent",
      stable_weight: 6.9,
      stable_volume: 25,
      pouring_volume: 0,
      last_pour_volume: 0.45,
      keg_volume: 1900,
      glass: 0.4
    }
  ],
  sensors: [
    {
      index: 0,
      id: "28d9cd48f6d63ccd",
      temperature: 22.55
    }
  ],
  ha: {
    push_age: 30568282,
    push_status: false,
    push_code: 0,
    push_response: "",
    push_used: false
  },
  brewspy: {
    push_age: 30568282,
    push_status: false,
    push_code: 0,
    push_response: "",
    push_used: false
  },
  brewlogger: {
    push_age: 30568282,
    push_status: false,
    push_code: 0,
    push_response: "",
    push_used: false
  },
  barhelper: {
    push_age: 30568282,
    push_status: false,
    push_code: 0,
    push_response: "",
    push_used: false
  },
  recent_events: [
    // Scale 0 events
    {
      type: 3,
      unit: 0,
      timestamp_ms: Date.now() - 60000,
      name: "pour_completed",
      data: {
        pre_weight_kg: "18.5000",
        post_weight_kg: "17.8000",
        weight_kg: "0.7000",
        volume_l: "0.2000",
        duration_ms: 556,
        avg_slope_kg_sec: "-0.001258"
      }
    },
    {
      type: 1,
      unit: 0,
      timestamp_ms: Date.now() - 300000,
      name: "stable_level",
      data: {
        stable_weight_kg: "18.5000",
        stable_volume_l: "5.2000",
        duration_ms: 4500
      }
    },
    // Scale 1 events
    {
      type: 2,
      unit: 1,
      timestamp_ms: Date.now() - 180000,
      name: "pouring",
      data: {
        pre_weight_kg: "22.1000"
      }
    },
    {
      type: 1,
      unit: 1,
      timestamp_ms: Date.now() - 600000,
      name: "stable_level",
      data: {
        stable_weight_kg: "22.1000",
        stable_volume_l: "5.5000",
        duration_ms: 3200
      }
    },
    // Scale 2 events
    {
      type: 3,
      unit: 2,
      timestamp_ms: Date.now() - 120000,
      name: "pour_completed",
      data: {
        pre_weight_kg: "12.3000",
        post_weight_kg: "12.0000",
        weight_kg: "0.3000",
        volume_l: "0.1500",
        duration_ms: 425,
        avg_slope_kg_sec: "-0.000706"
      }
    },
    {
      type: 3,
      unit: 2,
      timestamp_ms: Date.now() - 480000,
      name: "pour_completed",
      data: {
        pre_weight_kg: "12.6000",
        post_weight_kg: "12.3000",
        weight_kg: "0.3000",
        volume_l: "0.1500",
        duration_ms: 440,
        avg_slope_kg_sec: "-0.000682"
      }
    }
  ]
}

export var statisticsData = {
  level_statistics: [
    {
      index: 0,
      connected: true,
      state: "KegPresent",
      confidence: 98,
      total_pours: 45,
      total_pour_volume: 2250,
      avg_pour_volume: 50,
      max_pour_volume: 125,
      min_pour_volume: 15,
      avg_pour_duration_sec: 35.5,
      keg_replacements: 1,
      current_keg_age_hours: 72,
      last_keg_weight: 18.5,
      state_transitions: 3,
      stabilization_count: 156,
      avg_stabilization_time_ms: 2840,
      last_pour_time_ms: 1704979200000,
      last_stable_time_ms: 1704979235000,
      last_keg_removal_time_ms: 1704893400000
    },
    {
      index: 1,
      connected: true,
      state: "KegAbsent",
      confidence: 100,
      total_pours: 0,
      total_pour_volume: 0,
      avg_pour_volume: 0,
      max_pour_volume: 0,
      min_pour_volume: 1000,
      avg_pour_duration_sec: 0,
      keg_replacements: 0,
      current_keg_age_hours: 0,
      last_keg_weight: 0,
      state_transitions: 1,
      stabilization_count: 0,
      avg_stabilization_time_ms: 0,
      last_pour_time_ms: 0,
      last_stable_time_ms: 0,
      last_keg_removal_time_ms: 0
    },
    {
      index: 2,
      connected: true,
      state: "KegPresent",
      confidence: 95,
      total_pours: 28,
      total_pour_volume: 1400,
      avg_pour_volume: 50,
      max_pour_volume: 110,
      min_pour_volume: 20,
      avg_pour_duration_sec: 38.2,
      keg_replacements: 0,
      current_keg_age_hours: 168,
      last_keg_weight: 12.3,
      state_transitions: 2,
      stabilization_count: 98,
      avg_stabilization_time_ms: 3200,
      last_pour_time_ms: 1704975600000,
      last_stable_time_ms: 1704975640000,
      last_keg_removal_time_ms: 0
    },
    {
      index: 3,
      connected: false,
      state: "KegAbsent",
      confidence: 100,
      total_pours: 0,
      total_pour_volume: 0,
      avg_pour_volume: 0,
      max_pour_volume: 0,
      min_pour_volume: 1000,
      avg_pour_duration_sec: 0,
      keg_replacements: 0,
      current_keg_age_hours: 0,
      last_keg_weight: 0,
      state_transitions: 0,
      stabilization_count: 0,
      avg_stabilization_time_ms: 0,
      last_pour_time_ms: 0,
      last_stable_time_ms: 0,
      last_keg_removal_time_ms: 0
    }
  ],
  scale_statistics: [
    {
      index: 0,
      total_readings: 15680,
      valid_readings: 15450,
      invalid_readings: 230,
      reading_quality: 98.5,
      reading_frequency: 4.25,
      last_reading_time_ms: 1704979230000,
      first_reading_time_ms: 1704800400000,
      raw_min: 245680,
      raw_max: 268920,
      raw_average: 257300,
      current_variance: 145.32,
      stable_state_variance: 112.45,
      stable_state_samples: 8450,
      calibration_drift_per_hour: 0.082
    },
    {
      index: 1,
      total_readings: 0,
      valid_readings: 0,
      invalid_readings: 0,
      reading_quality: 0.0,
      reading_frequency: 0.0,
      last_reading_time_ms: 0,
      first_reading_time_ms: 0,
      raw_min: 0,
      raw_max: 0,
      raw_average: 0,
      current_variance: 0.0,
      stable_state_variance: 0.0,
      stable_state_samples: 0,
      calibration_drift_per_hour: 0.0
    },
    {
      index: 2,
      total_readings: 9240,
      valid_readings: 9180,
      invalid_readings: 60,
      reading_quality: 99.3,
      reading_frequency: 4.18,
      last_reading_time_ms: 1704975620000,
      first_reading_time_ms: 1704800400000,
      raw_min: 198450,
      raw_max: 215680,
      raw_average: 207100,
      current_variance: 98.76,
      stable_state_variance: 87.54,
      stable_state_samples: 5230,
      calibration_drift_per_hour: 0.056
    },
    {
      index: 3,
      total_readings: 0,
      valid_readings: 0,
      invalid_readings: 0,
      reading_quality: 0.0,
      reading_frequency: 0.0,
      last_reading_time_ms: 0,
      first_reading_time_ms: 0,
      raw_min: 0,
      raw_max: 0,
      raw_average: 0,
      current_variance: 0.0,
      stable_state_variance: 0.0,
      stable_state_samples: 0,
      calibration_drift_per_hour: 0.0
    }
  ]
}

export var featureData = {
  board: 'esp32_pro',
  platform: 'esp32s3',
  app_ver: '2.0.0',
  app_build: 'gitrev',
  firmware_file: 'firmware.bin',

  // Feature flags
  ble: true,
  no_scales: 4
}

export var scaleData = {
  weight_unit: "kg",
  volume_unit: "cl",
  temp_unit: "C",
  scale_busy: false,
  scales: [
    {
      index: 0,
      connected: true,
      scale_factor: 2048.5,
      scale_offset: -12345,
      state: "stable",
      stable_weight: 18.5,
      scale_raw: 38567890
    },
    {
      index: 1,
      connected: true,
      scale_factor: 2051.3,
      scale_offset: -12100,
      state: "stable",
      stable_weight: 22.1,
      scale_raw: 45234567
    },
    {
      index: 2,
      connected: false,
      scale_factor: 2048.0,
      scale_offset: 0
    },
    {
      index: 3,
      connected: false,
      scale_factor: 2048.0,
      scale_offset: 0
    }
  ]
}

// EOF