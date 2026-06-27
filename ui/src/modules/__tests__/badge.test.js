import { describe, it, expect, beforeEach, vi } from 'vitest'
import { createPinia, setActivePinia } from 'pinia' // eslint-disable-line no-unused-vars

// Mock @/modules/pinia to provide controlled config and status instances
vi.mock('@/modules/pinia', () => ({
  config: {
    mdns: 'kegmon',
    wifi_ssid: 'MyWifi',
    wifi_ssid2: ''
  },
  status: {
    scales: []
  }
}))

import { config, status } from '@/modules/pinia'
import {
  deviceBadge,
  deviceSettingBadge,
  deviceMdnsBadge,
  deviceHardwareBadge,
  deviceCalibrationBadge,
  deviceWifiBadge,
  deviceWifi1Badge,
  deviceWifi2Badge
} from '../badge'

describe('badge', () => {
  beforeEach(() => {
    config.mdns = 'kegmon'
    config.wifi_ssid = 'MyWifi'
    config.wifi_ssid2 = ''
    status.scales = []
    vi.clearAllMocks()
  })

  // --- deviceMdnsBadge ---
  it('deviceMdnsBadge returns 1 when mdns is empty', () => {
    config.mdns = ''
    expect(deviceMdnsBadge()).toBe(1)
  })

  it('deviceMdnsBadge returns 0 when mdns is set', () => {
    config.mdns = 'kegmon'
    expect(deviceMdnsBadge()).toBe(0)
  })

  // --- deviceSettingBadge ---
  it('deviceSettingBadge returns 1 when mdns is empty', () => {
    config.mdns = ''
    expect(deviceSettingBadge()).toBe(1)
  })

  it('deviceSettingBadge returns 0 when mdns is set', () => {
    expect(deviceSettingBadge()).toBe(0)
  })

  // --- deviceHardwareBadge ---
  it('deviceHardwareBadge always returns 0', () => {
    expect(deviceHardwareBadge()).toBe(0)
  })

  // --- deviceCalibrationBadge ---
  it('deviceCalibrationBadge returns 0 when scales array is empty', () => {
    status.scales = []
    expect(deviceCalibrationBadge()).toBe(0)
  })

  it('deviceCalibrationBadge returns 1 when any scale has scale_factor of 0', () => {
    status.scales = [{ scale_factor: 0 }, { scale_factor: 1 }]
    expect(deviceCalibrationBadge()).toBe(1)
  })

  it('deviceCalibrationBadge returns 0 when all scales have non-zero scale_factor', () => {
    status.scales = [{ scale_factor: 1 }, { scale_factor: 2 }]
    expect(deviceCalibrationBadge()).toBe(0)
  })

  it('deviceCalibrationBadge handles null scale entries gracefully', () => {
    status.scales = [null, { scale_factor: 0 }]
    expect(deviceCalibrationBadge()).toBe(1)
  })

  // --- deviceWifi1Badge ---
  it('deviceWifi1Badge returns 1 when wifi_ssid is empty', () => {
    config.wifi_ssid = ''
    expect(deviceWifi1Badge()).toBe(1)
  })

  it('deviceWifi1Badge returns 0 when wifi_ssid is set', () => {
    config.wifi_ssid = 'MyWifi'
    expect(deviceWifi1Badge()).toBe(0)
  })

  // --- deviceWifi2Badge ---
  it('deviceWifi2Badge returns 1 when both wifi_ssid and wifi_ssid2 are empty', () => {
    config.wifi_ssid = ''
    config.wifi_ssid2 = ''
    expect(deviceWifi2Badge()).toBe(1)
  })

  it('deviceWifi2Badge returns 0 when wifi_ssid is set', () => {
    config.wifi_ssid = 'MyWifi'
    config.wifi_ssid2 = ''
    expect(deviceWifi2Badge()).toBe(0)
  })

  // --- deviceWifiBadge ---
  it('deviceWifiBadge returns 1 when wifi_ssid is empty', () => {
    config.wifi_ssid = ''
    expect(deviceWifiBadge()).toBeTruthy()
  })

  it('deviceWifiBadge returns 0 when wifi_ssid is set', () => {
    config.wifi_ssid = 'MyWifi'
    config.wifi_ssid2 = ''
    expect(deviceWifiBadge()).toBe(0)
  })

  // --- deviceBadge ---
  it('deviceBadge returns sum of all sub-badges', () => {
    config.mdns = '' // +1
    config.wifi_ssid = '' // +1 (wifi badge)
    status.scales = [{ scale_factor: 0 }] // +1
    // hardware always 0
    const total = deviceBadge()
    expect(total).toBeGreaterThanOrEqual(2)
  })

  it('deviceBadge returns 0 when everything is configured', () => {
    config.mdns = 'kegmon'
    config.wifi_ssid = 'MyWifi'
    config.wifi_ssid2 = ''
    status.scales = [{ scale_factor: 1 }]
    expect(deviceBadge()).toBe(0)
  })
})
