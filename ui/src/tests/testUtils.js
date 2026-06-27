import { createPinia, setActivePinia } from 'pinia'
import { vi } from 'vitest'

/**
 * Creates a fresh Pinia instance for testing
 * @returns {object} pinia instance
 */
export function createTestingPinia() {
  const pinia = createPinia()
  setActivePinia(pinia)
  return pinia
}

/**
 * Resets all mocks to initial state
 */
export function resetAllMocks() {
  vi.clearAllMocks()
}

/**
 * Mock config object for testing - kegmon2-ui specific
 */
export const mockConfigState = {
  // Device
  id: 'test-id',
  mdns: 'kegmon-test',
  temp_unit: 'C',
  weight_unit: 'kg',
  volume_unit: 'L',
  dark_mode: false,
  display_layout: 0,
  // Hardware
  ota_url: '',
  // Wifi
  wifi_portal_timeout: 30,
  wifi_connect_timeout: 10,
  wifi_ssid: 'test-ssid',
  wifi_ssid2: '',
  wifi_pass: 'test-pass',
  wifi_pass2: '',
  // Integrations
  brewfather_apikey: '',
  brewfather_userkey: '',
  brewlogger_url: '',
  brewspy_tokens: ['', '', '', ''],
  barhelper_apikey: '',
  barhelper_monitors: ['', '', '', ''],
  // Push - Generic
  push_timeout: 10,
  // Push - Http Post
  http_post_target: '',
  http_post_header1: '',
  http_post_header2: '',
  http_post2_target: '',
  http_post2_header1: '',
  http_post2_header2: '',
  // Push - Http Get
  http_get_target: '',
  http_get_header1: '',
  http_get_header2: '',
  // Push - Influx
  influxdb2_target: '',
  influxdb2_org: '',
  influxdb2_bucket: '',
  influxdb2_token: '',
  // Push - MQTT
  mqtt_target: '',
  mqtt_port: 1883,
  mqtt_user: '',
  mqtt_pass: '',
  // Arrays
  scales: [
    {
      scale_factor: 0,
      scale_offset: 0,
      keg_weight: 0,
      keg_volume: 0,
      glass_volume: 0,
      temp_sensor_id: ''
    },
    {
      scale_factor: 0,
      scale_offset: 0,
      keg_weight: 0,
      keg_volume: 0,
      glass_volume: 0,
      temp_sensor_id: ''
    },
    {
      scale_factor: 0,
      scale_offset: 0,
      keg_weight: 0,
      keg_volume: 0,
      glass_volume: 0,
      temp_sensor_id: ''
    },
    {
      scale_factor: 0,
      scale_offset: 0,
      keg_weight: 0,
      keg_volume: 0,
      glass_volume: 0,
      temp_sensor_id: ''
    }
  ],
  beers: []
}

/**
 * Mock global app state - kegmon2-ui specific
 */
export const mockGlobalState = {
  id: 'kegmon-123',
  platform: 'ESP32',
  app_ver: '2.0.0',
  app_build: '1',
  initialized: true,
  disabled: false,
  configChanged: false,
  messageError: '',
  messageWarning: '',
  messageSuccess: '',
  messageInfo: '',
  // UI toggles specific to kegmon2-ui
  ui: {
    enableVoltageFragment: false,
    enableManualWifiEntry: false,
    enableScanForStrongestAp: false
  },
  // Feature flags specific to kegmon2-ui
  feature: {
    ble: false,
    tft: false,
    no_scales: 4,
    sd: false
  }
}
